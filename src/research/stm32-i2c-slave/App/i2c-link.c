#include "i2c-link.h"

#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


typedef struct i2c_link_pbuf_t
{
	//uint16_t packet_size;
	uint8_t packet_data[I2C_LINK_PACKET_SIZE];
} i2c_link_pbuf_t;


typedef struct i2c_link_pbuf_queue_t
{
	i2c_link_pbuf_t * begin;
	i2c_link_pbuf_t * end;
	i2c_link_pbuf_t * head;
	i2c_link_pbuf_t * tail;
	int full;
} i2c_link_pbuf_queue_t;


typedef enum i2c_link_state_t
{
	I2C_LINK_STATE_IDLE		= 0x00,

	I2C_LINK_STATE_RX,
	I2C_LINK_STATE_RX_DROP,
	I2C_LINK_STATE_RX_COMPLETE,

	I2C_LINK_STATE_TX,
	I2C_LINK_STATE_TX_ZERO,
	I2C_LINK_STATE_TX_COMPLETE,

} i2c_link_state_t;


struct i2c_link_ctx_t;
typedef struct i2c_link_ctx_t i2c_link_ctx_t;

struct i2c_link_ctx_t
{
	i2c_link_pbuf_t rx_bufs[I2C_LINK_RXBUF_COUNT];
	i2c_link_pbuf_queue_t rx_bufs_queue;

	i2c_link_pbuf_t tx_bufs[I2C_LINK_TXBUF_COUNT];
	i2c_link_pbuf_queue_t tx_bufs_queue;

	i2c_link_state_t state;
};

static i2c_link_ctx_t _ctx;
static i2c_link_pbuf_t _rx_fallback = {0};
static i2c_link_pbuf_t _tx_fallback = {0};


static i2c_link_pbuf_t * pbuf_queue_get_head(i2c_link_pbuf_queue_t * queue)
{
	if (queue->full)
		return 0;

	return queue->head;
}


static void pbuf_queue_push_head(i2c_link_pbuf_queue_t * queue)
{
	assert(!queue->full);

	i2c_link_pbuf_t * next_head = queue->head + 1;
	if (next_head == queue->end)
		next_head = queue->begin;

	if (next_head == queue->tail)
		queue->full = 1;

	queue->head = next_head;
}


static i2c_link_pbuf_t * pbuf_queue_get_tail(i2c_link_pbuf_queue_t * queue)
{
	if (queue->tail == queue->head)
		return 0;

	return queue->tail;
}


static void pbuf_queue_pop_tail(i2c_link_pbuf_queue_t * queue)
{
	assert(!(queue->head == queue->tail && !queue->full));

	i2c_link_pbuf_t * next_tail = queue->tail + 1;
	if (next_tail == queue->end)
		next_tail = queue->begin;

	queue->tail = next_tail;
	queue->full = 0;
}


static int _ctx_construct(i2c_link_ctx_t * ctx)
{
	ctx->rx_bufs_queue.begin =
	ctx->rx_bufs_queue.head =
	ctx->rx_bufs_queue.tail = ctx->rx_bufs;

	ctx->rx_bufs_queue.end = ctx->rx_bufs + I2C_LINK_RXBUF_COUNT;

	ctx->tx_bufs_queue.begin =
	ctx->tx_bufs_queue.head =
	ctx->tx_bufs_queue.tail = ctx->tx_bufs;

	ctx->tx_bufs_queue.end = ctx->tx_bufs + I2C_LINK_TXBUF_COUNT;

	ctx->state = I2C_LINK_STATE_IDLE;

	return 0;
}

static int _hal_status_to_errno(HAL_StatusTypeDef status)
{
	switch(status)
	{
	case HAL_OK:		return 0;
	case HAL_ERROR:		return -EIO;
	case HAL_BUSY:		return -EBUSY;
	case HAL_TIMEOUT:	return -ETIMEDOUT;
	default:			return -EBADMSG;
	}
}


int i2c_link_start()
{
	HAL_StatusTypeDef hal_rc;
	int rc;

	rc = _ctx_construct(&_ctx);
	if (0 != rc)
		return rc;

	hal_rc = HAL_I2C_EnableListen_IT(I2C_LINK_BUS_HANDLE);

	rc = _hal_status_to_errno(hal_rc);
	if (0 != rc)
		return rc;

	__HAL_I2C_ENABLE(I2C_LINK_BUS_HANDLE);

	return 0;
}


int i2c_link_write(const void * data, size_t data_size)
{
	i2c_link_ctx_t * const ctx = &_ctx;

	if (data_size > I2C_LINK_PACKET_SIZE)
		return -EINVAL;

	i2c_link_pbuf_t * buf = pbuf_queue_get_head(&ctx->tx_bufs_queue);
	if (0 == buf)
		return -EAGAIN;

	memcpy(buf->packet_data, data, data_size);
	// Все остальное зануляем, чтобы не гонять мусор с предыдиущих проходов мастеру
	memset(buf->packet_data + data_size, 0, I2C_LINK_PACKET_SIZE - data_size);
	pbuf_queue_push_head(&ctx->tx_bufs_queue);

	return data_size;
}


int i2c_link_read(void * buffer_, size_t buffer_size)
{
	i2c_link_ctx_t * const ctx = &_ctx;

	i2c_link_pbuf_t * buf = pbuf_queue_get_tail(&ctx->rx_bufs_queue);
	if (0 == buf)
		return -EAGAIN;

	uint8_t * data = (uint8_t*)buffer_;
	size_t portion_size;
	if (buffer_size < I2C_LINK_PACKET_SIZE)
		portion_size = buffer_size;
	else
		portion_size = I2C_LINK_PACKET_SIZE;

	memcpy(data, buf->packet_data, portion_size);
	// Очистим буфер, чтобы в следующем проходе в нем не было мусора
	memset(buf->packet_data, 0, I2C_LINK_PACKET_SIZE);

	pbuf_queue_pop_tail(&ctx->rx_bufs_queue);
	return I2C_LINK_PACKET_SIZE;
}


static int _link_tx_start(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	if (ctx->state != I2C_LINK_STATE_IDLE)
		return -EINVAL;

	// Мастер на шине что-то от нас хочет получить
	// Нужно посмотреть что у нас лежит в очереди на отправку
	i2c_link_pbuf_t * buf = pbuf_queue_get_tail(&ctx->tx_bufs_queue);
	i2c_link_state_t next_state = I2C_LINK_STATE_TX;
	if (0 == buf)
	{
		// У нас ничего нет для отправки
		// Будем отправлять нолики из фолбека
		buf = &_tx_fallback;
		next_state = I2C_LINK_STATE_TX_ZERO;
	}

	HAL_StatusTypeDef hal_rc;
	hal_rc = HAL_I2C_Slave_Seq_Transmit_DMA(
			hi2c,
			buf->packet_data, I2C_LINK_PACKET_SIZE,
			I2C_FIRST_AND_LAST_FRAME
	);

	int rc = _hal_status_to_errno(hal_rc);
	if (0 != rc)
		return rc;

	ctx->state = next_state;
	return 0;
}


static int _link_tx_complete(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	switch (ctx->state)
	{
	case I2C_LINK_STATE_TX:
		// Мы успешно отправили пакет!
		// выкидываем его из очереди на отправку
		pbuf_queue_pop_tail(&ctx->tx_bufs_queue);
		break;

	case I2C_LINK_STATE_TX_ZERO:
		// Мы успешно отправили нолики
		break;

	default:
		// Этого быть не должно
		assert(0);
	};

	ctx->state = I2C_LINK_STATE_TX_COMPLETE;
	return 0; // у нас все хорошо
}


static int _link_rx_start(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	if (ctx->state != I2C_LINK_STATE_IDLE)
		return -EINVAL;

	// Мастер на шине хочет что-то нам передать.
	// Есть ли у нас буфер для приёма?
	i2c_link_pbuf_t * buf = pbuf_queue_get_head(&ctx->rx_bufs_queue);
	i2c_link_state_t next_state = I2C_LINK_STATE_RX;
	if (0 == buf)
	{
		// Некуда класть, будем класть в мусорку
		buf = &_rx_fallback;
		next_state = I2C_LINK_STATE_RX_DROP;
	}

	HAL_StatusTypeDef hal_rc;
	hal_rc = HAL_I2C_Slave_Seq_Receive_DMA(
			hi2c,
			buf->packet_data, I2C_LINK_PACKET_SIZE,
			I2C_FIRST_AND_LAST_FRAME
	);

	int rc = _hal_status_to_errno(hal_rc);
	if (0 != rc)
		return rc;

	ctx->state = next_state;
	return 0;
}


static int _link_rx_complete(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	switch (ctx->state)
	{
	case I2C_LINK_STATE_RX:
		// Мы успешно получили пакет!
		// сохраняем его как успешно принятый
		pbuf_queue_push_head(&ctx->rx_bufs_queue);
		break;

	case I2C_LINK_STATE_RX_DROP:
		// Мы успешно получили данные и забываем их в печке
		break;

	default:
		// Этого быть не должно
		assert(0);
	};

	ctx->state = I2C_LINK_STATE_RX_COMPLETE;
	return 0; // у нас все хорошо
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t transfer_direction,
		uint16_t addr_match_code
){
	i2c_link_ctx_t * const ctx = &_ctx;

	ctx->state = I2C_LINK_STATE_IDLE;

	int rc;
	switch (transfer_direction)
	{
	case I2C_DIRECTION_RECEIVE:
		rc = _link_tx_start(hi2c, ctx);
		break;


	case I2C_DIRECTION_TRANSMIT:
		rc = _link_rx_start(hi2c, ctx);
		break;

	default:
		assert(0);
	};


	if (0 != rc)
		printf("addr error = %d\n", rc);
}


void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	i2c_link_ctx_t * const ctx = &_ctx;

	int rc = _link_tx_complete(hi2c, ctx);
	if (0 != rc)
		printf("slave_tx_cplt error = %d\n", rc);

}


void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	i2c_link_ctx_t * const ctx = &_ctx;

	int rc = _link_rx_complete(hi2c, ctx);
	if (0 != rc)
		printf("slave_tx_cplt error = %d\n", rc);
}


void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	HAL_StatusTypeDef hal_rc = HAL_I2C_EnableListen_IT(hi2c);
	int rc = _hal_status_to_errno(hal_rc);
	if (0 != rc)
		assert(0);
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	i2c_link_ctx_t * const ctx = &_ctx;

	switch (ctx->state)
	{
	case I2C_LINK_STATE_TX:
		// Если у нас не вышло отправить пакет по какой-то причине
		// не будем пытаться делать это вновь
		pbuf_queue_pop_tail(&ctx->tx_bufs_queue);
		break;

	default:
		// Во все остальных случаях делать вроде как ничего не надо

		break;
	}
}

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
	I2C_LINK_STATE_RX_DONE,

	I2C_LINK_STATE_TX,
	I2C_LINK_STATE_TX_DONE,

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
	i2c_link_stats_t stats;
};

static i2c_link_ctx_t _ctx;
static i2c_link_pbuf_t _rx_fallback = {0};
static i2c_link_pbuf_t _tx_fallback = {0};


static i2c_link_pbuf_t * _pbuf_queue_get_head(i2c_link_pbuf_queue_t * queue)
{
	if (queue->full)
		return 0;

	return queue->head;
}


static void _pbuf_queue_push_head(i2c_link_pbuf_queue_t * queue)
{
	assert(!queue->full);

	i2c_link_pbuf_t * next_head = queue->head + 1;
	if (next_head == queue->end)
		next_head = queue->begin;

	if (next_head == queue->tail)
		queue->full = 1;

	queue->head = next_head;
}


static i2c_link_pbuf_t * _pbuf_queue_get_tail(i2c_link_pbuf_queue_t * queue)
{
	if (queue->tail == queue->head && !queue->full)
		return 0;

	return queue->tail;
}


static void _pbuf_queue_pop_tail(i2c_link_pbuf_queue_t * queue)
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
	memset(ctx, 0, sizeof(*ctx));

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


static int _link_tx_start_zeroes(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	HAL_StatusTypeDef hal_rc;
	hal_rc = HAL_I2C_Slave_Seq_Transmit_DMA(
			hi2c,
			_tx_fallback.packet_data, I2C_LINK_PACKET_SIZE,
			I2C_FIRST_AND_LAST_FRAME
	);

	int rc = _hal_status_to_errno(hal_rc);
	if (0 != rc)
		return rc;

	ctx->state = I2C_LINK_STATE_TX_DONE;
	return 0;
}


static int _link_tx_start(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	if (ctx->state != I2C_LINK_STATE_IDLE)
		return -EINVAL;

	// Мастер на шине что-то от нас хочет получить
	// Нужно посмотреть что у нас лежит в очереди на отправку
	i2c_link_pbuf_t * buf = _pbuf_queue_get_tail(&ctx->tx_bufs_queue);
	if (0 == buf)
	{
		// У нас ничего нет для отправки
		// Будем отправлять нолики из фолбека
		return _link_tx_start_zeroes(hi2c, ctx);
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

	ctx->state = I2C_LINK_STATE_TX;
	return 0;
}


static int _link_tx_complete(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	switch (ctx->state)
	{
	case I2C_LINK_STATE_TX:
		// Мы успешно отправили пакет!
		// выкидываем его из очереди на отправку
		_pbuf_queue_pop_tail(&ctx->tx_bufs_queue);
		ctx->stats.tx_done_cnt++;
		break;

	case I2C_LINK_STATE_TX_DONE:
		// Мы успешно отправили нолики
		ctx->stats.tx_zeroes_cnt++;
		break;

	default:
		// Этого быть не должно
		assert(0);
	};

	// Шлем нолики, на случай, если хост вдруг попросит еще данных
	return _link_tx_start_zeroes(hi2c, ctx);
	//ctx->state = I2C_LINK_STATE_TX_DONE;
	//return 0;
}


static int _link_rx_start_drop(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	HAL_StatusTypeDef hal_rc;
	hal_rc = HAL_I2C_Slave_Seq_Receive_DMA(
			hi2c,
			_rx_fallback.packet_data, I2C_LINK_PACKET_SIZE,
			I2C_FIRST_AND_LAST_FRAME
	);

	int rc = _hal_status_to_errno(hal_rc);
	if (0 != rc)
		return rc;

	ctx->state = I2C_LINK_STATE_RX_DONE;
	return 0;
}


static int _link_rx_start(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	if (ctx->state != I2C_LINK_STATE_IDLE)
		return -EINVAL;

	// Мастер на шине хочет что-то нам передать.
	// Есть ли у нас буфер для приёма?
	i2c_link_pbuf_t * buf = _pbuf_queue_get_head(&ctx->rx_bufs_queue);
	if (0 == buf)
	{
		// Некуда класть, будем класть в мусорку
		return _link_rx_start_drop(hi2c, ctx);
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

	ctx->state = I2C_LINK_STATE_RX;
	return 0;
}


static int _link_rx_complete(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
	switch (ctx->state)
	{
	case I2C_LINK_STATE_RX:
		// Мы успешно получили пакет!
		// сохраняем его как успешно принятый
		_pbuf_queue_push_head(&ctx->rx_bufs_queue);
		ctx->stats.rx_done_cnt++;
		break;

	case I2C_LINK_STATE_RX_DONE:
		// Мы успешно получили данные и забываем их в печке
		ctx->stats.rx_dropped_cnt++;
		break;

	default:
		// Этого быть не должно
		assert(0);
	};

	// Все остальное, если вдруг что еще придет - дропаем
	return _link_rx_start_drop(hi2c, ctx);
	//ctx->state = I2C_LINK_STATE_RX_DONE;
	//return 0;
}


void _antihang(i2c_link_ctx_t * ctx)
{
	HAL_StatusTypeDef hal_rc;
	I2C_HandleTypeDef * h2ic = I2C_LINK_BUS_HANDLE;

	// В ряде случае HAL забрасывает I2C перефирию в состояние SWRST
	// Видимо из-за ошибки в силиконе, которая не позволяет адекватно
	// восстановиться после той или иной ситуации
	// Если такое обнаруживается - нужно перенициализировать модуль I2C
	if (READ_BIT(h2ic->Instance->CR1, I2C_CR1_SWRST))
	{
		ctx->stats.restarts_cnt++;

		CLEAR_BIT(I2C_LINK_BUS_HANDLE->Instance->CR1, I2C_CR1_SWRST);
		__HAL_I2C_RESET_HANDLE_STATE(h2ic);

		hal_rc = HAL_I2C_Init(h2ic);
		assert(HAL_OK == hal_rc);

		hal_rc = HAL_I2C_EnableListen_IT(I2C_LINK_BUS_HANDLE);
		assert(HAL_OK == hal_rc);
	}
}


int i2c_link_start()
{
	i2c_link_ctx_t * const ctx = &_ctx;

	HAL_StatusTypeDef hal_rc;
	int rc;

	rc = _ctx_construct(ctx);
	if (0 != rc)
		return rc;

	hal_rc = HAL_I2C_EnableListen_IT(I2C_LINK_BUS_HANDLE);
	rc = _hal_status_to_errno(hal_rc);
	if (0 != rc)
		return rc;

	return 0;
}


int i2c_link_write(const void * data, size_t data_size)
{
	i2c_link_ctx_t * const ctx = &_ctx;

	_antihang(ctx);

	if (data_size > I2C_LINK_PACKET_SIZE)
		return -EINVAL;

	i2c_link_pbuf_t * buf = _pbuf_queue_get_head(&ctx->tx_bufs_queue);
	if (0 == buf)
		return -EAGAIN;

	memcpy(buf->packet_data, data, data_size);
	// Все остальное зануляем, чтобы не гонять мусор с предыдиущих проходов мастеру
	memset(buf->packet_data + data_size, 0, I2C_LINK_PACKET_SIZE - data_size);
	_pbuf_queue_push_head(&ctx->tx_bufs_queue);

	return data_size;
}


int i2c_link_read(void * buffer_, size_t buffer_size)
{
	i2c_link_ctx_t * const ctx = &_ctx;

	_antihang(ctx);

	i2c_link_pbuf_t * buf = _pbuf_queue_get_tail(&ctx->rx_bufs_queue);
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

	_pbuf_queue_pop_tail(&ctx->rx_bufs_queue);
	return I2C_LINK_PACKET_SIZE;
}


void i2c_link_stats(i2c_link_stats_t * statsbuf)
{
	i2c_link_ctx_t * const ctx = &_ctx;

	*statsbuf = ctx->stats;
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t transfer_direction,
		uint16_t addr_match_code
){
	i2c_link_ctx_t * const ctx = &_ctx;
	ctx->state = I2C_LINK_STATE_IDLE;

	switch (transfer_direction)
	{
	case I2C_DIRECTION_RECEIVE:
		_link_tx_start(hi2c, ctx);
		break;

	case I2C_DIRECTION_TRANSMIT:
		_link_rx_start(hi2c, ctx);
		break;

	default:
		assert(0);
	};
}


void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	i2c_link_ctx_t * const ctx = &_ctx;
	_link_tx_complete(hi2c, ctx);
}


void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	i2c_link_ctx_t * const ctx = &_ctx;
	_link_rx_complete(hi2c, ctx);
}


void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	i2c_link_ctx_t * const ctx = &_ctx;
	ctx->stats.listen_done_cnt++;

	HAL_StatusTypeDef hal_rc = HAL_I2C_EnableListen_IT(hi2c);
	int rc = _hal_status_to_errno(hal_rc);
	if (0 != rc)
		assert(0);
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	i2c_link_ctx_t * const ctx = &_ctx;
	uint32_t error = HAL_I2C_GetError(hi2c);

	switch (ctx->state)
	{
	case I2C_LINK_STATE_TX:
		// Если у нас не вышло отправить пакет по какой-то причине
		// не будем пытаться делать это вновь
		ctx->stats.tx_error_cnt++;
		_pbuf_queue_pop_tail(&ctx->tx_bufs_queue);
		ctx->stats.last_error = error;

		// обязательно уходим из этого состояния
		// чтобы не удалить буфер по второй ошибке, если она будет
		ctx->state = I2C_LINK_STATE_TX_DONE;
		break;

	case I2C_LINK_STATE_RX:
		if (error == HAL_I2C_ERROR_AF && hi2c->XferCount == 0)
		{
			// Это совершенно нормальная ситуация - NACK на последний байт от мастера
			_link_rx_complete(hi2c, ctx);
		}
		else
		{
			ctx->stats.rx_error_cnt++;
			ctx->stats.last_error = error;
		}
		ctx->state = I2C_LINK_STATE_RX_DONE;
		break;

	default:
		// Во все остальных случаях делать вроде как ничего не надо
		break;
	}
}

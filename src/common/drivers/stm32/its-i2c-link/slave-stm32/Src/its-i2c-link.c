#include "../Inc/its-i2c-link.h"

#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "main.h"


//! Буфер для пакета (а может быть и для нескольких?
typedef struct its_i2c_link_pbuf_t
{
    uint16_t packet_size;
    uint8_t packet_data[I2C_LINK_PACKET_SIZE];
} its_i2c_link_pbuf_t;


//! Очередь пакетов - циклический буфер
typedef struct i2c_link_pbuf_queue_t
{
    //! начало базового линейного буфера
    its_i2c_link_pbuf_t * begin;
    //! Конец базового линейного буфера
    its_i2c_link_pbuf_t * end;
    //! Голова циклобуфера
    its_i2c_link_pbuf_t * head;
    //! Хвост циклобуфера
    its_i2c_link_pbuf_t * tail;

    //! Флаг заполненности
    /*! Для разрешения неоднозначности ситуации head == tail */
    int full;
} i2c_link_pbuf_queue_t;


//! Состояние i2c линка
typedef enum i2c_link_state_t
{
    //! Мы принимаем пакет
    I2C_LINK_STATE_RX,
    //! Мы закончили (а то и никогда не начинали) передавть пакет
    /*! И гоним вместо него нолики */
    I2C_LINK_STATE_RX_DONE,

    //! Мы передаем пакет
    I2C_LINK_STATE_TX,

    //! Мы закончили (а то и никогда не начинали) получать пакет
    /*! И все получаемые данные отправляем в помойку */
    I2C_LINK_STATE_TX_DONE,

} its_i2c_link_state_t;

typedef enum i2c_link_cmd_t
{
    I2C_LINK_CMD_NONE = 0x00,
    I2C_LINK_CMD_GET_SIZE = 0x01,
    I2C_LINK_CMD_GET_PACKET = 0x02,
    I2C_LINK_CMD_SET_PACKET = 0x04,
} its_i2c_link_cmd_t;


typedef struct i2c_link_iface_t {
    I2C_HandleTypeDef *hi2c;
} i2c_link_iface_t;

struct i2c_link_ctx_t;
typedef struct i2c_link_ctx_t i2c_link_ctx_t;


//! Контекст модуля
struct i2c_link_ctx_t
{
    //! Линейный набор приёмных буферов
    its_i2c_link_pbuf_t rx_bufs[I2C_LINK_RXBUF_COUNT];
    //! очередь линейных буферов
    i2c_link_pbuf_queue_t rx_bufs_queue;

    //! Линейный набор передаточных буферов
    its_i2c_link_pbuf_t tx_bufs[I2C_LINK_TXBUF_COUNT];
    //! очередь передаточных буферов
    i2c_link_pbuf_queue_t tx_bufs_queue;

    //! Состояние модуля
    its_i2c_link_state_t state;

    //! Выполняемая команда
    its_i2c_link_cmd_t cur_cmd;
    //! Выполняемая команда
    its_i2c_link_cmd_t prev_cmd;

    //! Статистика модуля (для телеметрии в основном)
    its_i2c_link_stats_t stats;

    //Интерфейс, для которого используется i2c_link
    i2c_link_iface_t iface;


};

//! Пока что мы поддерживаем ровно один i2c линк и поэтом его состояние
//! в одной глобальной переменной
static i2c_link_ctx_t _ctx;

static uint8_t _rx_fallback[I2C_LINK_RX_DUMP_SIZE] = {0};
static uint8_t _tx_fallback[I2C_LINK_TX_ZEROS_SIZE] = {0};
//static its_i2c_link_pbuf_t _tx_fallback_ = { 0 };
//static its_i2c_link_pbuf_t _rx_fallback_ = { 0 };


static its_i2c_link_pbuf_t * _pbuf_queue_get_head(i2c_link_pbuf_queue_t * queue)
{
    if (queue->full)
        return 0;

    return queue->head;
}

static int _pbuf_queue_is_empty(i2c_link_pbuf_queue_t * queue) {
    return (queue->head == queue->tail) && !queue->full;
}

static void _pbuf_queue_push_head(i2c_link_pbuf_queue_t * queue)
{
    assert(!queue->full);

    its_i2c_link_pbuf_t * next_head = queue->head + 1;
    if (next_head == queue->end)
        next_head = queue->begin;

    if (next_head == queue->tail)
        queue->full = 1;

    queue->head = next_head;
}


static its_i2c_link_pbuf_t * _pbuf_queue_get_tail(i2c_link_pbuf_queue_t * queue)
{
    if (queue->tail == queue->head && !queue->full)
        return 0;

    return queue->tail;
}


static void _pbuf_queue_pop_tail(i2c_link_pbuf_queue_t * queue)
{
    assert(!(queue->head == queue->tail && !queue->full));

    its_i2c_link_pbuf_t * next_tail = queue->tail + 1;
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
    return 0;
}


static int _hal_status_to_errno(HAL_StatusTypeDef status)
{
    switch(status)
    {
    case HAL_OK:        return 0;
    case HAL_ERROR:     return -EIO;
    case HAL_BUSY:      return -EBUSY;
    case HAL_TIMEOUT:   return -ETIMEDOUT;
    default:            return -EBADMSG;
    }
}

/*
 * Позволяет автоматически выбрать наиболее быстрый вариант
 */
inline static HAL_StatusTypeDef _i2c_send(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t XferOptions) {
    if (Size < I2C_LINK_SEND_THD) {
        return HAL_I2C_Slave_Seq_Transmit_IT(hi2c, pData, Size, XferOptions);
    } else {
        return HAL_I2C_Slave_Seq_Transmit_DMA(hi2c, pData, Size, XferOptions);
    }
}
inline static HAL_StatusTypeDef _i2c_recieve(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t XferOptions) {
    if (Size < I2C_LINK_SEND_THD) {
        return HAL_I2C_Slave_Seq_Receive_IT(hi2c, pData, Size, XferOptions);
    } else {
        return HAL_I2C_Slave_Seq_Receive_DMA(hi2c, pData, Size, XferOptions);
    }
}

inline static void _i2c_pull_int(void) {
    HAL_GPIO_WritePin(I2C_INT_GPIO_Port, I2C_INT_Pin, GPIO_PIN_RESET);
}
inline static void _i2c_push_int(void) {
    HAL_GPIO_WritePin(I2C_INT_GPIO_Port, I2C_INT_Pin, GPIO_PIN_SET);
}
/*
 * Отправляет нули
 */
static int _link_tx_start_zeroes(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
    HAL_StatusTypeDef hal_rc;

    hal_rc = HAL_I2C_Slave_Seq_Transmit_IT(
            hi2c,
            _tx_fallback,
            sizeof(_tx_fallback),
            I2C_FIRST_AND_LAST_FRAME
    );

    int rc = _hal_status_to_errno(hal_rc);
    if (0 != rc)
        return rc;

    ctx->state = I2C_LINK_STATE_TX_DONE;
    return 0;
}

/*
 * Отправляет сообщение по i2c
 */
static int _link_tx_send(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
    // Мастер на шине что-то от нас хочет получить
    // Нужно посмотреть что у нас лежит в очереди на отправку
    its_i2c_link_pbuf_t * buf = _pbuf_queue_get_tail(&ctx->tx_bufs_queue);
    if (0 == buf)
    {
        // У нас ничего нет для отправки
        // Будем отправлять нолики из фолбека
        return _link_tx_start_zeroes(hi2c, ctx);
    }

    int hal_rc = HAL_I2C_Slave_Seq_Transmit_DMA(
            hi2c,
            (uint8_t*) buf->packet_data,
            buf->packet_size,
            I2C_FIRST_AND_LAST_FRAME
    );

    int rc = _hal_status_to_errno(hal_rc);
    if (0 != rc)
        return rc;

    ctx->state = I2C_LINK_STATE_TX;
    return 0;
}

/*
 * Отправляет размер
 */
static int _link_tx_send_size(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx) {

    int rc = 0;

    uint16_t size = 0;
    its_i2c_link_pbuf_t * buf = _pbuf_queue_get_tail(&ctx->tx_bufs_queue);
    if (buf != 0) {
        size = buf->packet_size;
    }
    rc = _i2c_send(hi2c, (uint8_t*) &size, sizeof(size), I2C_FIRST_AND_LAST_FRAME);
    rc = _hal_status_to_errno(rc);

    ctx->state = I2C_LINK_STATE_TX;
    return rc;
}

/*
 * Обработка команды, полученной раннее
 */
static int _link_tx_start(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx) {
    int rc = 0;

    switch (ctx->cur_cmd) {
    case I2C_LINK_CMD_GET_PACKET:
        rc = _link_tx_send(hi2c, ctx);
        break;

    case I2C_LINK_CMD_GET_SIZE: {
        rc = _link_tx_send_size(hi2c, ctx);
        break;
    }
    case I2C_LINK_CMD_NONE:
        rc = _link_tx_start_zeroes(hi2c, ctx);
        break;

    default:
        rc = _link_tx_start_zeroes(hi2c, ctx);
        break;
    }
    ctx->prev_cmd = ctx->cur_cmd;
    ctx->cur_cmd = I2C_LINK_CMD_NONE;

    return rc;
}

static int _link_tx_complete(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
    switch (ctx->state)
    {
    case I2C_LINK_STATE_TX:
        // Мы успешно отправили пакет!
        // выкидываем его из очереди на отправку
        if (ctx->prev_cmd == I2C_LINK_CMD_GET_PACKET) {
            _pbuf_queue_pop_tail(&ctx->tx_bufs_queue);
            if (_pbuf_queue_is_empty(&ctx->tx_bufs_queue)) {
                _i2c_push_int();
            }
        }
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

    if(_pbuf_queue_is_empty(&ctx->tx_bufs_queue)) {
        _i2c_push_int();
    }
    // Шлем нолики, на случай, если хост вдруг попросит еще данных
    return _link_tx_start_zeroes(hi2c, ctx);
    //ctx->state = I2C_LINK_STATE_TX_DONE;
    //return 0;
}

/*
 * Записывает нули в fallback. Вызывается, когда к нам приходит мусор
 */
static int _link_rx_start_drop(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
    HAL_StatusTypeDef hal_rc = 0;

    hal_rc = HAL_I2C_Slave_Seq_Receive_DMA(
            hi2c,
            _rx_fallback,
            sizeof(_rx_fallback),
            I2C_FIRST_AND_LAST_FRAME
     );

    int rc = _hal_status_to_errno(hal_rc);
    if (0 != rc)
        return rc;

    ctx->state = I2C_LINK_STATE_RX_DONE;
    return 0;
}

static int _link_rx_recieve(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
    // Мастер на шине хочет что-то нам передать.
    // Есть ли у нас буфер для приёма?
    its_i2c_link_pbuf_t * buf = _pbuf_queue_get_head(&ctx->rx_bufs_queue);
    if (0 == buf)
    {
        // Некуда класть, будем класть в мусорку
        return _link_rx_start_drop(hi2c, ctx);
    }

    HAL_StatusTypeDef hal_rc = 0;

    hal_rc = HAL_I2C_Slave_Seq_Receive_DMA(
            hi2c,
            (uint8_t*) buf,
            sizeof(*buf),
            I2C_FIRST_AND_LAST_FRAME
    );

    int rc = _hal_status_to_errno(hal_rc);
    if (0 != rc)
        return rc;

    ctx->state = I2C_LINK_STATE_RX;
    return 0;
}

static int _link_rx_recieve_cmd(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx) {
    int rc = 0;
    rc = _i2c_recieve(hi2c, (uint8_t*) &ctx->cur_cmd, 1, I2C_FIRST_FRAME);
    rc = _hal_status_to_errno(rc);
    ctx->state = I2C_LINK_STATE_RX;
    return rc;
}

/*
 * Обрабатывает команды и получает команду
 */
static int _link_rx_start(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx) {
    int rc = 0;
    switch (ctx->cur_cmd) {
    case I2C_LINK_CMD_SET_PACKET:
        rc = _link_rx_recieve(hi2c, ctx);
        break;

    default:
        rc = _link_rx_recieve_cmd(hi2c, ctx);
    }
    ctx->prev_cmd = ctx->cur_cmd;
    ctx->cur_cmd = I2C_LINK_CMD_NONE;

    return rc;
}

static int _link_rx_complete(I2C_HandleTypeDef *hi2c, i2c_link_ctx_t * ctx)
{
    switch (ctx->state)
    {
    case I2C_LINK_STATE_RX:
        // Мы успешно получили пакет!
        // сохраняем его как успешно принятый

        if (ctx->prev_cmd == I2C_LINK_CMD_SET_PACKET) {
            _pbuf_queue_push_head(&ctx->rx_bufs_queue);
        }
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



// В ряде случае HAL забрасывает I2C перефирию в состояние SWRST
// Видимо из-за ошибки в силиконе, которая не позволяет адекватно
// восстановиться после той или иной ситуации
// Если такое обнаруживается - нужно перенициализировать модуль I2C
void _antihang(i2c_link_ctx_t * ctx)
{
    HAL_StatusTypeDef hal_rc;
    I2C_HandleTypeDef * hi2c = I2C_LINK_BUS_HANDLE;

    if (READ_BIT(hi2c->Instance->CR1, I2C_CR1_SWRST))
    {
        ctx->stats.restarts_cnt++;

        CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_SWRST);
        __HAL_I2C_RESET_HANDLE_STATE(hi2c);

        hal_rc = HAL_I2C_Init(hi2c);
        assert(HAL_OK == hal_rc);

        hal_rc = HAL_I2C_EnableListen_IT(hi2c);
        assert(HAL_OK == hal_rc);
    }
}


int its_i2c_link_start(I2C_HandleTypeDef *hi2c)
{
    i2c_link_ctx_t *  ctx = &_ctx;
    int rc = _ctx_construct(ctx);
    ctx->iface.hi2c = hi2c;
    if (0 != rc)
        return rc;

    HAL_StatusTypeDef hal_rc = HAL_I2C_EnableListen_IT(I2C_LINK_BUS_HANDLE);
    rc = _hal_status_to_errno(hal_rc);
    if (0 != rc)
        return rc;

    return 0;
}


int its_i2c_link_write(const void * data, size_t data_size)
{
    i2c_link_ctx_t * const ctx = &_ctx;

    _antihang(ctx);

    if (data_size > I2C_LINK_PACKET_SIZE)
        return -EINVAL;

    its_i2c_link_pbuf_t * buf = _pbuf_queue_get_head(&ctx->tx_bufs_queue);
    if (0 == buf)
    {
        ctx->stats.tx_overrun_cnt++;
        return -EAGAIN;
    }

    buf->packet_size = data_size;
    memcpy(buf->packet_data, data, data_size);

    _pbuf_queue_push_head(&ctx->tx_bufs_queue);

    if (!_pbuf_queue_is_empty(&ctx->tx_bufs_queue)) {
        _i2c_pull_int();
    }

    return data_size;
}


int its_i2c_link_read(void * buffer_, size_t buffer_size)
{
    i2c_link_ctx_t * const ctx = &_ctx;

    _antihang(ctx);

    its_i2c_link_pbuf_t * buf = _pbuf_queue_get_tail(&ctx->rx_bufs_queue);
    if (0 == buf)
        return -EAGAIN;

    uint8_t * data = (uint8_t*)buffer_;
    size_t portion_size;
    if (buffer_size < buf->packet_size)
        portion_size = buffer_size;
    else
        portion_size = buf->packet_size;

    memcpy(data, buf->packet_data, portion_size);
    // Очистим буфер, чтобы в следующем проходе в нем не было мусора
    //memset(buf->packet_data, 0, I2C_LINK_PACKET_SIZE);

    _pbuf_queue_pop_tail(&ctx->rx_bufs_queue);
    return buf->packet_size;
}


void its_i2c_link_stats(its_i2c_link_stats_t * statsbuf)
{
    i2c_link_ctx_t * const ctx = &_ctx;

    *statsbuf = ctx->stats;
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t transfer_direction,
        uint16_t addr_match_code
){
    i2c_link_ctx_t * const ctx = &_ctx;
    if (ctx->iface.hi2c->Instance != hi2c->Instance) {
        return;
    }

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
    if (ctx->iface.hi2c != hi2c) {
        return;
    }
    _link_tx_complete(hi2c, ctx);
}


void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2c_link_ctx_t * const ctx = &_ctx;
    if (ctx->iface.hi2c != hi2c) {
        return;
    }
    _link_rx_complete(hi2c, ctx);
}


void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2c_link_ctx_t * const ctx = &_ctx;

    if (ctx->iface.hi2c != hi2c) {
        return;
    }

    ctx->stats.listen_done_cnt++;

    HAL_StatusTypeDef hal_rc = HAL_I2C_EnableListen_IT(hi2c);
    int rc = _hal_status_to_errno(hal_rc);
    if (0 != rc)
        assert(0);
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    i2c_link_ctx_t * const ctx = &_ctx;
    if (ctx->iface.hi2c != hi2c) {
        return;
    }
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

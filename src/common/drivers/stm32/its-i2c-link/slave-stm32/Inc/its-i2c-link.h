#ifndef I2C_INC_H_
#define I2C_INC_H_

#include <stddef.h>

#include "its-i2c-link-conf.h"


//! Статистика i2c-линка
typedef struct its_i2c_link_stats_t
{
	//! Количество полученных пакетов
	uint16_t rx_done_cnt;
	//! Количество не полученных пакетов из переполнения буферов
	uint16_t rx_dropped_cnt;
	//! Количество ошибок при получении пакетов
	uint16_t rx_error_cnt;

	//! Количество переданных пакетов
	uint16_t tx_done_cnt;
	//! Количесто переданных пустых пакетов
	/*! (когда мастер просил данные, но у нас их не было */
	uint16_t tx_zeroes_cnt;
	//! Количество ошибок при передаче пакетов
	uint16_t tx_error_cnt;
	//! Количество попыток отправить сообщение при полном отправном буфере
	uint16_t tx_overrun_cnt;

	//! Количетво рестартов i2c перефирии из-за ошибок на шине
	uint16_t restarts_cnt;
	//! Количество халовский завершенных listen циклов
	uint16_t listen_done_cnt;
	//! Код последней ошибки в терминах HAL
	uint32_t last_error;
} its_i2c_link_stats_t;


//! Запуск модуля
/*! Нужно вызывать лишь однажды */
int its_i2c_link_start(I2C_HandleTypeDef *hi2c);

int its_i2c_link_write(const void * data, size_t data_size);

int its_i2c_link_read(void * buffer, size_t buffer_size);

void its_i2c_link_stats(its_i2c_link_stats_t * statsbuf);

#endif /* I2C_INC_H_ */

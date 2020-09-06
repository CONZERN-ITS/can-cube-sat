/*
 * log_collector.h
 *
 *  Created on: Aug 19, 2020
 *      Author: sereshotes
 */

#ifndef COMPONENTS_LOG_COLLECTOR_INC_LOG_COLLECTOR_H_
#define COMPONENTS_LOG_COLLECTOR_INC_LOG_COLLECTOR_H_

#include <stdint.h>

#define LOG_COLLECTOR_SEND_PERIOD 1000 //ms
#define LOG_COLLECTOR_ADD_PERIOD_COMMON LOG_COLLECTOR_SEND_PERIOD //ms


#define LOG_COLLECTOR_INIT(name) \
		xTaskCreatePinnedToCore(_log, name, configMINIMAL_STACK_SIZE + 1500, 0, 1, &log_task, tskNO_AFFINITY)
#define LOG_COLLECTOR_DEINIT() \
		vTaskDelay(log_task)

typedef struct {
	int32_t last_error;
	uint32_t error_count;
	uint32_t ellapsed_time;
	uint32_t last_state;
} log_data_t;

typedef enum {
	LOG_COMP_ID_SD,
	LOG_COMP_ID_IMI,
	LOG_COMP_ID_SINC_COMM,
	LOG_COMP_ID_WIFI,
	LOG_COMP_ID_NETWORK,
	LOG_COMP_ID_SENSORS,
	LOG_COMP_ID_TIME_SYNC,
	LOG_COMP_ID_SHIFT_REG,
	LOG_COMP_ID_RADIO,
	log_comp_name_size
} log_comp_id_t;

typedef struct {
	log_data_t log_data[log_comp_name_size];
} log_collector_t;

typedef enum {
	LOG_ERROR_OK = 0,
	LOG_ERROR_LOW_MEMORY,
	LOG_ERROR_TIMEOUT,
	LOG_ERROR_HARDWARE,
	LOG_ERROR_LL_API,
} log_error_t;
typedef enum {
	LOG_STATE_ON = 0,
	LOG_STATE_OFF,
	LOG_STATE_SUSPENDED,
} log_state_t;

void log_collector_init(log_collector_t * coll);

void log_collector_add_to(log_collector_t *hlc, log_comp_id_t id, const log_data_t *data);

void log_collector_add(log_comp_id_t id, const log_data_t *data);

void log_collector_log_task(log_data_t *data);

#endif /* COMPONENTS_LOG_COLLECTOR_INC_LOG_COLLECTOR_H_ */

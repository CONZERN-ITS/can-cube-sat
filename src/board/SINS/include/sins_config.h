#ifndef INC_SINS_CONFIG_H_
#define INC_SINS_CONFIG_H_


//! CORTEX-M приоритет для прерывания на байты UART-а
#define ITS_SINS_GPS_UART_IRQ_PRIORITY (4)

//! CORTEX-M приоритет для прерывания на PPS сигнал
#define ITS_SINS_GPS_PPS_IRQ_PRIORITY (0)

//! Приоритет прерывания переполнения таймера для миллисекунд недели службы времени
#define ITS_SINS_TIME_SVC_TOW_OVF_PRIOIRITY (3)

//! Приоритет прерывания для запуска таймеров службы времени по прерыванию RTC
#define ITS_SINS_TIME_SVC_ALARM_IRQ_PRIORITY (1)



//! Размер для циклобуфера уарта, по которому приходят входящие GPS сообщения
#define ITS_SINS_GPS_UART_CYCLE_BUFFER_SIZE (500)

//! Размер линейного буфера для накопления UBX сообщений
/*! Должен быть не меньше чем самое большое обрабатываемое UBX сообщение */
#define ITS_SINS_GPS_UBX_SPARSER_BUFFER_SIZE (100)

//! Максимальное количество байт обрабатываемое за один вызов gps_poll
#define ITS_SINS_GPS_MAX_POLL_SIZE (ITS_SINS_GPS_UART_CYCLE_BUFFER_SIZE)



//! Базовый год для RTC, так как оно работает в двух последних цифрах года
#define ITS_SINS_TIME_SVC_RTC_BASE_YEAR (2000)

#endif /* INC_SINS_CONFIG_H_*/

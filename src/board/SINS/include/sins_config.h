#ifndef INC_SINS_CONFIG_H_
#define INC_SINS_CONFIG_H_


//! CORTEX-M приоритет для прерывания на байты UART-а
#define GPS_UART_IRQ_PRIORITY (4)

//! CORTEX-M приоритет для прерывания на PPS сигнал
#define GPS_PPS_IRQ_PRIORITY (0)

//! Приоритет прерывания переполнения таймера для миллисекунд недели службы времени
#define TIME_SVC_TOW_OVF_PRIOIRITY (4)



//! Размер для циклобуфера уарта, по которому приходят входящие GPS сообщения
#define GPS_UART_CYCLE_BUFFER_SIZE (500)

//! Размер линейного буфера для накопления UBX сообщений
/*! Должен быть не меньше чем самое большое обрабатываемое UBX сообщение */
#define GPS_UBX_SPARSER_BUFFER_SIZE (100)

//! Максимальное количество байт обрабатываемое за один вызов gps_poll
#define GPS_MAX_POLL_SIZE (GPS_UART_CYCLE_BUFFER_SIZE)


#endif /* INC_SINS_CONFIG_H_*/

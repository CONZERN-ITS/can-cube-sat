#ifndef INC_SINS_CONFIG_H_
#define INC_SINS_CONFIG_H_



//! Использовать LSI в качестве источника частоты для RTC
/*! Если эта переменная имеет значение 0 или не определена, то используется LSE */
#define ITS_SINS_RTC_CLKSOURCE_LSI 0


//! Нужно ли делать по запуску насильный сброс всего бекап домена?
/*! Если используется LSE для RTC, то это делать нужно - иначе RTC не запустится */
#define ITS_SINS_RTC_FORCERESET 0



//! CORTEX-M приоритет для прерывания на байты UART-а
#define ITS_SINS_GPS_UART_IRQ_PRIORITY (4)

//! CORTEX-M приоритет для прерывания на PPS сигнал
#define ITS_SINS_GPS_PPS_IRQ_PRIORITY (0)

//! Приоритет прерывания переполнения таймера для миллисекунд недели службы времени
#define ITS_SINS_TIME_SVC_TOW_OVF_PRIOIRITY (3)

//! Приоритет прерывания для запуска таймеров службы времени по прерыванию RTC
#define ITS_SINS_TIME_SVC_ALARM_IRQ_PRIORITY (1)

//! Приоритет прерывания таймера стабильной службы времени
#define ITS_SINS_TIME_SVC_STEADY_OVF_PRIORITY (2)


//! Количество попыток на отправку одного и тот же конфигурационного сообщения
//! В UBX приёмник
#define ITS_SINS_GPS_CONFIGURE_ATTEMPTS (3)
//! Таймаут на ожидание ACK/NACK пакета при конфигурации GPS (в мс)
#define ITS_SINS_GPS_CONFIGURE_TIMEOUT (1500)

//! Размер для циклобуфера уарта, по которому приходят входящие GPS сообщения
#define ITS_SINS_GPS_UART_CYCLE_BUFFER_SIZE (4000)

//! Размер линейного буфера для накопления UBX сообщений
/*! Должен быть не меньше чем самое большое обрабатываемое UBX сообщение */
#define ITS_SINS_GPS_UBX_SPARSER_BUFFER_SIZE (100)

//! Максимальное количество байт обрабатываемое за один вызов gps_poll
#define ITS_SINS_GPS_MAX_POLL_SIZE (ITS_SINS_GPS_UART_CYCLE_BUFFER_SIZE)

//! Максимальное время в течение которого от gps вообще не приходит пакетов
//! Нужных нам классов и при этом GPS считается рабочим (в мс)
#define ITS_SINS_GPS_MAX_NOPACKET_TIME (60*1000)

//! Максимальное время в течение которого GPS не сообщает о фиксе
//! и при этом считается рабочим (в мс)
#define ITS_SINS_GPS_MAX_NOFIX_TIME (20*60*1000)


//! Базовый год для RTC, так как оно работает в двух последних цифрах года
#define ITS_SINS_TIME_SVC_RTC_BASE_YEAR (2000)

#endif /* INC_SINS_CONFIG_H_*/

/*
 * its-time.c
 *
 *  Created on: 28 мар. 2020 г.
 *      Author: sereshotes
 */

#ifndef ITS_TIME_INC_ITS_TIME_C_
#define ITS_TIME_INC_ITS_TIME_C_

#include <stm32f1xx_hal.h>

/*
 * May be 0 - 0x00FFFFFF
 */
#define ITS_TIME_PRESCALER 39740
#define ITS_TIME_AUTO_PRESCALER 0

typedef struct {
    uint64_t sec;
    uint16_t usec;
} its_time_t;



int its_time_init(void);
void its_gettimeofday(its_time_t *time);
void its_settimeofday(its_time_t *time);
uint32_t its_time_gettick(void);


void its_sync_time(its_time_t *from_bcs);


/*
 * Delay in microseconds (not milliseconds)
 * us - count of microseconds
 */
inline __attribute__((always_inline)) void its_delay_us(uint32_t us)
{
    volatile uint32_t startTick = DWT->CYCCNT;
    volatile uint32_t delayTicks = us * (SystemCoreClock/1000000);

    while (DWT->CYCCNT - startTick < delayTicks);
}

#endif /* ITS_TIME_INC_ITS_TIME_C_ */

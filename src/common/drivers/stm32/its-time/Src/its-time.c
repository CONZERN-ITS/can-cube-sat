/*
 * its-time.c
 *
 *  Created on: 28 мар. 2020 г.
 *      Author: sereshotes
 */


#include "../Inc/its-time.h"
#include "sys/time.h"
#include "main.h"


typedef struct {
    RTC_TypeDef *RTCx;
    its_time_t globalTime;
    uint16_t usec_shift;
    uint32_t div_load;
} its_time_handler_t;

//-------------------------------------------------------------------------------------
//PRIVATE

static its_time_handler_t its_time_h;

static void _read_DIV_CNT_timesafe(RTC_TypeDef *RTCx, uint32_t *div, uint32_t *cnt);
static uint16_t _div_tick_to_usec(uint32_t tick, uint32_t max);
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
//PUBLIC

int its_time_init(void);
void its_gettimeofday(its_time_t *time);
void its_settimeofday(its_time_t *time);
uint32_t its_time_gettick(void);
struct Time gettime(void);




//-------------------------------------------------------------------------------------


/*
 * Initializes DWT timer for microsecond clock and
 * sets RTC prescaler value
 */
int its_time_init(void)
{
    if (1) {
        //DWT->LAR = 0xC5ACCE55;
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
    its_time_h.RTCx = RTC;

    uint32_t prescaler = ITS_TIME_PRESCALER;
    if (ITS_TIME_AUTO_PRESCALER) {
        prescaler = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_RTC);

        /* Check that RTC clock is enabled*/
        if (prescaler == 0U) {
            /* Should not happen. Frequency is not available*/
            return HAL_ERROR;
        } else {
            /* RTC period = RTCCLK/(RTC_PR + 1) */
            prescaler = prescaler - 1U;
        }
    }
    its_time_h.div_load = prescaler;

    LL_RTC_DisableWriteProtection(its_time_h.RTCx);
    LL_RTC_SetAsynchPrescaler(its_time_h.RTCx, prescaler);
    LL_RTC_EnableWriteProtection(its_time_h.RTCx);

    return 0;
}

/*
 * Reads time from the RTC.
 * RTC resets prescaler counter after setting CNT. So
 * we have to save microseconds in variable and add everytime
 * when we want to get time
 */
void __attribute__((weak)) its_gettimeofday(its_time_t *time) {
    uint32_t cnt, div;
    _read_DIV_CNT_timesafe(its_time_h.RTCx, &div, &cnt);
    uint16_t usec = _div_tick_to_usec(div + 1, its_time_h.div_load + 1);
    usec += its_time_h.usec_shift;
    time->usec = usec % 1000;
    time->sec  = cnt + usec / 1000;
}

/*
 * Sets time in the RTC
 * We have to save microseconds because of reasons
 * explained in comments for its_gettimeofday
 */
void __attribute__((weak)) its_settimeofday(its_time_t *time) {
    its_time_h.usec_shift = time->usec;
    LL_RTC_DisableWriteProtection(its_time_h.RTCx);
    LL_RTC_TIME_Set(its_time_h.RTCx, time->sec);
    LL_RTC_EnableWriteProtection(its_time_h.RTCx);
}


/*
 * Returns count of system ticks
 * This clock ticks evey 1 / SystemCoreClock of seconds. So,
 * to get microseconds divide it by (SystemCoreClock/1000000)
 */
uint32_t its_time_gettick(void) {
    return DWT->CYCCNT;
}


/*
 * Safes us from the situation, when one of the regs is overflowed
 * in the moment between reading them. It would be bad because
 * global time bias increases every time when that happens.
 */
static void _read_DIV_CNT_timesafe(RTC_TypeDef *RTCx, uint32_t *div, uint32_t *cnt) {
    /*
    SET_BIT(RTCx->CRL, RTC_CRL_CNF);
    LL_RTC_WaitForSynchro(RTCx);
    CLEAR_BIT(RTCx->CRL, RTC_CRL_CNF);*/

    uint16_t val[2][4];
    val[0][3] = READ_REG(RTCx->CNTH & RTC_CNTH_RTC_CNT);
    val[0][2] = READ_REG(RTCx->CNTL & RTC_CNTL_RTC_CNT);
    val[0][1] = READ_REG(RTCx->DIVH & RTC_DIVH_RTC_DIV);
    val[0][0] = READ_REG(RTCx->DIVL & RTC_DIVL_RTC_DIV);

    val[1][3] = READ_REG(RTCx->CNTH & RTC_CNTH_RTC_CNT);
    val[1][2] = READ_REG(RTCx->CNTL & RTC_CNTL_RTC_CNT);
    val[1][1] = READ_REG(RTCx->DIVH & RTC_DIVH_RTC_DIV);
    val[1][0] = READ_REG(RTCx->DIVL & RTC_DIVL_RTC_DIV);


    int test = 0;
    for (int i = 1; i < 4; i++) {
        if (val[0][i] != val[1][i]) {
            test = 1;
            break;
        }
    }


    *cnt = ((uint32_t)(((uint32_t) val[test][3] << 16U) | val[test][2]));
    //DIVH: [15:4] - reserved, [3:0] - actual value
    *div = ((uint32_t)((((uint32_t) val[test][1] & 0x000F) << 16U) | val[test][0]));
}

/*
 * Transfers ticks from DIV register to milliseconds
 */
static uint16_t _div_tick_to_usec(uint32_t tick, uint32_t max) {
    return (max - tick) * 1000 / max;
}

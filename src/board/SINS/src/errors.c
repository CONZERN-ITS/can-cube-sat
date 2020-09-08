/*
 * errors.c
 *
 *  Created on: 19 авг. 2020 г.
 *      Author: developer
 */

#include <stm32f405xx.h>
#include "drivers/mems/mems.h"
#include "state.h"
#include "drivers/led.h"

#include "sensors.h"

#if 0

void i2c_error_handler(I2C_HandleTypeDef *hi2c)
{
  uint32_t tmp1 = 0U, tmp2 = 0U, tmp3 = 0U, tmp4 = 0U;
  uint32_t sr1itflags = READ_REG(hi2c->Instance->SR1);
  uint32_t itsources  = READ_REG(hi2c->Instance->CR2);

  /* I2C Bus error interrupt occurred ----------------------------------------*/
  if(((sr1itflags & I2C_FLAG_BERR) != RESET) && ((itsources & I2C_IT_ERR) != RESET))
  {
    hi2c->ErrorCode |= HAL_I2C_ERROR_BERR;

    SET_BIT(hi2c->Instance->CR1, I2C_CR1_SWRST);

    mems_init_bus();

    /* Clear BERR flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_BERR);
  }

  /* I2C Arbitration Loss error interrupt occurred ---------------------------*/
  if(((sr1itflags & I2C_FLAG_ARLO) != RESET) && ((itsources & I2C_IT_ERR) != RESET))
  {
    hi2c->ErrorCode |= HAL_I2C_ERROR_ARLO;

    SET_BIT(hi2c->Instance->CR1, I2C_CR1_SWRST);

	mems_init_bus();

    /* Clear ARLO flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ARLO);
  }

  /* I2C Acknowledge failure error interrupt occurred ------------------------*/
  if(((sr1itflags & I2C_FLAG_AF) != RESET) && ((itsources & I2C_IT_ERR) != RESET))
  {
      hi2c->ErrorCode |= HAL_I2C_ERROR_AF;

      /* Generate Stop */
      SET_BIT(hi2c->Instance->CR1,I2C_CR1_STOP);

      /* Clear AF flag */
      __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);
  }

  if(((sr1itflags & I2C_FLAG_) != RESET) && ((itsources & I2C_IT_ERR) != RESET))
    {
      hi2c->ErrorCode |= HAL_I2C_ERROR_DMA;

      SET_BIT(hi2c->Instance->CR1, I2C_CR1_SWRST);

  	mems_init_bus();

      /* Clear ARLO flag */
      __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ARLO);
    }
  //FIXME: для всех ошибок кроме AF переинициализировать шину
}
#endif

void error_system_check(void)
{
	if ((error_system.analog_sensor_init_error == 0) 	&&
		/* (error_system.gps_config_error == 0)			&& */ // Не проверяем, оно настроится потом
		(error_system.gps_init_error == 0)				&&
		(error_system.gps_uart_init_error == 0)			&&
		(error_system.mems_i2c_error == 0)				&&
		(error_system.lis3mdl_error == 0)				&&
		(error_system.lsm6ds3_error == 0)				&&
		(error_system.rtc_error == 0)					&&
		(error_system.timers_error == 0)				&&
		(error_system.uart_transfer_init_error == 0))
	{
		led_blink(3, 400);
	}
	else
		led_blink(4, 400);
}

void error_mems_read(void)
{
	error_system.mems_i2c_error = mems_state.bus_error;
	error_system.mems_i2c_error_counter = mems_state.bus_error_counter;

	error_system.lsm6ds3_error = mems_state.lsm6ds3_error;
	error_system.lsm6ds3_error_counter = mems_state.lsm6ds3_error_counter;

	error_system.lis3mdl_error = mems_state.lis3mdl_error;
	error_system.lis3mdl_error_counter = mems_state.lis3mdl_error_counter;
}

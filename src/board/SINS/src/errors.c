/*
 * errors.c
 *
 *  Created on: 19 авг. 2020 г.
 *      Author: developer
 */

#include <stm32f405xx.h>


void i2c_error_handler(I2C_HandleTypeDef *hi2c)
{
  uint32_t tmp1 = 0U, tmp2 = 0U, tmp3 = 0U, tmp4 = 0U;
  uint32_t sr1itflags = READ_REG(hi2c->Instance->SR1);
  uint32_t itsources  = READ_REG(hi2c->Instance->CR2);

  /* I2C Bus error interrupt occurred ----------------------------------------*/
  if(((sr1itflags & I2C_FLAG_BERR) != RESET) && ((itsources & I2C_IT_ERR) != RESET))
  {
    hi2c->ErrorCode |= HAL_I2C_ERROR_BERR;

    /* Clear BERR flag */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_SWRST);

    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_BERR);
  }

  /* I2C Arbitration Loss error interrupt occurred ---------------------------*/
  if(((sr1itflags & I2C_FLAG_ARLO) != RESET) && ((itsources & I2C_IT_ERR) != RESET))
  {
    hi2c->ErrorCode |= HAL_I2C_ERROR_ARLO;

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
}

#include "main.h"
#include "bme280.h"
#include "ds18b20.h"
#include "ina219.h"
#include "ina_helper.h"

void SystemClock_Config(void);


int main(void)
{
    HAL_Init();

    __HAL_RCC_PWR_CLK_ENABLE();

    SystemClock_Config();

    __HAL_RCC_AFIO_CLK_ENABLE();

    I2C_HandleTypeDef hi2c = {0};
    /*
    onewire_t how;
    onewire_ResetSearch()
    uint8_t status = onewire_First(&how);
    while (status) {
        //Save ROM number from device
        onewire_GetFullROM(ROM_Array_Pointer);
        //Check for new device
        status = onewire_Next(&OneWireStruct);
    }*/

    __HAL_RCC_I2C1_CLK_ENABLE();
    hi2c.Instance = I2C1;
    hi2c.Init.ClockSpeed = 100000;
    hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c.Init.OwnAddress1 = 0;
    hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c.Init.OwnAddress2 = 0;
    hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c) != HAL_OK)
    {
        Error_Handler();
    }
    //__HAL_UNLOCK(&hi2c);

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

    struct bme280_dev_s hbme280 = {0};
    bme280_register_i2c(&hbme280, &hi2c, BME280_ADDRESS_VCC << 1);
    if (bme280_init(&hbme280) < 0) {
        trace_printf("Oh my god!\n");
    }

    bme280_pull_sensor_conf(&hbme280);
    ina219_t hina;
    //ina219_init(&hina, &hi2c, INA219_I2CADDR_A1_GND_A0_GND);
    //_ina_init(&hina, INA219_I2CADDR_A1_GND_A0_GND);

    int k = 0;
    while (1)
    {
        trace_printf("\nIteration: %d\n", k++);

        float current, power;
        //_ina_read(&hina, &current, &power);
        //trace_printf("current: %f power: %f\n", current, power);

        struct bme280_float_data_s data = {0};
        bme280_read(&hbme280, (char*)&data, sizeof(data));
        trace_printf("pressure: %f temp: %f humidity: %f\n", data.pressure,
                data.temperature, data.humidity);

        HAL_Delay(1000);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

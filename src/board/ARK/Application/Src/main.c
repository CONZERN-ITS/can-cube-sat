
int main(void)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    onewire_t how = {0};
    //onewire_pin_init(&how, GPIOB, GPIO_PIN_10);
    onewire_Init(&how, GPIOB, GPIO_PIN_10);
    ow_test(&how);
    onewire_Reset(&how);

    HAL_Delay(100);

    ds18b20_config_t hds[DS18B20_COUNT];
    int ds_count = 0;


    hds[0].rom = 0;
    onewire_ReadRom(&how, &hds[0].rom);
    hds[0].resolution = ds18b20_Resolution_12bits;
    hds[0].how = &how;
    ds18b20_SetResolution(&hds[0], hds[0].resolution);
    ds18b20_Start(&hds[0]);

    while(1) {
        float t = 0.1;
        if (ds18b20_Read(&hds[0], &t)) {
            trace_printf("%0.2f\n", t);
        }

    }
    /*
    uint8_t status = onewire_First(&how);

    for (; ds_count < DS18B20_COUNT && status; ds_count++) {
        hds[ds_count].how = &how;
        hds[ds_count].resolution = ds18b20_Resolution_12bits;
        hds[ds_count].rom = onewire_GetFullROM(&how);

        HAL_Delay(100);
        status = onewire_Next(&how);
    }
    if (!ds_count) {
        trace_printf("Oh no: there are no ds18b20\n");
        while (1) {
        }
    }

    int k = 0;
    trace_printf("\n", k++);
    for (int i = 0; i < ds_count; i++) {
        trace_printf("T%d\t", i);
    }
    trace_printf("\n");
    HAL_Delay(1000);
    while (1) {
        ds18b20_StartAll(&hds[0]);
        HAL_Delay(1000);
        trace_printf("\r");
        for (int i = 0; i < ds_count; i++) {
            float t = 0;
            ds18b20_Read(&hds[i], &t);
            trace_printf("%0.2f\t", t);
        }

        float current, power;

    }*/
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


#include <stdio.h>
#include <errno.h>

#include <stm32f1xx_hal.h>

#include "main.h"
#include "i2c-link.h"


int user_main()
{
	i2c_link_start();

	int i = 0;
	printf("hello stm32\n");
	while(1)
	{
		printf("teak = %d\n", i++);
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		uint8_t packet_data[I2C_LINK_PACKET_SIZE] = {0};
		int rc = i2c_link_read(packet_data, I2C_LINK_PACKET_SIZE);
		printf("i2c_link_read rc = %d\n", rc);

		if (rc > 0)
		{
			rc = i2c_link_write(packet_data, I2C_LINK_PACKET_SIZE);
			printf("i2c_link_write rc = %d\n", rc);
		}

		HAL_Delay(500);
	}

	return 0;
}



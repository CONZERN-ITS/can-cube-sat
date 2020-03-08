#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;
static volatile int busy = 0;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
}


int _write(int file, char *ptr, int len)
{
	while(huart1.gState != HAL_UART_STATE_READY)
	{}
	HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
	return len;
}



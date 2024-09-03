/*
 * debug.c
 *
 *  Created on: Aug 8, 2024
 *      Author: devilalprajapat
 */

#include "main.h"
#include <string.h>
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void send_log(char *pdata)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)pdata, strlen(pdata), 1000);
}


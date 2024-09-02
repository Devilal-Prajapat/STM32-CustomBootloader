/*
 * debug.c
 *
 *  Created on: Aug 1, 2024
 *      Author: devilalprajapat
 */


#include "debug.h"
#include <string.h>

int __io_putchar(int ch)
{
	HAL_UART_Transmit(DEBUG_UART, (uint8_t *)&ch , 1, 100);
	return ch;
}

void send_log(char *ptr)
{
	HAL_UART_Transmit(DEBUG_UART, (uint8_t *)ptr , strlen(ptr), 1000);
}

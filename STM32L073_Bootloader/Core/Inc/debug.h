/*
 * debug.h
 *
 *  Created on: Aug 1, 2024
 *      Author: devilalprajapat
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "main.h"
#include <stdio.h>

//#include <stdarg.h>

#define DEBUG_EN

#define DEBUG_UART    &huart2
#define COM_UART      &huart1

#ifdef DEBUG_EN
//#define DEBUG_LOG(...)  printf(__VA_ARGS__)
#define DEBUG_LOG(tag, msg)  do{\
						send_log("\r\n[ ");\
						send_log((char *)tag);\
						send_log(" ] : ");\
						send_log((char *)msg);\
						}while(0);
#else
#define DEBUG_LOG(msg)
#endif

void send_log(char *ptr);

#endif /* DEBUG_H_ */

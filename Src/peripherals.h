#ifndef _PERIPHERALS_H
#define _PERIPHERALS_H
#include "main.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

#define  TIME_HUART huart1
#define  HOST_HUART huart2

#endif

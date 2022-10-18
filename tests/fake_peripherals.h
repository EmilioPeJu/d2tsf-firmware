#ifndef _FAKE_PERIPHERALS_H
#define _FAKE_PERIPHERALS_H

#include <stdint.h>

typedef enum {
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET
} GPIO_PinState;

typedef struct {
    int _dummy;
} UART_HandleTypeDef;

typedef struct {
    int _dummy;
} GPIO_TypeDef;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

#define  TIME_HUART huart1
#define  HOST_HUART huart2

extern GPIO_TypeDef GPIO1;

#define SHIFT0_GPIO_Port (&GPIO1)
#define SHIFT1_GPIO_Port (&GPIO1)

#define SHIFT0_Pin 0
#define SHIFT1_Pin 1


int HAL_UART_Transmit(
    UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size,
    uint32_t Timeout);

void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

#endif

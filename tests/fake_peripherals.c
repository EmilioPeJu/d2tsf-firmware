#include "fake_peripherals.h"
#include "serial.h"

struct serial_buffer_t time_serial_buffer;
struct serial_buffer_t host_serial_buffer;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

GPIO_TypeDef GPIO1;


int HAL_UART_Transmit(
    UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size,
    uint32_t Timeout)
{
    return 0;
}


void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
}

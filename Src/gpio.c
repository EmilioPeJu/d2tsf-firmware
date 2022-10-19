#include "peripherals.h"

#include "gpio.h"


static void shift_bit(uint32_t bit)
{
    GPIO_TypeDef *gpio = bit ? SHIFT1_GPIO_Port : SHIFT0_GPIO_Port;
    uint16_t pin = bit ? SHIFT1_Pin : SHIFT0_Pin;
    HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}


void shift_timestamp(uint32_t ts)
{
    for (uint8_t i=0; i < 32; i++) {
        shift_bit(!!(ts & 0x80000000));
        ts <<= 1;
    }
}

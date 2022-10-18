#include "peripherals.h"

#include "gpio.h"

static void delay_noops(uint32_t count)
{
    for (uint32_t i=0; i < count; i++) {
        asm("nop");
    }
}


static void shift_bit(uint8_t val)
{
    GPIO_TypeDef *gpio = val ? SHIFT1_GPIO_Port : SHIFT0_GPIO_Port;
    uint16_t pin = val ? SHIFT1_Pin : SHIFT0_Pin;
    HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);
    delay_noops(4);
    HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);
    delay_noops(4);
}


void shift_timestamp(uint32_t ts)
{
    for (uint8_t i=0; i < 32; i++)
    {
        shift_bit(ts & 1);
        ts >>= 1;
    }
}

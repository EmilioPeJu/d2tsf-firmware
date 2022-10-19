#include <string.h>

#include "serial.h"

struct ring_buffer time_serial_buffer;
struct ring_buffer host_serial_buffer;


void serial_init()
{
    TIME_HUART.Instance->CR1 |= USART_CR1_RXNEIE;
    HOST_HUART.Instance->CR1 |= USART_CR1_RXNEIE;
}


void serial_handle_rx(UART_HandleTypeDef *huart, struct ring_buffer *buffer)
{
    uint32_t isrflags = READ_REG(huart->Instance->SR);
    if (isrflags & USART_SR_RXNE) {
        rb_put_isr(buffer, READ_REG(huart->Instance->DR));
        if (rb_is_empty(buffer)) {
            buffer->buffer_overruns++;
        }
    }
    if (isrflags & USART_SR_ORE) {
        buffer->uart_overruns++;
    }
}


int _write(int file, char *ptr, int len)
{
    if (len < 0 || len >= 65536)
        return -1;

    return HAL_UART_Transmit(
        &HOST_HUART, (uint8_t *) ptr, (uint16_t) len,
        TX_SERIAL_TIMEOUT) == HAL_OK ? len : -1;
}

#include <string.h>

#include "serial.h"

struct serial_buffer_t time_serial_buffer;
struct serial_buffer_t host_serial_buffer;


void serial_init()
{
    TIME_HUART.Instance->CR1 |= USART_CR1_RXNEIE;
    HOST_HUART.Instance->CR1 |= USART_CR1_RXNEIE;
}


void serial_handle_rx(UART_HandleTypeDef *huart, struct serial_buffer_t *buffer)
{
    uint32_t isrflags = READ_REG(huart->Instance->SR);
    if (isrflags & USART_SR_RXNE) {
        uint8_t rx_char = READ_REG(huart->Instance->DR);
        uint16_t index = buffer->len++;
        if (buffer->len < SERIAL_BUFFER_SIZE) {
            buffer->data[index] = rx_char;
        } else {
            buffer->too_long++;
            buffer->data[index] = TERMINATOR;
        }
        if (buffer->data[index] == TERMINATOR) {
            if (buffer->got_cmd) {
                buffer->overruns++;
            } else {
                buffer->got_cmd = true;
                buffer->cmd_len = buffer->len;
                memcpy(buffer->command, buffer->data, buffer->cmd_len);
                buffer->command[buffer->cmd_len] = 0;
            }
            buffer->len = 0;
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

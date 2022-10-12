#include "serial.h"

uint8_t host_uart_rx_buffer[HOST_RX_BUFFER_SIZE];
uint16_t host_uart_rx_filled = 0;
uint16_t host_uart_message_len = 0;


void host_uart_start_it()
{
    host_uart_rx_filled = 0;
    host_uart_message_len = 0;
    HAL_UART_Receive_IT(&HOST_HUART, host_uart_rx_buffer, 1);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &HOST_HUART) {
        uint16_t index = host_uart_rx_filled;
        host_uart_rx_filled++;
        if (host_uart_rx_filled >= HOST_RX_BUFFER_SIZE - 1) {
            // on overflow, we truncate the message received
            host_uart_rx_buffer[index] = TERMINATOR;
        }
        uint8_t rx_char = host_uart_rx_buffer[index];
        if (rx_char == TERMINATOR) {
            host_uart_message_len = host_uart_rx_filled;
            host_uart_rx_buffer[index + 1] = 0;
        } else {
            HAL_UART_Receive_IT(
                &HOST_HUART, host_uart_rx_buffer + host_uart_rx_filled, 1);
        }
    }
}


int _write(int file, char *ptr, int len)
{
    if (len < 0 || len >= 65536)
        return -1;

    return HAL_UART_Transmit(
        &HOST_HUART, (uint8_t *) ptr, (uint16_t) len, 100) == HAL_OK ? len : -1;
}

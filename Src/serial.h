#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdbool.h>
#include <stdint.h>

#include "peripherals.h"

#define TERMINATOR 10
#define HOST_RX_BUFFER_SIZE 32

extern uint8_t host_uart_rx_buffer[HOST_RX_BUFFER_SIZE];
extern uint16_t host_uart_message_len;

// start receiving data using USART interrupt handler
// once a message is received, we need to call this function to
// start receiving the next one
void host_uart_start_it();

#endif

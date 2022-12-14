#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdbool.h>
#include <stdint.h>

#include "peripherals.h"
#include "ring_buffer.h"

#define MAX_COMMAND_SIZE 256
#define TERMINATOR 10
#define TX_SERIAL_TIMEOUT 16  // ms
#define RX_SERIAL_TIMEOUT 16  // ms

extern struct ring_buffer time_serial_buffer;

extern struct ring_buffer host_serial_buffer;

bool serial_init();

void serial_handle_rx(UART_HandleTypeDef *huart, struct ring_buffer *buffer);

#endif

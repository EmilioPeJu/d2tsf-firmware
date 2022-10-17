#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdbool.h>
#include <stdint.h>

#include "peripherals.h"

#define TERMINATOR 10
#define SERIAL_BUFFER_SIZE 64
#define TX_SERIAL_TIMEOUT 5  // ms

struct serial_buffer_t {
    uint8_t data[SERIAL_BUFFER_SIZE];
    // extra byte to include NULL ending
    uint8_t command[SERIAL_BUFFER_SIZE + 1];
    uint16_t len;
    uint16_t cmd_len;
    bool got_cmd;
    // the thread hasn't finished with last command when we received a new one
    uint32_t overruns;
    // new byte arrived without reading the last one
    uint32_t uart_overruns;
    // message was too long and was truncated
    uint32_t too_long;
};

extern struct serial_buffer_t time_serial_buffer;

extern struct serial_buffer_t host_serial_buffer;

void serial_init();

void serial_handle_rx(UART_HandleTypeDef *huart, struct serial_buffer_t *buffer);

#endif

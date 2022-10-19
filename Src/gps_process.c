#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "gpio.h"
#include "serial.h"

#include "gps_process.h"

static bool _forward_to_host = false;
static bool _notify_timestamp = false;
static uint32_t _ts_update_counter = 0;
static struct gps_data _last_gps_data;

static uint8_t _gps_cmd[MAX_COMMAND_SIZE + 1];
static uint8_t _gps_cmd_i = 0;


void gps_process()
{
    uint8_t rx_char = 0;
    while (!rb_is_empty(&time_serial_buffer) &&
            rx_char != TERMINATOR &&
            _gps_cmd_i < MAX_COMMAND_SIZE) {
        rx_char = rb_get(&time_serial_buffer);
        _gps_cmd[_gps_cmd_i++] = rx_char;
    }
    _gps_cmd[_gps_cmd_i] = 0;
    if (rx_char == TERMINATOR || _gps_cmd_i >= MAX_COMMAND_SIZE) {
        if (_forward_to_host) {
            HAL_UART_Transmit(
                &HOST_HUART, _gps_cmd, _gps_cmd_i, TX_SERIAL_TIMEOUT);
        }
        struct gps_data gps_data = parse_gps_message((char *) _gps_cmd);
        if (gps_data.valid) {
            _ts_update_counter++;
            _last_gps_data = gps_data;
            HAL_Delay(1);  // make sure PPS happens before
            shift_timestamp(gps_data.timestamp + 1);
            if (_notify_timestamp)
                printf("%" PRIu32 "\n", gps_data.timestamp);
        }
        _gps_cmd_i = 0;
    }
}


void gps_set_forward_to_host(bool enable)
{
    _forward_to_host = enable;
}


void gps_set_notify_timestamp(bool enable)
{
    _notify_timestamp = enable;
}


uint32_t gps_get_ts_update_counter()
{
    return _ts_update_counter;
}


struct gps_data gps_get_last_data()
{
    return _last_gps_data;
}

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


void gps_process()
{
    if (time_serial_buffer.got_cmd) {
        if (_forward_to_host) {
            HAL_UART_Transmit(
                &HOST_HUART, time_serial_buffer.command,
                time_serial_buffer.cmd_len,
                TX_SERIAL_TIMEOUT);
        }
        struct gps_data gps_data = parse_gps_message(
            (char *) time_serial_buffer.command);
        if (gps_data.valid) {
            _ts_update_counter++;
            _last_gps_data = gps_data;
            shift_timestamp(gps_data.timestamp);
            if (_notify_timestamp)
                printf("%" PRIu32 "\n", gps_data.timestamp);
        }
        time_serial_buffer.got_cmd = false;
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

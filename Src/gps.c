#include "gps.h"
#include "serial.h"


static bool _forward_to_host = false;


void gps_process()
{
    if (time_serial_buffer.got_cmd) {
        if (_forward_to_host) {
            HAL_UART_Transmit(
                &HOST_HUART, time_serial_buffer.command,
                time_serial_buffer.cmd_len,
                TX_SERIAL_TIMEOUT);
        }
        time_serial_buffer.got_cmd = false;
    }
}


void gps_forward_to_host(bool enable)
{
    _forward_to_host = enable;
}

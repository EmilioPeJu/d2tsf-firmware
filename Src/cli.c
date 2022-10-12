#include "commands.h"
#include "cli.h"
#include "serial.h"


void cli_process()
{
    if (host_uart_message_len > 0) {
        execute_command_line((char *) host_uart_rx_buffer);
        host_uart_start_it();
    }
}

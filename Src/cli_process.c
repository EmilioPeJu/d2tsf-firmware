#include <stdbool.h>

#include "commands.h"
#include "serial.h"

#include "cli_process.h"


void cli_process()
{
    if (host_serial_buffer.got_cmd) {
        execute_command_line((char *) host_serial_buffer.command);
        host_serial_buffer.got_cmd = false;
    }
}

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "commands.h"
#include "gps_process.h"
#include "serial.h"
#include "util.h"


static bool gps_forward_command(char *args)
{
    unsigned int enable;
    int nargs = sscanf(args, "%u", &enable);
    if (nargs != 1) {
        printf("ERR invalid format\n");
        return false;
    }
    gps_set_forward_to_host(enable ? true : false);
    printf("OK gps_forward done\n");
    return true;
}


static bool gps_notify_command(char *args)
{
    unsigned int enable;
    int nargs = sscanf(args, "%u", &enable);
    if (nargs != 1) {
        printf("ERR invalid format\n");
        return false;
    }
    gps_set_notify_timestamp(enable ? true : false);
    printf("OK gps_notify done\n");
    return true;
}


static bool help_command(char *args)
{
    printf("+The following commands are supported:\n");
    struct command_description *command;
    size_t command_index;
    for_each_command (command, command_index) {
        printf("+ %16s -> %s\n", command->name, command->help);
    }
    printf(".\n");

    return true;
}


static bool ping_command(char *args)
{
    printf("OK pong\n");
    return true;
}


static bool reset_command(char *args)
{
    printf("OK Resetting the MCU\n");
    // give the UART chance to send out the message
    HAL_Delay(50);
    HAL_NVIC_SystemReset();
    return true;
}


static bool stat_command(char *args)
{
    printf("+ host serial overruns: %" PRIu32 "\n",host_serial_buffer.overruns);
    printf("+ host serial byte overruns: %" PRIu32 "\n",
        host_serial_buffer.uart_overruns);
    printf("+ time serial overruns: %" PRIu32 "\n",time_serial_buffer.overruns);
    printf("+ time serial byte overruns: %" PRIu32 "\n",
        time_serial_buffer.uart_overruns);
    printf("+ uptime: %" PRIu32 "\n", util_get_uptime());
    struct gps_data data = gps_get_last_data();
    printf("+ GPS data valid: %s\n", data.valid ? "True" : "False");
    printf("+ GPS timestamp: %" PRIu32 "\n", data.timestamp);
    printf("+ GPS timestamp updates: %" PRIu32 "\n",
        gps_get_ts_update_counter());
    printf("+ GPS coords: %f%c,%f%c\n", data.lat, data.ns, data.lon, data.ew);
    printf("+ GPS speed: %f\n", data.speed);
    printf(".\n");
    return true;
}


static bool ver_command(char *args)
{
    printf("OK EVG2EVR-FEEDER " RELEASE "\n");
    return true;
}


struct command_description command_table[] = {
    {"gps_forward", gps_forward_command, "forward GPS messages to host serial"},
    {"gps_notify", gps_notify_command, "notify GPS timestamp"},
    {"help", help_command, "generates a command list"},
    {"ping", ping_command, "return a pong"},
    {"reset", reset_command, "reset MCU"},
    {"stat", stat_command, "show statistics"},
    {"ver", ver_command, "show version"}
};


size_t command_table_size = ARRAY_SIZE(command_table);


void execute_command_line(char *line)
{
    struct command_description *command_entry;
    size_t command_index;
    char *cmd_end = strchr(line, ' ') ?:
                    strchr(line, 13) ?:
                    strchr(line, 10) ?:
                    line + strlen(line);
    size_t line_command_len = cmd_end - line;
    for_each_command (command_entry, command_index) {
        size_t command_len = strlen(command_entry->name);
        if (line_command_len == command_len
                && strncmp(line, command_entry->name, command_len) == 0) {
            while (*cmd_end == ' ' && *cmd_end != 0)
                cmd_end++;
            command_entry->function(cmd_end);
            return;
        }
    }
    printf("ERR Command not found: %s", line);
}

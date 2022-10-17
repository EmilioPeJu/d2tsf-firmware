#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "commands.h"
#include "util.h"


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


static bool stat_command(char *args)
{
    printf("+ host serial overruns %" PRIu32 "\n", host_serial_buffer.overruns);
    printf("+ host serial byte overruns %" PRIu32 "\n", host_serial_buffer.uart_overruns);
    printf("+ time serial overruns %" PRIu32 "\n", time_serial_buffer.overruns);
    printf("+ time serial byte overruns %" PRIu32 "\n", time_serial_buffer.uart_overruns);
    printf(".\n");
    return true;
}


struct command_description command_table[] = {
    {"help", help_command, "generates a command list"},
    {"ping", ping_command, "return a pong"},
    {"stat", stat_command, "show statistics"},
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

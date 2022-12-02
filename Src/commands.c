#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "commands.h"
#include "gps.h"
#include "gps_nmea.h"
#include "gps_ubx.h"
#include "serial.h"
#include "util.h"

#ifdef TEST
#include "tests.h"
static bool run_tests_command(char *args)
{
    bool result = test_gps();
    printf(".\n");
    return result;
}
#endif


static bool gps_forward_command(char *args)
{
    unsigned int enable;
    int nargs = sscanf(args, "%u", &enable);
    if (nargs != 1) {
        printf("ERR invalid format\n");
        return false;
    }
    gps_set_forward_nmea_to_host(enable ? true : false);
    printf("OK gps_forward done\n");
    return true;
}


static bool gps_data_command(char *args)
{
    struct gps_nmea_rmc_data data = gps_nmea_get_last_rmc_data();
    printf("+ GPS timestamp updates: %" PRIu32 "\n",
        gps_get_ts_update_counter());
    if (data.valid_timestamp)
        printf("+ GPS timestamp: %" PRIu32 "\n", data.timestamp);
    if (data.valid_coords)
        printf("+ GPS coords: %f%c,%f%c\n",
            data.lat / 100.0, data.ns, data.lon / 100.0, data.ew);
    if (data.valid_speed)
        printf("+ GPS speed: %f\n", data.speed);
    printf("+ Using local timestamp: %s\n",
        gps_local_timestamp_on() ? "yes" : "no");
    printf("+ Local timestamp use count: %" PRIu32 "\n",
        gps_local_timestamp_use_count());
    printf(".\n");
    return true;
}


static bool gps_mon_rf_command(char *args)
{
    return gps_ubx_print_last_mon_rf();
}


static bool gps_nav_sat_command(char *args)
{
    return gps_ubx_print_last_nav_sat();
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


static bool gps_valget_command(char *args)
{
    unsigned int key;
    uint32_t val;
    int nargs = sscanf(args, "%x", &key);
    if (nargs != 1) {
        printf("ERR invalid format\n");
        return false;
    }
    if (gps_ubx_val_get_int((uint32_t) key, &val)) {
        printf("OK gps_valget *%x == %" PRIu32 "\n", key, val);
        return true;
    } else {
        printf("ERR gps_valget failed\n");
        return false;
    }
}


static bool gps_valset_command(char *args)
{
    unsigned int key, val;
    int nargs = sscanf(args, "%x %x", &key, &val);
    if (nargs != 2) {
        printf("ERR invalid format\n");
        return false;
    }
    if (gps_ubx_val_set_int((uint32_t) key, (uint32_t) val)) {
        printf("OK gps_valset *%x = %x\n", key, val);
        return true;
    } else {
        printf("ERR gps_valset failed\n");
        return false;
    }
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


static bool mon_command(char *args)
{
    printf("OK ");
    printf("%" PRIu32 ",", util_get_uptime());
    printf("%u,", (unsigned int) gps_local_timestamp_on());
    printf("%" PRIu32 ",", gps_get_last_timestamp());
    printf("%" PRIu16 ",", gps_satellites_used_count());
    gps_ubx_print_last_mon_rf_short();
    printf("\n");
    return true;
}


static bool mon2_command(char *args)
{
    printf("+ Uptime: %" PRIu32 "\n", util_get_uptime());
    printf("+ Using local timestamp: %u\n",
        (unsigned int) gps_local_timestamp_on());
    printf("+ Last timestamp: %" PRIu32 "\n", gps_get_last_timestamp());
    printf("+ Satellites used: %" PRIu16 "\n", gps_satellites_used_count());
    gps_ubx_print_last_mon_rf();
    return true;
}


static bool msg_stat_command(char *args)
{
    struct gps_msg_stats stats = gps_get_msg_stats();
    printf("+ Unexpected start count: %" PRIu32 "\n", stats.unexpected_start);
    printf("+ Overflow count: %" PRIu32 "\n", stats.overflow);
    printf("+ NMEA count: %" PRIu32 "\n", stats.nmea_count);
    printf("+ NMEA unknown count: %" PRIu32 "\n", stats.nmea_unknown_count);
    printf("+ NMEA bad checksum count: %" PRIu32 "\n", stats.nmea_bad_checksum);
    printf("+ UBX count: %" PRIu32 "\n", stats.ubx_count);
    printf("+ UBX malformed count: %" PRIu32 "\n", stats.ubx_malformed);
    printf("+ UBX bad checksum count: %" PRIu32 "\n", stats.ubx_bad_checksum);
    printf("+ UBX unexpected count: %" PRIu32 "\n", stats.ubx_unexpected_count);
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
    printf("+ host serial byte overruns: %" PRIu32 "\n",
        host_serial_buffer.uart_overruns);
    printf("+ host serial buffer overruns: %" PRIu32 "\n",
        host_serial_buffer.buffer_overruns);
    printf("+ time serial byte overruns: %" PRIu32 "\n",
        time_serial_buffer.uart_overruns);
    printf("+ time serial buffer overruns: %" PRIu32 "\n",
        time_serial_buffer.buffer_overruns);
    printf("+ uptime: %" PRIu32 "\n", util_get_uptime());
    printf(".\n");
    return true;
}


static bool ver_command(char *args)
{
    printf("OK D2TSF FIRMWARE " RELEASE "\n");
    return true;
}


struct command_description command_table[] = {
#ifdef TEST
    {"run_tests", run_tests_command, "run tests"},
#endif
    {"gps_data", gps_data_command, "show GPS data"},
    {"gps_forward", gps_forward_command,
        "forward GPS NMEA messages to host serial"},
    {"gps_mon_rf", gps_mon_rf_command, "show RF block information"},
    {"gps_nav_sat", gps_nav_sat_command, "show satellite information"},
    {"gps_notify", gps_notify_command, "notify GPS timestamp"},
    {"gps_valget", gps_valget_command,
        "Get a GPS configuration parameter. gps_valget <hex-key>"},
    {"gps_valset", gps_valset_command,
        "set a GPS configuration parameter. gps_valset <hex-key> <hex-value>"},
    {"help", help_command, "generates a command list"},
    {"mon", mon_command, "show monitoring information"},
    {"mon2", mon2_command, "show monitoring information explained"},
    {"msg_stat", msg_stat_command, "show message statistics"},
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

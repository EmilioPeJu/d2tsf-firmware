#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "gpio.h"
#include "peripherals.h"
#include "serial.h"

#include "gps_nmea.h"

static bool _forward_nmea_to_host = false;
static bool _notify_timestamp = false;
static uint32_t _ts_update_counter = 0;
static struct gps_nmea_rmc_data _last_nmea_rmc_data;
static uint32_t _hold_ts;
static bool _using_hold_ts;
static uint32_t _hold_ts_use_count;


struct gps_nmea_rmc_data gps_nmea_get_last_rmc_data()
{
    return _last_nmea_rmc_data;
}


uint32_t gps_get_ts_update_counter()
{
    return _ts_update_counter;
}


void gps_set_forward_nmea_to_host(bool enable)
{
    _forward_nmea_to_host = enable;
}


void gps_set_notify_timestamp(bool enable)
{
    _notify_timestamp = enable;
}


bool gps_nmea_is_rmc(char *msg)
{
    return msg[3] == 'R' && msg[4] == 'M' && msg[5] == 'C';
}


struct gps_nmea_rmc_data gps_nmea_parse_rmc(char *msg)
{
    struct gps_nmea_rmc_data result = (struct gps_nmea_rmc_data) {
        .valid = false
    };

    float lat, lon, speed;
    char status, ns, ew;
    uint32_t time, subtime, date;
    int nparsed = sscanf(msg,
        "$%*c%*cRMC,%" PRIu32 ".%" PRIu32",%c,"
        "%f,%c,%f,%c,"
        "%f,%*s",
        &time, &subtime, &status,
        &lat, &ns, &lon, &ew,
        &speed);
    result.valid = nparsed >= 3 && status == 'A';
    result.valid_timestamp = result.valid;
    if (!result.valid)
        return result;

    if (nparsed >= 7) {
        result.valid_coords = true;
        result.lat = lat;
        result.ns = ns;
        result.lon = lon;
        result.ew = ew;
    }

    if (nparsed >= 8) {
        result.valid_speed = true;
        result.speed = speed;
    }

    char *ptr = msg;
    for (int i=0; i < 9; i++) {
        ptr = strchr(ptr+1, ',');
        if (ptr == NULL) {
            result.valid = false;
            return result;
        }
    }
    nparsed = sscanf(ptr, ",%" PRIu32, &date);
    result.valid = nparsed == 1;

    if (!result.valid)
        return result;

    int itime = (int) time;
    int idate= (int) date;
    struct tm tm1 = (struct tm) {
        .tm_sec = itime % 100,
        .tm_min = (itime / 100) % 100,
        .tm_hour = itime / 10000,
        .tm_year = (idate % 100) + 100,
        .tm_mon = (idate / 100) % 100 - 1,
        .tm_mday = idate / 10000,
    };
    time_t _ts = mktime(&tm1);
    result.valid = _ts != (time_t) -1;
    result.timestamp = (uint32_t) _ts;
    return result;
}


bool gps_nmea_validate_checksum(char *line)
{
    uint8_t acc = 0;
    unsigned int expected_checksum;
    char *checksum_ptr = strrchr(line, '*');
    if (checksum_ptr == NULL)
        return false;

    int nparsed = sscanf(checksum_ptr, "*%x", &expected_checksum);
    if (nparsed != 1)
        return false;

    for (char *ptr = line + 1; ptr < checksum_ptr; ptr++)
        acc ^= *ptr;

    return acc == (uint8_t) expected_checksum;
}


bool gps_local_timestamp_on()
{
    return _using_hold_ts;
}


uint32_t gps_local_timestamp_use_count()
{
    return _hold_ts_use_count;
}


// we assume we get a RMC message every second shortly after the PPS
bool gps_nmea_process_msg(uint8_t *buffer, uint16_t len)
{
    if (_forward_nmea_to_host)
        HAL_UART_Transmit(&HOST_HUART, buffer, len, TX_SERIAL_TIMEOUT);

    if (!gps_nmea_is_rmc((char *) buffer))
        return false;

    struct gps_nmea_rmc_data gps_data = gps_nmea_parse_rmc((char *) buffer);
    if (gps_data.valid) {
        _ts_update_counter++;
        _last_nmea_rmc_data = gps_data;
        _hold_ts = gps_data.timestamp;
        _using_hold_ts = false;
        gpio_shift_timestamp(gps_data.timestamp + 1);
        if (_notify_timestamp)
            printf("%" PRIu32 "\n", gps_data.timestamp);
        return true;
    } else {
        _hold_ts++;
        _using_hold_ts = true;
        _hold_ts_use_count++;
        gpio_shift_timestamp(_hold_ts + 1);
        if (_notify_timestamp)
            printf("%" PRIu32 "\n", _hold_ts);
        return false;
    }

    return false;
}

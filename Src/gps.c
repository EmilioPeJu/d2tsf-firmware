#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "gps.h"


struct gps_data parse_gps_message(char *msg)
{
    struct gps_data result = (struct gps_data) {
        .valid = false
    };
    if (strlen(msg) <= 6 ||
            msg[3] != 'R' ||
            msg[4] != 'M' ||
            msg[5] != 'C') {
        return result;
    }

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
    if (!result.valid)
        return result;

    if (nparsed >= 7) {
        result.lat = lat;
        result.ns = ns;
        result.lon = lon;
        result.ew = ew;
    }

    if (nparsed >= 8) {
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

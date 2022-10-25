#ifndef _GPS_H_
#define _GPS_H_

#include <stdbool.h>
#include <stdint.h>

#define NMEA_START '$'
#define NMEA_END 10

struct gps_nmea_rmc_data {
    bool valid;
    bool valid_timestamp;
    uint32_t timestamp;
    bool valid_coords;
    float lat;
    char ns;
    float lon;
    char ew;
    bool valid_speed;
    float speed;
    float course;
};

struct gps_nmea_rmc_data gps_nmea_get_last_rmc_data();

uint32_t gps_get_ts_update_counter();

void gps_set_forward_nmea_to_host(bool enable);

void gps_set_notify_timestamp(bool enable);

bool gps_nmea_is_rmc(char *msg);

struct gps_nmea_rmc_data gps_nmea_parse_rmc(char *msg);

bool gps_nmea_validate_checksum(char *line);

bool gps_local_timestamp_on();

uint32_t gps_local_timestamp_use_count();

bool gps_nmea_process_msg(uint8_t *buffer, uint16_t len);

#endif

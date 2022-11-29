#ifndef _GPS_PROCESS_H
#define _GPS_PROCESS_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_GPS_MSG_SIZE 1024

struct gps_msg_stats {
    uint32_t unexpected_start;
    uint32_t overflow;
    uint32_t nmea_count;
    uint32_t nmea_unknown_count;
    uint32_t nmea_bad_checksum;
    uint32_t ubx_count;
    uint32_t ubx_malformed;
    uint32_t ubx_bad_checksum;
    uint32_t ubx_unexpected_count;
    uint32_t recv_timeout_count;
};

enum gps_msg_type {
    MSG_NONE,
    MSG_NMEA,
    MSG_UBX
};

void gps_handle();

struct gps_msg_stats gps_get_msg_stats();

uint8_t *gps_receive_msg(
    uint16_t *nbytes, enum gps_msg_type *type, uint16_t timeout);

#endif

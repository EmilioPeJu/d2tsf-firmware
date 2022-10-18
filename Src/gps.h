#ifndef _GPS_H_
#define _GPS_H_

#include <stdbool.h>
#include <stdint.h>

struct gps_data {
    bool valid;
    uint32_t timestamp;
    float lat;
    char ns;
    float lon;
    char ew;
    float speed;
    float course;
};

struct gps_data parse_gps_message(char *msg);

#endif

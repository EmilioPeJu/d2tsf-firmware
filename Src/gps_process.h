#ifndef _GPS_PROCESS_H
#define _GPS_PROCESS_H

#include <stdbool.h>
#include "gps.h"

void gps_process();

void gps_set_forward_to_host(bool enable);

void gps_set_notify_timestamp(bool enable);

uint32_t gps_get_ts_update_counter();

struct gps_data gps_get_last_data();

#endif

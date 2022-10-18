#include <stdio.h>

#include "cli_process.h"
#include "gps_process.h"
#include "peripherals.h"
#include "serial.h"
#include "util.h"

#include "evg_ts_feeder_main.h"


void evg_ts_feeder_main()
{
    serial_init();
    while (1) {
        cli_process();
        gps_process();
        util_update_uptime(HAL_GetTick());
    }
}

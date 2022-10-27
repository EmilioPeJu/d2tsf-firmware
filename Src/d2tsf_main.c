#include <stdio.h>

#include "cli.h"
#include "gps_config.h"
#include "gps.h"
#include "peripherals.h"
#include "serial.h"
#include "util.h"

#include "d2tsf_main.h"


void d2tsf_main()
{
    serial_init();
    gps_wait_for_receiver_up();

    if (!gps_config())
        printf("GPS configuration failed\n");

    while (1) {
        cli_handle();
        gps_handle();
        util_update_uptime(HAL_GetTick());
    }
}

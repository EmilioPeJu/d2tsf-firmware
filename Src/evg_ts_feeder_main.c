#include <stdio.h>

#include "cli.h"
#include "serial.h"

#include "evg_ts_feeder_main.h"


void evg_ts_feeder_main()
{
    serial_init();
    while (1) {
        cli_process();
    }
}

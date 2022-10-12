#include <stdio.h>

#include "cli.h"
#include "serial.h"

#include "evg_ts_feeder_main.h"

void evg_ts_feeder_main()
{
    host_uart_start_it();
    while (1) {
        cli_process();
    }
}

#include "util.h"

static uint32_t _uptime;


void util_update_uptime(uint32_t tick)
{
    static uint32_t _last_tick = 0;
    // tick happen every millisecond
    if (tick >= _last_tick + 1000) {
        _uptime++;
        _last_tick = tick;
    }
}


uint32_t util_get_uptime()
{
    return _uptime;
}

#ifndef _UTIL_H_
#define _UTIL_H_
#include <stdint.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void util_update_uptime(uint32_t tick);
uint32_t util_get_uptime();

#endif /* UTIL_H_ */

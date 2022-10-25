#include <stdio.h>
#include <stdint.h>

#include "gps_ubx.h"
#include "gps_ubx_config_keys.h"
#include "peripherals.h"

#include "gps_config.h"

#define TP_USE_FREQUENCY 1
#define TP_USE_RATIO 0
#define TP_TIMEGRID_UTC 0

#define GPS_RECEIVER_UP_TIMEOUT 2000  // ms

#define ERROR_IF_FALSE(expr) \
    do { \
        if (!expr) { \
            printf("Error in gps_config.c:%d\n", __LINE__); \
            result = false; \
        } \
    } while (0)


bool gps_config()
{
    bool result = true;
    // Disable unused NMEA messages
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_MSGOUT_NMEA_ID_GGA_UART1, 0));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_MSGOUT_NMEA_ID_GSA_UART1, 0));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_MSGOUT_NMEA_ID_GSV_UART1, 0));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_MSGOUT_NMEA_ID_GLL_UART1, 0));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_MSGOUT_NMEA_ID_VTG_UART1, 0));
    // Configure time pulses
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_PULSE_DEF, TP_USE_FREQUENCY));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_PULSE_LENGTH_DEF, TP_USE_RATIO));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_USE_LOCKED_TP1, 1));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_FREQ_TP1, 1));  // 1 Hz
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_FREQ_LOCK_TP1, 1));
    ERROR_IF_FALSE(gps_ubx_val_set_double(CFG_TP_DUTY_TP1, 25.0));
    ERROR_IF_FALSE(gps_ubx_val_set_double(CFG_TP_DUTY_LOCK_TP1, 50.0));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_TIMEGRID_TP1, TP_TIMEGRID_UTC));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_TP1_ENA, 1));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_USE_LOCKED_TP2, 1));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_FREQ_TP2, 1000000));  // 1 MHz
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_FREQ_LOCK_TP2, 1000000));
    ERROR_IF_FALSE(gps_ubx_val_set_double(CFG_TP_DUTY_TP2, 25.0));
    ERROR_IF_FALSE(gps_ubx_val_set_double(CFG_TP_DUTY_LOCK_TP2, 50.0));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_TIMEGRID_TP2, TP_TIMEGRID_UTC));
    ERROR_IF_FALSE(gps_ubx_val_set_int(CFG_TP_TP2_ENA, 1));
    return result;
}


void gps_wait_for_receiver_up()
{
    uint32_t timeout_tick = HAL_GetTick() + GPS_RECEIVER_UP_TIMEOUT;
    while (HAL_GetTick() < timeout_tick) {
        uint32_t val;
        // get a random parameter to know if receiver is up
        if (gps_ubx_val_get_int(CFG_TP_PULSE_DEF, &val))
            return;
    }
    printf("GPS Receiver unresponsive\n");
}



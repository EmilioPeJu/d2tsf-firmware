#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "gpio.h"
#include "gps_nmea.h"
#include "gps_ubx.h"
#include "peripherals.h"
#include "serial.h"

#include "gps.h"

static struct gps_msg_stats _gps_msg_stats;

enum gps_rx_state {
    RX_IDLE,
    RX_NMEA,
    RX_UBX
};


struct gps_msg_stats gps_get_msg_stats()
{
    return _gps_msg_stats;
}


// Return buffer with full message or NULL if there is no message
static uint8_t *_receive_msg(
    uint16_t *nbytes, enum gps_msg_type *type)
{
    static uint8_t buffer[MAX_GPS_MSG_SIZE + 1];

    static enum gps_rx_state state = RX_IDLE;
    static uint16_t count = 0;
    static uint16_t payload_size = 0;
    while (!rb_is_empty(&time_serial_buffer)) {
        uint8_t rx_byte = rb_get(&time_serial_buffer);
        buffer[count++] = rx_byte;
        switch (state) {
            case RX_IDLE:
                if (rx_byte == NMEA_START) {
                    state = RX_NMEA;
                } else if (rx_byte == UBX_MAGIC_START1) {
                    state = RX_UBX;
                } else {
                    count = 0;
                    _gps_msg_stats.unexpected_start++;
                }
                break;
            case RX_NMEA:
                if (rx_byte == NMEA_END) {
                    // message end
                    state = RX_IDLE;
                    buffer[count] = 0;
                    *nbytes = count;
                    count = 0;
                    *type = MSG_NMEA;
                    _gps_msg_stats.nmea_count++;
                    if (gps_nmea_validate_checksum((char *) buffer))
                        return buffer;
                    else
                        _gps_msg_stats.nmea_bad_checksum++;
                }
                break;
            case RX_UBX:
                if (count == 2 && rx_byte != UBX_MAGIC_START2) {
                    state = RX_IDLE;
                    _gps_msg_stats.ubx_malformed++;
                    count = 0;
                } else if (count == 6) {
                    payload_size =
                        ((uint16_t) buffer[count - 1] << 8) + buffer[count - 2];
                } else if (count >= 8 + payload_size) {
                    // got to the message end
                    state = RX_IDLE;
                    *nbytes = count;
                    count = 0;
                    *type = MSG_UBX;
                    _gps_msg_stats.ubx_count++;
                    if (gps_ubx_validate_checksum(buffer, *nbytes)) {
                        return buffer;
                    } else {
                        _gps_msg_stats.ubx_bad_checksum++;
                    }
                }
                break;
            default:
                state = RX_IDLE;
        }
        if (count >= MAX_GPS_MSG_SIZE) {
            state = RX_IDLE;
            _gps_msg_stats.overflow++;
            count = 0;
        }
    }
    *type = MSG_NONE;
    return NULL;
}


// Return buffer with full message or NULL if there is no message
uint8_t *gps_receive_msg(
    uint16_t *nbytes, enum gps_msg_type *type, uint16_t timeout)
{
    uint32_t timeout_tick = HAL_GetTick() + timeout;
    uint8_t *buffer = NULL;
    bool first = true;
    while ((!buffer && HAL_GetTick() < timeout_tick) || first) {
        buffer = _receive_msg(nbytes, type);
        first = false;
    }

    return buffer;
}


void gps_handle()
{
    uint16_t nbytes;
    enum gps_msg_type type;
    uint8_t *buffer = _receive_msg(&nbytes, &type);
    if (buffer) {
        if (type == MSG_NMEA) {
            gps_nmea_process_msg(buffer, nbytes);
        } else if (type == MSG_UBX) {
            if (!gps_ubx_process_msg(buffer, nbytes))
                _gps_msg_stats.ubx_unexpected_count++;
        }
    }
}

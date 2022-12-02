#ifndef _GPS_UBX_H
#define _GPS_UBX_H
#include <stdbool.h>
#include <stdint.h>

#define UBX_MAGIC_START1 0xb5
#define UBX_MAGIC_START2 0x62
#define UBX_CLASS_VALSET 0x06
#define UBX_ID_VALSET 0x8a
#define UBX_VALSET_MESSAGE_VERSION 0x0
#define UBX_VALSET_TO_RAM_LAYER 1
#define UBX_CLASS_VALGET 0x06
#define UBX_ID_VALGET 0x8b
#define UBX_VALGET_FROM_RAM_LAYER 0
#define UBX_CLASS_ACK 0x5
#define UBX_ID_ACK 0x1
#define UBX_CLASS_MON_RF 0x0a
#define UBX_ID_MON_RF 0x38
#define UBX_CLASS_NACK 0x5
#define UBX_ID_NACK 0x0
#define UBX_CLASS_NAV_SAT 0x1
#define UBX_ID_NAV_SAT 0x35

uint16_t gps_satellites_used_count();

uint8_t gps_ubx_config_val_len(uint32_t key);

uint16_t gps_ubx_make_val_set_packet(uint32_t key, void *val, uint8_t *buffer);

bool gps_ubx_val_set_int(uint32_t key, uint32_t val);

bool gps_ubx_val_set_double(uint32_t key, double val);

uint16_t gps_ubx_make_val_get_packet(uint32_t key, uint8_t *buffer);

bool gps_ubx_val_get_int(uint32_t key, uint32_t *val);

uint16_t gps_ubx_make_request_for(
    uint8_t *buffer, uint8_t class_id, uint8_t cmd_id);

bool gps_ubx_print_last_mon_rf();

bool gps_ubx_send_mon_rf();

bool gps_ubx_print_last_nav_sat();

bool gps_ubx_send_nav_sat();

bool gps_ubx_is_ack(uint8_t *msg);

bool gps_ubx_is_ack_for(uint8_t *msg, uint8_t class_id, uint8_t cmd_id);

bool gps_ubx_is_nack(uint8_t *msg);

bool gps_ubx_matches_command(uint8_t *msg, uint8_t class_id, uint8_t cmd_id);

bool gps_ubx_validate_checksum(uint8_t *msg, uint16_t len);

uint8_t *gps_ubx_receive(uint16_t *nbytes);

bool gps_ubx_process_msg(uint8_t *buffer, uint16_t len);

#endif

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "gps.h"
#include "gps_nmea.h"
#include "serial.h"

#include "gps_ubx.h"


uint8_t gps_ubx_config_val_len(uint32_t key)
{
    switch ((key >> 28) & 0x7) {
        case 1:
        case 2:
            return 1;
            break;
        case 3:
            return 2;
            break;
        case 4:
            return 4;
            break;
        default:
            return 8;
    }
}


static uint16_t calculate_ubx_checksum(uint8_t *msg, uint16_t len)
{
    uint8_t cka=0;
    uint8_t ckb=0;

    for (uint16_t i=2; i < len - 2; i++) {
        cka += msg[i];
        ckb += cka;
    }

    return ((uint16_t) ckb << 8) + cka;
}


// output will always be 16 bytes
uint16_t gps_ubx_make_val_get_packet(uint32_t key, uint8_t *buffer)
{
    uint8_t packet[] = {
        // Common header
        UBX_MAGIC_START1, UBX_MAGIC_START2, UBX_CLASS_VALGET, UBX_ID_VALGET,
        8, 0,
        // Payload start
        UBX_VALSET_MESSAGE_VERSION, UBX_VALGET_FROM_RAM_LAYER, 0, 0,
        key & 0xff, (key >> 8) & 0xff, (key >> 16) & 0xff, (key >> 24) & 0xff,
        0, 0 // checksum part
    };
    const uint16_t total_size = 16;
    uint16_t checksum = calculate_ubx_checksum(packet, total_size);
    packet[14] = checksum & 0xff;
    packet[15] = (checksum >> 8) & 0xff;
    memcpy(buffer, packet, total_size);
    return total_size;
}


bool gps_ubx_val_get_int(uint32_t key, uint32_t *val)
{
    uint8_t tx_buffer[16];
    uint16_t nbytes = gps_ubx_make_val_get_packet((uint32_t) key, tx_buffer);
    HAL_UART_Transmit(&TIME_HUART, tx_buffer, nbytes, TX_SERIAL_TIMEOUT);
    uint8_t *msg_buffer = gps_ubx_receive(&nbytes);
    if (!msg_buffer ||
            nbytes < 15 ||
            msg_buffer[2] != UBX_CLASS_VALGET ||
            msg_buffer[3] != UBX_ID_VALGET ||
            *(uint32_t *) (msg_buffer + 10) != key)
        return false;

    *val = *(uint32_t *) (msg_buffer + 14);
    // mask based on value's length
    *val &= (1 << (8*gps_ubx_config_val_len(key))) - 1;
    msg_buffer = gps_ubx_receive(&nbytes);
    return msg_buffer && gps_ubx_is_ack_for(
        msg_buffer, UBX_CLASS_VALGET, UBX_ID_VALGET);
}


// output will always be 24 bytes or less
uint16_t gps_ubx_make_val_set_packet(uint32_t key, void *val, uint8_t *buffer)
{
    uint8_t payload_len = 8;  // payload header + key
    uint8_t val_len = gps_ubx_config_val_len(key);
    payload_len += val_len;
    uint8_t packet[] = {
        // Common header
        UBX_MAGIC_START1, UBX_MAGIC_START2, UBX_CLASS_VALSET, UBX_ID_VALSET,
        payload_len, 0,
        // Payload start
        UBX_VALSET_MESSAGE_VERSION, UBX_VALSET_TO_RAM_LAYER, 0, 0,
        key & 0xff, (key >> 8) & 0xff, (key >> 16) & 0xff, (key >> 24) & 0xff,
        0, 0, 0, 0, 0, 0, 0, 0, // value part
        0, 0  // checksum part
    };
    memcpy(packet + 14, val, val_len);
    uint16_t total_size = 8 + payload_len;
    uint16_t checksum = calculate_ubx_checksum(packet, total_size);
    packet[6 + payload_len] = checksum & 0xff;
    packet[7 + payload_len] = (checksum >> 8) & 0xff;
    memcpy(buffer, packet, total_size);
    return total_size;
}


bool gps_ubx_val_set_int(uint32_t key, uint32_t val)
{
    uint8_t tx_buffer[32];
    uint16_t nbytes = gps_ubx_make_val_set_packet((uint32_t) key, &val, tx_buffer);
    HAL_UART_Transmit(&TIME_HUART, tx_buffer, nbytes, TX_SERIAL_TIMEOUT);
    uint8_t *msg_buffer = gps_ubx_receive(&nbytes);
    return msg_buffer && gps_ubx_is_ack_for(
        msg_buffer, UBX_CLASS_VALSET, UBX_ID_VALSET);
}


bool gps_ubx_val_set_double(uint32_t key, double val)
{
    uint8_t tx_buffer[32];
    uint16_t nbytes = gps_ubx_make_val_set_packet(
        (uint32_t) key, &val, tx_buffer);
    HAL_UART_Transmit(&TIME_HUART, tx_buffer, nbytes, TX_SERIAL_TIMEOUT);
    uint8_t *msg_buffer = gps_ubx_receive(&nbytes);
    return msg_buffer && gps_ubx_is_ack_for(
        msg_buffer, UBX_CLASS_VALSET, UBX_ID_VALSET);
}


uint16_t gps_ubx_make_request_for(
    uint8_t *buffer, uint8_t class_id, uint8_t cmd_id)
{
    uint8_t packet[] = {
        // Common header
        UBX_MAGIC_START1, UBX_MAGIC_START2, class_id, cmd_id,
        0, 0,
        0, 0  // checksum part
    };
    const uint16_t total_size = 8;
    uint16_t checksum = calculate_ubx_checksum(packet, total_size);
    packet[total_size - 2] = checksum & 0xff;
    packet[total_size - 1] = (checksum >> 8) & 0xff;
    memcpy(buffer, packet, total_size);
    return total_size;
}


static char *_rf_status_to_string(unsigned int status)
{
    if (status > 4)
        return "Unknown";

    char *status_names[] = {"INIT", "DONTKNOW", "OK", "SHORT", "OPEN"};
    return status_names[status];
}


static char *_rf_power_to_string(unsigned int power)
{
    if (power == 0)
        return "OFF";
    else if (power == 1)
        return "ON";
    else
        return "DONTKNOW";
}


void gps_ubx_print_mon_rf(uint8_t *msg, uint16_t len)
{
    printf("+ Number of RF blocks: %u\n", (unsigned int) msg[7]);
    for (uint8_t i=0; i < msg[7]; i++) {
        unsigned int status =  (unsigned int) msg[12 + i*24];
        printf("+ %u Ant status: %u (%s)\n",
            i, status, _rf_status_to_string(status));
        unsigned int power =  (unsigned int) msg[13 + i*24];
        printf("+ %u Ant power: %u (%s)\n",
            i, power, _rf_power_to_string(power));
        printf("+ %u GPS noise level: %u\n",
            i, (unsigned int) msg[22 + i*24] + (msg[23 + i*24] << 8));
    }
    printf(".\n");
}


bool gps_ubx_send_mon_rf()
{
    uint8_t buffer[16];
    uint16_t nbytes = gps_ubx_make_request_for(
        buffer, UBX_CLASS_MON_RF, UBX_ID_MON_RF);
    HAL_UART_Transmit(&TIME_HUART, buffer, nbytes, TX_SERIAL_TIMEOUT);
    return true;
}


static char *_gnss_id_to_string(unsigned int id)
{
    if (id > 7)
        return "Unknown";

    char *gnss_names[] = {
        "GPS", "SBAS", "Galileo", "BeiDou", "IMES", "QZSS", "GLONASS", "NavIC"};
    return gnss_names[id];
}


void gps_ubx_print_nav_sat(uint8_t *msg, uint16_t len)
{
    uint16_t used_count = 0;
    printf("+ Number of satellites: %u\n", (unsigned int) msg[11]);
    for (uint8_t i=0; i < msg[11]; i++) {
        unsigned int gnss_id = (unsigned int) msg[14 + i*12];
        printf("+ %u GNSS: %s (%u)\n", i, _gnss_id_to_string(gnss_id), gnss_id);
        printf("+ %u Sat ID: %u\n", i, msg[15 + i*12]);
        printf("+ %u Signal strength: %u dBHz\n",
            i, (unsigned int) msg[16 + i*12]);
        unsigned int flags = *(unsigned int*) (msg + 22 + i*12);
        printf("+ %u Flags: %x (", i, flags);
        switch (flags & 0x7) {
            case 0:
                printf("NO SIGNAL; ");
                break;
            case 1:
                printf("SEARCHING; ");
                break;
            case 2:
                printf("ACQUIRED; ");
                break;
            case 3:
                printf("DETECTED; ");
                break;
            case 4:
                printf("CODE_LOCKED; ");
                break;
            default:
                printf("ALL_LOCKED; ");
        }
        bool used = (flags >> 3) & 1;
        printf("%s; ", used ? "USED" : "NOT_USED");
        if (used)
            used_count++;
        printf("%s; ", ((flags >> 4) & 3) == 1 ? "HEALTHY" : "NOT_HEALTHY");
        printf(")\n");
    }
    printf("+ Satellites used = %" PRIu16 "\n", used_count);
    printf(".\n");
}


bool gps_ubx_send_nav_sat()
{
    uint8_t buffer[16];
    uint16_t nbytes = gps_ubx_make_request_for(
        buffer, UBX_CLASS_NAV_SAT, UBX_ID_NAV_SAT);
    HAL_UART_Transmit(&TIME_HUART, buffer, nbytes, TX_SERIAL_TIMEOUT);
    return true;
}


bool gps_ubx_is_ack_for(uint8_t *msg, uint8_t class_id, uint8_t cmd_id)
{
    return gps_ubx_matches_command(msg, UBX_CLASS_ACK, UBX_ID_ACK) &&
        msg[6] == class_id && msg[7] == cmd_id;
}


bool gps_ubx_is_ack(uint8_t *msg)
{
    return gps_ubx_matches_command(msg, UBX_CLASS_ACK, UBX_ID_ACK);
}


bool gps_ubx_is_nack(uint8_t *msg)
{
    return gps_ubx_matches_command(msg, UBX_CLASS_NACK, UBX_ID_NACK);
}


bool gps_ubx_matches_command(uint8_t *msg, uint8_t class_id, uint8_t cmd_id)
{
    return msg[2] == class_id && msg[3] == cmd_id;
}


bool gps_ubx_validate_checksum(uint8_t *msg, uint16_t len)
{
    if (len < 4)
        return false;

    uint16_t checksum = calculate_ubx_checksum(msg, len);
    uint16_t expected = *((uint16_t *) (msg + len - 2));
    return checksum == expected;
}


uint8_t *gps_ubx_receive(uint16_t *nbytes)
{
    enum gps_msg_type type;
    uint8_t *buffer = NULL;
    while (true) {
        buffer = gps_receive_msg(nbytes, &type, RX_SERIAL_TIMEOUT);
        if (!buffer) {
            return NULL;
        }
        if (type == MSG_UBX) {
            return buffer;
        } else if (type == MSG_NMEA) {
            gps_nmea_process_msg(buffer, *nbytes);
        }
    }
    return NULL;
}


bool gps_ubx_process_msg(uint8_t *buffer, uint16_t len)
{
    if (gps_ubx_matches_command(buffer, UBX_CLASS_NAV_SAT, UBX_ID_NAV_SAT)) {
        gps_ubx_print_nav_sat(buffer, len);
        return true;
    } else if (
            gps_ubx_matches_command(buffer, UBX_CLASS_MON_RF, UBX_ID_MON_RF)) {
        gps_ubx_print_mon_rf(buffer, len);
        return true;
    } else {
        return false;
    }
}

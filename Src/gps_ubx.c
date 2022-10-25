#include <stdio.h>
#include <string.h>

#include "gps.h"
#include "gps_nmea.h"
#include "serial.h"

#include "gps_ubx.h"

static uint8_t aux_recv_buffer[MAX_COMMAND_SIZE + 1];


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
    bool got_msg = gps_ubx_receive(aux_recv_buffer, MAX_COMMAND_SIZE, &nbytes);
    if (!got_msg ||
            nbytes < 15 ||
            aux_recv_buffer[2] != UBX_CLASS_VALGET ||
            aux_recv_buffer[3] != UBX_ID_VALGET ||
            *(uint32_t *) (aux_recv_buffer + 10) != key)
        return false;

    *val = *(uint32_t *) (aux_recv_buffer + 14);
    // mask based on value's length
    *val &= (1 << (8*gps_ubx_config_val_len(key))) - 1;
    got_msg = gps_ubx_receive(aux_recv_buffer, MAX_COMMAND_SIZE, &nbytes);
    return got_msg && gps_ubx_is_ack_for(
        aux_recv_buffer, UBX_CLASS_VALGET, UBX_ID_VALGET);
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
    uint8_t buffer[32];
    uint16_t nbytes = gps_ubx_make_val_set_packet((uint32_t) key, &val, buffer);
    HAL_UART_Transmit(&TIME_HUART, buffer, nbytes, TX_SERIAL_TIMEOUT);
    bool got_msg = gps_ubx_receive(aux_recv_buffer, MAX_COMMAND_SIZE, &nbytes);
    return got_msg && gps_ubx_is_ack_for(
        aux_recv_buffer, UBX_CLASS_VALSET, UBX_ID_VALSET);
}


bool gps_ubx_val_set_double(uint32_t key, double val)
{
    uint8_t buffer[32];
    uint16_t nbytes = gps_ubx_make_val_set_packet((uint32_t) key, &val, buffer);
    HAL_UART_Transmit(&TIME_HUART, buffer, nbytes, TX_SERIAL_TIMEOUT);
    bool got_msg = gps_ubx_receive(aux_recv_buffer, MAX_COMMAND_SIZE, &nbytes);
    return got_msg && gps_ubx_is_ack_for(
        aux_recv_buffer, UBX_CLASS_VALSET, UBX_ID_VALSET);
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


void gps_ubx_print_mon_rf(uint8_t *msg, uint16_t len)
{
    printf("+ Number of RF blocks: %u\n", (unsigned int) msg[7]);
    for (uint8_t i=0; i < msg[7]; i++) {
        printf("+ Ant status %u: %u\n", i, (unsigned int) msg[12 + i*24]);
        printf("+ Ant power %u: %u\n", i, (unsigned int) msg[13 + i*24]);
        printf("+ GPS noise level %u: %u\n",
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


void gps_ubx_print_nav_sat(uint8_t *msg, uint16_t len)
{
    printf("+ Number of satellites: %u\n", (unsigned int) msg[11]);
    for (uint8_t i=0; i < msg[11]; i++) {
        printf("+ GNSS %u: %u\n", i, (unsigned int) msg[14 + i*12]);
        printf("+ ID %u: %u\n", i, msg[15 + i*12]);
        printf("+ Signal strength %u: %u dBHz\n",
            i, (unsigned int) msg[16 + i*12]);
        printf("+ Flags %u: %x\n",
            i, (unsigned int)  *(uint32_t *) (msg + 22 + i*12));
    }
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


bool gps_ubx_receive(uint8_t *buffer, uint16_t buffer_len, uint16_t *nbytes)
{
    enum gps_msg_type type;
    bool got_msg = true;
    while (true) {
        got_msg = gps_receive_msg(
            buffer, nbytes, &type, buffer_len, RX_SERIAL_TIMEOUT);
        if (!got_msg) {
            return false;
        }
        if (type == MSG_UBX) {
            return true;
        } else if (type == MSG_NMEA) {
            gps_nmea_process_msg(aux_recv_buffer, *nbytes);
        }
    }
    return false;
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

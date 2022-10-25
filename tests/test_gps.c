#include <stdio.h>
#include <stdint.h>
#include "minunit.h"
#include "gps.h"
#include "gps_nmea.h"
#include "gps_ubx.h"

#include "tests.h"

static int tests_run = 0;

char nmea_message_not_interesting[] =
    "$GPGGA,002153.000,3342.6618,N,11751.3858,W,1,10,1.2,27.0,M,-34.2,M,,0000*5E";
char nmea_message_not_valid[] =
    "$GPRMC,161229.487,V,3723.2475,N,12158.3416,W,0.13,309.62,120598, ,*27";
char nmea_message_valid1[] =
    "$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120500, ,*31";
char nmea_message_valid2[] =
    "$GNRMC,123222.00,A,5134.41843,N,00118.81205,W,0.054,,181022,,,A*78";
char nmea_message_bad_checksum[] =
    "$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120500, ,*10";
uint8_t ubx_message_valid1[] = {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19};
uint8_t ubx_message_bad_checksum[] =
    {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x29};


static char *test_gps_nmea_parse_rmc_filters_non_interesting_message()
{
    struct gps_nmea_rmc_data result = gps_nmea_parse_rmc(nmea_message_not_interesting);
    mu_assert("+ Non interesting message not filtered", !result.valid);
    return NULL;
}


static char *test_gps_nmea_parse_rmc_filters_not_valid_data()
{
    struct gps_nmea_rmc_data result = gps_nmea_parse_rmc(nmea_message_not_valid);
    mu_assert("+ Invalid message not filtered", !result.valid);
    return NULL;
}


static char *test_gps_parse_rmc_nmea1_with_valid_data()
{
    struct gps_nmea_rmc_data result = gps_nmea_parse_rmc(nmea_message_valid1);
    mu_assert("+ Valid message filtered", result.valid);
    mu_assert("+ Timestamp doesn't match", result.timestamp == 958147949);
    return NULL;
}


static char *test_gps_parse_rmc_nmea2_with_valid_data()
{
    struct gps_nmea_rmc_data result = gps_nmea_parse_rmc(nmea_message_valid2);
    mu_assert("+ Valid message filtered", result.valid);
    mu_assert("+ Timestamp doesn't match", result.timestamp == 1666096342);
    return NULL;
}


static char *test_gps_nmea_validate_checksum_with_good_checksum()
{
    bool result = gps_nmea_validate_checksum(nmea_message_valid1);
    mu_assert("+ Valid checksum not detected", result);
    return NULL;
}


static char *test_gps_nmea_validate_checksum_with_bad_checksum()
{
    bool result = gps_nmea_validate_checksum(nmea_message_bad_checksum);
    mu_assert("+ Invalid checksum not detected", !result);
    return NULL;
}


static char *test_gps_ubx_validate_checksum_with_good_checksum()
{
    bool result = gps_ubx_validate_checksum(
        ubx_message_valid1, sizeof(ubx_message_valid1));
    mu_assert("+ Valid checksum not detected", result);
    return NULL;
}


static char *test_gps_ubx_validate_checksum_with_bad_checksum()
{
    bool result = gps_ubx_validate_checksum(
        ubx_message_bad_checksum, sizeof(ubx_message_bad_checksum));
    mu_assert("+ Invalid checksum not detected", !result);
    return NULL;
}


static char *all_tests()
{
    mu_run_test(test_gps_nmea_parse_rmc_filters_non_interesting_message);
    mu_run_test(test_gps_nmea_parse_rmc_filters_not_valid_data);
    mu_run_test(test_gps_parse_rmc_nmea1_with_valid_data);
    mu_run_test(test_gps_parse_rmc_nmea2_with_valid_data);
    mu_run_test(test_gps_nmea_validate_checksum_with_good_checksum);
    mu_run_test(test_gps_nmea_validate_checksum_with_bad_checksum);
    mu_run_test(test_gps_ubx_validate_checksum_with_good_checksum);
    mu_run_test(test_gps_ubx_validate_checksum_with_bad_checksum);
    return NULL;
}


// true means OK
bool test_gps()
{
    printf("+ Testing gps functions\n");
    char *result = all_tests();
    if (result) {
        printf("%s\n", result);
    } else {
        printf("+ ALL PASSED\n");
    }
    printf("+ Tests run: %d\n", tests_run);
    return result ? false : true;
}

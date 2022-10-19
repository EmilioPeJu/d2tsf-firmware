#include <stdio.h>
#include "minunit.h"
#include "gps.h"

int tests_run = 0;

char message_not_interesting[] =
    "$GPGGA,002153.000,3342.6618,N,11751.3858,W,1,10,1.2,27.0,M,-34.2,M,,0000*5E";
char message_not_valid[] =
    "$GPRMC,161229.487,V,3723.2475,N,12158.3416,W,0.13,309.62,120598, ,*27";
char message_bad_checksum[] =
    "$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120500, ,*10";
char message_valid1[] =
    "$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120500, ,*31";

char message_valid2[] =
    "$GNRMC,123222.00,A,5134.41843,N,00118.81205,W,0.054,,181022,,,A*78";


static char *test_parse_gps_message_filters_non_interesting_message()
{
    struct gps_data result = parse_gps_message(message_not_interesting);
    mu_assert("Non interesting message not filtered", !result.valid);
    return NULL;
}


static char *test_parse_gps_message_filters_not_valid_data()
{
    struct gps_data result = parse_gps_message(message_not_valid);
    mu_assert("Invalid message not filtered", !result.valid);
    return NULL;
}

static char *test_parse_gps_message_with_bad_checksum()
{
    struct gps_data result = parse_gps_message(message_bad_checksum);
    mu_assert("Message with bad checksum not filtered", !result.valid);
    return NULL;
}

static char *test_parse_gps_message1_with_valid_data()
{
    struct gps_data result = parse_gps_message(message_valid1);
    mu_assert("Valid message filtered", result.valid);
    mu_assert("Timestamp doesn't match", result.timestamp == 958147949);
    return NULL;
}


static char *test_parse_gps_message2_with_valid_data()
{
    struct gps_data result = parse_gps_message(message_valid2);
    mu_assert("Valid message filtered", result.valid);
    mu_assert("Timestamp doesn't match", result.timestamp == 1666096342);
    return NULL;
}


static char *all_tests()
{
    mu_run_test(test_parse_gps_message_filters_non_interesting_message);
    mu_run_test(test_parse_gps_message_filters_not_valid_data);
    mu_run_test(test_parse_gps_message_with_bad_checksum);
    mu_run_test(test_parse_gps_message1_with_valid_data);
    mu_run_test(test_parse_gps_message2_with_valid_data);
    return NULL;
}


int main()
{
    char *result = all_tests();
    if (result) {
        printf("%s\n", result);
    } else {
        printf("ALL PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result ? 1 : 0;
}

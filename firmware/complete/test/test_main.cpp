#include <unity.h>

#include "test_interpolate.h"
#include "test_limits.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_adc_voltage);
    RUN_TEST(test_adc_current);
    RUN_TEST(test_adc_temp1);
    RUN_TEST(test_adc_temp2);

    RUN_TEST(test_dac_voltage);
    RUN_TEST(test_dac_current);

    RUN_TEST(test_limits_current);
    RUN_TEST(test_limits_power);

    UNITY_END();
}

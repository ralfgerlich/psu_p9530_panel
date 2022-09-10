#include <Arduino.h>
#include <unity.h>

#include <stdint.h>
#include <math.h>

#define private public
#define protected public

#include "ps9530_ctrl.h"

#define ADC_REF_VALUE 2.5f
#define ADC_RESOLUTION_BITS 10
#define ADC_RESOLUTION_STEPS(bits) (1 << bits)
#define VOLTS_PER_STEP(value, bits) (value / ADC_RESOLUTION_STEPS(bits))

void setUp(void) {
    // set stuff up here
    PS9530_Ctrl::getInstance();
}

void tearDown(void) {
    // clean stuff up here
}

void test_temp_1() {
    PS9530_Ctrl& ctrl = PS9530_Ctrl::getInstance();
    double volts_per_Step = VOLTS_PER_STEP(ADC_REF_VALUE, ADC_RESOLUTION_BITS);
    //uint16_t full_range_steps = (2.1f - 1.2f)/volts_per_Step;
    //double steps_per_degree_c = (130-20)/full_range_steps;

    TEST_ASSERT_INT16_WITHIN(10, 20, ctrl.interpolateADCTemp(0, uint16_t(2/volts_per_Step)));
    TEST_ASSERT_INT16_WITHIN(10, 25, ctrl.interpolateADCTemp(0, uint16_t(1.75f/volts_per_Step)));
    TEST_ASSERT_INT16_WITHIN(10, 77, ctrl.interpolateADCTemp(0, uint16_t(1.545f/volts_per_Step)));
    TEST_ASSERT_INT16_WITHIN(10, 130, ctrl.interpolateADCTemp(0, uint16_t(1.34f/volts_per_Step)));
}

void test_temp_2() {
    PS9530_Ctrl& ctrl = PS9530_Ctrl::getInstance();
    double volts_per_Step = VOLTS_PER_STEP(ADC_REF_VALUE, ADC_RESOLUTION_BITS);

    TEST_ASSERT_INT16_WITHIN(10, 25, ctrl.interpolateADCTemp(1, uint16_t(2/volts_per_Step)));
    TEST_ASSERT_INT16_WITHIN(10, 77, ctrl.interpolateADCTemp(1, uint16_t(1.62f/volts_per_Step)));
    TEST_ASSERT_INT16_WITHIN(10, 130, ctrl.interpolateADCTemp(1, uint16_t(1.24f/volts_per_Step)));
}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_temp_1);
    RUN_TEST(test_temp_2);

    UNITY_END();
}
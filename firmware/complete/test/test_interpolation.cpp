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
#define VOLTS_PER_STEP(value, bits) (value / (ADC_RESOLUTION_STEPS(bits)-1))

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_temp_1() {
    double volts_per_Step = VOLTS_PER_STEP(ADC_REF_VALUE, ADC_RESOLUTION_BITS);
    //uint16_t full_range_steps = (2.1f - 1.2f)/volts_per_Step;
    //double steps_per_degree_c = (130-20)/full_range_steps;

    TEST_ASSERT_INT16_WITHIN(2, -25, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.994f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 20, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.821f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 25, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.801f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 77, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.595f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 130, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.400f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
}

void test_temp_2() {
    double volts_per_Step = VOLTS_PER_STEP(ADC_REF_VALUE, ADC_RESOLUTION_BITS);

    TEST_ASSERT_INT16_WITHIN(2, -15, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(2.448f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 20, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(2.085f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 25, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(2.036f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 77, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(1.599f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 130, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(1.267f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
}

void test_adc_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 37, PS9530_Ctrl::interpolateADCVoltage(0));
    TEST_ASSERT_UINT16_WITHIN(10, 6110, PS9530_Ctrl::interpolateADCVoltage(181));
    TEST_ASSERT_UINT16_WITHIN(10, 7640, PS9530_Ctrl::interpolateADCVoltage(228));
    TEST_ASSERT_UINT16_WITHIN(10, 9170, PS9530_Ctrl::interpolateADCVoltage(274));
    TEST_ASSERT_UINT16_WITHIN(10, 10690, PS9530_Ctrl::interpolateADCVoltage(320));
    TEST_ASSERT_UINT16_WITHIN(10, 12210, PS9530_Ctrl::interpolateADCVoltage(367));
    TEST_ASSERT_UINT16_WITHIN(100, 22900, PS9530_Ctrl::interpolateADCVoltage(690));
    TEST_ASSERT_UINT16_WITHIN(100, 30500, PS9530_Ctrl::interpolateADCVoltage(920));
}

void test_adc_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 2, PS9530_Ctrl::interpolateADCCurrent(0));
    TEST_ASSERT_UINT16_WITHIN(10, 2140, PS9530_Ctrl::interpolateADCCurrent(215));
    TEST_ASSERT_UINT16_WITHIN(10, 4930, PS9530_Ctrl::interpolateADCCurrent(508));
    TEST_ASSERT_UINT16_WITHIN(10, 7170, PS9530_Ctrl::interpolateADCCurrent(741));
    TEST_ASSERT_UINT16_WITHIN(10, 9670, PS9530_Ctrl::interpolateADCCurrent(1004));
}

void test_dac_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACVoltage(0));
    TEST_ASSERT_UINT16_WITHIN(2, 1332, PS9530_Ctrl::interpolateDACVoltage(2500));
    TEST_ASSERT_UINT16_WITHIN(2, 1783, PS9530_Ctrl::interpolateDACVoltage(3340));
    TEST_ASSERT_UINT16_WITHIN(2, 9257, PS9530_Ctrl::interpolateDACVoltage(17250));
    TEST_ASSERT_UINT16_WITHIN(2, 11963, PS9530_Ctrl::interpolateDACVoltage(22310));
    TEST_ASSERT_UINT16_WITHIN(2, 14805, PS9530_Ctrl::interpolateDACVoltage(27520));
}

void test_dac_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACCurrent(0));
    TEST_ASSERT_UINT16_WITHIN(2, 2097, PS9530_Ctrl::interpolateDACCurrent(1347));
    TEST_ASSERT_UINT16_WITHIN(2, 3031, PS9530_Ctrl::interpolateDACCurrent(1984));
    TEST_ASSERT_UINT16_WITHIN(2, 3553, PS9530_Ctrl::interpolateDACCurrent(2340));
    TEST_ASSERT_UINT16_WITHIN(2, 5220, PS9530_Ctrl::interpolateDACCurrent(3479));
    TEST_ASSERT_UINT16_WITHIN(2, 8292, PS9530_Ctrl::interpolateDACCurrent(5570));
    TEST_ASSERT_UINT16_WITHIN(2, 9263, PS9530_Ctrl::interpolateDACCurrent(6233));
    TEST_ASSERT_UINT16_WITHIN(2, 13271, PS9530_Ctrl::interpolateDACCurrent(8970));
    TEST_ASSERT_UINT16_WITHIN(2, 14788, PS9530_Ctrl::interpolateDACCurrent(10000));
}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_adc_voltage);
    RUN_TEST(test_adc_current);
    RUN_TEST(test_temp_1);
    RUN_TEST(test_temp_2);

    RUN_TEST(test_dac_voltage);
    RUN_TEST(test_dac_current);

    UNITY_END();
}

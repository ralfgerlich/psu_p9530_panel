#include <Arduino.h>
#include <unity.h>

#include <math.h>

#define private public
#define protected public

#include "ps9530_ctrl.h"

#define ADC_REF_VALUE 2.5f
#define ADC_RESOLUTION_BITS 10
#define ADC_RESOLUTION_STEPS(bits) (1 << bits)
#define VOLTS_PER_STEP(value, bits) (value / (ADC_RESOLUTION_STEPS(bits)-1))

void test_adc_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 5413, PS9530_Ctrl::interpolateADCVoltage(157));
    TEST_ASSERT_UINT16_WITHIN(2, 11294, PS9530_Ctrl::interpolateADCVoltage(335));
    TEST_ASSERT_UINT16_WITHIN(2, 11294, PS9530_Ctrl::interpolateADCVoltage(335));
    TEST_ASSERT_UINT16_WITHIN(2, 11824, PS9530_Ctrl::interpolateADCVoltage(351));
    TEST_ASSERT_UINT16_WITHIN(2, 12650, PS9530_Ctrl::interpolateADCVoltage(376));
    TEST_ASSERT_UINT16_WITHIN(2, 14547, PS9530_Ctrl::interpolateADCVoltage(433));
    TEST_ASSERT_UINT16_WITHIN(2, 15575, PS9530_Ctrl::interpolateADCVoltage(464));
    TEST_ASSERT_UINT16_WITHIN(2, 18421, PS9530_Ctrl::interpolateADCVoltage(550));
    TEST_ASSERT_UINT16_WITHIN(2, 27059, PS9530_Ctrl::interpolateADCVoltage(811));
    TEST_ASSERT_UINT16_WITHIN(2, 32600, PS9530_Ctrl::interpolateADCVoltage(951));
}

void test_adc_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 2812, PS9530_Ctrl::interpolateADCCurrent(285));
    TEST_ASSERT_UINT16_WITHIN(2, 3453, PS9530_Ctrl::interpolateADCCurrent(353));
    TEST_ASSERT_UINT16_WITHIN(2, 5828, PS9530_Ctrl::interpolateADCCurrent(601));
    TEST_ASSERT_UINT16_WITHIN(2, 6164, PS9530_Ctrl::interpolateADCCurrent(636));
    TEST_ASSERT_UINT16_WITHIN(2, 7064, PS9530_Ctrl::interpolateADCCurrent(730));
    TEST_ASSERT_UINT16_WITHIN(2, 7360, PS9530_Ctrl::interpolateADCCurrent(761));
    TEST_ASSERT_UINT16_WITHIN(2, 7758, PS9530_Ctrl::interpolateADCCurrent(803));
    TEST_ASSERT_UINT16_WITHIN(2, 8367, PS9530_Ctrl::interpolateADCCurrent(867));
    TEST_ASSERT_UINT16_WITHIN(2, 8579, PS9530_Ctrl::interpolateADCCurrent(889));
    TEST_ASSERT_UINT16_WITHIN(2, 9730, PS9530_Ctrl::interpolateADCCurrent(1010));
}

void test_adc_temp1() {
    double volts_per_Step = VOLTS_PER_STEP(ADC_REF_VALUE, ADC_RESOLUTION_BITS);

    TEST_ASSERT_INT16_WITHIN(2, -25, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.994f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 20, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.821f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 25, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.801f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 77, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.595f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 130, PS9530_Ctrl::interpolateADCTemp(0, uint16_t(1.400f/volts_per_Step), PS9530_ADC_TEMP1_SHIFT));
}

void test_adc_temp2() {
    double volts_per_Step = VOLTS_PER_STEP(ADC_REF_VALUE, ADC_RESOLUTION_BITS);

    TEST_ASSERT_INT16_WITHIN(2, -15, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(2.448f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 20, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(2.085f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 25, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(2.036f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 77, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(1.599f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
    TEST_ASSERT_INT16_WITHIN(2, 130, PS9530_Ctrl::interpolateADCTemp(1, uint16_t(1.267f/volts_per_Step), PS9530_ADC_TEMP2_SHIFT));
}

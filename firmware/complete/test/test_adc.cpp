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
    TEST_ASSERT_UINT16_WITHIN(2, 5537, PS9530_Ctrl::interpolateADCVoltage(161));
    TEST_ASSERT_UINT16_WITHIN(2, 12037, PS9530_Ctrl::interpolateADCVoltage(358));
    TEST_ASSERT_UINT16_WITHIN(2, 14370, PS9530_Ctrl::interpolateADCVoltage(428));
    TEST_ASSERT_UINT16_WITHIN(2, 16087, PS9530_Ctrl::interpolateADCVoltage(480));
    TEST_ASSERT_UINT16_WITHIN(2, 17049, PS9530_Ctrl::interpolateADCVoltage(509));
    TEST_ASSERT_UINT16_WITHIN(2, 17181, PS9530_Ctrl::interpolateADCVoltage(513));
    TEST_ASSERT_UINT16_WITHIN(2, 17506, PS9530_Ctrl::interpolateADCVoltage(523));
    TEST_ASSERT_UINT16_WITHIN(2, 21620, PS9530_Ctrl::interpolateADCVoltage(648));
    TEST_ASSERT_UINT16_WITHIN(2, 29710, PS9530_Ctrl::interpolateADCVoltage(891));
    TEST_ASSERT_UINT16_WITHIN(2, 31543, PS9530_Ctrl::interpolateADCVoltage(954));
}

void test_adc_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 1635, PS9530_Ctrl::interpolateADCCurrent(161));
    TEST_ASSERT_UINT16_WITHIN(2, 3518, PS9530_Ctrl::interpolateADCCurrent(358));
    TEST_ASSERT_UINT16_WITHIN(2, 4188, PS9530_Ctrl::interpolateADCCurrent(428));
    TEST_ASSERT_UINT16_WITHIN(2, 4683, PS9530_Ctrl::interpolateADCCurrent(480));
    TEST_ASSERT_UINT16_WITHIN(2, 4958, PS9530_Ctrl::interpolateADCCurrent(509));
    TEST_ASSERT_UINT16_WITHIN(2, 4996, PS9530_Ctrl::interpolateADCCurrent(513));
    TEST_ASSERT_UINT16_WITHIN(2, 5093, PS9530_Ctrl::interpolateADCCurrent(523));
    TEST_ASSERT_UINT16_WITHIN(2, 6296, PS9530_Ctrl::interpolateADCCurrent(648));
    TEST_ASSERT_UINT16_WITHIN(2, 8641, PS9530_Ctrl::interpolateADCCurrent(891));
    TEST_ASSERT_UINT16_WITHIN(2, 9256, PS9530_Ctrl::interpolateADCCurrent(954));
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

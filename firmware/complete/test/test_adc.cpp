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

void test_adc_temp1() {
    double volts_per_Step = VOLTS_PER_STEP(ADC_REF_VALUE, ADC_RESOLUTION_BITS);
    //uint16_t full_range_steps = (2.1f - 1.2f)/volts_per_Step;
    //double steps_per_degree_c = (130-20)/full_range_steps;

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

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
    TEST_ASSERT_UINT16_WITHIN(2, 104, PS9530_Ctrl::interpolateADCVoltage(0));
    TEST_ASSERT_UINT16_WITHIN(2, 166, PS9530_Ctrl::interpolateADCVoltage(1));
    TEST_ASSERT_UINT16_WITHIN(2, 228, PS9530_Ctrl::interpolateADCVoltage(2));
    TEST_ASSERT_UINT16_WITHIN(2, 291, PS9530_Ctrl::interpolateADCVoltage(3));
    TEST_ASSERT_UINT16_WITHIN(2, 291, PS9530_Ctrl::interpolateADCVoltage(3));
    TEST_ASSERT_UINT16_WITHIN(2, 461, PS9530_Ctrl::interpolateADCVoltage(7));
    TEST_ASSERT_UINT16_WITHIN(2, 824, PS9530_Ctrl::interpolateADCVoltage(18));
    TEST_ASSERT_UINT16_WITHIN(2, 1063, PS9530_Ctrl::interpolateADCVoltage(25));
    TEST_ASSERT_UINT16_WITHIN(2, 1063, PS9530_Ctrl::interpolateADCVoltage(25));
    TEST_ASSERT_UINT16_WITHIN(2, 1156, PS9530_Ctrl::interpolateADCVoltage(28));
    TEST_ASSERT_UINT16_WITHIN(2, 6419, PS9530_Ctrl::interpolateADCVoltage(188));
    TEST_ASSERT_UINT16_WITHIN(2, 8106, PS9530_Ctrl::interpolateADCVoltage(239));
    TEST_ASSERT_UINT16_WITHIN(2, 10713, PS9530_Ctrl::interpolateADCVoltage(318));
    TEST_ASSERT_UINT16_WITHIN(2, 12723, PS9530_Ctrl::interpolateADCVoltage(379));
    TEST_ASSERT_UINT16_WITHIN(2, 13879, PS9530_Ctrl::interpolateADCVoltage(413));
    TEST_ASSERT_UINT16_WITHIN(2, 16153, PS9530_Ctrl::interpolateADCVoltage(482));
    TEST_ASSERT_UINT16_WITHIN(2, 17049, PS9530_Ctrl::interpolateADCVoltage(509));
    TEST_ASSERT_UINT16_WITHIN(2, 30110, PS9530_Ctrl::interpolateADCVoltage(904));
    TEST_ASSERT_UINT16_WITHIN(2, 31457, PS9530_Ctrl::interpolateADCVoltage(951));
    TEST_ASSERT_UINT16_WITHIN(2, 32374, PS9530_Ctrl::interpolateADCVoltage(983));

}

void test_adc_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateADCCurrent(0));
    TEST_ASSERT_UINT16_WITHIN(2, 18, PS9530_Ctrl::interpolateADCCurrent(1));
    TEST_ASSERT_UINT16_WITHIN(2, 36, PS9530_Ctrl::interpolateADCCurrent(2));
    TEST_ASSERT_UINT16_WITHIN(2, 55, PS9530_Ctrl::interpolateADCCurrent(3));
    TEST_ASSERT_UINT16_WITHIN(2, 55, PS9530_Ctrl::interpolateADCCurrent(3));
    TEST_ASSERT_UINT16_WITHIN(2, 127, PS9530_Ctrl::interpolateADCCurrent(7));
    TEST_ASSERT_UINT16_WITHIN(2, 266, PS9530_Ctrl::interpolateADCCurrent(18));
    TEST_ASSERT_UINT16_WITHIN(2, 331, PS9530_Ctrl::interpolateADCCurrent(25));
    TEST_ASSERT_UINT16_WITHIN(2, 331, PS9530_Ctrl::interpolateADCCurrent(25));
    TEST_ASSERT_UINT16_WITHIN(2, 359, PS9530_Ctrl::interpolateADCCurrent(28));
    TEST_ASSERT_UINT16_WITHIN(2, 1894, PS9530_Ctrl::interpolateADCCurrent(188));
    TEST_ASSERT_UINT16_WITHIN(2, 2383, PS9530_Ctrl::interpolateADCCurrent(239));
    TEST_ASSERT_UINT16_WITHIN(2, 3133, PS9530_Ctrl::interpolateADCCurrent(318));
    TEST_ASSERT_UINT16_WITHIN(2, 3719, PS9530_Ctrl::interpolateADCCurrent(379));
    TEST_ASSERT_UINT16_WITHIN(2, 4045, PS9530_Ctrl::interpolateADCCurrent(413));
    TEST_ASSERT_UINT16_WITHIN(2, 4702, PS9530_Ctrl::interpolateADCCurrent(482));
    TEST_ASSERT_UINT16_WITHIN(2, 4958, PS9530_Ctrl::interpolateADCCurrent(509));
    TEST_ASSERT_UINT16_WITHIN(2, 8767, PS9530_Ctrl::interpolateADCCurrent(904));
    TEST_ASSERT_UINT16_WITHIN(2, 9226, PS9530_Ctrl::interpolateADCCurrent(951));
    TEST_ASSERT_UINT16_WITHIN(2, 9534, PS9530_Ctrl::interpolateADCCurrent(983));

}

void test_dac_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACVoltage(11));
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACVoltage(95));
    TEST_ASSERT_UINT16_WITHIN(2, 42, PS9530_Ctrl::interpolateDACVoltage(225));
    TEST_ASSERT_UINT16_WITHIN(2, 97, PS9530_Ctrl::interpolateDACVoltage(324));
    TEST_ASSERT_UINT16_WITHIN(2, 98, PS9530_Ctrl::interpolateDACVoltage(325));
    TEST_ASSERT_UINT16_WITHIN(2, 197, PS9530_Ctrl::interpolateDACVoltage(500));
    TEST_ASSERT_UINT16_WITHIN(2, 313, PS9530_Ctrl::interpolateDACVoltage(714));
    TEST_ASSERT_UINT16_WITHIN(2, 349, PS9530_Ctrl::interpolateDACVoltage(779));
    TEST_ASSERT_UINT16_WITHIN(2, 428, PS9530_Ctrl::interpolateDACVoltage(925));
    TEST_ASSERT_UINT16_WITHIN(2, 459, PS9530_Ctrl::interpolateDACVoltage(983));
    TEST_ASSERT_UINT16_WITHIN(2, 1935, PS9530_Ctrl::interpolateDACVoltage(3712));
    TEST_ASSERT_UINT16_WITHIN(2, 4641, PS9530_Ctrl::interpolateDACVoltage(8758));
    TEST_ASSERT_UINT16_WITHIN(2, 4976, PS9530_Ctrl::interpolateDACVoltage(9380));
    TEST_ASSERT_UINT16_WITHIN(2, 5924, PS9530_Ctrl::interpolateDACVoltage(11140));
    TEST_ASSERT_UINT16_WITHIN(2, 6253, PS9530_Ctrl::interpolateDACVoltage(11755));
    TEST_ASSERT_UINT16_WITHIN(2, 7099, PS9530_Ctrl::interpolateDACVoltage(13329));
    TEST_ASSERT_UINT16_WITHIN(2, 8486, PS9530_Ctrl::interpolateDACVoltage(15905));
    TEST_ASSERT_UINT16_WITHIN(2, 12263, PS9530_Ctrl::interpolateDACVoltage(22908));
    TEST_ASSERT_UINT16_WITHIN(2, 13621, PS9530_Ctrl::interpolateDACVoltage(25436));
    TEST_ASSERT_UINT16_WITHIN(2, 15262, PS9530_Ctrl::interpolateDACVoltage(28495));

}

void test_dac_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACCurrent(19));
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACCurrent(29));
    TEST_ASSERT_UINT16_WITHIN(2, 143, PS9530_Ctrl::interpolateDACCurrent(200));
    TEST_ASSERT_UINT16_WITHIN(2, 157, PS9530_Ctrl::interpolateDACCurrent(213));
    TEST_ASSERT_UINT16_WITHIN(2, 179, PS9530_Ctrl::interpolateDACCurrent(234));
    TEST_ASSERT_UINT16_WITHIN(2, 218, PS9530_Ctrl::interpolateDACCurrent(270));
    TEST_ASSERT_UINT16_WITHIN(2, 245, PS9530_Ctrl::interpolateDACCurrent(294));
    TEST_ASSERT_UINT16_WITHIN(2, 285, PS9530_Ctrl::interpolateDACCurrent(331));
    TEST_ASSERT_UINT16_WITHIN(2, 371, PS9530_Ctrl::interpolateDACCurrent(410));
    TEST_ASSERT_UINT16_WITHIN(2, 383, PS9530_Ctrl::interpolateDACCurrent(421));
    TEST_ASSERT_UINT16_WITHIN(2, 1385, PS9530_Ctrl::interpolateDACCurrent(850));
    TEST_ASSERT_UINT16_WITHIN(2, 2966, PS9530_Ctrl::interpolateDACCurrent(1928));
    TEST_ASSERT_UINT16_WITHIN(2, 4259, PS9530_Ctrl::interpolateDACCurrent(2811));
    TEST_ASSERT_UINT16_WITHIN(2, 4390, PS9530_Ctrl::interpolateDACCurrent(2901));
    TEST_ASSERT_UINT16_WITHIN(2, 5343, PS9530_Ctrl::interpolateDACCurrent(3552));
    TEST_ASSERT_UINT16_WITHIN(2, 10560, PS9530_Ctrl::interpolateDACCurrent(7117));
    TEST_ASSERT_UINT16_WITHIN(2, 11037, PS9530_Ctrl::interpolateDACCurrent(7445));
    TEST_ASSERT_UINT16_WITHIN(2, 11488, PS9530_Ctrl::interpolateDACCurrent(7755));
    TEST_ASSERT_UINT16_WITHIN(2, 11810, PS9530_Ctrl::interpolateDACCurrent(7978));
    TEST_ASSERT_UINT16_WITHIN(2, 12104, PS9530_Ctrl::interpolateDACCurrent(8181));

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

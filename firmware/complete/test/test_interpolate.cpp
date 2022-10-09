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
    TEST_ASSERT_UINT16_WITHIN(2, 166, PS9530_Ctrl::interpolateADCVoltage(1));
    TEST_ASSERT_UINT16_WITHIN(2, 166, PS9530_Ctrl::interpolateADCVoltage(1));
    TEST_ASSERT_UINT16_WITHIN(2, 228, PS9530_Ctrl::interpolateADCVoltage(2));
    TEST_ASSERT_UINT16_WITHIN(2, 353, PS9530_Ctrl::interpolateADCVoltage(4));
    TEST_ASSERT_UINT16_WITHIN(2, 564, PS9530_Ctrl::interpolateADCVoltage(10));
    TEST_ASSERT_UINT16_WITHIN(2, 824, PS9530_Ctrl::interpolateADCVoltage(18));
    TEST_ASSERT_UINT16_WITHIN(2, 893, PS9530_Ctrl::interpolateADCVoltage(20));
    TEST_ASSERT_UINT16_WITHIN(2, 893, PS9530_Ctrl::interpolateADCVoltage(20));
    TEST_ASSERT_UINT16_WITHIN(2, 1063, PS9530_Ctrl::interpolateADCVoltage(25));
    TEST_ASSERT_UINT16_WITHIN(2, 1249, PS9530_Ctrl::interpolateADCVoltage(31));
    TEST_ASSERT_UINT16_WITHIN(2, 6681, PS9530_Ctrl::interpolateADCVoltage(196));
    TEST_ASSERT_UINT16_WITHIN(2, 10053, PS9530_Ctrl::interpolateADCVoltage(298));
    TEST_ASSERT_UINT16_WITHIN(2, 14305, PS9530_Ctrl::interpolateADCVoltage(426));
    TEST_ASSERT_UINT16_WITHIN(2, 14370, PS9530_Ctrl::interpolateADCVoltage(428));
    TEST_ASSERT_UINT16_WITHIN(2, 17116, PS9530_Ctrl::interpolateADCVoltage(511));
    TEST_ASSERT_UINT16_WITHIN(2, 17928, PS9530_Ctrl::interpolateADCVoltage(536));
    TEST_ASSERT_UINT16_WITHIN(2, 20016, PS9530_Ctrl::interpolateADCVoltage(600));
    TEST_ASSERT_UINT16_WITHIN(2, 24369, PS9530_Ctrl::interpolateADCVoltage(730));
    TEST_ASSERT_UINT16_WITHIN(2, 25750, PS9530_Ctrl::interpolateADCVoltage(773));
    TEST_ASSERT_UINT16_WITHIN(2, 30310, PS9530_Ctrl::interpolateADCVoltage(911));

}

void test_adc_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 18, PS9530_Ctrl::interpolateADCCurrent(1));
    TEST_ASSERT_UINT16_WITHIN(2, 18, PS9530_Ctrl::interpolateADCCurrent(1));
    TEST_ASSERT_UINT16_WITHIN(2, 36, PS9530_Ctrl::interpolateADCCurrent(2));
    TEST_ASSERT_UINT16_WITHIN(2, 73, PS9530_Ctrl::interpolateADCCurrent(4));
    TEST_ASSERT_UINT16_WITHIN(2, 177, PS9530_Ctrl::interpolateADCCurrent(10));
    TEST_ASSERT_UINT16_WITHIN(2, 266, PS9530_Ctrl::interpolateADCCurrent(18));
    TEST_ASSERT_UINT16_WITHIN(2, 284, PS9530_Ctrl::interpolateADCCurrent(20));
    TEST_ASSERT_UINT16_WITHIN(2, 284, PS9530_Ctrl::interpolateADCCurrent(20));
    TEST_ASSERT_UINT16_WITHIN(2, 331, PS9530_Ctrl::interpolateADCCurrent(25));
    TEST_ASSERT_UINT16_WITHIN(2, 388, PS9530_Ctrl::interpolateADCCurrent(31));
    TEST_ASSERT_UINT16_WITHIN(2, 1971, PS9530_Ctrl::interpolateADCCurrent(196));
    TEST_ASSERT_UINT16_WITHIN(2, 2944, PS9530_Ctrl::interpolateADCCurrent(298));
    TEST_ASSERT_UINT16_WITHIN(2, 4169, PS9530_Ctrl::interpolateADCCurrent(426));
    TEST_ASSERT_UINT16_WITHIN(2, 4188, PS9530_Ctrl::interpolateADCCurrent(428));
    TEST_ASSERT_UINT16_WITHIN(2, 4977, PS9530_Ctrl::interpolateADCCurrent(511));
    TEST_ASSERT_UINT16_WITHIN(2, 5218, PS9530_Ctrl::interpolateADCCurrent(536));
    TEST_ASSERT_UINT16_WITHIN(2, 5839, PS9530_Ctrl::interpolateADCCurrent(600));
    TEST_ASSERT_UINT16_WITHIN(2, 7085, PS9530_Ctrl::interpolateADCCurrent(730));
    TEST_ASSERT_UINT16_WITHIN(2, 7501, PS9530_Ctrl::interpolateADCCurrent(773));
    TEST_ASSERT_UINT16_WITHIN(2, 8835, PS9530_Ctrl::interpolateADCCurrent(911));

}

void test_dac_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACVoltage(64));
    TEST_ASSERT_UINT16_WITHIN(2, 54, PS9530_Ctrl::interpolateDACVoltage(252));
    TEST_ASSERT_UINT16_WITHIN(2, 63, PS9530_Ctrl::interpolateDACVoltage(267));
    TEST_ASSERT_UINT16_WITHIN(2, 70, PS9530_Ctrl::interpolateDACVoltage(279));
    TEST_ASSERT_UINT16_WITHIN(2, 144, PS9530_Ctrl::interpolateDACVoltage(402));
    TEST_ASSERT_UINT16_WITHIN(2, 242, PS9530_Ctrl::interpolateDACVoltage(583));
    TEST_ASSERT_UINT16_WITHIN(2, 257, PS9530_Ctrl::interpolateDACVoltage(611));
    TEST_ASSERT_UINT16_WITHIN(2, 283, PS9530_Ctrl::interpolateDACVoltage(658));
    TEST_ASSERT_UINT16_WITHIN(2, 335, PS9530_Ctrl::interpolateDACVoltage(753));
    TEST_ASSERT_UINT16_WITHIN(2, 336, PS9530_Ctrl::interpolateDACVoltage(755));
    TEST_ASSERT_UINT16_WITHIN(2, 1545, PS9530_Ctrl::interpolateDACVoltage(2991));
    TEST_ASSERT_UINT16_WITHIN(2, 3090, PS9530_Ctrl::interpolateDACVoltage(5848));
    TEST_ASSERT_UINT16_WITHIN(2, 5317, PS9530_Ctrl::interpolateDACVoltage(10002));
    TEST_ASSERT_UINT16_WITHIN(2, 5880, PS9530_Ctrl::interpolateDACVoltage(11056));
    TEST_ASSERT_UINT16_WITHIN(2, 5939, PS9530_Ctrl::interpolateDACVoltage(11167));
    TEST_ASSERT_UINT16_WITHIN(2, 6808, PS9530_Ctrl::interpolateDACVoltage(12788));
    TEST_ASSERT_UINT16_WITHIN(2, 10578, PS9530_Ctrl::interpolateDACVoltage(19774));
    TEST_ASSERT_UINT16_WITHIN(2, 13395, PS9530_Ctrl::interpolateDACVoltage(25015));
    TEST_ASSERT_UINT16_WITHIN(2, 13910, PS9530_Ctrl::interpolateDACVoltage(25975));
    TEST_ASSERT_UINT16_WITHIN(2, 14344, PS9530_Ctrl::interpolateDACVoltage(26784));

}

void test_dac_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACCurrent(1));
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACCurrent(48));
    TEST_ASSERT_UINT16_WITHIN(2, 21, PS9530_Ctrl::interpolateDACCurrent(88));
    TEST_ASSERT_UINT16_WITHIN(2, 65, PS9530_Ctrl::interpolateDACCurrent(135));
    TEST_ASSERT_UINT16_WITHIN(2, 99, PS9530_Ctrl::interpolateDACCurrent(163));
    TEST_ASSERT_UINT16_WITHIN(2, 150, PS9530_Ctrl::interpolateDACCurrent(207));
    TEST_ASSERT_UINT16_WITHIN(2, 259, PS9530_Ctrl::interpolateDACCurrent(307));
    TEST_ASSERT_UINT16_WITHIN(2, 362, PS9530_Ctrl::interpolateDACCurrent(402));
    TEST_ASSERT_UINT16_WITHIN(2, 425, PS9530_Ctrl::interpolateDACCurrent(460));
    TEST_ASSERT_UINT16_WITHIN(2, 456, PS9530_Ctrl::interpolateDACCurrent(489));
    TEST_ASSERT_UINT16_WITHIN(2, 1322, PS9530_Ctrl::interpolateDACCurrent(807));
    TEST_ASSERT_UINT16_WITHIN(2, 2995, PS9530_Ctrl::interpolateDACCurrent(1948));
    TEST_ASSERT_UINT16_WITHIN(2, 4841, PS9530_Ctrl::interpolateDACCurrent(3209));
    TEST_ASSERT_UINT16_WITHIN(2, 9121, PS9530_Ctrl::interpolateDACCurrent(6126));
    TEST_ASSERT_UINT16_WITHIN(2, 9125, PS9530_Ctrl::interpolateDACCurrent(6129));
    TEST_ASSERT_UINT16_WITHIN(2, 10298, PS9530_Ctrl::interpolateDACCurrent(6935));
    TEST_ASSERT_UINT16_WITHIN(2, 11232, PS9530_Ctrl::interpolateDACCurrent(7579));
    TEST_ASSERT_UINT16_WITHIN(2, 11344, PS9530_Ctrl::interpolateDACCurrent(7656));
    TEST_ASSERT_UINT16_WITHIN(2, 11942, PS9530_Ctrl::interpolateDACCurrent(8069));
    TEST_ASSERT_UINT16_WITHIN(2, 14044, PS9530_Ctrl::interpolateDACCurrent(9514));

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

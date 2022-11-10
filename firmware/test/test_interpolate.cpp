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
    TEST_ASSERT_UINT16_WITHIN(2, 55, PS9530_Ctrl::interpolateADCVoltage(5));
    TEST_ASSERT_UINT16_WITHIN(2, 262, PS9530_Ctrl::interpolateADCVoltage(24));
    TEST_ASSERT_UINT16_WITHIN(2, 400, PS9530_Ctrl::interpolateADCVoltage(45));
    TEST_ASSERT_UINT16_WITHIN(2, 403, PS9530_Ctrl::interpolateADCVoltage(46));
    TEST_ASSERT_UINT16_WITHIN(2, 575, PS9530_Ctrl::interpolateADCVoltage(89));
    TEST_ASSERT_UINT16_WITHIN(2, 621, PS9530_Ctrl::interpolateADCVoltage(100));
    TEST_ASSERT_UINT16_WITHIN(2, 696, PS9530_Ctrl::interpolateADCVoltage(118));
    TEST_ASSERT_UINT16_WITHIN(2, 767, PS9530_Ctrl::interpolateADCVoltage(135));
    TEST_ASSERT_UINT16_WITHIN(2, 1036, PS9530_Ctrl::interpolateADCVoltage(200));
    TEST_ASSERT_UINT16_WITHIN(2, 1144, PS9530_Ctrl::interpolateADCVoltage(227));
    TEST_ASSERT_UINT16_WITHIN(2, 1273, PS9530_Ctrl::interpolateADCVoltage(257));
    TEST_ASSERT_UINT16_WITHIN(2, 4137, PS9530_Ctrl::interpolateADCVoltage(949));
    TEST_ASSERT_UINT16_WITHIN(2, 5416, PS9530_Ctrl::interpolateADCVoltage(1259));
    TEST_ASSERT_UINT16_WITHIN(2, 7919, PS9530_Ctrl::interpolateADCVoltage(1867));
    TEST_ASSERT_UINT16_WITHIN(2, 14597, PS9530_Ctrl::interpolateADCVoltage(3482));
    TEST_ASSERT_UINT16_WITHIN(2, 15990, PS9530_Ctrl::interpolateADCVoltage(3822));
    TEST_ASSERT_UINT16_WITHIN(2, 18005, PS9530_Ctrl::interpolateADCVoltage(4309));
    TEST_ASSERT_UINT16_WITHIN(2, 18181, PS9530_Ctrl::interpolateADCVoltage(4352));
    TEST_ASSERT_UINT16_WITHIN(2, 21352, PS9530_Ctrl::interpolateADCVoltage(5123));
    TEST_ASSERT_UINT16_WITHIN(2, 29551, PS9530_Ctrl::interpolateADCVoltage(7103));

}

void test_adc_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 174, PS9530_Ctrl::interpolateADCCurrent(5));
    TEST_ASSERT_UINT16_WITHIN(2, 198, PS9530_Ctrl::interpolateADCCurrent(24));
    TEST_ASSERT_UINT16_WITHIN(2, 222, PS9530_Ctrl::interpolateADCCurrent(45));
    TEST_ASSERT_UINT16_WITHIN(2, 223, PS9530_Ctrl::interpolateADCCurrent(46));
    TEST_ASSERT_UINT16_WITHIN(2, 272, PS9530_Ctrl::interpolateADCCurrent(89));
    TEST_ASSERT_UINT16_WITHIN(2, 285, PS9530_Ctrl::interpolateADCCurrent(100));
    TEST_ASSERT_UINT16_WITHIN(2, 306, PS9530_Ctrl::interpolateADCCurrent(118));
    TEST_ASSERT_UINT16_WITHIN(2, 327, PS9530_Ctrl::interpolateADCCurrent(135));
    TEST_ASSERT_UINT16_WITHIN(2, 404, PS9530_Ctrl::interpolateADCCurrent(200));
    TEST_ASSERT_UINT16_WITHIN(2, 437, PS9530_Ctrl::interpolateADCCurrent(227));
    TEST_ASSERT_UINT16_WITHIN(2, 473, PS9530_Ctrl::interpolateADCCurrent(257));
    TEST_ASSERT_UINT16_WITHIN(2, 1314, PS9530_Ctrl::interpolateADCCurrent(949));
    TEST_ASSERT_UINT16_WITHIN(2, 1685, PS9530_Ctrl::interpolateADCCurrent(1259));
    TEST_ASSERT_UINT16_WITHIN(2, 2395, PS9530_Ctrl::interpolateADCCurrent(1867));
    TEST_ASSERT_UINT16_WITHIN(2, 4323, PS9530_Ctrl::interpolateADCCurrent(3482));
    TEST_ASSERT_UINT16_WITHIN(2, 4731, PS9530_Ctrl::interpolateADCCurrent(3822));
    TEST_ASSERT_UINT16_WITHIN(2, 5316, PS9530_Ctrl::interpolateADCCurrent(4309));
    TEST_ASSERT_UINT16_WITHIN(2, 5368, PS9530_Ctrl::interpolateADCCurrent(4352));
    TEST_ASSERT_UINT16_WITHIN(2, 6289, PS9530_Ctrl::interpolateADCCurrent(5123));
    TEST_ASSERT_UINT16_WITHIN(2, 8677, PS9530_Ctrl::interpolateADCCurrent(7103));

}

void test_dac_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACVoltage(42));
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACVoltage(59));
    TEST_ASSERT_UINT16_WITHIN(2, 18, PS9530_Ctrl::interpolateDACVoltage(162));
    TEST_ASSERT_UINT16_WITHIN(2, 46, PS9530_Ctrl::interpolateDACVoltage(215));
    TEST_ASSERT_UINT16_WITHIN(2, 78, PS9530_Ctrl::interpolateDACVoltage(275));
    TEST_ASSERT_UINT16_WITHIN(2, 83, PS9530_Ctrl::interpolateDACVoltage(285));
    TEST_ASSERT_UINT16_WITHIN(2, 226, PS9530_Ctrl::interpolateDACVoltage(545));
    TEST_ASSERT_UINT16_WITHIN(2, 251, PS9530_Ctrl::interpolateDACVoltage(590));
    TEST_ASSERT_UINT16_WITHIN(2, 317, PS9530_Ctrl::interpolateDACVoltage(712));
    TEST_ASSERT_UINT16_WITHIN(2, 379, PS9530_Ctrl::interpolateDACVoltage(828));
    TEST_ASSERT_UINT16_WITHIN(2, 852, PS9530_Ctrl::interpolateDACVoltage(1703));
    TEST_ASSERT_UINT16_WITHIN(2, 4372, PS9530_Ctrl::interpolateDACVoltage(8217));
    TEST_ASSERT_UINT16_WITHIN(2, 4887, PS9530_Ctrl::interpolateDACVoltage(9180));
    TEST_ASSERT_UINT16_WITHIN(2, 5850, PS9530_Ctrl::interpolateDACVoltage(10978));
    TEST_ASSERT_UINT16_WITHIN(2, 7506, PS9530_Ctrl::interpolateDACVoltage(14064));
    TEST_ASSERT_UINT16_WITHIN(2, 9514, PS9530_Ctrl::interpolateDACVoltage(17804));
    TEST_ASSERT_UINT16_WITHIN(2, 10376, PS9530_Ctrl::interpolateDACVoltage(19406));
    TEST_ASSERT_UINT16_WITHIN(2, 12889, PS9530_Ctrl::interpolateDACVoltage(24074));
    TEST_ASSERT_UINT16_WITHIN(2, 13852, PS9530_Ctrl::interpolateDACVoltage(25867));
    TEST_ASSERT_UINT16_WITHIN(2, 13881, PS9530_Ctrl::interpolateDACVoltage(25921));

}

void test_dac_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 0, PS9530_Ctrl::interpolateDACCurrent(16));
    TEST_ASSERT_UINT16_WITHIN(2, 56, PS9530_Ctrl::interpolateDACCurrent(117));
    TEST_ASSERT_UINT16_WITHIN(2, 71, PS9530_Ctrl::interpolateDACCurrent(131));
    TEST_ASSERT_UINT16_WITHIN(2, 156, PS9530_Ctrl::interpolateDACCurrent(211));
    TEST_ASSERT_UINT16_WITHIN(2, 159, PS9530_Ctrl::interpolateDACCurrent(214));
    TEST_ASSERT_UINT16_WITHIN(2, 194, PS9530_Ctrl::interpolateDACCurrent(244));
    TEST_ASSERT_UINT16_WITHIN(2, 205, PS9530_Ctrl::interpolateDACCurrent(253));
    TEST_ASSERT_UINT16_WITHIN(2, 252, PS9530_Ctrl::interpolateDACCurrent(296));
    TEST_ASSERT_UINT16_WITHIN(2, 348, PS9530_Ctrl::interpolateDACCurrent(385));
    TEST_ASSERT_UINT16_WITHIN(2, 466, PS9530_Ctrl::interpolateDACCurrent(494));
    TEST_ASSERT_UINT16_WITHIN(2, 2348, PS9530_Ctrl::interpolateDACCurrent(1524));
    TEST_ASSERT_UINT16_WITHIN(2, 3135, PS9530_Ctrl::interpolateDACCurrent(2045));
    TEST_ASSERT_UINT16_WITHIN(2, 3160, PS9530_Ctrl::interpolateDACCurrent(2062));
    TEST_ASSERT_UINT16_WITHIN(2, 4350, PS9530_Ctrl::interpolateDACCurrent(2884));
    TEST_ASSERT_UINT16_WITHIN(2, 8434, PS9530_Ctrl::interpolateDACCurrent(5679));
    TEST_ASSERT_UINT16_WITHIN(2, 9861, PS9530_Ctrl::interpolateDACCurrent(6655));
    TEST_ASSERT_UINT16_WITHIN(2, 10160, PS9530_Ctrl::interpolateDACCurrent(6861));
    TEST_ASSERT_UINT16_WITHIN(2, 11234, PS9530_Ctrl::interpolateDACCurrent(7601));
    TEST_ASSERT_UINT16_WITHIN(2, 12578, PS9530_Ctrl::interpolateDACCurrent(8528));
    TEST_ASSERT_UINT16_WITHIN(2, 12777, PS9530_Ctrl::interpolateDACCurrent(8665));

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

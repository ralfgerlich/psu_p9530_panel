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
    TEST_ASSERT_UINT16_WITHIN(2, 143, PS9530_Ctrl::interpolateADCVoltage(5));
    TEST_ASSERT_UINT16_WITHIN(2, 389, PS9530_Ctrl::interpolateADCVoltage(40));
    TEST_ASSERT_UINT16_WITHIN(2, 398, PS9530_Ctrl::interpolateADCVoltage(42));
    TEST_ASSERT_UINT16_WITHIN(2, 474, PS9530_Ctrl::interpolateADCVoltage(59));
    TEST_ASSERT_UINT16_WITHIN(2, 510, PS9530_Ctrl::interpolateADCVoltage(67));
    TEST_ASSERT_UINT16_WITHIN(2, 789, PS9530_Ctrl::interpolateADCVoltage(136));
    TEST_ASSERT_UINT16_WITHIN(2, 828, PS9530_Ctrl::interpolateADCVoltage(145));
    TEST_ASSERT_UINT16_WITHIN(2, 962, PS9530_Ctrl::interpolateADCVoltage(176));
    TEST_ASSERT_UINT16_WITHIN(2, 1055, PS9530_Ctrl::interpolateADCVoltage(198));
    TEST_ASSERT_UINT16_WITHIN(2, 1137, PS9530_Ctrl::interpolateADCVoltage(219));
    TEST_ASSERT_UINT16_WITHIN(2, 14382, PS9530_Ctrl::interpolateADCVoltage(3427));
    TEST_ASSERT_UINT16_WITHIN(2, 17049, PS9530_Ctrl::interpolateADCVoltage(4072));
    TEST_ASSERT_UINT16_WITHIN(2, 19394, PS9530_Ctrl::interpolateADCVoltage(4645));
    TEST_ASSERT_UINT16_WITHIN(2, 23098, PS9530_Ctrl::interpolateADCVoltage(5537));
    TEST_ASSERT_UINT16_WITHIN(2, 25051, PS9530_Ctrl::interpolateADCVoltage(6010));
    TEST_ASSERT_UINT16_WITHIN(2, 27421, PS9530_Ctrl::interpolateADCVoltage(6577));
    TEST_ASSERT_UINT16_WITHIN(2, 29240, PS9530_Ctrl::interpolateADCVoltage(7017));
    TEST_ASSERT_UINT16_WITHIN(2, 30110, PS9530_Ctrl::interpolateADCVoltage(7232));
    TEST_ASSERT_UINT16_WITHIN(2, 30920, PS9530_Ctrl::interpolateADCVoltage(7458));
    TEST_ASSERT_UINT16_WITHIN(2, 31797, PS9530_Ctrl::interpolateADCVoltage(7703));

}

void test_adc_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 11, PS9530_Ctrl::interpolateADCCurrent(5));
    TEST_ASSERT_UINT16_WITHIN(2, 91, PS9530_Ctrl::interpolateADCCurrent(40));
    TEST_ASSERT_UINT16_WITHIN(2, 96, PS9530_Ctrl::interpolateADCCurrent(42));
    TEST_ASSERT_UINT16_WITHIN(2, 134, PS9530_Ctrl::interpolateADCCurrent(59));
    TEST_ASSERT_UINT16_WITHIN(2, 151, PS9530_Ctrl::interpolateADCCurrent(67));
    TEST_ASSERT_UINT16_WITHIN(2, 256, PS9530_Ctrl::interpolateADCCurrent(136));
    TEST_ASSERT_UINT16_WITHIN(2, 267, PS9530_Ctrl::interpolateADCCurrent(145));
    TEST_ASSERT_UINT16_WITHIN(2, 303, PS9530_Ctrl::interpolateADCCurrent(176));
    TEST_ASSERT_UINT16_WITHIN(2, 329, PS9530_Ctrl::interpolateADCCurrent(198));
    TEST_ASSERT_UINT16_WITHIN(2, 353, PS9530_Ctrl::interpolateADCCurrent(219));
    TEST_ASSERT_UINT16_WITHIN(2, 4192, PS9530_Ctrl::interpolateADCCurrent(3427));
    TEST_ASSERT_UINT16_WITHIN(2, 4958, PS9530_Ctrl::interpolateADCCurrent(4072));
    TEST_ASSERT_UINT16_WITHIN(2, 5655, PS9530_Ctrl::interpolateADCCurrent(4645));
    TEST_ASSERT_UINT16_WITHIN(2, 6718, PS9530_Ctrl::interpolateADCCurrent(5537));
    TEST_ASSERT_UINT16_WITHIN(2, 7290, PS9530_Ctrl::interpolateADCCurrent(6010));
    TEST_ASSERT_UINT16_WITHIN(2, 7980, PS9530_Ctrl::interpolateADCCurrent(6577));
    TEST_ASSERT_UINT16_WITHIN(2, 8507, PS9530_Ctrl::interpolateADCCurrent(7017));
    TEST_ASSERT_UINT16_WITHIN(2, 8767, PS9530_Ctrl::interpolateADCCurrent(7232));
    TEST_ASSERT_UINT16_WITHIN(2, 9042, PS9530_Ctrl::interpolateADCCurrent(7458));
    TEST_ASSERT_UINT16_WITHIN(2, 9342, PS9530_Ctrl::interpolateADCCurrent(7703));

}

void test_dac_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 20, PS9530_Ctrl::interpolateDACVoltage(173));
    TEST_ASSERT_UINT16_WITHIN(2, 49, PS9530_Ctrl::interpolateDACVoltage(239));
    TEST_ASSERT_UINT16_WITHIN(2, 115, PS9530_Ctrl::interpolateDACVoltage(353));
    TEST_ASSERT_UINT16_WITHIN(2, 228, PS9530_Ctrl::interpolateDACVoltage(558));
    TEST_ASSERT_UINT16_WITHIN(2, 231, PS9530_Ctrl::interpolateDACVoltage(563));
    TEST_ASSERT_UINT16_WITHIN(2, 256, PS9530_Ctrl::interpolateDACVoltage(608));
    TEST_ASSERT_UINT16_WITHIN(2, 324, PS9530_Ctrl::interpolateDACVoltage(734));
    TEST_ASSERT_UINT16_WITHIN(2, 373, PS9530_Ctrl::interpolateDACVoltage(824));
    TEST_ASSERT_UINT16_WITHIN(2, 383, PS9530_Ctrl::interpolateDACVoltage(842));
    TEST_ASSERT_UINT16_WITHIN(2, 464, PS9530_Ctrl::interpolateDACVoltage(993));
    TEST_ASSERT_UINT16_WITHIN(2, 601, PS9530_Ctrl::interpolateDACVoltage(1246));
    TEST_ASSERT_UINT16_WITHIN(2, 3882, PS9530_Ctrl::interpolateDACVoltage(7321));
    TEST_ASSERT_UINT16_WITHIN(2, 6468, PS9530_Ctrl::interpolateDACVoltage(12158));
    TEST_ASSERT_UINT16_WITHIN(2, 8259, PS9530_Ctrl::interpolateDACVoltage(15484));
    TEST_ASSERT_UINT16_WITHIN(2, 9860, PS9530_Ctrl::interpolateDACVoltage(18443));
    TEST_ASSERT_UINT16_WITHIN(2, 10496, PS9530_Ctrl::interpolateDACVoltage(19622));
    TEST_ASSERT_UINT16_WITHIN(2, 10882, PS9530_Ctrl::interpolateDACVoltage(20341));
    TEST_ASSERT_UINT16_WITHIN(2, 13784, PS9530_Ctrl::interpolateDACVoltage(25739));
    TEST_ASSERT_UINT16_WITHIN(2, 14651, PS9530_Ctrl::interpolateDACVoltage(27357));
    TEST_ASSERT_UINT16_WITHIN(2, 15923, PS9530_Ctrl::interpolateDACVoltage(29732));

}

void test_dac_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 89, PS9530_Ctrl::interpolateDACCurrent(155));
    TEST_ASSERT_UINT16_WITHIN(2, 95, PS9530_Ctrl::interpolateDACCurrent(160));
    TEST_ASSERT_UINT16_WITHIN(2, 198, PS9530_Ctrl::interpolateDACCurrent(251));
    TEST_ASSERT_UINT16_WITHIN(2, 207, PS9530_Ctrl::interpolateDACCurrent(260));
    TEST_ASSERT_UINT16_WITHIN(2, 223, PS9530_Ctrl::interpolateDACCurrent(274));
    TEST_ASSERT_UINT16_WITHIN(2, 283, PS9530_Ctrl::interpolateDACCurrent(329));
    TEST_ASSERT_UINT16_WITHIN(2, 335, PS9530_Ctrl::interpolateDACCurrent(377));
    TEST_ASSERT_UINT16_WITHIN(2, 356, PS9530_Ctrl::interpolateDACCurrent(396));
    TEST_ASSERT_UINT16_WITHIN(2, 423, PS9530_Ctrl::interpolateDACCurrent(458));
    TEST_ASSERT_UINT16_WITHIN(2, 428, PS9530_Ctrl::interpolateDACCurrent(463));
    TEST_ASSERT_UINT16_WITHIN(2, 3826, PS9530_Ctrl::interpolateDACCurrent(2515));
    TEST_ASSERT_UINT16_WITHIN(2, 5352, PS9530_Ctrl::interpolateDACCurrent(3558));
    TEST_ASSERT_UINT16_WITHIN(2, 8026, PS9530_Ctrl::interpolateDACCurrent(5374));
    TEST_ASSERT_UINT16_WITHIN(2, 9492, PS9530_Ctrl::interpolateDACCurrent(6380));
    TEST_ASSERT_UINT16_WITHIN(2, 9574, PS9530_Ctrl::interpolateDACCurrent(6436));
    TEST_ASSERT_UINT16_WITHIN(2, 9837, PS9530_Ctrl::interpolateDACCurrent(6616));
    TEST_ASSERT_UINT16_WITHIN(2, 11887, PS9530_Ctrl::interpolateDACCurrent(8031));
    TEST_ASSERT_UINT16_WITHIN(2, 12637, PS9530_Ctrl::interpolateDACCurrent(8540));
    TEST_ASSERT_UINT16_WITHIN(2, 13758, PS9530_Ctrl::interpolateDACCurrent(9315));
    TEST_ASSERT_UINT16_WITHIN(2, 14663, PS9530_Ctrl::interpolateDACCurrent(9940));

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

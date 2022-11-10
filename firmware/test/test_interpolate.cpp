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
    TEST_ASSERT_UINT16_WITHIN(2, 131, PS9530_Ctrl::interpolateADCVoltage(12));
    TEST_ASSERT_UINT16_WITHIN(2, 219, PS9530_Ctrl::interpolateADCVoltage(20));
    TEST_ASSERT_UINT16_WITHIN(2, 461, PS9530_Ctrl::interpolateADCVoltage(61));
    TEST_ASSERT_UINT16_WITHIN(2, 509, PS9530_Ctrl::interpolateADCVoltage(73));
    TEST_ASSERT_UINT16_WITHIN(2, 563, PS9530_Ctrl::interpolateADCVoltage(86));
    TEST_ASSERT_UINT16_WITHIN(2, 709, PS9530_Ctrl::interpolateADCVoltage(121));
    TEST_ASSERT_UINT16_WITHIN(2, 1187, PS9530_Ctrl::interpolateADCVoltage(237));
    TEST_ASSERT_UINT16_WITHIN(2, 1217, PS9530_Ctrl::interpolateADCVoltage(244));
    TEST_ASSERT_UINT16_WITHIN(2, 1217, PS9530_Ctrl::interpolateADCVoltage(244));
    TEST_ASSERT_UINT16_WITHIN(2, 1234, PS9530_Ctrl::interpolateADCVoltage(248));
    TEST_ASSERT_UINT16_WITHIN(2, 5156, PS9530_Ctrl::interpolateADCVoltage(1196));
    TEST_ASSERT_UINT16_WITHIN(2, 6391, PS9530_Ctrl::interpolateADCVoltage(1496));
    TEST_ASSERT_UINT16_WITHIN(2, 6515, PS9530_Ctrl::interpolateADCVoltage(1526));
    TEST_ASSERT_UINT16_WITHIN(2, 11322, PS9530_Ctrl::interpolateADCVoltage(2691));
    TEST_ASSERT_UINT16_WITHIN(2, 21381, PS9530_Ctrl::interpolateADCVoltage(5130));
    TEST_ASSERT_UINT16_WITHIN(2, 22854, PS9530_Ctrl::interpolateADCVoltage(5488));
    TEST_ASSERT_UINT16_WITHIN(2, 24246, PS9530_Ctrl::interpolateADCVoltage(5824));
    TEST_ASSERT_UINT16_WITHIN(2, 33368, PS9530_Ctrl::interpolateADCVoltage(7901));
    TEST_ASSERT_UINT16_WITHIN(2, 33677, PS9530_Ctrl::interpolateADCVoltage(7965));
    TEST_ASSERT_UINT16_WITHIN(2, 33740, PS9530_Ctrl::interpolateADCVoltage(7978));

}

void test_adc_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 183, PS9530_Ctrl::interpolateADCCurrent(12));
    TEST_ASSERT_UINT16_WITHIN(2, 193, PS9530_Ctrl::interpolateADCCurrent(20));
    TEST_ASSERT_UINT16_WITHIN(2, 239, PS9530_Ctrl::interpolateADCCurrent(61));
    TEST_ASSERT_UINT16_WITHIN(2, 253, PS9530_Ctrl::interpolateADCCurrent(73));
    TEST_ASSERT_UINT16_WITHIN(2, 268, PS9530_Ctrl::interpolateADCCurrent(86));
    TEST_ASSERT_UINT16_WITHIN(2, 310, PS9530_Ctrl::interpolateADCCurrent(121));
    TEST_ASSERT_UINT16_WITHIN(2, 449, PS9530_Ctrl::interpolateADCCurrent(237));
    TEST_ASSERT_UINT16_WITHIN(2, 457, PS9530_Ctrl::interpolateADCCurrent(244));
    TEST_ASSERT_UINT16_WITHIN(2, 457, PS9530_Ctrl::interpolateADCCurrent(244));
    TEST_ASSERT_UINT16_WITHIN(2, 462, PS9530_Ctrl::interpolateADCCurrent(248));
    TEST_ASSERT_UINT16_WITHIN(2, 1610, PS9530_Ctrl::interpolateADCCurrent(1196));
    TEST_ASSERT_UINT16_WITHIN(2, 1952, PS9530_Ctrl::interpolateADCCurrent(1496));
    TEST_ASSERT_UINT16_WITHIN(2, 1986, PS9530_Ctrl::interpolateADCCurrent(1526));
    TEST_ASSERT_UINT16_WITHIN(2, 3377, PS9530_Ctrl::interpolateADCCurrent(2691));
    TEST_ASSERT_UINT16_WITHIN(2, 6297, PS9530_Ctrl::interpolateADCCurrent(5130));
    TEST_ASSERT_UINT16_WITHIN(2, 6726, PS9530_Ctrl::interpolateADCCurrent(5488));
    TEST_ASSERT_UINT16_WITHIN(2, 7130, PS9530_Ctrl::interpolateADCCurrent(5824));
    TEST_ASSERT_UINT16_WITHIN(2, 9606, PS9530_Ctrl::interpolateADCCurrent(7901));
    TEST_ASSERT_UINT16_WITHIN(2, 9692, PS9530_Ctrl::interpolateADCCurrent(7965));
    TEST_ASSERT_UINT16_WITHIN(2, 9711, PS9530_Ctrl::interpolateADCCurrent(7978));

}

void test_dac_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 1, PS9530_Ctrl::interpolateDACVoltage(130));
    TEST_ASSERT_UINT16_WITHIN(2, 11, PS9530_Ctrl::interpolateDACVoltage(149));
    TEST_ASSERT_UINT16_WITHIN(2, 89, PS9530_Ctrl::interpolateDACVoltage(296));
    TEST_ASSERT_UINT16_WITHIN(2, 237, PS9530_Ctrl::interpolateDACVoltage(565));
    TEST_ASSERT_UINT16_WITHIN(2, 303, PS9530_Ctrl::interpolateDACVoltage(687));
    TEST_ASSERT_UINT16_WITHIN(2, 322, PS9530_Ctrl::interpolateDACVoltage(722));
    TEST_ASSERT_UINT16_WITHIN(2, 324, PS9530_Ctrl::interpolateDACVoltage(726));
    TEST_ASSERT_UINT16_WITHIN(2, 368, PS9530_Ctrl::interpolateDACVoltage(807));
    TEST_ASSERT_UINT16_WITHIN(2, 432, PS9530_Ctrl::interpolateDACVoltage(926));
    TEST_ASSERT_UINT16_WITHIN(2, 468, PS9530_Ctrl::interpolateDACVoltage(993));
    TEST_ASSERT_UINT16_WITHIN(2, 900, PS9530_Ctrl::interpolateDACVoltage(1792));
    TEST_ASSERT_UINT16_WITHIN(2, 2055, PS9530_Ctrl::interpolateDACVoltage(3924));
    TEST_ASSERT_UINT16_WITHIN(2, 3074, PS9530_Ctrl::interpolateDACVoltage(5809));
    TEST_ASSERT_UINT16_WITHIN(2, 3923, PS9530_Ctrl::interpolateDACVoltage(7315));
    TEST_ASSERT_UINT16_WITHIN(2, 4699, PS9530_Ctrl::interpolateDACVoltage(8828));
    TEST_ASSERT_UINT16_WITHIN(2, 5477, PS9530_Ctrl::interpolateDACVoltage(10281));
    TEST_ASSERT_UINT16_WITHIN(2, 6113, PS9530_Ctrl::interpolateDACVoltage(11469));
    TEST_ASSERT_UINT16_WITHIN(2, 7854, PS9530_Ctrl::interpolateDACVoltage(14711));
    TEST_ASSERT_UINT16_WITHIN(2, 14614, PS9530_Ctrl::interpolateDACVoltage(27290));
    TEST_ASSERT_UINT16_WITHIN(2, 14823, PS9530_Ctrl::interpolateDACVoltage(27679));

}

void test_dac_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 52, PS9530_Ctrl::interpolateDACCurrent(113));
    TEST_ASSERT_UINT16_WITHIN(2, 151, PS9530_Ctrl::interpolateDACCurrent(207));
    TEST_ASSERT_UINT16_WITHIN(2, 156, PS9530_Ctrl::interpolateDACCurrent(211));
    TEST_ASSERT_UINT16_WITHIN(2, 231, PS9530_Ctrl::interpolateDACCurrent(277));
    TEST_ASSERT_UINT16_WITHIN(2, 275, PS9530_Ctrl::interpolateDACCurrent(317));
    TEST_ASSERT_UINT16_WITHIN(2, 393, PS9530_Ctrl::interpolateDACCurrent(427));
    TEST_ASSERT_UINT16_WITHIN(2, 403, PS9530_Ctrl::interpolateDACCurrent(436));
    TEST_ASSERT_UINT16_WITHIN(2, 414, PS9530_Ctrl::interpolateDACCurrent(446));
    TEST_ASSERT_UINT16_WITHIN(2, 417, PS9530_Ctrl::interpolateDACCurrent(449));
    TEST_ASSERT_UINT16_WITHIN(2, 460, PS9530_Ctrl::interpolateDACCurrent(489));
    TEST_ASSERT_UINT16_WITHIN(2, 4117, PS9530_Ctrl::interpolateDACCurrent(2725));
    TEST_ASSERT_UINT16_WITHIN(2, 4347, PS9530_Ctrl::interpolateDACCurrent(2882));
    TEST_ASSERT_UINT16_WITHIN(2, 6261, PS9530_Ctrl::interpolateDACCurrent(4189));
    TEST_ASSERT_UINT16_WITHIN(2, 8666, PS9530_Ctrl::interpolateDACCurrent(5838));
    TEST_ASSERT_UINT16_WITHIN(2, 8731, PS9530_Ctrl::interpolateDACCurrent(5882));
    TEST_ASSERT_UINT16_WITHIN(2, 8745, PS9530_Ctrl::interpolateDACCurrent(5892));
    TEST_ASSERT_UINT16_WITHIN(2, 12451, PS9530_Ctrl::interpolateDACCurrent(8441));
    TEST_ASSERT_UINT16_WITHIN(2, 12958, PS9530_Ctrl::interpolateDACCurrent(8782));
    TEST_ASSERT_UINT16_WITHIN(2, 13605, PS9530_Ctrl::interpolateDACCurrent(9189));
    TEST_ASSERT_UINT16_WITHIN(2, 14576, PS9530_Ctrl::interpolateDACCurrent(9862));

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

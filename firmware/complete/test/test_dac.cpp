#include <unity.h>

#define private public
#define protected public

#include "ps9530_ctrl.h"

void test_dac_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 656, PS9530_Ctrl::interpolateDACVoltage(1347));
    TEST_ASSERT_UINT16_WITHIN(2, 7166, PS9530_Ctrl::interpolateDACVoltage(13435));
    TEST_ASSERT_UINT16_WITHIN(2, 8478, PS9530_Ctrl::interpolateDACVoltage(15870));
    TEST_ASSERT_UINT16_WITHIN(2, 9083, PS9530_Ctrl::interpolateDACVoltage(16988));
    TEST_ASSERT_UINT16_WITHIN(2, 9333, PS9530_Ctrl::interpolateDACVoltage(17449));
    TEST_ASSERT_UINT16_WITHIN(2, 10447, PS9530_Ctrl::interpolateDACVoltage(19494));
    TEST_ASSERT_UINT16_WITHIN(2, 11564, PS9530_Ctrl::interpolateDACVoltage(21571));
    TEST_ASSERT_UINT16_WITHIN(2, 11790, PS9530_Ctrl::interpolateDACVoltage(21987));
    TEST_ASSERT_UINT16_WITHIN(2, 12824, PS9530_Ctrl::interpolateDACVoltage(23884));
    TEST_ASSERT_UINT16_WITHIN(2, 15378, PS9530_Ctrl::interpolateDACVoltage(28670));
}

void test_dac_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 387, PS9530_Ctrl::interpolateDACCurrent(222));
    TEST_ASSERT_UINT16_WITHIN(2, 3212, PS9530_Ctrl::interpolateDACCurrent(2096));
    TEST_ASSERT_UINT16_WITHIN(2, 5927, PS9530_Ctrl::interpolateDACCurrent(3951));
    TEST_ASSERT_UINT16_WITHIN(2, 7464, PS9530_Ctrl::interpolateDACCurrent(4992));
    TEST_ASSERT_UINT16_WITHIN(2, 7516, PS9530_Ctrl::interpolateDACCurrent(5028));
    TEST_ASSERT_UINT16_WITHIN(2, 8240, PS9530_Ctrl::interpolateDACCurrent(5525));
    TEST_ASSERT_UINT16_WITHIN(2, 11014, PS9530_Ctrl::interpolateDACCurrent(7418));
    TEST_ASSERT_UINT16_WITHIN(2, 11865, PS9530_Ctrl::interpolateDACCurrent(7996));
    TEST_ASSERT_UINT16_WITHIN(2, 14178, PS9530_Ctrl::interpolateDACCurrent(9573));
    TEST_ASSERT_UINT16_WITHIN(2, 14698, PS9530_Ctrl::interpolateDACCurrent(9924));
}

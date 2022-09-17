#include <unity.h>

#define private public
#define protected public

#include "ps9530_ctrl.h"

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

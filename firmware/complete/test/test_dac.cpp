#include <unity.h>

#define private public
#define protected public

#include "ps9530_ctrl.h"

void test_dac_voltage() {
    TEST_ASSERT_UINT16_WITHIN(2, 395, PS9530_Ctrl::interpolateDACVoltage(840));
    TEST_ASSERT_UINT16_WITHIN(2, 410, PS9530_Ctrl::interpolateDACVoltage(872));
    TEST_ASSERT_UINT16_WITHIN(2, 589, PS9530_Ctrl::interpolateDACVoltage(1224));
    TEST_ASSERT_UINT16_WITHIN(2, 3636, PS9530_Ctrl::interpolateDACVoltage(6860));
    TEST_ASSERT_UINT16_WITHIN(2, 8310, PS9530_Ctrl::interpolateDACVoltage(15578));
    TEST_ASSERT_UINT16_WITHIN(2, 9322, PS9530_Ctrl::interpolateDACVoltage(17450));
    TEST_ASSERT_UINT16_WITHIN(2, 9399, PS9530_Ctrl::interpolateDACVoltage(17592));
    TEST_ASSERT_UINT16_WITHIN(2, 12563, PS9530_Ctrl::interpolateDACVoltage(23464));
    TEST_ASSERT_UINT16_WITHIN(2, 14397, PS9530_Ctrl::interpolateDACVoltage(26883));
    TEST_ASSERT_UINT16_WITHIN(2, 15538, PS9530_Ctrl::interpolateDACVoltage(29010));
}

void test_dac_current() {
    TEST_ASSERT_UINT16_WITHIN(2, 1584, PS9530_Ctrl::interpolateDACCurrent(986));
    TEST_ASSERT_UINT16_WITHIN(2, 2074, PS9530_Ctrl::interpolateDACCurrent(1319));
    TEST_ASSERT_UINT16_WITHIN(2, 2089, PS9530_Ctrl::interpolateDACCurrent(1329));
    TEST_ASSERT_UINT16_WITHIN(2, 3748, PS9530_Ctrl::interpolateDACCurrent(2462));
    TEST_ASSERT_UINT16_WITHIN(2, 4573, PS9530_Ctrl::interpolateDACCurrent(3026));
    TEST_ASSERT_UINT16_WITHIN(2, 4819, PS9530_Ctrl::interpolateDACCurrent(3194));
    TEST_ASSERT_UINT16_WITHIN(2, 5110, PS9530_Ctrl::interpolateDACCurrent(3393));
    TEST_ASSERT_UINT16_WITHIN(2, 7237, PS9530_Ctrl::interpolateDACCurrent(4840));
    TEST_ASSERT_UINT16_WITHIN(2, 10174, PS9530_Ctrl::interpolateDACCurrent(6849));
    TEST_ASSERT_UINT16_WITHIN(2, 13574, PS9530_Ctrl::interpolateDACCurrent(9187));
}

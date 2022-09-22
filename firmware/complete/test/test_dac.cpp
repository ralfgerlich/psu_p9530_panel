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
    TEST_ASSERT_UINT16_WITHIN(2, 1986, PS9530_Ctrl::interpolateDACCurrent(1270));
    TEST_ASSERT_UINT16_WITHIN(2, 2399, PS9530_Ctrl::interpolateDACCurrent(1553));
    TEST_ASSERT_UINT16_WITHIN(2, 3815, PS9530_Ctrl::interpolateDACCurrent(2519));
    TEST_ASSERT_UINT16_WITHIN(2, 4795, PS9530_Ctrl::interpolateDACCurrent(3189));
    TEST_ASSERT_UINT16_WITHIN(2, 6645, PS9530_Ctrl::interpolateDACCurrent(4453));
    TEST_ASSERT_UINT16_WITHIN(2, 9256, PS9530_Ctrl::interpolateDACCurrent(6228));
    TEST_ASSERT_UINT16_WITHIN(2, 9984, PS9530_Ctrl::interpolateDACCurrent(6726));
    TEST_ASSERT_UINT16_WITHIN(2, 10385, PS9530_Ctrl::interpolateDACCurrent(7000));
    TEST_ASSERT_UINT16_WITHIN(2, 10874, PS9530_Ctrl::interpolateDACCurrent(7337));
    TEST_ASSERT_UINT16_WITHIN(2, 14500, PS9530_Ctrl::interpolateDACCurrent(9801));
}

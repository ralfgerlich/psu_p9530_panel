#include <unity.h>

#define private public
#define protected public

#include "ps9530_ui.h"
#include "ps_display.h"

/* We're pulling in the ui from the actual application.
 * This is bad practice, but we get redefined-symbol-errors
 * because the main application is automatically pulled in.
 */
extern PS9530_UI ui;
extern PsDisplay display;

void test_limits_power() {
    ui.setVoltageSetpointMilliVolts(30000);
    ui.setCurrentLimitMilliAmps(10000);
    ui.setPowerLimitCentiWatt(15000);
    TEST_ASSERT_EQUAL(30000, ui.voltageSetpointMilliVolts);
    TEST_ASSERT_EQUAL(15000, ui.powerLimitCentiWatt);
    TEST_ASSERT_EQUAL(PS9530_Ctrl::interpolateDACVoltage(ui.voltageSetpointMilliVolts),
                      PS9530_Ctrl::getInstance().rawDACValue[PS9530_Ctrl::muxChannel_voltage]);
    TEST_ASSERT_EQUAL(PS9530_Ctrl::interpolateDACCurrent(5000),
                      PS9530_Ctrl::getInstance().rawDACValue[PS9530_Ctrl::muxChannel_current]);
    TEST_ASSERT_EQUAL(PS9530_Ctrl::CurrentLimitFromPower, PS9530_Ctrl::getInstance().currentLimitReason);
    PS9530_Ctrl::getInstance().standbyMode = true;
    TEST_ASSERT_TRUE(PS9530_Ctrl::getInstance().isStandbyEnabled());
    TEST_ASSERT_EQUAL_MESSAGE(PS9530_Ctrl::LimitingMode_Inactive, PS9530_Ctrl::getInstance().getLimitingMode(false), "Inactive limiting mode expected in standby (voltage limiter)");
    TEST_ASSERT_EQUAL_MESSAGE(PS9530_Ctrl::LimitingMode_Inactive, PS9530_Ctrl::getInstance().getLimitingMode(true), "Inactive limiting mode expected in standby (current limiter)");
    PS9530_Ctrl::getInstance().standbyMode = false;
    TEST_ASSERT_FALSE(PS9530_Ctrl::getInstance().isStandbyEnabled());
    TEST_ASSERT_EQUAL_MESSAGE(PS9530_Ctrl::LimitingMode_Voltage, PS9530_Ctrl::getInstance().getLimitingMode(false), "Voltage limiting mode expected");
    TEST_ASSERT_EQUAL_MESSAGE(PS9530_Ctrl::LimitingMode_Power, PS9530_Ctrl::getInstance().getLimitingMode(true), "Power limiting mode expected");
}

void test_limits_current() {
    ui.setVoltageSetpointMilliVolts(30000);
    ui.setCurrentLimitMilliAmps(5000);
    ui.setPowerLimitCentiWatt(30000);
    TEST_ASSERT_EQUAL(30000, ui.voltageSetpointMilliVolts);
    TEST_ASSERT_EQUAL(5000, ui.currentLimitMilliAmps);
    TEST_ASSERT_EQUAL(30000, ui.powerLimitCentiWatt);
    TEST_ASSERT_EQUAL(PS9530_Ctrl::interpolateDACVoltage(ui.voltageSetpointMilliVolts),
                      PS9530_Ctrl::getInstance().rawDACValue[PS9530_Ctrl::muxChannel_voltage]);
    TEST_ASSERT_EQUAL(PS9530_Ctrl::interpolateDACCurrent(ui.currentLimitMilliAmps),
                      PS9530_Ctrl::getInstance().rawDACValue[PS9530_Ctrl::muxChannel_current]);
    TEST_ASSERT_EQUAL(PS9530_Ctrl::CurrentLimitFromCurrent, PS9530_Ctrl::getInstance().currentLimitReason);
    PS9530_Ctrl::getInstance().standbyMode = true;
    TEST_ASSERT_TRUE(PS9530_Ctrl::getInstance().isStandbyEnabled());
    TEST_ASSERT_EQUAL_MESSAGE(PS9530_Ctrl::LimitingMode_Inactive, PS9530_Ctrl::getInstance().getLimitingMode(false), "Inactive limiting mode expected in standby (voltage limiter)");
    TEST_ASSERT_EQUAL_MESSAGE(PS9530_Ctrl::LimitingMode_Inactive, PS9530_Ctrl::getInstance().getLimitingMode(true), "Inactive limiting mode expected in standby (current limiter)");
    PS9530_Ctrl::getInstance().standbyMode = false;
    TEST_ASSERT_FALSE(PS9530_Ctrl::getInstance().isStandbyEnabled());
    TEST_ASSERT_EQUAL_MESSAGE(PS9530_Ctrl::LimitingMode_Voltage, PS9530_Ctrl::getInstance().getLimitingMode(false), "Voltage limiting mode expected");
    TEST_ASSERT_EQUAL_MESSAGE(PS9530_Ctrl::LimitingMode_Current, PS9530_Ctrl::getInstance().getLimitingMode(true), "Current limiting mode expected");
}
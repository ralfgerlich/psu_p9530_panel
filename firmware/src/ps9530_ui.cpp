#include <Arduino.h>
#include "ps9530_ui.h"
#include "ps9530_ctrl.h"
#include "ps_display.h"

#define MAX_VOLTAGE_CENTIVOLTS 3000UL
#define MAX_CURRENT_CENTIAMPS 1000UL
#define MAX_POWER_DECIWATT 3000UL

const uint16_t PS9530_UI::inputFactorLookup[4] PROGMEM = {1, 10, 100, 1000};

PS9530_UI::PS9530_UI(PS9530_Ctrl& control,
                     PsDisplay& display):
    control(control),
    display(display),
    currentInputMode(InputNone)
{
}

void PS9530_UI::init() {
    control.init();
    display.init();
    display.setScreenMode(display.SCREEN_MODE_LOGO);
    display.render();
    _delay_ms(7000);
    display.setScreenMode(display.SCREEN_MODE_MAINSCREEN);
    setVoltageSetpointMilliVolts(30000);
    setCurrentLimitMilliAmps(10000);
    setPowerLimitCentiWatt(30000);
}

void PS9530_UI::update() {
    handleKeyboardEvents();
    updateMeasurements();
    display.setOvertemp(control.isOvertemp());
    display.setStandby(control.isStandbyEnabled());
    PS9530_Ctrl::LimitingMode limitingMode = control.getLimitingMode();
    display.setLimitedV(limitingMode==PS9530_Ctrl::LimitingMode_Voltage);
    display.setLimitedA(limitingMode==PS9530_Ctrl::LimitingMode_Current);
    display.setLimitedP(limitingMode==PS9530_Ctrl::LimitingMode_Power);
    Serial.print(F("U: ADC="));
    Serial.print(control.rawADCMeasurements[PS9530_Ctrl::adcChannel_voltage]);
    Serial.print(F(" DAC="));
    Serial.print(control.rawDACValue[PS9530_Ctrl::muxChannel_voltage]);
    Serial.print(F(" I: ADC="));
    Serial.print(control.rawADCMeasurements[PS9530_Ctrl::adcChannel_current]);
    Serial.print(F(" DAC="));
    Serial.println(control.rawDACValue[PS9530_Ctrl::muxChannel_current]);
}

void PS9530_UI::handleKeyboardEvents() {
    bool hadEvent = false;
    KeyCode keyCode;
    while ((keyCode = control.readKeycode()) != kbd_none) {
        hadEvent = true;
        Serial.println(keyCode);
        if (currentInputMode != InputLocked) {
            switch (keyCode) {
                case kbd_u:
                    changeInputMode(InputVoltage);
                    break;
                case kbd_i:
                    changeInputMode(InputCurrent);
                    break;
                case kbd_p:
                    changeInputMode(InputPower);
                    break;
                case kbd_u_long:
                    if (display.getScreenMode() == display.SCREEN_MODE_FULL_GRAPH) {
                        display.setScreenMode(display.SCREEN_MODE_MAINSCREEN);
                    } else if (display.getScreenMode() == display.SCREEN_MODE_VOLTS) {
                        display.setScreenMode(display.SCREEN_MODE_FULL_GRAPH);
                    } else {
                        display.setScreenMode(display.SCREEN_MODE_VOLTS);
                    }
                    break;
                case kbd_i_long:
                    if (display.getScreenMode() == display.SCREEN_MODE_FULL_GRAPH) {
                        display.setScreenMode(display.SCREEN_MODE_MAINSCREEN);
                    } else if (display.getScreenMode() == display.SCREEN_MODE_AMPS) {
                        display.setScreenMode(display.SCREEN_MODE_FULL_GRAPH);
                    } else {
                        display.setScreenMode(display.SCREEN_MODE_AMPS);
                    }
                    break;
                case kbd_p_long:
                    if (display.getScreenMode() == display.SCREEN_MODE_FULL_GRAPH) {
                        display.setScreenMode(display.SCREEN_MODE_MAINSCREEN);
                    } else if (display.getScreenMode() == display.SCREEN_MODE_WATTS) {
                        display.setScreenMode(display.SCREEN_MODE_FULL_GRAPH);
                    } else {
                        display.setScreenMode(display.SCREEN_MODE_WATTS);
                    }
                    break;
                case kbd_enc_cw: case kbd_enc_ccw:
                    handleEncoderRotation(keyCode);
                    break;
                case kbd_1: case kbd_2: case kbd_3:
                case kbd_4: case kbd_5: case kbd_6:
                case kbd_7: case kbd_8: case kbd_9:
                case kbd_0:
                case kbd_1_long: case kbd_2_long: case kbd_3_long:
                case kbd_4_long: case kbd_5_long: case kbd_6_long:
                case kbd_7_long: case kbd_8_long: case kbd_9_long:
                case kbd_0_long:
                    handleDigitKey(keyCode);
                    break;
                case kbd_dot:
                case kbd_dot_long:
                    handleDotKey();
                    break;
                case kbd_ce:
                case kbd_ce_long:
                    handleCEKey();
                    break;
                case kbd_enter:
                case kbd_enter_long:
                    handleEnterKey();
                    break;
                case kbd_left: case kbd_right:
                case kbd_left_long: case kbd_right_long:
                    handleDirectionKey(keyCode);
                    break;
                case kbd_lock:
                case kbd_lock_long:
                    handleLockKey();
                    break;
                case kbd_memory:
                case kbd_memory_long:
                    handleMemoryKey();
                    break;
                case kbd_standby:
                case kbd_standby_long:
                    handleStandbyKey();
                    break;
                case kbd_remote:
                case kbd_remote_long:
                    handleRemoteKey();
                    break;
                default:
                    /* Ingore any other keys */
                    break;
            }
        } else {
            switch (keyCode) {
                case kbd_lock:
                case kbd_lock_long:
                    handleLockKey();
                    break;
                case kbd_standby:
                case kbd_standby_long:
                    handleStandbyKey();
                    break;
                default:
                    /* Ingore any other keys */
                    break;
            }
        }
    }
    display.render();
    if (hadEvent) {
        Serial.print(F("standbyMode="));
        Serial.println(control.isStandbyEnabled());
        Serial.print(F("voltageSetpointMilliVolts="));
        Serial.println(voltageSetpointMilliVolts);
        Serial.print(F("currentLimitMilliAmps="));
        Serial.println(currentLimitMilliAmps);
        Serial.print(F("powerLimitCentiWatt="));
        Serial.println(powerLimitCentiWatt);
        Serial.print(F("currentInputMode="));
        Serial.println(currentInputMode);
        Serial.print(F("currentInputValue="));
        Serial.println(currentInputValue);
        Serial.print(F("currentInputFactorIndex="));
        Serial.println(pgm_read_word(&inputFactorLookup[currentInputFactorIndex]));
        Serial.print(F("currentInputDigit="));
        Serial.println(currentInputDigit);
        Serial.print(F("currentInputDigitMax="));
        Serial.println(currentInputDigitMax);
        Serial.print(F("convertCurrentInputValue="));
        Serial.println(currentInputValue);
        Serial.print(F("currentMaximumValue="));
        Serial.println(currentMaximumValue);
    }
}

void PS9530_UI::updateMeasurements() {
    display.setMilliVolts(control.getMilliVoltsMeasurement());
    display.setMilliAmps(control.getMilliAmpsMeasurement());
    display.setCentiWatts(control.getCentiWattsMeasurement());
}

void PS9530_UI::changeInputMode(InputMode newMode) {
    /* If the current input mode is not InputNone, reset the
     * respective limit to the original value. */
    switch (currentInputMode) {
    case InputVoltage:
        setVoltageSetpointMilliVolts(originalLimitValue);
        break;
    case InputCurrent:
        setCurrentLimitMilliAmps(originalLimitValue);
        break;
    case InputPower:
        setPowerLimitCentiWatt(originalLimitValue);
        break;
    default:
        /* Nothing to do */
        break;
    }
    currentInputMode = newMode;
    currentInputValue = 0;
    /* Set up the value to edit */
    switch (currentInputMode) {
    case InputVoltage:
        currentInputValue = voltageSetpointMilliVolts/10;
        originalLimitValue = voltageSetpointMilliVolts;
        currentMaximumValue = MAX_VOLTAGE_CENTIVOLTS;
        currentInputOnesIndex = 2;
        break;
    case InputCurrent:
        currentInputValue = currentLimitMilliAmps/10;
        originalLimitValue = currentLimitMilliAmps;
        currentMaximumValue = MAX_CURRENT_CENTIAMPS;
        currentInputOnesIndex = 2;
        break;
    case InputPower:
        currentInputValue = powerLimitCentiWatt/10;
        originalLimitValue = powerLimitCentiWatt;
        currentMaximumValue = MAX_POWER_DECIWATT;
        currentInputOnesIndex = 1;
        break;
    default:
        /* Nothing to be done here */
        break;
    }
    /* Reset the cursor position */
    switch (currentInputMode) {
    case InputVoltage:
    case InputCurrent:
    case InputPower:
        // reset to second digit
        currentInputDigit = currentInputDigitMax-1;
        currentInputFactorIndex = 2;
        updateEditedValue();
        break;
    default:
        /* Nothing to be done here */
        break;
    }
}

void PS9530_UI::updateEditedValue() {
    /* Adjust the value to the maximum allowed value */
    switch (currentInputMode) {
    case InputNone:
        /* Nothing to be done here */
        break;
    default:
        /* Limit the value to its maximum */
        if (currentInputValue<0) {
            currentInputValue = 0;
        } else if (currentInputValue>currentMaximumValue) {
            currentInputValue = currentMaximumValue;
        }
        break;
    }
    switch (currentInputMode) {
    case InputNone:
        display.setCurser(PsDisplay::ROW_NULL, 0);
        break;
    case InputVoltage:
        display.setCurser(PsDisplay::ROW_VOLTS, currentInputDigitMax-currentInputDigit);
        display.setMilliVoltsSetpoint(currentInputValue*10);
        break;
    case InputCurrent:
        display.setCurser(PsDisplay::ROW_AMPS, currentInputDigitMax-currentInputDigit);
        display.setMilliAmpsLimit(currentInputValue*10);
        break;
    case InputPower:
        display.setCurser(PsDisplay::ROW_WATTS, currentInputDigitMax-currentInputDigit);
        display.setCentiWattsLimit(currentInputValue*10);
        break;
    default:
        break;
    }
}

void PS9530_UI::handleEncoderRotation(KeyCode direction) {
    if (currentInputMode == InputNone) {
        /* When not input mode, ignore the rotation */
        return;
    }
    switch (direction) {
    case kbd_enc_cw:
        currentInputValue += pgm_read_word(&inputFactorLookup[currentInputFactorIndex]);
        updateEditedValue();
        break;
    case kbd_enc_ccw:
        currentInputValue -= pgm_read_word(&inputFactorLookup[currentInputFactorIndex]);
        updateEditedValue();
        break;
    default:
        /* Shouldn't happen, is ignored. This is just to placate the compiler */
        break;
    }
}

void PS9530_UI::handleDigitKey(KeyCode keycode) {
    if (currentInputMode==InputNone) {
        /* Ignore digit keys if not in any input mode */
        return;
    }
    uint8_t digit;
    switch (keycode) {
    case kbd_0: digit=0; break;
    case kbd_1: digit=1; break;
    case kbd_2: digit=2; break;
    case kbd_3: digit=3; break;
    case kbd_4: digit=4; break;
    case kbd_5: digit=5; break;
    case kbd_6: digit=6; break;
    case kbd_7: digit=7; break;
    case kbd_8: digit=8; break;
    case kbd_9: digit=9; break;
    default:
        /* This should not happen, but we exit just to be safe */
        return;
    }
    uint8_t digitAtPos = uint16_t(currentInputValue/pgm_read_word(&inputFactorLookup[currentInputFactorIndex])) % 10;
    currentInputValue -= digitAtPos*pgm_read_word(&inputFactorLookup[currentInputFactorIndex]);
    currentInputValue += digit*pgm_read_word(&inputFactorLookup[currentInputFactorIndex]);
    moveCurser(CURSER_RIGHT);
    updateEditedValue();
}

void PS9530_UI::handleDotKey() {
    if (currentInputMode == InputNone) {
        /* Ignore this keypress if we are in none of the input modes */
        return;
    }
    if (currentInputDigit<currentInputOnesIndex && currentInputValue <= 100) {
        //move everything to the left
        while (currentInputDigit<currentInputOnesIndex) {
            currentInputValue *= 10;
            moveCurser(CURSER_LEFT);
        }
        moveCurser(CURSER_RIGHT);
    } else if (currentInputDigit>=currentInputOnesIndex) {
        //move everything to the right
        while (currentInputDigit>=currentInputOnesIndex) {
            currentInputValue /= 10;
            moveCurser(CURSER_RIGHT);
        }
    }
    updateEditedValue();
}

void PS9530_UI::handleCEKey() {
    if (currentInputMode == InputNone) {
        /* Ignore this keypress if we are in none of the input modes */
        return;
    }
    /* Clear the whole number */
    currentInputValue = 0;
    /* Reset to the first digit */
    currentInputDigit = currentInputDigitMax;
    currentInputFactorIndex = 3;
    updateEditedValue();
}

void PS9530_UI::handleEnterKey() {
    switch (currentInputMode) {
    case InputNone:
        /* If we are in no input move, ignore the key */
        return;
    case InputVoltage:
        setVoltageSetpointMilliVolts(currentInputValue*10);
        break;
    case InputCurrent:
        setCurrentLimitMilliAmps(currentInputValue*10);
        break;
    case InputPower:
        setPowerLimitCentiWatt(currentInputValue*10);
        break;
    default:
        break;
    }
    /* Leave input mode */
    currentInputMode=InputNone;
    updateEditedValue();
}

void PS9530_UI::handleDirectionKey(KeyCode keycode) {
    if (currentInputMode==InputNone) {
        /* Ignore this keypress if we are in none of the input modes */
        return;
    }
    switch (keycode) {
    case kbd_left:
        moveCurser(CURSER_LEFT);
        updateEditedValue();
        break;
    case kbd_right:
        moveCurser(CURSER_RIGHT);
        updateEditedValue();
        break;
    default:
        /* Shouldn't happen, is ignored. This is just to placate the compiler */
        break;
    }
}

void PS9530_UI::moveCurser(CurserDirection direction) {
    if (direction == CURSER_LEFT) {
        if (currentInputDigit<currentInputDigitMax) {
            currentInputDigit++;
            currentInputFactorIndex++;
        }
    } else {
        if (currentInputDigit>0) {
            currentInputDigit--;
            currentInputFactorIndex--;
        }
    }
}

void PS9530_UI::handleLockKey() {
    if (currentInputMode == InputLocked) {
        currentInputMode = lastInputMode;
        display.setLocked(false);
    } else {
        lastInputMode = currentInputMode;
        currentInputMode = InputLocked;
        display.setLocked(true);
    }
}

void PS9530_UI::handleMemoryKey() {
    
}

void PS9530_UI::handleStandbyKey() {
    control.toggleStandbyMode();
}

void PS9530_UI::handleRemoteKey() {
    
}

void PS9530_UI::setVoltageSetpointMilliVolts(uint16_t milliVolts) {
    voltageSetpointMilliVolts = milliVolts;
    control.setMilliVoltSetpoint(milliVolts);
    display.setMilliVoltsSetpoint(milliVolts);
}

void PS9530_UI::setCurrentLimitMilliAmps(uint16_t milliAmps) {
    currentLimitMilliAmps = milliAmps;
    control.setMilliAmpsLimit(milliAmps);
    display.setMilliAmpsLimit(milliAmps);
}

void PS9530_UI::setPowerLimitCentiWatt(uint16_t centiWatt) {
    powerLimitCentiWatt = centiWatt;
    control.setCentiWattLimit(centiWatt);
    display.setCentiWattsLimit(centiWatt);
}

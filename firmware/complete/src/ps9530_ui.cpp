#include <Arduino.h>
#include "ps9530_ui.h"
#include "ps9530_ctrl.h"
#include "ps_display.h"

PS9530_UI::PS9530_UI(PS9530_Ctrl& control,
                     PsDisplay& display):
    control(control),
    display(display),
    currentInputMode(InputNone),
    standbyMode(false)
{
}

void PS9530_UI::init() {
    display.init();
    cli();
    display.clear();
    display.renderLogo();
    delay(1000);
    display.clear();
    sei();
    control.init();
    setVoltageSetpointsMilliVolts(30000);
    setCurrentLimitMilliAmps(10000);
    setPowerLimitCentiWatt(30000);
    setStandbyMode(true);
}

void PS9530_UI::update() {
    handleKeyboardEvents();
    //updateMeasurements();
}

void PS9530_UI::handleKeyboardEvents() {
    bool hadEvent = false;
    KeyCode keyCode;
    while ((keyCode = control.readKeycode()) != kbd_none) {
        hadEvent = true;
        Serial.println(keyCode);
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
            case kbd_enc_cw: case kbd_enc_ccw:
                handleEncoderRotation(keyCode);
                break;
            case kbd_1: case kbd_2: case kbd_3:
            case kbd_4: case kbd_5: case kbd_6:
            case kbd_7: case kbd_8: case kbd_9:
            case kbd_0:
                handleDigitKey(keyCode);
                break;
            case kbd_dot:
                handleDotKey();
                break;
            case kbd_ce:
                handleCEKey();
                break;
            case kbd_enter:
                handleEnterKey();
                break;
            case kbd_left: case kbd_right:
                handleDirectionKey(keyCode);
                break;
            case kbd_lock:
                handleLockKey();
                break;
            case kbd_memory:
                handleMemoryKey();
                break;
            case kbd_standby:
                handleStandbyKey();
                break;
            case kbd_remote:
                handleRemoteKey();
                break;
            default:
                /* Ingore any other keys */
                break;
        }
    }
    display.renderMainscreen();
    if (hadEvent) {
        Serial.print("standbyMode=");
        Serial.println(standbyMode);
        Serial.print("voltageSetpointMilliVolts=");
        Serial.println(voltageSetpointMilliVolts);
        Serial.print("currentLimitMilliAmps=");
        Serial.println(currentLimitMilliAmps);
        Serial.print("powerLimitCentiWatt=");
        Serial.println(powerLimitCentiWatt);
        Serial.print("currentInputMode=");
        Serial.println(currentInputMode);
        Serial.print("currentInputValue=");
        Serial.println(currentInputValue);
        Serial.print("currentInputDigit=");
        Serial.println(currentInputDigit);
    }
}

void PS9530_UI::updateMeasurements() {
    // FIXME: This should only happen when we actually have a new measurement!
    display.setMilliVolts(control.getMilliVoltsMeasurement());
    display.setMilliAmps(control.getMilliAmpsMeasurement());
    uint16_t centiWattPowerMeasurement = (uint32_t)control.getMilliVoltsMeasurement()*control.getMilliAmpsMeasurement()/10000UL;
    display.setCentiWatts(centiWattPowerMeasurement);
}

void PS9530_UI::changeInputMode(InputMode newMode) {
    currentInputMode = newMode;
    memset(currentInputValue, '0', sizeof(currentInputValue)-1);
    /* Set up the value to edit */
    switch (currentInputMode) {
    case InputNone:
        /* Nothing to be done here */
        break;
    case InputVoltage:
        sprintf(currentInputValue, "%06u", voltageSetpointMilliVolts);
        currentInputOnesIndex = 1;
        break;
    case InputCurrent:
        sprintf(currentInputValue, "%06u", currentLimitMilliAmps);
        currentInputOnesIndex = 1;
        break;
    case InputPower:
        sprintf(currentInputValue, "%06u", powerLimitCentiWatt*10);
        currentInputOnesIndex = 2;
        break;
    }
    /* Reset the cursor position */
    switch (currentInputMode) {
    case InputNone:
        /* Nothing to be done here */
        break;
    case InputVoltage:
    case InputCurrent:
    case InputPower:
        currentInputDigit = 0;
        currentInputDigitCount = 4;
        updateEditedValue();
        break;
    }
}

void PS9530_UI::updateEditedValue() {
    switch (currentInputMode) {
    case InputNone:
        display.setCurser(PsDisplay::ROW_NULL, 0);
        break;
    case InputVoltage:
        display.setCurser(PsDisplay::ROW_VOLTS, currentInputDigit);
        display.setMilliVoltsSetpoint(convertCurrentInputValue());
        break;
    case InputCurrent:
        display.setCurser(PsDisplay::ROW_AMPS, currentInputDigit);
        display.setMilliAmpsLimit(convertCurrentInputValue());
        break;
    case InputPower:
        display.setCurser(PsDisplay::ROW_WATTS, currentInputDigit);
        display.setCentiWattsLimit(convertCurrentInputValue());
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
        if (currentInputValue[currentInputDigit]<'9') {
            /* Increase the value of the current digit */
            currentInputValue[currentInputDigit]++;
        } else {
            /* Wrap around after '9' */
            currentInputValue[currentInputDigit]='0';
        }
        updateEditedValue();
        break;
    case kbd_enc_ccw:
        if (currentInputValue[currentInputDigit]>'0') {
            /* Decrease the value of the current digit */
            currentInputValue[currentInputDigit]--;
        } else {
            /* Wrap around after '0' */
            currentInputValue[currentInputDigit]='9';
        }
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
    char digit;
    switch (keycode) {
    case kbd_0: digit='0'; break;
    case kbd_1: digit='1'; break;
    case kbd_2: digit='2'; break;
    case kbd_3: digit='3'; break;
    case kbd_4: digit='4'; break;
    case kbd_5: digit='5'; break;
    case kbd_6: digit='6'; break;
    case kbd_7: digit='7'; break;
    case kbd_8: digit='8'; break;
    case kbd_9: digit='9'; break;
    default:
        /* This should not happen, but we exit just to be safe */
        return;
    }
    currentInputValue[currentInputDigit] = digit;
    if (currentInputDigit+1<currentInputDigitCount) {
        currentInputDigit++;
    }
    updateEditedValue();
}

void PS9530_UI::handleDotKey() {
    if (currentInputMode == InputNone) {
        /* Ignore this keypress if we are in none of the input modes */
        return;
    }
    if (currentInputDigit<=currentInputOnesIndex) {
        /* Move everything to the right as required */
        const size_t startIndex =  currentInputOnesIndex+1-currentInputDigit;
        memmove(&currentInputValue[startIndex], currentInputValue, currentInputDigit);
        /* Clear the digits in front */
        memset(currentInputValue, '0', startIndex);
    } else if (currentInputDigit>currentInputOnesIndex+1) {
        const size_t srcIndex = currentInputDigit-currentInputOnesIndex-1;
        /* Move everything to the left as required */
        memmove(currentInputValue, &currentInputValue[srcIndex], currentInputOnesIndex);
        /* Clear the digits in the back */
        memset(&currentInputValue[srcIndex], '0', sizeof(currentInputValue)-srcIndex-1);
    }
    /* Place the cursor behind the ones index */
    if (currentInputOnesIndex+1<currentInputDigitCount) {
        currentInputDigit = currentInputOnesIndex+1;
    } else {
        currentInputDigit = currentInputDigitCount-1;
    }
    updateEditedValue();
}

void PS9530_UI::handleCEKey() {
    if (currentInputMode == InputNone) {
        /* Ignore this keypress if we are in none of the input modes */
        return;
    }
    /* Clear the whole number */
    memset(currentInputValue, '0', sizeof(currentInputValue)-1);
    /* Reset to the first digit */
    currentInputDigit = 0;
    updateEditedValue();
}

uint32_t PS9530_UI::convertCurrentInputValue() {
    /* Convert the value in the input buffer to an integer value */
    uint32_t value = 0;
    for (uint8_t idx = 0; idx<currentInputDigitCount; idx++) {
        value *= 10;
        value += currentInputValue[idx]-'0';
    }
    return value;
}

void PS9530_UI::handleEnterKey() {
    switch (currentInputMode) {
    case InputNone:
        /* If we are in no input move, ignore the key */
        return;
    case InputVoltage:
        setVoltageSetpointsMilliVolts(convertCurrentInputValue());
        break;
    case InputCurrent:
        setCurrentLimitMilliAmps(convertCurrentInputValue());
        break;
    case InputPower:
        setPowerLimitCentiWatt(convertCurrentInputValue());
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
        if (currentInputDigit>0) {
            currentInputDigit--;
            updateEditedValue();
        }
        break;
    case kbd_right:
        if (currentInputDigit+1<currentInputDigitCount) {
            currentInputDigit++;
            updateEditedValue();
        }
        break;
    default:
        /* Shouldn't happen, is ignored. This is just to placate the compiler */
        break;
    }
}

void PS9530_UI::handleLockKey() {
    
}

void PS9530_UI::handleMemoryKey() {
    
}

void PS9530_UI::handleStandbyKey() {
    setStandbyMode(!standbyMode);
}

void PS9530_UI::handleRemoteKey() {
    
}

void PS9530_UI::setVoltageSetpointsMilliVolts(uint16_t milliVolts) {
    // TODO: Clamp the setpoint according to hardware limits
    voltageSetpointMilliVolts = milliVolts;
    Serial.print("voltageSetpointMilliVolts=");
    Serial.println(voltageSetpointMilliVolts);
    display.setMilliVoltsSetpoint(milliVolts);
    updateControlLimits();
}

void PS9530_UI::setCurrentLimitMilliAmps(uint16_t milliAmps) {
    // TODO: Clamp the setpoint according to hardware limits
    currentLimitMilliAmps = milliAmps;
    Serial.print("currentLimitMilliAmps=");
    Serial.println(currentLimitMilliAmps);
    display.setMilliAmpsLimit(milliAmps);
    updateControlLimits();
}

void PS9530_UI::setPowerLimitCentiWatt(uint16_t centiWatt) {
    // TODO: Clamp the setpoint according to hardware limits
    powerLimitCentiWatt = centiWatt;
    Serial.print("powerLimitCentiWatt=");
    Serial.println(powerLimitCentiWatt);
    display.setCentiWattsLimit(centiWatt);
    updateControlLimits();
}

void PS9530_UI::updateControlLimits() {
    if (standbyMode) {
        /* We're in standby mode, set the controller to 0V and 0A */
        control.setMilliVoltSetpoint(0);
        control.setMilliAmpsLimit(0);
    } else {
        /* We're not in standby mode, set the voltage and current limit */
        control.setMilliVoltSetpoint(voltageSetpointMilliVolts);

        /* Both the current and the power limit define a maximum current.
        * Set the one that actuall implies the lower current limit.
        */
        if (powerLimitCentiWatt*10UL>(uint32_t)voltageSetpointMilliVolts*currentLimitMilliAmps/1000UL) {
            /* The current limit is more limiting */
            Serial.println("currentLimit is lower");
            control.setMilliAmpsLimit(currentLimitMilliAmps);
        } else {
            /* The power limit is more limiting */
            uint16_t actualCurrentLimitMilliAmps = powerLimitCentiWatt*10000UL/voltageSetpointMilliVolts;
            Serial.println("powerLimit is lower");
            control.setMilliAmpsLimit(actualCurrentLimitMilliAmps);
        }
    }
}

void PS9530_UI::setStandbyMode(bool newMode) {
    if (newMode==standbyMode) {
        /* Mode does not change, so we do nothing */
        return;
    }

    display.setStandby(newMode);
    standbyMode = newMode;
    updateControlLimits();
}
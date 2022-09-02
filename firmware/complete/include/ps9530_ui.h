/* ps9530_ui.h - Controller for PS9530 User Interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#ifndef PS9530_UI_H
#define PS9530_UI_H

#include "ps9530_ctrl.h"

class PsDisplay;

class PS9530_UI {
public:
    PS9530_UI(PS9530_Ctrl& control,
              PsDisplay& display);

    void init();
    
    /** Handle all pending events and update the system state */
    void update();
protected:
    /** Handle all pending keyboard events */
    void handleKeyboardEvents();

    enum InputMode {
        InputNone,
        InputVoltage,
        InputCurrent,
        InputPower
    };

    /** Change the input mode */
    void changeInputMode(InputMode newMode);

    /** Handle a press on the numeric keypad */
    void handleDigitKey(KeyCode keycode);

    /** Handle a press of the dot key */
    void handleDotKey();

    /** Handle a press of the CE key */
    void handleCEKey();

    /** Handle a press of the enter key */
    void handleEnterKey();

    /** Handle a press of a direction key (left or right) */
    void handleDirectionKey(KeyCode keycode);

    /** Handle a press of the LOCK key */
    void handleLockKey();

    /** Handle a press of the MEMORY key */
    void handleMemoryKey();

    /** Handle a press of the STANDBY key */
    void handleStandbyKey();

    /** Handle a press of the REMOTE key */
    void handleRemoteKey();

    /** Handle a movement of the encoder input */
    void handleEncoderRotation(KeyCode direction);

    /** Update the edited value in the display */
    void updateEditedValue();
protected:
    PS9530_Ctrl& control;
    PsDisplay& display;

    /** The selected input mode.
     * This specifies the field that is modified by
     * key presses on the numeric keypad or the rotary
     * encoder.
     */
    InputMode currentInputMode;
    /** The number of the digit currently being modified */
    uint8_t currentInputDigit;
    /** The number of editable digits in the currently edited value */
    uint8_t currentInputDigitCount;
    /** The index of the one place in the current input mode */
    uint8_t currentInputOnesIndex;
    /** The value currently being edited (including terminating
     * zero, not including the comma).
     * The worst case is the power, which could have 3 digits before the
     * decimal point and up to 3 (Milliwatts) digits after the decimal
     * point. That is 6 digits, plus terminating zero.
     */
    char currentInputValue[7];
    /** The maximum value that can be entered for the currently edited quantity.
     */
    uint32_t currentMaximumValue;

    /** Flag indicating whether we are in standby mode */
    bool standbyMode;

    void setVoltageSetpointMilliVolts(uint16_t milliVolts);
    void setCurrentLimitMilliAmps(uint16_t milliAmps);
    void setPowerLimitCentiWatt(uint16_t centiWatt);

    void setStandbyMode(bool newMode);

    void updateControlLimits();

    uint32_t convertCurrentInputValue();

    void updateMeasurements();
private:
    uint16_t voltageSetpointMilliVolts;
    uint16_t currentLimitMilliAmps;
    uint16_t powerLimitCentiWatt;
};


#endif /* PS9530_UI_H */
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
    /** Set the UI limiter flags according to the current limiting controller */
    void setLimiterFlags(PS9530_Ctrl::LimitingMode limitingController);
    /** Handle all pending keyboard events */
    void handleKeyboardEvents();

    enum InputMode {
        InputNone,
        InputVoltage,
        InputCurrent,
        InputPower,
        InputLocked
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
    InputMode lastInputMode;
    /** The number of the digit currently being modified */
    uint8_t currentInputDigit;
    uint16_t currentInputFactor;
    /** The number of editable digits in the currently edited value */
    const uint16_t currentInputDigitCount = 4;
    /** The index of the one place in the current input mode */
    uint8_t currentInputOnesIndex;
    /** The value currently being edited (including terminating
     * zero, not including the comma).
     * The worst case is the power, which could have 3 digits before the
     * decimal point and up to 3 (Milliwatts) digits after the decimal
     * point. That is 6 digits, plus terminating zero.
     */
    int16_t currentInputValue;
    /** The maximum value that can be entered for the currently edited quantity. */
    uint16_t currentMaximumValue;
    /** The original value of the currently edited value. */
    uint16_t originalLimitValue;

    void setVoltageSetpointMilliVolts(uint16_t milliVolts);
    void setCurrentLimitMilliAmps(uint16_t milliAmps);
    void setPowerLimitCentiWatt(uint16_t centiWatt);

    void setStandbyMode(bool newMode);

    void updateMeasurements();
private:
    enum CurserDirection {
        CURSER_LEFT,
        CURSER_RIGHT
    };

    void moveCurser(CurserDirection direction);

    uint16_t voltageSetpointMilliVolts;
    uint16_t currentLimitMilliAmps;
    uint16_t powerLimitCentiWatt;
};


#endif /* PS9530_UI_H */
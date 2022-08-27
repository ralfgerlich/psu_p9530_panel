/* ps9530_ctrl.h - Hardware Controller interface for the PS9530 */
#ifndef PS9530_CTRL_H
#define PS9530_CTRL_H

#include <stdint.h>
#include "kbd.h"

class PS9530_Ctrl {
public:
    static PS9530_Ctrl& getInstance();

    /** Initialize the hardware interface */
    void init();

    /** Set the setpoint for the voltage in millivolts */
    void setMilliVoltSetpoint(uint16_t milliVolts);
    /** Set the limit for the current in millivolts */
    void setMilliAmpsLimit(uint16_t milliAmpere);

    /** Get the voltage measured in millivolts */
    uint16_t getMilliVoltsMeasurement() const;
    /** Get the current measured in milliampere */
    uint16_t getMilliAmpsMeasurement() const;

    KeyCode readKeycode();

    /** Update the hardware interface.
     * This method is intended to be called from an interrupt and
     * aims for low execution time. */
    void update();

    /** Update data from the ADC.
     * This method is intended to be called from an interrupt and
     * aims for low execution time. */
    void updateADC();
private:
    PS9530_Ctrl();

    void updateDAC();

    void startADCConversion();

    static PS9530_Ctrl instance;

    uint16_t milliVoltSetpoint;
    uint16_t milliAmpsLimit;

    enum {
        muxChannel_voltage = 0,
        muxChannel_current
    } currentMuxChannel;

    uint16_t milliVoltsMeasurement;
    uint16_t milliAmpsMeasurement;

    enum {
        adcChannel_voltage = 0,
        adcChannel_current,
        adcChannel_temp1,
        adcChannel_temp2
    } currentADCChannel;
};

#endif /* PS9530_CTRL_H */
/* ps9530_ctrl.h - Hardware Controller interface for the PS9530 */
#ifndef PS9530_CTRL_H
#define PS9530_CTRL_H

#include <stdint.h>
#include <avr/pgmspace.h>
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
    uint16_t getMilliVoltsMeasurement(bool clearFlag=true);
    /** Get the current measured in milliampere */
    uint16_t getMilliAmpsMeasurement(bool clearFlag=true);

    KeyCode readKeycode();

    /** Update the hardware interface.
     * This method is intended to be called from an interrupt and
     * aims for low execution time. */
    void update();

    /** Update data from the ADC.
     * This method is intended to be called from an interrupt and
     * aims for low execution time. */
    void updateADC();

    /** Return true if and only if there is a new voltage measurement available */
    bool haveNewVoltageMeasurement() const { return (measurementsAvailable & measurementVoltage) != 0; }

    /** Return true if and only if there is a new current measurement available */
    bool haveNewCurrentMeasurement() const { return (measurementsAvailable & measurementCurrent) != 0; }
private:
    PS9530_Ctrl();

    /** Update one of the DAC channels */
    void updateDAC();

    /** Start the ADC cycle */
    void startADCCycle();

    /** Start the next conversion in the cycle, as defined by currentADCChannel */
    void startADCConversion();

    static PS9530_Ctrl instance;

    uint16_t milliVoltSetpoint;
    uint16_t milliAmpsLimit;

    enum {
        muxChannel_voltage = 0,
        muxChannel_current
    } currentMuxChannel;

    /** Measured voltage in mV */
    uint16_t milliVoltsMeasurement;
    /** Measured current in mA */
    uint16_t milliAmpsMeasurement;

    enum {
        /** Index for temperature Sensor 1 */
        tempSensor_1=0,
        /** Index for temperature Sensor 2 */
        tempSensor_2
    };

    /** Minimum ADC value for temperature ADC conversion */
    static const uint16_t minTempADC[2] PROGMEM;
    /** Maximum ADC value for temperature ADC conversion */
    static const uint16_t maxTempADC[2] PROGMEM;
    /** Bit shift value for temperature ADC conversion */
    static const uint8_t shiftTempADC[2] PROGMEM;
    /** Temperature offset for ADC conversion */
    static const int16_t tempOffset[2][11] PROGMEM;
    /** Temperature gradient for ADC conversion */
    static const int16_t tempGradient[2][11] PROGMEM;

    static int16_t interpolate_temp(uint8_t index, uint16_t adcValue);
    /** Measured temperatures in °C  */
    uint8_t tempDegCMeasurement[2];

    enum {
        /** ADC channel for measured voltage */
        adcChannel_voltage = 0,
        adcChannel_current,
        adcChannel_temp1,
        adcChannel_temp2,
        adcChannel__idle
    } currentADCChannel;

    enum {
        measurementVoltage = 1,
        measurementCurrent = 2
    };

    uint8_t measurementsAvailable;
};

#endif /* PS9530_CTRL_H */
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

    KeyCode readKeycode();

    /** Update the hardware interface.
     * This method is intended to be called from an interrupt and
     * aims for low execution time. */
    void update();

    /** Update data from the ADC.
     * This method is intended to be called from an interrupt and
     * aims for low execution time. */
    void updateADC();

    /** Set standby mode */
    void setStandbyMode(bool standbyMode);

    /** Toggle standby mode */
    void toggleStandbyMode();

    /** Determine whether we are in standby mode */
    bool isStandbyEnabled() const;

    /** Set the setpoint for the voltage in millivolts */
    void setMilliVoltSetpoint(uint16_t milliVolts);
    /** Set the limit for the current in millivolts */
    void setMilliAmpsLimit(uint16_t milliAmpere);

    /** Get the voltage measured in millivolts */
    uint16_t getMilliVoltsMeasurement() const;
    /** Get the current measured in milliampere */
    uint16_t getMilliAmpsMeasurement() const;

    /** Get the temperature at sensor 1 in degrees C */
    int16_t getTemperature1() const;
    /** Get the temperature at sensor 2 in degrees C */
    int16_t getTemperature2() const;

    /** Indicate whether we have overtemperature */
    bool isOvertemp() const;

    enum LimitingMode {
        LimitingMode_Voltage=0,
        LimitingMode_Current
    };
    /** Get the current limiter mode */
    LimitingMode getCurrentLimitingMode();
private:
    PS9530_Ctrl();

    /** Update one of the DAC channels */
    void updateDAC();

    /** Start the ADC cycle */
    void startADCCycle();

    /** Start the next conversion in the cycle, as defined by currentADCChannel */
    void startADCConversion();

    static PS9530_Ctrl instance;

    /** Flag indicating standby mode */
    bool standbyMode;

    /** Raw values for the DAC channels */
    uint16_t rawDACValue[2];

    enum {
        muxChannel_voltage = 0,
        muxChannel_current
    } currentMuxChannel;

    /** Voltage offset for ADC conversion */
    static const uint16_t adcVoltageOffset[32] PROGMEM;
    /** Voltage gradient for ADC conversion */
    static const uint16_t adcVoltageGradient[32] PROGMEM;

    /**
     * @brief Convert an ADC measurement into a voltage
     * 
     * @param adcValue The value from the ADC (in the range 0...1023)
     * @return The voltage in Millivolt.
     */
    static uint16_t interpolateADCVoltage(uint16_t adcValue);

    /** Current offset for ADC conversion */
    static const uint16_t adcCurrentOffset[32] PROGMEM;
    /** Current gradient for ADC conversion */
    static const uint16_t adcCurrentGradient[32] PROGMEM;

    /**
     * @brief Convert an ADC measurement into a current
     * 
     * @param adcValue The value from the ADC (in the range 0...1023)
     * @return The voltage in Milliampere.
     */
    static uint16_t interpolateADCCurrent(uint16_t adcValue);

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

    /**
     * @brief Convert an ADC measurement into a temperature
     * 
     * @param index  The sensor index
     * @param adcValue The value from the ADC (in the range 0...1023)
     * @return The temperature in Degree Celsius
     */
    static int16_t interpolateADCTemp(uint8_t index, uint16_t adcValue);

    /** Flags indicating whether we are in overtemperature mode and why */
    enum {
        overTempMode_1=1,
        overTempMode_2=2
    };
    int8_t overTempMode;

    /** Check for overtemperature and put system in standby in that case */
    void updateOvertemperature();
public:
    /** Raw ADC measurements */
    uint16_t rawADCMeasurements[4];

    enum {
        /** ADC channel for measured voltage */
        adcChannel_voltage = 0,
        /** ADC channel for measured current */
        adcChannel_current,
        /** ADC channel for measured coil temperature */
        adcChannel_temp1,
        /** ADC channel for measured heatsink temperature */
        adcChannel_temp2,
        adcChannel__idle
    };

    /** Current ADC state
     * Lowest bit indicates whether this is the dummy measurement (0) or the
     * actual measurement (1).
     * Upper bits indicate the channel being measured.
     */
    uint8_t currentADCState;
};

#endif /* PS9530_CTRL_H */
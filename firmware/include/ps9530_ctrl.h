/* ps9530_ctrl.h - Hardware Controller interface for the PS9530 */
#ifndef PS9530_CTRL_H
#define PS9530_CTRL_H

#include <stdint.h>
#include <avr/pgmspace.h>
#include "kbd.h"

#define PS9530_ADC_VOLTAGE_SMALL 256U
#define PS9530_ADC_VOLTAGE_SHIFT 8U
#define PS9530_ADC_VOLTAGE_SHIFT_SMALL 5U
#define PS9530_ADC_CURRENT_SMALL 256U
#define PS9530_ADC_CURRENT_SHIFT 8U
#define PS9530_ADC_CURRENT_SHIFT_SMALL 5U
#define PS9530_DAC_VOLTAGE_SMALL 1024U
#define PS9530_DAC_VOLTAGE_SHIFT 10U
#define PS9530_DAC_VOLTAGE_SHIFT_SMALL 7U
#define PS9530_DAC_CURRENT_SMALL 512U
#define PS9530_DAC_CURRENT_SHIFT 9U
#define PS9530_DAC_CURRENT_SHIFT_SMALL 6U
#define PS9530_ADC_TEMP1_SHIFT 5U
#define PS9530_ADC_TEMP2_SHIFT 6U

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

    /** Toggle standby mode */
    void toggleStandbyMode();

    /** Determine whether we are in standby mode */
    bool isStandbyEnabled() const;

    /** Set the setpoint for the voltage in Millivolts */
    void setMilliVoltSetpoint(uint16_t milliVolts);
    /** Set the limit for the current in Milliampere */
    void setMilliAmpsLimit(uint16_t milliAmpere);
    /** Set the limit for the power in Centiwatt */
    void setCentiWattLimit(uint16_t centiWatt);

    /** Update the hardware voltage and current limit */
    void updateLimits();

    /** Get the voltage measured in Millivolts */
    uint16_t getMilliVoltsMeasurement() const;
    /** Get the current measured in Milliampere */
    uint16_t getMilliAmpsMeasurement() const;
    /** Get the power measured in Centiwatts */
    uint16_t getCentiWattsMeasurement() const;

    /** Get the temperature at sensor 1 in degrees C */
    int16_t getTemperature1() const;
    /** Get the temperature at sensor 2 in degrees C */
    int16_t getTemperature2() const;

    /** Indicate whether we have overtemperature */
    bool isOvertemp() const;

    enum LimitingMode {
        /** Power supply is inactive (due to standby or overtemperature) */
        LimitingMode_Inactive=0,
        /** Output is limited due to voltage setpoint */
        LimitingMode_Voltage,
        /** Output is limited due to current limit */
        LimitingMode_Current,
        /** Output is limited due to power limit */
        LimitingMode_Power
    };
    /** Get the current limiter mode */
    LimitingMode getLimitingMode();
private:
    PS9530_Ctrl();

    /** Update one of the DAC channels */
    void updateDAC();

    static PS9530_Ctrl instance;

    /** Flag indicating standby mode */
    bool standbyMode;

    /** Voltage setpoint in Millivolts */
    uint16_t voltageSetpointMilliVolts;
    /** Current limit in Milliampere */
    uint16_t currentLimitMilliAmps;
    /** Power limit in Centiwatt */
    uint16_t powerLimitCentiWatt;

    /** Flag indicating whether we set the hardware current limit from
     * the current limit or the power limit.
     */
    enum {
        CurrentLimitFromCurrent,
        CurrentLimitFromPower
    } currentLimitReason;

    /** Get the limiter mode based on the value of the limiter pin */
    LimitingMode getLimitingMode(bool limiterPin);

    /** Raw values for the DAC channels */
public:
    uint16_t rawDACValue[2];

    enum {
        muxChannel_voltage = 0,
        muxChannel_current
    } currentMuxChannel;
protected:

    /** Voltage offset for ADC conversion */
    static const uint16_t adcVoltageOffset[] PROGMEM;
    static const uint16_t adcVoltageOffsetSmall[] PROGMEM;

    /**
     * @brief Convert an ADC measurement into a voltage
     * 
     * @param adcValue The value from the ADC (in the range 0...1023)
     * @return The voltage in Millivolt.
     */
    static uint16_t interpolateADCVoltage(uint16_t adcValue);

    /** Current offset for ADC conversion */
    static const uint16_t adcCurrentOffset[] PROGMEM;
    static const uint16_t adcCurrentOffsetSmall[] PROGMEM;

    /**
     * @brief Convert an ADC measurement into a current
     * 
     * @param adcValue The value from the ADC (in the range 0...1023)
     * @return The voltage in Milliampere.
     */
    static uint16_t interpolateADCCurrent(uint16_t adcValue);

    /** DAC offset for voltage conversion */
    static const uint16_t voltageDACOffset[] PROGMEM;
    static const uint16_t voltageDACOffsetSmall[] PROGMEM;

    /**
     * @brief Convert a voltage target value to a DAC value
     * 
     * @param milliVolts The target voltage in Millivolts
     * @return The value for the voltage DAC channel
     */
    static uint16_t interpolateDACVoltage(uint16_t milliVolts);

    /** DAC offset for voltage conversion */
    static const uint16_t currentDACOffset[] PROGMEM;
    static const uint16_t currentDACOffsetSmall[] PROGMEM;

    /**
     * @brief Convert a current target value to a DAC value
     * 
     * @param milliAmps The target current in Milliamps
     * @return The value for the current DAC channel
     */
    static uint16_t interpolateDACCurrent(uint16_t milliAmps);

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
    /** Temperature offset for ADC conversion */
    static const int16_t tempOffset[2][12] PROGMEM;

    /**
     * @brief Convert an ADC measurement into a temperature
     * 
     * @param index  The sensor index
     * @param adcValue The value from the ADC (in the range 0...1023)
     * @return The temperature in Degree Celsius
     */
    static int16_t interpolateADCTemp(uint8_t index, uint16_t adcValue, uint8_t shift);

    /** Flags indicating whether we are in overtemperature mode and why */
    enum {
        overTempMode_1=1,
        overTempMode_2=2
    };
    int8_t overTempMode;

    /** Check for overtemperature and put system in standby in that case */
    void updateOvertemperature();
public:
    /** Raw (unconverted) ADC measurements */
    uint16_t rawADCMeasurements[4];
    /** Sum of ADC measurements for current channel oversampling */
    uint16_t oversamplingADCSum;
    /** Number of oversampling measurements performed for current channel */
    uint8_t oversamplingADCCount;

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
protected:
    
    /** Current ADC state
     * Lowest bit indicates whether this is the dummy measurement (0) or the
     * actual measurement (1).
     * Upper bits indicate the channel being measured. */
    uint8_t currentADCState;

    /** Scaling counter for ADC cycle frequency.
     * An ADC cycle is started every second hardware update cycle. */
    uint8_t adcCycleFlag;

    /** Start the ADC cycle */
    void startADCCycle();

    /** Start the next conversion in the cycle, as defined by currentADCChannel */
    void startADCConversion();

    /** Update the current ADC channel value */
    void updateADCChannel(uint8_t channelIndex, uint16_t adcValue);
public:
    /** Update data from the ADC.
     * This method is intended to be called from an interrupt and
     * aims for low execution time. */
    void updateADC();
};

#endif /* PS9530_CTRL_H */
#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "dac.h"
#include "kbd.h"
#include "hal_spi.h"
#include "ps_display.h"

#define PORT_MUX PORTC
#define DDR_MUX DDRC
#define PIN_MUX PIN4
#define MASK_MUX _BV(PIN_MUX)

#define PORT_CTRL PORTD
#define PIN_CTRL PIND
#define DDR_CTRL DDRD
#define MASK_CTRL _BV(PIN7)

#define TEMP1_LOWER_LIMIT 80
#define TEMP1_UPPER_LIMIT 100
#define TEMP2_LOWER_LIMIT 50
#define TEMP2_UPPER_LIMIT 60

#include "ps9530_ctrl.h"

PS9530_Ctrl PS9530_Ctrl::instance;

PS9530_Ctrl &PS9530_Ctrl::getInstance() {
    return instance;
}

PS9530_Ctrl::PS9530_Ctrl():
    standbyMode(true),
    rawDACValue{0, 0},
    currentMuxChannel(muxChannel_voltage),
    overTempMode(0),
    rawADCMeasurements{0, 0, 0, 0},
    currentADCState(adcChannel__idle << 1)
{
}

void PS9530_Ctrl::init() {
    spi_init();
    dac_init();
    kbd_init();

    /* Initialite MUX selector pin */
    DDR_MUX |= MASK_MUX;

    /* Initialize control input pin */
    DDR_CTRL &= ~MASK_CTRL;
    PORT_CTRL &= ~MASK_CTRL;

    /* We set the timer interrupt to about 100Hz.
     * Clear-Timer-on-Compare (CTC) with fCPU/1024 and OCRA = 155
     * => f = 16MHZ/1024/156 = 100Hz */
    TCCR0A = (1 << WGM01) | (0 << WGM00);
    TCCR0B = (0 << WGM02) | (1 << CS02) | (0 << CS01) | (1 << CS00);
    OCR0A = 155;
    TIMSK0 = (1 << OCIE0A);
    TIFR0 = (1 << OCIE0A);
}

KeyCode PS9530_Ctrl::readKeycode() {
    return kbd_remove();
}

void PS9530_Ctrl::update() {
    kbd_update();
    updateDAC();
    /* Every second update cycle, start an ADC cycle */
    adcCycleFlag ^= 1;
    if (adcCycleFlag) {
        startADCCycle();
    }
}

void PS9530_Ctrl::toggleStandbyMode() {
    standbyMode = !standbyMode;
}

bool PS9530_Ctrl::isStandbyEnabled() const {
    // Standby can be enabled externally or due to overtemperature detection
    return (standbyMode || isOvertemp());
}

void PS9530_Ctrl::updateDAC() {
    /* Alternate between both channels */
    switch (currentMuxChannel) {
    case muxChannel_voltage: {
        /* Select voltage channel on MUX */
        PORT_MUX &= ~MASK_MUX;
        dac_set_unsafe(isStandbyEnabled() ? 0 : rawDACValue[0]);
        currentMuxChannel = muxChannel_current;
        break;
    }
    case muxChannel_current:
        /* Select voltage channel on MUX */
        PORT_MUX |= MASK_MUX;
        dac_set_unsafe(isStandbyEnabled() ? 0 : rawDACValue[1]);
        currentMuxChannel = muxChannel_voltage;
        break;
    }
}

const uint16_t PS9530_Ctrl::adcVoltageOffset[33] PROGMEM = 
{0, 1269, 2319, 3388, 4448, 5502, 6556, 7608, 8670, 9723,
10782, 11838, 12896, 13963, 15017, 16064, 17133, 18181, 19246, 20281,
21340, 22394, 23446, 24512, 25567, 26627, 27691, 28756, 29821, 31060,
32298, 33537, 34776};
const uint16_t PS9530_Ctrl::adcVoltageOffsetSmall[9] PROGMEM = 
{0, 350, 472, 604, 738, 869, 1004, 1131, 1269};
const uint16_t PS9530_Ctrl::adcCurrentOffset[33] PROGMEM = 
{167, 472, 790, 1097, 1404, 1710, 1997, 2305, 2612, 2917,
3221, 3526, 3833, 4139, 4445, 4753, 5060, 5368, 5674, 5980,
6285, 6591, 6899, 7207, 7515, 7825, 8138, 8448, 8755, 9030,
9332, 9649, 10027};
const uint16_t PS9530_Ctrl::adcCurrentOffsetSmall[9] PROGMEM = 
{167, 209, 242, 280, 318, 357, 395, 433, 472};
const uint16_t PS9530_Ctrl::voltageDACOffset[33] PROGMEM = 
{0, 485, 1039, 1594, 2148, 2701, 3255, 3850, 4359, 4906,
5455, 6003, 6554, 7104, 7652, 8203, 8753, 9302, 9850, 10403,
10957, 11507, 12058, 12609, 13159, 13709, 14257, 14806, 15356, 15905,
16383, 16383, 16383};
const uint16_t PS9530_Ctrl::voltageDACOffsetSmall[9] PROGMEM = 
{0, 0, 68, 134, 208, 278, 347, 416, 485};
const uint16_t PS9530_Ctrl::currentDACOffset[33] PROGMEM = 
{145, 878, 1617, 2366, 3140, 3876, 4625, 5374, 6125, 6874,
7619, 8365, 9114, 9862, 10606, 11349, 12088, 12834, 13648, 14374,
15145, 15921, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383,
16383, 16383, 16383};
const uint16_t PS9530_Ctrl::currentDACOffsetSmall[9] PROGMEM = 
{0, 0, 68, 134, 208, 278, 347, 416, 485};
const uint16_t PS9530_Ctrl::minTempADC[2] PROGMEM = {550, 484};
const uint16_t PS9530_Ctrl::maxTempADC[2] PROGMEM = {860, 1195};
const int16_t PS9530_Ctrl::tempOffset[2][12] PROGMEM = {
    {150, 123, 101, 81, 61, 41, 21, 1, -18, -39, -62, -86},
    {150, 116, 91, 70, 50, 32, 16, 0, -13, -27, -40, -53},
};

void PS9530_Ctrl::setMilliVoltSetpoint(uint16_t milliVolts) {
    voltageSetpointMilliVolts = milliVolts;
    updateLimits();
}

void PS9530_Ctrl::setMilliAmpsLimit(uint16_t milliAmpere) {
    currentLimitMilliAmps = milliAmpere;
    updateLimits();
}

void PS9530_Ctrl::setCentiWattLimit(uint16_t centiWatts) {
    powerLimitCentiWatt = centiWatts;
    updateLimits();
}

void PS9530_Ctrl::updateLimits() {
    uint16_t actualCurrentLimitMilliAmps;

    /* Both the current and the power limit define a maximum current.
    * Set the one that actuall implies the lower current limit.
    */
    if (powerLimitCentiWatt*10UL>(uint32_t)voltageSetpointMilliVolts*currentLimitMilliAmps/1000UL) {
        /* The current limit is more limiting */
        actualCurrentLimitMilliAmps = currentLimitMilliAmps;
        currentLimitReason = CurrentLimitFromCurrent;
    } else {
        /* The power limit is more limiting */
        actualCurrentLimitMilliAmps = powerLimitCentiWatt*10000UL/voltageSetpointMilliVolts;
        currentLimitReason = CurrentLimitFromPower;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rawDACValue[muxChannel_voltage] = interpolateDACVoltage(voltageSetpointMilliVolts);
        rawDACValue[muxChannel_current] = interpolateDACCurrent(actualCurrentLimitMilliAmps);
    }
}

uint16_t PS9530_Ctrl::getMilliVoltsMeasurement() const {
    uint16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        result = interpolateADCVoltage(rawADCMeasurements[adcChannel_voltage]);
    }
    return result;
}

uint16_t PS9530_Ctrl::getMilliAmpsMeasurement() const {
    uint16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        result = interpolateADCCurrent(rawADCMeasurements[adcChannel_current]);
    }
    return result;
}

uint16_t PS9530_Ctrl::getCentiWattsMeasurement() const {
    const uint16_t milliVolts = getMilliVoltsMeasurement();
    const uint16_t milliAmpere = getMilliAmpsMeasurement();
    const uint16_t centiWattsMeasurement = (uint32_t)milliVolts*milliAmpere/10000UL;
    return centiWattsMeasurement;
}

int16_t PS9530_Ctrl::getTemperature1() const {
    int16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        result = interpolateADCTemp(tempSensor_1, rawADCMeasurements[adcChannel_temp1], PS9530_ADC_TEMP1_SHIFT);
    }
    return result;
}

int16_t PS9530_Ctrl::getTemperature2() const {
    int16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        result = interpolateADCTemp(tempSensor_2, rawADCMeasurements[adcChannel_temp2], PS9530_ADC_TEMP2_SHIFT);
    }
    return result;
}

bool PS9530_Ctrl::isOvertemp() const {
    return overTempMode != 0;
}

template <typename InputType,
          typename OutputType,
          typename IntermediateType = OutputType>
OutputType interpolate(InputType value, const OutputType offsetTable[] PROGMEM, unsigned int shift) {
    const size_t tableIndex = value >> shift;
    const OutputType rest = value & (~((~0)<<shift));
    const OutputType start = pgm_read_word(&offsetTable[tableIndex]);
    const OutputType end = pgm_read_word(&offsetTable[tableIndex+1]);
    const IntermediateType gradient = end-start;
    return start + ((rest * gradient)>>shift);
}

uint16_t PS9530_Ctrl::interpolateADCVoltage(uint16_t adcValue) {
    if (adcValue < PS9530_ADC_VOLTAGE_SMALL) {
        return interpolate<uint16_t, uint16_t, uint32_t>(adcValue, adcVoltageOffsetSmall, PS9530_ADC_VOLTAGE_SHIFT_SMALL);
    } else {
        return interpolate<uint16_t, uint16_t, uint32_t>(adcValue, adcVoltageOffset, PS9530_ADC_VOLTAGE_SHIFT);
    }
}

uint16_t PS9530_Ctrl::interpolateADCCurrent(uint16_t adcValue) {
    if (adcValue < PS9530_ADC_CURRENT_SMALL) {
        return interpolate<uint16_t, uint16_t, uint32_t>(adcValue, adcCurrentOffsetSmall, PS9530_ADC_CURRENT_SHIFT_SMALL);
    } else {
        return interpolate<uint16_t, uint16_t, uint32_t>(adcValue, adcCurrentOffset, PS9530_ADC_CURRENT_SHIFT);
    }
}

uint16_t PS9530_Ctrl::interpolateDACVoltage(uint16_t milliVolts) {
    if (milliVolts < PS9530_DAC_VOLTAGE_SMALL) {
        return interpolate<uint16_t, uint16_t, uint32_t>(milliVolts, voltageDACOffsetSmall, PS9530_DAC_VOLTAGE_SHIFT_SMALL);
    } else {
        return interpolate<uint16_t, uint16_t, uint32_t>(milliVolts, voltageDACOffset, PS9530_DAC_VOLTAGE_SHIFT);
    }
}

uint16_t PS9530_Ctrl::interpolateDACCurrent(uint16_t milliAmps) {
    if (milliAmps < PS9530_DAC_CURRENT_SMALL) {
        return interpolate<uint16_t, uint16_t, uint32_t>(milliAmps, currentDACOffsetSmall, PS9530_DAC_CURRENT_SHIFT_SMALL);
    } else {
        return interpolate<uint16_t, uint16_t, uint32_t>(milliAmps, currentDACOffset, PS9530_DAC_CURRENT_SHIFT);
    }
}

int16_t PS9530_Ctrl::interpolateADCTemp(uint8_t index, uint16_t adcValue, uint8_t shift) {
    const uint16_t minADC = pgm_read_word(&minTempADC[index]);
    const uint16_t maxADC = pgm_read_word(&maxTempADC[index]);
    if (adcValue < minADC) {
        adcValue = minADC;
    } else if (adcValue > maxADC) {
        adcValue = maxADC;
    }
    const uint16_t adcOffset = adcValue - minADC;
    return interpolate(adcOffset, tempOffset[index], shift);
}

void PS9530_Ctrl::updateOvertemperature() {
    int16_t temp1 = getTemperature1();
    if (overTempMode & overTempMode_1) {
        // We are in overtemperature for temperature 1
        if (temp1 < TEMP1_LOWER_LIMIT) {
            // Temperature has fallen again
            overTempMode &= ~overTempMode_1;
        }
    } else if (temp1 > TEMP1_UPPER_LIMIT) {
        // We have exceeded the upper temperature limit 1
        overTempMode |= overTempMode_1;
    }
    int16_t temp2 = getTemperature2();
    if (overTempMode & overTempMode_2) {
        // We are in overtemperature for temperature 2
        if (temp2 < TEMP2_LOWER_LIMIT) {
            // Temperature has fallen again
            overTempMode &= ~overTempMode_2;
        }
    } else if (temp2 > TEMP2_UPPER_LIMIT) {
        // We have exceeded the upper temperature limit 2
        overTempMode |= overTempMode_2;
    }
}

PS9530_Ctrl::LimitingMode PS9530_Ctrl::getLimitingMode() {
    return getLimitingMode((PIN_CTRL & MASK_CTRL)!=0);
}

PS9530_Ctrl::LimitingMode PS9530_Ctrl::getLimitingMode(bool currentLimiterPin) {
    if (isStandbyEnabled()) {
        return LimitingMode_Inactive;
    } else if (!currentLimiterPin) {
        return LimitingMode_Voltage;
    } else if (currentLimitReason==CurrentLimitFromCurrent) {
        return LimitingMode_Current;
    } else {
        return LimitingMode_Power;
    }
}

void PS9530_Ctrl::startADCCycle() {
    currentADCState = adcChannel_voltage << 1;
    startADCConversion();
}

void PS9530_Ctrl::startADCConversion() {
    const uint8_t currentADCChannel = currentADCState >> 1;
    if (currentADCChannel < adcChannel__idle) {
        /* Set reference voltage to AREF and select channel */
        ADMUX = (0 << REFS1) | (0 << REFS0) | (currentADCChannel << MUX0);
        /* Start the conversion with a clock of fCPU/64 (ca. 125kHz) in free-running mode. */
        ADCSRB = 0;
        ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADIF) | _BV(ADIE) | _BV(ADATE) | (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0);
    } else {
        /* Deactivate ADC */
        ADCSRA = 0;
    }
}

void PS9530_Ctrl::updateADCChannel(uint8_t channelIndex, uint16_t newValue) {
    /* Apply a filter with a cut-off-frequency of ca. 20Hz.
        * We use the filter concept
        *   y[t+1] = alpha*x[t+1] + (1-alpha)*y[t]
        * where x[t] is the sequence of measurements,
        *       y[t] is the sequence of filtered values, and
        *       alpha is the smoothing factor with
        *   alpha= dT/(T+dT)
        * where dT is the sampling time (1/f), and
        *        T is the filtering time constant.
        * 
        * With dT=1/50 s and T = 1/20s we get alpha=2/7, which is about 1/4.
        * 
        * We use a scale factor of 4, as the ADC values are in the range [0;8192],
        * and this multiplied by 4 gives a maximum range of [0;32767], which
        * still fits into a 16-bit unsigned integer. Also, using factor 8
        * does not improve the accuracy of alpha, as alpha*8 rounded to the next
        * integer is 2, so that the factor still remains 1/4.
        */
    const uint16_t oldValue = rawADCMeasurements[channelIndex];
    const uint16_t filteredValue = (1U*newValue+3U*oldValue)>>2;
    rawADCMeasurements[channelIndex] = filteredValue;
    /* Advance to the next channel */
    currentADCState++;
}

void PS9530_Ctrl::updateADC() {
    if (currentADCState & 1) {
        /* We just performed an actual measurement */
        const uint8_t currentADCChannel = currentADCState >> 1;
        switch (currentADCChannel) {
        case adcChannel_voltage:
        case adcChannel_current:
            /* Perform oversampling */
            oversamplingADCSum += ADC;
            if (++oversamplingADCCount>=64) {
                /* We have collected all samples */
                updateADCChannel(currentADCChannel, oversamplingADCSum>>3);
                oversamplingADCCount = 0;
                oversamplingADCSum = 0;
            }
            break;
        default:
            updateADCChannel(currentADCChannel, ADC);
            break;
        }
    } else {
        /* We just performed the dummy measurement. */
        currentADCState++;
    }
    startADCConversion();
    if (currentADCState>=2*adcChannel__idle) {
        /* We have a full measurement cycle */
        updateOvertemperature();
    }
}

struct __arduino_savepin {
    uint8_t pin;
    int value;
    uint8_t todo;
};

static inline struct __arduino_savepin __arduino_savepin(uint8_t pin) {
    struct __arduino_savepin savepin = {
        .pin = pin,
        .value = digitalRead(pin),
        .todo = 1
    };
    return savepin;
}

static inline void __arduino_restorepin(struct __arduino_savepin *savepin) {
    digitalWrite(savepin->pin, savepin->value);
}

#define ARDUINO_SAVEPIN(pin) for (struct __arduino_savepin __save __attribute__((__cleanup__(__arduino_restorepin))) = __arduino_savepin(pin); __save.todo; __save.todo = 0)

ISR(TIMER0_COMPA_vect) {
    /* Wait until any running SPI transaction is done for sure.
     * I deplore waiting in an interrupt, but this is the easiest way.
     * Waiting for the SPI interrupt flag would be better in general,
     * but would block if no SPI transaction is running and the
     * interrupt flag had been cleared. */
    PORTD |= _BV(PIN7);
    _delay_loop_2(8 * 8);

    ARDUINO_SAVEPIN(TFT_CS) {
        /* Pull up the TFT_CS pin to deactivate the display connection */
        digitalWrite(TFT_CS, HIGH);

        ISOLATE_SPI {
            /* Update the controller */
            PS9530_Ctrl::getInstance().update();
        }
        /* Initiate sending a dummy over the SPI so that the
         * interrupt flag is set for anybody who has been sending.
         * This is pretty hacky, but it might just as well work.
         */
        SPDR = 42;
        spi_wait_until_tx_complete();
    }
    PORTD &= ~_BV(PIN7);
}

ISR(ADC_vect) {
    PS9530_Ctrl::getInstance().updateADC();
}
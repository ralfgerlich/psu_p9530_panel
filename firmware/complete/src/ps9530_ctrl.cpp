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
    startADCCycle();
}

void PS9530_Ctrl::updateADC() {
    if (currentADCState & 1) {
        /* We just did the actual measurement */
        const uint8_t currentADCChannel = currentADCState >> 1;
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
         * With dT=1/100 s and T = 1/20s we get alpha=1/6, which is about 11/64.
         * 
         * We use a scale factor of 64, as the ADC values are in the range [0;1023],
         * and this multiplied by 64 gives a maximum range of [0;64449], which
         * still fits into a 16-bit unsigned integer.
         */
        const uint16_t newValue = ADC;
        const uint16_t oldValue = rawADCMeasurements[currentADCChannel];
        const uint16_t filteredValue = (11U*newValue+53U*oldValue)>>6;
        rawADCMeasurements[currentADCChannel] = filteredValue;
    }
    /* Update the state */
    currentADCState++;
    startADCConversion();
    if (currentADCState>=2*adcChannel__idle) {
        /* We have a full measurement cycle */
        updateOvertemperature();
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

void PS9530_Ctrl::startADCCycle() {
    currentADCState = adcChannel_voltage << 1;
    startADCConversion();
}

void PS9530_Ctrl::startADCConversion() {
    const uint8_t currentADCChannel = currentADCState >> 1;
    if (currentADCChannel < adcChannel__idle) {
        /* Set reference voltage to AREF and select channel */
        ADMUX = (0 << REFS1) | (0 << REFS0) | (currentADCChannel << MUX0);
        /* Start the conversion with a clock of fCPU/64 (ca. 125kHz) */
        ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADIF) | _BV(ADIE) | (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0);
    } else {
        /* Deactivate ADC */
        ADCSRA = 0;
    }
}

const uint16_t PS9530_Ctrl::adcVoltageOffset[33] PROGMEM = 
{27, 1250, 2333, 3390, 4480, 5509, 6565, 7612, 8680, 9750,
10797, 11857, 12915, 13980, 15047, 16103, 17160, 18225, 19272, 20300,
21367, 22419, 23467, 24516, 25580, 26700, 27743, 28800, 29850, 31450,
33050, 34650, 36250};
const uint16_t PS9530_Ctrl::adcCurrentOffset[33] PROGMEM =
{3, 386, 691, 1000, 1311, 1614, 1924, 2240, 2539, 2841,
3145, 3453, 3761, 4068, 4364, 4668, 4981, 5286, 5588, 5896,
6207, 6521, 6825, 7127, 7428, 7733, 8041, 8349, 8657, 8960,
9263, 9580, 9912};
const uint16_t PS9530_Ctrl::voltageDACOffset[33] PROGMEM =
{0, 481, 1035, 1590, 2147, 2699, 3252, 3805, 4339, 4895,
5458, 6043, 6546, 7100, 7650, 8202, 8756, 9311, 9864, 10426,
10983, 11528, 12083, 12643, 13202, 13751, 14268, 14825, 15379, 15955,
16332, 16383, 16383};
const uint16_t PS9530_Ctrl::currentDACOffset[33] PROGMEM =
{0, 893, 1644, 2393, 3142, 3889, 4638, 5388, 6140, 6901,
7651, 8396, 9147, 9896, 10647, 11398, 12155, 12908, 13652, 14406,
15169, 15933, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383,
16383, 16383, 16383};
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


uint16_t PS9530_Ctrl::interpolateADCVoltage(uint16_t adcValue) {
    const uint16_t tableIndex = adcValue >> PS9530_ADC_VOLTAGE_SHIFT;
    const uint8_t adcRest = adcValue & (~((~0)<<PS9530_ADC_VOLTAGE_SHIFT));
    const uint16_t start = pgm_read_word(&adcVoltageOffset[tableIndex]);
    const uint16_t end = pgm_read_word(&adcVoltageOffset[tableIndex+1]);
    const uint16_t gradient = end-start;
    return start + ((adcRest * gradient) >> PS9530_ADC_VOLTAGE_SHIFT);
}

uint16_t PS9530_Ctrl::interpolateADCCurrent(uint16_t adcValue) {
    const uint16_t tableIndex = adcValue >> PS9530_ADC_CURRENT_SHIFT;
    const uint8_t adcRest = adcValue & (~((~0)<<PS9530_ADC_CURRENT_SHIFT));
    const uint16_t start = pgm_read_word(&adcCurrentOffset[tableIndex]);
    const uint16_t end = pgm_read_word(&adcCurrentOffset[tableIndex+1]);
    const uint16_t gradient = end-start;
    return start + ((adcRest * gradient) >> PS9530_ADC_CURRENT_SHIFT);
}

uint16_t PS9530_Ctrl::interpolateDACVoltage(uint16_t milliVolts) {
    const uint16_t tableIndex = milliVolts >> PS9530_DAC_VOLTAGE_SHIFT;
    const uint16_t adcRest = milliVolts & (~((~0)<<PS9530_DAC_VOLTAGE_SHIFT));
    const uint16_t start = pgm_read_word(&voltageDACOffset[tableIndex]);
    const uint16_t end = pgm_read_word(&voltageDACOffset[tableIndex+1]);
    const uint32_t gradient = end-start;
    return start + ((adcRest * gradient) >> PS9530_DAC_VOLTAGE_SHIFT);
}

uint16_t PS9530_Ctrl::interpolateDACCurrent(uint16_t milliAmps) {
    const uint16_t tableIndex = milliAmps >> PS9530_DAC_CURRENT_SHIFT;
    const uint16_t adcRest = milliAmps & (~((~0)<<PS9530_DAC_CURRENT_SHIFT));
    const uint16_t start = pgm_read_word(&currentDACOffset[tableIndex]);
    const uint16_t end = pgm_read_word(&currentDACOffset[tableIndex+1]);
    const uint32_t gradient = end-start;
    return start + ((adcRest * gradient) >> PS9530_DAC_CURRENT_SHIFT);
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
    const uint8_t tableIndex = adcOffset >> shift;
    const int16_t adcRest = adcOffset - (tableIndex << shift);
    const int16_t start = pgm_read_word(&tempOffset[index][tableIndex]);
    const int16_t end = pgm_read_word(&tempOffset[index][tableIndex+1]);
    const int16_t gradient = end-start;
    return start + (int16_t(adcRest * gradient) >> shift);
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
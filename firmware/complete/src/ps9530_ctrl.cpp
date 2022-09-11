#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "dac.h"
#include "kbd.h"
#include "spi.h"
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
    OCR0A = 49;
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
        rawADCMeasurements[currentADCChannel] = ADC;
    }
    /* Update the state */
    currentADCState++;
    startADCConversion();
    if (currentADCState>=2*adcChannel__idle) {
        /* We have a full measurement cycle */
        updateOvertemperature();
    }
}

void PS9530_Ctrl::setStandbyMode(bool standbyMode) {
    standbyMode = standbyMode;
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

void PS9530_Ctrl::setMilliVoltSetpoint(uint16_t milliVolts) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rawDACValue[muxChannel_voltage] = interpolateDACVoltage(milliVolts);
    }
}

void PS9530_Ctrl::setMilliAmpsLimit(uint16_t milliAmpere) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rawDACValue[muxChannel_current] = interpolateDACCurrent(milliAmpere);
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

int16_t PS9530_Ctrl::getTemperature1() const {
    int16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        result = interpolateADCTemp(tempSensor_1, rawADCMeasurements[adcChannel_temp1]);
    }
    return result;
}

int16_t PS9530_Ctrl::getTemperature2() const {
    int16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        result = interpolateADCTemp(tempSensor_2, rawADCMeasurements[adcChannel_temp2]);
    }
    return result;
}

bool PS9530_Ctrl::isOvertemp() const {
    return overTempMode != 0;
}

#define PS9530_ADC_VOLTAGE_SHIFT 5
#define PS9530_ADC_CURRENT_SHIFT 5
#define PS9530_DAC_VOLTAGE_SHIFT 10
#define PS9530_DAC_CURRENT_SHIFT 9
const uint16_t PS9530_Ctrl::adcVoltageOffset[32] PROGMEM = 
{37, 1165, 2192, 3260, 4320, 5405, 6472, 7509, 8569, 9636,
10690, 11718, 12785, 13873, 14903, 15954, 17022, 18079, 19137, 20213,
21301, 22328, 23351, 24400, 25473, 26505, 27534, 28655, 29736, 30746,
31686, 32555};
const uint16_t PS9530_Ctrl::adcVoltageGradient[32] PROGMEM = 
{1128, 1027, 1068, 1060, 1085, 1067, 1037, 1060, 1067, 1054,
1028, 1067, 1088, 1030, 1051, 1068, 1057, 1058, 1076, 1088,
1027, 1023, 1049, 1073, 1032, 1029, 1121, 1081, 1010, 940,
869, 799};
const uint16_t PS9530_Ctrl::adcCurrentOffset[32] PROGMEM = 
{2, 374, 695, 990, 1302, 1617, 1921, 2227, 2537, 2841,
3138, 3441, 3752, 4063, 4371, 4669, 4968, 5276, 5587, 5897,
6203, 6505, 6812, 7122, 7427, 7730, 8033, 8338, 8646, 8957,
9265, 9545};
const uint16_t PS9530_Ctrl::adcCurrentGradient[32] PROGMEM = 
{372, 321, 295, 312, 315, 304, 306, 310, 304, 297,
303, 311, 311, 308, 298, 299, 308, 311, 310, 306,
302, 307, 310, 305, 303, 303, 305, 308, 311, 308,
280, 316};
const uint16_t PS9530_Ctrl::voltageDACOffset[32] PROGMEM = 
{0, 540, 1091, 1639, 2190, 2742, 3289, 3840, 4393, 4944,
5492, 6043, 6595, 7143, 7693, 8245, 8792, 9343, 9895, 10444,
10984, 11524, 12083, 12644, 13202, 13760, 14324, 14874, 15393, 15935,
16383, 16383};
const uint16_t PS9530_Ctrl::voltageDACGradient[32] PROGMEM = 
{540, 551, 548, 551, 552, 547, 551, 553, 551, 548,
551, 552, 548, 550, 552, 547, 551, 552, 549, 540,
540, 559, 561, 558, 558, 564, 550, 519, 542, 448,
0, 0};
const uint16_t PS9530_Ctrl::currentDACOffset[32] PROGMEM = 
{0, 874, 1626, 2373, 3126, 3876, 4624, 5374, 6123, 6871,
7624, 8385, 9133, 9882, 10631, 11366, 12127, 12877, 13637, 14391,
15139, 15888, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383,
16383, 16383};
const uint16_t PS9530_Ctrl::currentDACGradient[32] PROGMEM = 
{874, 752, 747, 753, 750, 748, 750, 749, 748, 753,
761, 748, 749, 749, 735, 761, 750, 760, 754, 748,
749, 495, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0};
const uint16_t PS9530_Ctrl::minTempADC[2] PROGMEM = {550, 485};
const uint16_t PS9530_Ctrl::maxTempADC[2] PROGMEM = {860, 1194};
const uint8_t PS9530_Ctrl::shiftTempADC[2] PROGMEM = {5, 6};
const int16_t PS9530_Ctrl::tempOffset[2][11] PROGMEM = {
    {150, 124, 102, 81, 61, 42, 22, 2, -19, -40, -55},
    {150, 116, 91, 70, 50, 32, 16, 1, -14, -28, -41},
};
const int16_t PS9530_Ctrl::tempGradient[2][11] PROGMEM = {
    {-26, -22, -21, -20, -19, -20, -20, -21, -21, -15, 0},
    {-34, -25, -21, -20, -18, -16, -15, -15, -14, -13, -13},
};

uint16_t PS9530_Ctrl::interpolateADCVoltage(uint16_t adcValue) {
    const uint16_t tableIndex = adcValue >> PS9530_ADC_VOLTAGE_SHIFT;
    const uint8_t adcRest = adcValue & (~((~0)<<PS9530_ADC_VOLTAGE_SHIFT));
    const uint16_t base = pgm_read_word(&adcVoltageOffset[tableIndex]);
    const uint16_t gradient = pgm_read_word(&adcVoltageGradient[tableIndex]);
    return base + ((adcRest * gradient) >> PS9530_ADC_VOLTAGE_SHIFT);
}

uint16_t PS9530_Ctrl::interpolateADCCurrent(uint16_t adcValue) {
    const uint16_t tableIndex = adcValue >> PS9530_ADC_CURRENT_SHIFT;
    const uint8_t adcRest = adcValue & (~((~0)<<PS9530_ADC_CURRENT_SHIFT));
    const uint16_t base = pgm_read_word(&adcCurrentOffset[tableIndex]);
    const uint16_t gradient = pgm_read_word(&adcCurrentGradient[tableIndex]);
    return base + ((adcRest * gradient) >> PS9530_ADC_CURRENT_SHIFT);
}

uint16_t PS9530_Ctrl::interpolateDACVoltage(uint16_t milliVolts) {
    const uint16_t tableIndex = milliVolts >> PS9530_DAC_VOLTAGE_SHIFT;
    const uint8_t milliAmpsRest = milliVolts & (~((~0)<<PS9530_DAC_VOLTAGE_SHIFT));
    const uint16_t base = pgm_read_word(&voltageDACOffset[tableIndex]);
    const uint16_t gradient = pgm_read_word(&voltageDACGradient[tableIndex]);
    return base + ((milliAmpsRest * gradient) >> PS9530_DAC_VOLTAGE_SHIFT);
}

uint16_t PS9530_Ctrl::interpolateDACCurrent(uint16_t milliAmps) {
    const uint16_t tableIndex = milliAmps >> PS9530_DAC_CURRENT_SHIFT;
    const uint8_t milliAmpsRest = milliAmps & (~((~0)<<PS9530_DAC_CURRENT_SHIFT));
    const uint16_t base = pgm_read_word(&currentDACOffset[tableIndex]);
    const uint16_t gradient = pgm_read_word(&currentDACGradient[tableIndex]);
    return base + ((milliAmpsRest * gradient) >> PS9530_DAC_CURRENT_SHIFT);
}

int16_t PS9530_Ctrl::interpolateADCTemp(uint8_t index, uint16_t adcValue) {
    const uint16_t minADC = pgm_read_word(&minTempADC[index]);
    const uint16_t maxADC = pgm_read_word(&maxTempADC[index]);
    const uint8_t shift = pgm_read_byte(&shiftTempADC[index]);
    if (adcValue < minADC) {
        adcValue = minADC;
    } else if (adcValue > maxADC) {
        adcValue = maxADC;
    }
    const uint16_t adcOffset = adcValue - minADC;
    const uint8_t tableIndex = adcOffset >> shift;
    const int16_t adcRest = adcOffset - (tableIndex << shift);
    const int16_t base = pgm_read_word(&tempOffset[index][tableIndex]);
    const int16_t gradient = pgm_read_word(&tempGradient[index][tableIndex]);
    return base + (int16_t(adcRest * gradient) >> shift);
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

PS9530_Ctrl::LimitingMode PS9530_Ctrl::getCurrentLimitingMode() {
    if (PIN_CTRL & MASK_CTRL) {
        return LimitingMode_Current;
    } else {
        return LimitingMode_Voltage;
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
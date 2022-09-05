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

#include "ps9530_ctrl.h"

PS9530_Ctrl PS9530_Ctrl::instance;

PS9530_Ctrl& PS9530_Ctrl::getInstance() {
    return instance;
}

PS9530_Ctrl::PS9530_Ctrl():
    milliVoltSetpoint(0U),
    milliAmpsLimit(0U),
    currentMuxChannel(muxChannel_voltage),
    milliVoltsMeasurement(0),
    milliAmpsMeasurement(0),
    tempDegCMeasurement{25, 25},
    currentADCChannel(adcChannel__idle),
    measurementsAvailable(0)
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
    TCCR0A = (1<<WGM01)|(0<<WGM00);
    TCCR0B = (0<<WGM02)|(1<<CS02)|(0<<CS01)|(1<<CS00);
    OCR0A = 49;
    TIMSK0 = (1<<OCIE0A);
    TIFR0 = (1<<OCIE0A);
}

void PS9530_Ctrl::setMilliVoltSetpoint(uint16_t milliVolts) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        milliVoltSetpoint = milliVolts;
    }
}

void PS9530_Ctrl::setMilliAmpsLimit(uint16_t milliAmpere) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        milliAmpsLimit = milliAmpere;
    }
}

uint16_t PS9530_Ctrl::getMilliVoltsMeasurement(bool clearFlag) {
    uint16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        result = milliVoltsMeasurement;
        if (clearFlag) {
            measurementsAvailable &= ~measurementVoltage;
        }
    }
    return result;
}

uint16_t PS9530_Ctrl::getMilliAmpsMeasurement(bool clearFlag) {
    uint16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        result = milliAmpsMeasurement;
        if (clearFlag) {
            measurementsAvailable &= ~measurementVoltage;
        }
    }
    return result;
}

KeyCode PS9530_Ctrl::readKeycode() {
    return kbd_remove();
}

void PS9530_Ctrl::update() {
    kbd_update();
    updateDAC();
    startADCCycle();
}

void PS9530_Ctrl::updateDAC() {
    /* Alternate between both channels */
    switch (currentMuxChannel) {
    case muxChannel_voltage: {
        /* Select voltage channel on MUX */
        PORT_MUX &= ~MASK_MUX;
        // TODO: Use proper calibration table
        uint32_t dac_value = milliVoltSetpoint;
        dac_value <<= 14UL;
        dac_value /= 30000UL;
        dac_set_unsafe(dac_value);
        currentMuxChannel = muxChannel_current;
        break;
    }
    case muxChannel_current:
        PORT_MUX |= MASK_MUX;
        // TODO: Use proper calibration table
        uint32_t dac_value = milliAmpsLimit;
        dac_value <<= 14UL;
        dac_value /= 10000UL;
        dac_set_unsafe(dac_value);
        currentMuxChannel = muxChannel_voltage;
        break;
    }
}

void PS9530_Ctrl::startADCCycle() {
    currentADCChannel = adcChannel_voltage;
    startADCConversion();
}

void PS9530_Ctrl::startADCConversion() {
    if (currentADCChannel!=adcChannel__idle) {
        /* Set reference voltage to AREF and select channel */
        ADMUX = (0<<REFS1)|(0<<REFS0)|(currentADCChannel<<MUX0);
        /* Start the conversion with a clock of fCPU/64 (ca. 125kHz) */
        ADCSRA = _BV(ADEN)|_BV(ADSC)|_BV(ADIF)|_BV(ADIE)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);
    } else {
        /* Deactivate ADC */
        ADCSRA = 0;
    }
}

const uint16_t PS9530_Ctrl::adcVoltageOffset[32] PROGMEM = {
     0,  1034,  2068,  3102,  4136,  5170,  6204,  7238,  8272,  9306, 10340, 11374,
 12408, 13442, 14476, 15510, 16544, 17578, 18612, 19646, 20680, 21714, 22748, 23782,
 24816, 25850, 26884, 27918, 28952, 29986, 31020, 32054
};
const uint16_t PS9530_Ctrl::adcVoltageGradient[32] PROGMEM = {
  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034, 
  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034, 
  1034,  1034,  1034,  1034,  1034,  1034,  1034,  1034
};

uint16_t PS9530_Ctrl::interpolateADCVoltage(uint16_t adcValue) {
    const uint16_t tableIndex = adcValue >> 5;
    const uint8_t adcRest = adcValue & 31;
    const uint16_t base = pgm_read_word(&adcVoltageOffset[tableIndex]);
    const uint16_t gradient = pgm_read_word(&adcVoltageGradient[tableIndex]);
    return base + ((adcRest * gradient)>>5);
}

const uint16_t PS9530_Ctrl::adcCurrentOffset[32] PROGMEM = {
    0,  317,  635,  952, 1269, 1587, 1904, 2221, 2539, 2856, 3173, 3491, 3808, 4125,
 4443, 4760, 5077, 5395, 5712, 6029, 6347, 6664, 6981, 7299, 7616, 7933, 8251, 8568,
 8885, 9203, 9520, 9838,
};
const uint16_t PS9530_Ctrl::adcCurrentGradient[32] PROGMEM = {
  317,  318,  317,  317,  318,  317,  317,  318,  317,  317,  318,  317,  317,  318,
  317,  317,  318,  317,  317,  318,  317,  317,  318,  317,  317,  318,  317,  317,
  318,  317,  318,  317
};

uint16_t PS9530_Ctrl::interpolateADCCurrent(uint16_t adcValue) {
    const uint16_t tableIndex = adcValue >> 5;
    const uint8_t adcRest = adcValue & 31;
    const uint16_t base = pgm_read_word(&adcCurrentOffset[tableIndex]);
    const uint16_t gradient = pgm_read_word(&adcCurrentGradient[tableIndex]);
    return base + ((adcRest * gradient)>>5);
}

int16_t PS9530_Ctrl::interpolateADCTemp(uint8_t index, uint16_t adcValue) {
    const uint16_t minADC = pgm_read_word(&minTempADC[index]);
    const uint16_t maxADC = pgm_read_word(&maxTempADC[index]);
    const uint8_t shift = pgm_read_byte(&shiftTempADC[index]);
    if (adcValue<minADC) {
        adcValue = minADC;
    } else if (adcValue>maxADC) {
        adcValue = maxADC;
    }
    const uint16_t adcOffset = adcValue - minTempADC[index];
    const uint8_t tableIndex = adcOffset >> shift;
    const uint16_t adcRest = adcOffset - (tableIndex << shift);
    const int16_t base = pgm_read_word(&tempOffset[index][tableIndex]);
    const int16_t gradient = pgm_read_word(&tempGradient[index][tableIndex]);
    return base + ((adcRest * gradient)>>shift);
}

const uint16_t PS9530_Ctrl::minTempADC[2] PROGMEM = { 488, 375 };
const uint16_t PS9530_Ctrl::maxTempADC[2] PROGMEM = { 821, 1030};
const uint8_t PS9530_Ctrl::shiftTempADC[2] PROGMEM = { 5, 6 };

const int16_t PS9530_Ctrl::tempOffset[2][11] PROGMEM = {
    {150, 124, 102,  82,  64,  45,  27,   9,  -9, -28, -47},
    {150, 112,  84,  60,  39,  21,   4, -11, -26, -39, -52}
};
const int16_t PS9530_Ctrl::tempGradient[2][11] PROGMEM = {
    {-26, -22, -20, -18, -19, -18, -18, -18, -19, -19},
    {-38, -28, -24, -21, -18, -17, -15, -15, -13, -13}
};

void PS9530_Ctrl::updateADC() {
    switch (currentADCChannel) {
    case adcChannel_voltage:
        milliVoltsMeasurement = interpolateADCVoltage(ADC);
        measurementsAvailable |= measurementVoltage;
    case adcChannel_current:
        milliAmpsMeasurement = interpolateADCCurrent(ADC);
        measurementsAvailable |= measurementCurrent;
        currentADCChannel = adcChannel_temp1;
        break;
    case adcChannel_temp1:
        tempDegCMeasurement[0] = interpolateADCTemp(tempSensor_1, ADC);
        currentADCChannel = adcChannel_temp2;
        break;
    case adcChannel_temp2:
        tempDegCMeasurement[1] = interpolateADCTemp(tempSensor_2, ADC);
        currentADCChannel = adcChannel__idle;
        break;
    default:
        /* Do nothing */
        break;
    }
    startADCConversion();
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

static inline void __arduino_restorepin(struct __arduino_savepin* savepin) {
    digitalWrite(savepin->pin, savepin->value);
}

#define ARDUINO_SAVEPIN(pin) for (struct __arduino_savepin __save __attribute__((__cleanup__(__arduino_restorepin))) = __arduino_savepin(pin); __save.todo; __save.todo=0)

ISR(TIMER0_COMPA_vect) {
    /* Wait until any running SPI transaction is done for sure.
     * I deplore waiting in an interrupt, but this is the easiest way.
     * Waiting for the SPI interrupt flag would be better in general,
     * but would block if no SPI transaction is running and the
     * interrupt flag had been cleared. */
    PORTD |= _BV(PIN7);
    _delay_loop_2(8*8);

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
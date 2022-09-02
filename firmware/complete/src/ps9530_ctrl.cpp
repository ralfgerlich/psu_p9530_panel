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
    currentADCChannel(adcChannel_voltage),
    measurementsAvailable(0)
{
}

void PS9530_Ctrl::init() {
    spi_init();
    dac_init();
    kbd_init();

    /* Initialite MUX selector pin */
    DDR_MUX |= MASK_MUX;

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
        // TODO: Use proper calibration table
        result = (milliVoltsMeasurement * 30000UL)>>10UL;
        if (clearFlag) {
            measurementsAvailable &= ~measurementVoltage;
        }
    }
    return result;
}

uint16_t PS9530_Ctrl::getMilliAmpsMeasurement(bool clearFlag) {
    uint16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // TODO: Use proper calibration table
        result = (milliAmpsMeasurement * 10000UL)>>10UL;
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
    startADCConversion();
}

void PS9530_Ctrl::updateDAC() {
    // TODO: include temperature measurements
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

void PS9530_Ctrl::startADCConversion() {
    /* Set reference voltage to AREF and select channel */
    ADMUX = (0<<REFS1)|(0<<REFS0)|(currentADCChannel<<MUX0);
    /* Start the conversion with a clock of fCPU/64 (ca. 125kHz) */
    ADCSRA = _BV(ADEN)|_BV(ADSC)|_BV(ADIF)|_BV(ADIE)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);
}

void PS9530_Ctrl::updateADC() {
    switch (currentADCChannel) {
    case adcChannel_voltage:
        // TODO: Use proper calibration table
        milliVoltsMeasurement = ADC * 30000UL / 1024UL;
        measurementsAvailable |= measurementVoltage;
    default:
        currentADCChannel = adcChannel_current;
        break;
    case adcChannel_current:
        // TODO: Use proper calibration table
        milliAmpsMeasurement = ADC * 10000UL / 1024UL;
        measurementsAvailable |= measurementCurrent;
        currentADCChannel = adcChannel_voltage;
        break;
    }
    startADCConversion();
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
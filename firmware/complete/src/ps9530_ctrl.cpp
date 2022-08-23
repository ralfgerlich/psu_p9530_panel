#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "dac.h"
#include "kbd.h"
#include "spi.h"

#define PORT_MUX PORTC
#define DDR_MUX DDRC
#define PIN_MUX PIN4
#define MASK_MUX _BV(PIN_MUX)

    // _delay_ms(5);
    // mux_select_channel(mux_channel_voltage);
    // dac_set(voltage_value);
    // _delay_ms(1); /* Wait for the sample-and-hold element to settle */
    // voltage_value += 128;
    // if (voltage_value>=16384) {
    //     voltage_value=0;
    // }
    // mux_select_channel(mux_channel_current);
    // current_value -=128;
    // if (current_value==0) {
    //     current_value=16384;
    // }
    // dac_set(current_value);
    // _delay_ms(1); /* Wait for the sample-and-hold element to settle */
#include "ps9530_ctrl.h"

PS9530_Ctrl PS9530_Ctrl::instance;

PS9530_Ctrl& PS9530_Ctrl::getInstance() {
    return instance;
}

PS9530_Ctrl::PS9530_Ctrl():
    milliVoltSetpoint(0U),
    milliAmpsLimit(0U),
    currentMuxChannel(muxChannel_voltage),
    counter50Hz(0),
    milliVoltsMeasurement(0),
    milliAmpsMeasurement(0),
    currentADCChannel(adcChannel_voltage)
{
}

void PS9530_Ctrl::init() {
    spi_init();
    dac_init();
    kbd_init();

    /* Initialite MUX selector pin */
    DDR_MUX |= MASK_MUX;
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

uint16_t PS9530_Ctrl::getMilliVoltsMeasurement() const {
    uint16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // TODO: Use proper calibration table
        result = (milliVoltsMeasurement * 30000UL)>>10UL;
    }
    return result;
}

uint16_t PS9530_Ctrl::getMilliAmpsMeasurement() const {
    uint16_t result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // TODO: Use proper calibration table
        result = (milliVoltsMeasurement * 10000UL)>>10UL;
    }
    return result;
}

KeyCode PS9530_Ctrl::readKeycode() {
    return kbd_remove();
}

void PS9530_Ctrl::update5kHz() {
    kbd_update();
    if (++counter50Hz>=100) {
        counter50Hz = 0;
        update50Hz();
    }
}

void PS9530_Ctrl::update50Hz() {
    /* We have to update the DAC at reduced speed to allow the
     * sample-and-hold element to follow.
     */
    updateDAC();
    //startADCConversion();
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
        dac_set(dac_value);
        currentMuxChannel = muxChannel_current;
        break;
    }
    case muxChannel_current:
        PORT_MUX |= MASK_MUX;
        // TODO: Use proper calibration table
        uint32_t dac_value = milliAmpsLimit;
        dac_value <<= 14UL;
        dac_value /= 10000UL;
        dac_set(dac_value);
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
    default:
        currentADCChannel = adcChannel_current;
        break;
    case adcChannel_current:
        // TODO: Use proper calibration table
        milliAmpsMeasurement = ADC * 10000UL / 1024UL;
        currentADCChannel = adcChannel_voltage;
        break;
    }
    startADCConversion();
}
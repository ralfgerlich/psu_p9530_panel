/* adc.cpp - Definitions for the ADC interface
 * Copyright (c) 2022, Ralf Gerlich
 */
#include "adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>

uint16_t adc_results[adc_channel__count];

static uint8_t adc_next_channel;

static void adc_start_next_conversion() {
    /* Set reference voltage to AREF and select channel */
    ADMUX = (0<<REFS1)|(0<<REFS0)|(adc_next_channel<<MUX0);
    /* Start the conversion with a clock of fCPU/64 (ca. 125kHz) */
    ADCSRA = _BV(ADEN)|_BV(ADSC)|_BV(ADIF)|_BV(ADIE)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);
}

void adc_init() {
    adc_next_channel = adc_channel_voltage;
    adc_start_next_conversion();
}

ISR(ADC_vect) {
    /* Read the conversion result */
    adc_results[adc_next_channel] = ADC; 
    /* Advance to the next channel */
    adc_next_channel++;
    if (adc_next_channel>=adc_channel__count) {
        adc_next_channel = adc_channel_voltage;
    }
    /* Start the next conversion */
    adc_start_next_conversion();
}
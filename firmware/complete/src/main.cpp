#include <Arduino.h>
#include <util/delay.h>
#include "adc.h"
#include "dac.h"
#include "kbd.h"
#include "mux.h"
#include "spi.h"

void setup() {
    Serial.begin(57600);
    spi_init();
    dac_init();
    kbd_init();
    mux_init();
    adc_init();
    sei();
}

uint16_t voltage_value = 0;
uint16_t current_value = 16384;

void loop() {
    kbd_code_t code;
    kbd_update();
    while ((code = kbd_remove()) != kbd_none) {
        Serial.println(code);
    }
    _delay_ms(5);
    mux_select_channel(mux_channel_voltage);
    dac_set(voltage_value);
    _delay_ms(1); /* Wait for the sample-and-hold element to settle */
    voltage_value += 128;
    if (voltage_value>=16384) {
        voltage_value=0;
    }
    mux_select_channel(mux_channel_current);
    current_value -=128;
    if (current_value==0) {
        current_value=16384;
    }
    dac_set(current_value);
    _delay_ms(1); /* Wait for the sample-and-hold element to settle */
    Serial.println();
    Serial.println(adc_results[adc_channel_voltage]);
}
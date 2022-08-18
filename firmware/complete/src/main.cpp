#include <Arduino.h>
#include <util/delay.h>
#include "dac.h"
#include "kbd.h"
#include "spi.h"

void setup() {
    Serial.begin(57600);
    spi_init();
    dac_init();
    kbd_init();
}

uint16_t value = 0;

void loop() {
    kbd_code_t code;
    kbd_update();
    while ((code = kbd_remove()) != kbd_none) {
        Serial.println(code);
    }
    _delay_ms(5);
    dac_set(value);
    value += 512;
    if (value>=16384) {
        value=0;
    }
}
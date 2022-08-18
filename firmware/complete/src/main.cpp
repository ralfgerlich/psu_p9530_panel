#include <Arduino.h>
#include <util/delay.h>
#include "kbd.h"

void setup() {
    Serial.begin(57600);
    kbd_init();
}

void loop() {
    kbd_code_t code;
    kbd_update();
    while ((code = kbd_remove()) != kbd_none) {
        Serial.println(code);
    }
    _delay_ms(5);
}
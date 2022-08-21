#include <Arduino.h>
#include <util/delay.h>
#include "dac.h"
#include "kbd.h"
#include "spi.h"
#include "ps_display.h"

#define TFT_DC 6
#define TFT_CS 5

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
PsDisplay display = PsDisplay(tft);

void setup() {
    Serial.begin(57600);
    spi_init();
    dac_init();
    kbd_init();
    display.clear();
    display.renderLogo();
    delay(1000);
    display.clear();
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
    display.renderMainscreen();
}
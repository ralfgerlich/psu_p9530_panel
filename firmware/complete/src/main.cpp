#include <Arduino.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "dac.h"
#include "kbd.h"
#include "spi.h"
#include "ps9530_ctrl.h"
#include "ps_display.h"
#include "ps9530_ui.h"

#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
PsDisplay display = PsDisplay(tft);
PS9530_UI ui(PS9530_Ctrl::getInstance(), display);

void setup() {
    /* Activate debug LED */
    DDRD |= _BV(PIN7);
    Serial.begin(57600);
    ui.init();
    Serial.println("Ready");
}

void loop() {
    ui.update();
}

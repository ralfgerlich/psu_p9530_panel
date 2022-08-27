#include <Arduino.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "dac.h"
#include "kbd.h"
#include "spi.h"
#include "ps9530_ctrl.h"
#include "ps_display.h"

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
PsDisplay display = PsDisplay(tft);

void setup() {
    /* Activate debug LED */
    DDRD |= _BV(PIN7);
    Serial.begin(57600);
    PS9530_Ctrl::getInstance().init();
    cli();
    display.clear();
    display.renderLogo();
    delay(1000);
    display.clear();
    sei();
    Serial.println("Ready");
}

static uint16_t voltageSetpoint = 0;
static uint16_t currentSetpoint = 10000;

void loop() {
    KeyCode code;
    while ((code = PS9530_Ctrl::getInstance().readKeycode()) != kbd_none) {
        Serial.print("keycode=");
        Serial.println(code);
        switch (code) {
        case kbd_enc_cw:
            voltageSetpoint += 100;
            if (voltageSetpoint>=30000) {
                voltageSetpoint = 30000;
            }
            if (currentSetpoint<=100) {
                currentSetpoint = 0;
            } else {
                currentSetpoint -= 100;
            }
            PS9530_Ctrl::getInstance().setMilliVoltSetpoint(voltageSetpoint);
            display.setMilliVoltsSetpoint(voltageSetpoint);
            PS9530_Ctrl::getInstance().setMilliAmpsLimit(currentSetpoint);
            display.setMilliAmpsLimit(currentSetpoint);
            break;
        case kbd_enc_ccw:
            if (voltageSetpoint<=100) {
                voltageSetpoint = 0;
            } else {
                voltageSetpoint -= 100;
            }
            currentSetpoint += 100;
            if (currentSetpoint>=10000) {
                currentSetpoint = 10000;
            }
            PS9530_Ctrl::getInstance().setMilliVoltSetpoint(voltageSetpoint);
            display.setMilliVoltsSetpoint(voltageSetpoint);
            PS9530_Ctrl::getInstance().setMilliAmpsLimit(currentSetpoint);
            display.setMilliAmpsLimit(currentSetpoint);
            break;
        }
        Serial.print("voltageSetpoint=");
        Serial.println(voltageSetpoint);
        Serial.print("currentSetpoint=");
        Serial.println(currentSetpoint);
    }
    display.renderMainscreen();
}

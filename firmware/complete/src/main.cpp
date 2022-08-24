#include <Arduino.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "kbd.h"
#include "ps9530_ctrl.h"
#include "ps_display.h"

#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
PsDisplay display = PsDisplay(tft);

static void timer_init();

void setup() {
    Serial.begin(57600);
    PS9530_Ctrl::getInstance().init();
    timer_init();
    //display.clear();
    //display.renderLogo();
    //delay(1000);
    //display.clear();
}

static uint16_t voltageSetpoint = 0;
static uint16_t currentSetpoint = 10000;

void loop() {
    KeyCode code;
    while ((code = PS9530_Ctrl::getInstance().readKeycode()) != kbd_none) {
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
        Serial.println(voltageSetpoint);
        Serial.println(currentSetpoint);
        Serial.println(PS9530_Ctrl::getInstance().getMilliVoltsMeasurement());
        Serial.println(PS9530_Ctrl::getInstance().getMilliAmpsMeasurement());
    }
    //display.renderMainscreen();
}

/** Initialize the timer interrupt */
static void timer_init() {
    /* We set the timer interrupt to about 100Hz.
     * Clear-Timer-on-Compare (CTC) with fCPU/1024 and OCRA = 155
     * => f = 16MHZ/1024/156 = 100Hz */
    TCCR0A = (1<<WGM01)|(0<<WGM00);
    TCCR0B = (0<<WGM02)|(1<<CS02)|(0<<CS01)|(1<<CS00);
    OCR0A = 49;
    TIMSK0 = (1<<OCIE0A);
    TIFR0 = (1<<OCIE0A);
}

ISR(TIMER0_COMPA_vect) {
#if 0
    /* Wait for SPI to finish */
    loop_until_bit_is_set(SPSR, SPIF);
    /* Save SPI configuration and the status of the TFT chip-select.
     * The Contents of the SPI Data Register can be ignored, as the
     * TFT does not use the MISO pin anyway. */
    const uint8_t bu_spcr = SPCR;
    const uint8_t bu_spsr = SPSR;
    const auto bu_tft_cs = digitalRead(TFT_CS);

    /* Pull up the TFT_CS pin */
    digitalWrite(TFT_CS, HIGH);
#endif
    /* Update the controller */
    PS9530_Ctrl::getInstance().update();
#if 0
    /* Restore the SPI configuration and the TFT chip-select */
    SPCR = bu_spcr;
    SPSR = bu_spsr;
    digitalWrite(TFT_CS, bu_tft_cs);
#endif
}

ISR(ADC_vect) {
    PS9530_Ctrl::getInstance().updateADC();
}
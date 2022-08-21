#include "ps_display.h"
#include "toolbox_logo.xbm"
#include "Fonts/FreeMono18pt7b.h"
#include "Fonts/FreeMonoBold18pt7b.h"

PsDisplay::PsDisplay( Adafruit_ILI9341 & tft) :
    tft(tft) {
}

void PsDisplay::init() {
    if (!init_done) {
        tft.begin();
        tft.setRotation(3);
        init_done = true;
    }
}

void PsDisplay::clear() {
    init();
    tft.fillScreen(ILI9341_BLACK);
    yield();
}

void PsDisplay::renderLogo() {
    tft.drawXBitmap((PS_DISPLAY_WIDTH-toolbox_logo_width)/2, (PS_DISPLAY_HEIGHT-toolbox_logo_height)/2,
#if defined(__AVR__) || defined(ESP8266)
        toolbox_logo_bits,
#else
        // Some non-AVR MCU's have a "flat" memory model and don't
        // distinguish between flash and RAM addresses.  In this case,
        // the RAM-resident-optimized drawRGBBitmap in the ILI9341
        // library can be invoked by forcibly type-converting the
        // PROGMEM bitmap pointer to a non-const uint16_t *.
        (uint16_t *)toolbox_logo_bits,
#endif
        toolbox_logo_width, toolbox_logo_height,
        ILI9341_WHITE);
}

void PsDisplay::formatMilliNumber(char * buffer, int value, char unit) {
    sprintf(buffer, "%2d.%02d%c", value/1000, (value%1000)/10, unit);
}

void PsDisplay::formatCentiNumber(char * buffer, int value, char unit) {
    sprintf(buffer, "%3d.%1d%c", value/100, (value%100)/10, unit);
}

void PsDisplay::fastStringPrint(char * buffer, char * old_buffer, int font_width) {
    for (int i=0; i<8; i++) {
        if (old_buffer[i] != buffer[i]) {
            tft.setTextColor(ILI9341_BLACK);
            int cx = tft.getCursorX();
            int cy = tft.getCursorY();
            tft.print(old_buffer[i]);
            tft.setCursor(cx, cy);
            tft.setTextColor(ILI9341_WHITE);
            tft.print(buffer[i]);
            old_buffer[i] = buffer[i];
        } else {
            tft.setCursor(tft.getCursorX()+font_width, tft.getCursorY());
        }
    }
}

void PsDisplay::renderMainscreen() {
    //layout is:
    //---------------------
    // voltage setpoint 24px
    // VOLTAGE 48px
    // amps limit 24px
    // AMPS 48px
    // watts limit 24px
    // WATTS 48px
    // additional info 24px
    //---------------------
    char buffer[8];
    //font init
    tft.setTextWrap(0);
    tft.setFont(&FreeMonoBold18pt7b);
    //actual paint
    tft.setTextSize(1);
    if (painted_standby != standby) {
        painted_standby = standby;
        if (painted_standby) {
            tft.setTextColor(ILI9341_RED);
        } else {
            tft.setTextColor(ILI9341_BLACK);
        }
        tft.setCursor(10, PT18_IN_PX+5);
        tft.print("Standby");
    }
    if (painted_limited_a != limited_a) {
        painted_limited_a = limited_a;
        if (painted_limited_a) {
            tft.setTextColor(ILI9341_RED);
        } else {
            tft.setTextColor(ILI9341_BLACK);
        }
        tft.setCursor(10, PT18_IN_PX*4+5*2);
        tft.print("Limited");
    }
    if (painted_limited_p != limited_p) {
        painted_limited_p = limited_p;
        if (painted_limited_p) {
            tft.setTextColor(ILI9341_RED);
        } else {
            tft.setTextColor(ILI9341_BLACK);
        }
        tft.setCursor(10, PT18_IN_PX*7+5*3);
        tft.print("Limited");
    }
    painted_locked = locked;
    painted_memory = memory;
    painted_remote = remote;
    yield();
    //optimized hybrid
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setFont(&FreeMono18pt7b);
    tft.setCursor(60+21*6, PT18_IN_PX+5);
    formatMilliNumber(buffer, milli_volts_setpoint, 'V');
    fastStringPrint(buffer, buffer_volts_setp, 21);
    tft.setCursor(60+21*6, PT18_IN_PX*4+5*2);
    formatMilliNumber(buffer, milli_amps_limit, 'A');
    fastStringPrint(buffer, buffer_amps_limit, 21);
    tft.setCursor(60+21*6, PT18_IN_PX*7+5*3);
    formatCentiNumber(buffer, centi_watts_limit, 'W');
    fastStringPrint(buffer, buffer_watts_limit, 21);
    tft.setTextSize(2);
    tft.setCursor(60, PT18_IN_PX*3+5);
    formatMilliNumber(buffer, milli_volts, 'V');
    fastStringPrint(buffer, buffer_volts, 21*2);
    tft.setCursor(60, PT18_IN_PX*6+5*2);
    formatMilliNumber(buffer, milli_amps, 'A');
    fastStringPrint(buffer, buffer_amps, 21*2);
    tft.setCursor(60, PT18_IN_PX*9+5*3);
    formatCentiNumber(buffer, centi_watts, 'W');
    fastStringPrint(buffer, buffer_watts, 21*2);
    yield();
}

void PsDisplay::setStandby(bool standby) {
    this->standby = standby;
}

void PsDisplay::setLocked(bool locked) {
    this->locked = locked;
}

void PsDisplay::setMemory(bool memory) {
    this->memory = memory;
}

void PsDisplay::setRemote(bool remote) {
    this->remote = remote;
}

void PsDisplay::setLimitedA(bool limited_a) {
    this->limited_a = limited_a;
}

void PsDisplay::setLimitedP(bool limited_p) {
    this->limited_p = limited_p;
}

void PsDisplay::setOvertemp(bool overtemp) {
    this->overtemp = overtemp;
}

void PsDisplay::setMVoltsSetpoint(int voltage_setpoint) {
    this->milli_volts_setpoint = voltage_setpoint;
}

void PsDisplay::setMAmpsLimit(int amps_limit) {
    this->milli_amps_limit = amps_limit;
}

void PsDisplay::setCWattsLimit(int watts_limit) {
    this->centi_watts_limit = watts_limit;
}

void PsDisplay::setMVolts(int voltage) {
    this->milli_volts = voltage;
}

void PsDisplay::setMAmps(int amps) {
    this->milli_amps = amps;
}

void PsDisplay::setCWatts(int watts) {
    this->centi_watts = watts;
}

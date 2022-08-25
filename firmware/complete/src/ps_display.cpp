#include "ps_display.h"
#include "toolbox_logo_only.xbm"
#include "toolbox_logo_font_toolbox.xbm"
#include "toolbox_logo_font_bodensee.xbm"
#include "Fonts/FreeMono18pt7b.h"
#include "Fonts/FreeMonoBold18pt7b.h"

//manually reduced blue a bit especially in the red ones
//looks like the display overdoes blue a bit
//16bit RGB565
#define TOOLBOX_LOGO_LIGHT_GREY 0x630B
#define TOOLBOX_LOGO_DARK_GREY 0x3985
#define TOOLBOX_LOGO_LIGHT_RED 0xE901
#define TOOLBOX_LOGO_MEDIUM_RED 0xC100
#define TOOLBOX_LOGO_DARK_RED 0x9080

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

void PsDisplay::drawXBitmapPartial(int16_t x, int16_t y, const uint8_t bitmap[], int16_t sw, int16_t sh,
                               int16_t w, int16_t h, uint16_t color) {

  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t b = 0;

  tft.startWrite();
  for (int16_t j = sh; j < h; j++, y++) {
    for (int16_t i = sw; i < w; i++) {
      if (i & 7)
        b >>= 1;
      else
        b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      // Nearly identical to drawBitmap(), only the bit order
      // is reversed here (left-to-right = LSB to MSB):
      if (b & 0x01)
        tft.writePixel(x + i, y, color);
    }
  }
  tft.endWrite();
}

void PsDisplay::renderLogo() {
    tft.drawXBitmap(0, (PS_DISPLAY_HEIGHT-toolbox_logo_only_height)/2,
#if defined(__AVR__) || defined(ESP8266)
        toolbox_logo_only_bits,
#else
        // Some non-AVR MCU's have a "flat" memory model and don't
        // distinguish between flash and RAM addresses.  In this case,
        // the RAM-resident-optimized drawRGBBitmap in the ILI9341
        // library can be invoked by forcibly type-converting the
        // PROGMEM bitmap pointer to a non-const uint16_t *.
        (uint16_t *)toolbox_logo_only_bits,
#endif
        toolbox_logo_only_width, toolbox_logo_only_height/3,
        TOOLBOX_LOGO_LIGHT_RED);
    uint8_t pixel_per_iteration = toolbox_logo_only_height/3*2/11;
    for (uint8_t i = 0; i<11; i++) {
        //red upper part 11101 0x1d << 0xB
        //red middle part 11000 0x18 << 0xB
        //red lower part 10010 0x12 << 0xB
        //green bit 8 stuck on 0x100 or 0x8 << 5
        //blue bit 0 on light
        uint16_t color = 0x100 | ((0x1c - i) << 0xB);
        //looping 10 times to get from the bright red to the dark red + 12th time to render the rest dark
        drawXBitmapPartial(0, (PS_DISPLAY_HEIGHT-toolbox_logo_only_height)/2+toolbox_logo_only_height/3+pixel_per_iteration*i,
    #if defined(__AVR__) || defined(ESP8266)
            toolbox_logo_only_bits,
    #else
            // Some non-AVR MCU's have a "flat" memory model and don't
            // distinguish between flash and RAM addresses.  In this case,
            // the RAM-resident-optimized drawRGBBitmap in the ILI9341
            // library can be invoked by forcibly type-converting the
            // PROGMEM bitmap pointer to a non-const uint16_t *.
            (uint16_t *)toolbox_logo_only_bits,
    #endif
            0, toolbox_logo_only_height/3+pixel_per_iteration*i,
            toolbox_logo_only_width, toolbox_logo_only_height/3+pixel_per_iteration*(i+1),
            color);
    }
    drawXBitmapPartial(0, (PS_DISPLAY_HEIGHT-toolbox_logo_only_height)/2+toolbox_logo_only_height/3+pixel_per_iteration*11,
    #if defined(__AVR__) || defined(ESP8266)
            toolbox_logo_only_bits,
    #else
            // Some non-AVR MCU's have a "flat" memory model and don't
            // distinguish between flash and RAM addresses.  In this case,
            // the RAM-resident-optimized drawRGBBitmap in the ILI9341
            // library can be invoked by forcibly type-converting the
            // PROGMEM bitmap pointer to a non-const uint16_t *.
            (uint16_t *)toolbox_logo_only_bits,
    #endif
            0, toolbox_logo_only_height/3+pixel_per_iteration*11,
            toolbox_logo_only_width, toolbox_logo_only_height,
            TOOLBOX_LOGO_DARK_RED);
    tft.drawXBitmap(PS_DISPLAY_WIDTH-toolbox_logo_font_toolbox_width-1, (PS_DISPLAY_HEIGHT-toolbox_logo_only_height)/2 + 12,
#if defined(__AVR__) || defined(ESP8266)
        toolbox_logo_font_toolbox_bits,
#else
        // Some non-AVR MCU's have a "flat" memory model and don't
        // distinguish between flash and RAM addresses.  In this case,
        // the RAM-resident-optimized drawRGBBitmap in the ILI9341
        // library can be invoked by forcibly type-converting the
        // PROGMEM bitmap pointer to a non-const uint16_t *.
        (uint16_t *)toolbox_logo_font_toolbox_bits,
#endif
        toolbox_logo_font_toolbox_width, toolbox_logo_font_toolbox_height,
        TOOLBOX_LOGO_LIGHT_GREY);
    tft.drawXBitmap(PS_DISPLAY_WIDTH-toolbox_logo_font_bodensee_width-1, (PS_DISPLAY_HEIGHT-toolbox_logo_only_height)/2 + 68,
#if defined(__AVR__) || defined(ESP8266)
        toolbox_logo_font_bodensee_bits,
#else
        // Some non-AVR MCU's have a "flat" memory model and don't
        // distinguish between flash and RAM addresses.  In this case,
        // the RAM-resident-optimized drawRGBBitmap in the ILI9341
        // library can be invoked by forcibly type-converting the
        // PROGMEM bitmap pointer to a non-const uint16_t *.
        (uint16_t *)toolbox_logo_font_bodensee_bits,
#endif
        toolbox_logo_font_bodensee_width, toolbox_logo_font_bodensee_height,
        TOOLBOX_LOGO_DARK_GREY);
}

void PsDisplay::formatNumber(char * buffer, char * format, int16_t value_a, int16_t value_b, row_t row) {
    char unit;
    if (row == ROW_VOLTS) {
        unit = 'V';
    } else if (row == ROW_AMPS) {
        unit = 'A';
    } else {
        unit = 'W';
    }
    sprintf(buffer, format, value_a, value_b, unit);
}

void PsDisplay::formatMilliNumber(char * buffer, int16_t value, row_t row, bool zero_padding) {
    char format[12];
    if (zero_padding) {
        strcpy(format, "%02d.%02d%c");
    } else {
        strcpy(format, "%2d.%02d%c");
    }
    formatNumber(buffer, format, value/1000, (value%1000)/10, row);
}

void PsDisplay::formatCentiNumber(char * buffer, int16_t value, row_t row, bool zero_padding) {
    char format[11];
    if (zero_padding) {
        strcpy(format, "%03d.%1d%c");
    } else {
        strcpy(format, "%3d.%1d%c");
    }
    formatNumber(buffer, format, value/100, (value%100)/10, row);
}

void PsDisplay::fastStringPrint(char * buffer, char * old_buffer, uint8_t font_width, row_t row, uint16_t fg_color, uint16_t se_color, uint16_t bg_color) {
    for (uint8_t i=0; i<PS_DISPLAY_BUFFER_LENGTH-1; i++) {
        uint16_t char_color = fg_color;
        bool force_paint = false;
        if (row != ROW_NULL && selected_pos != painted_selected_pos) {
            // valid row, changed since last time
            if ((selected_pos & 0x0f) == i && (selected_pos >> 4) == row) {
                //correct highlight position, correct highlight row
                char_color = se_color;
                force_paint = true;
            } else if ((painted_selected_pos & 0x0f) == i && (painted_selected_pos >> 4) == row) {
                //old highlight position, old highlight row
                force_paint = true;
            }
        }
        if (old_buffer[i] != buffer[i] || force_paint) {
            tft.setTextColor(bg_color);
            int16_t cx = tft.getCursorX();
            int16_t cy = tft.getCursorY();
            tft.print(old_buffer[i]);
            tft.setCursor(cx, cy);
            tft.setTextColor(char_color);
            tft.print(buffer[i]);
            old_buffer[i] = buffer[i];
        } else {
            // manually forward the curser by one character width
            tft.setCursor(tft.getCursorX()+font_width, tft.getCursorY());
        }
    }
}

void PsDisplay::paintStandby(bool visible) {
    painted_standby = visible;
    if (painted_standby) {
        tft.setTextColor(ILI9341_RED);
    } else {
        tft.setTextColor(ILI9341_BLACK);
    }
    tft.setCursor(10, PT18_IN_PXH+5);
    tft.print("Standby");
}

void PsDisplay::paintOvertemp(bool visible) {
    painted_overtemp = overtemp;
    if (painted_standby) {
        paintStandby(false);
    }
    if (painted_overtemp) {
        tft.setTextColor(ILI9341_RED);
    } else {
        tft.setTextColor(ILI9341_BLACK);
    }
    tft.setCursor(10, PT18_IN_PXH+5);
    tft.print("Overtemp");
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
    char buffer[PS_DISPLAY_BUFFER_LENGTH];
    //font init
    tft.setTextWrap(0);
    tft.setFont(&FreeMonoBold18pt7b);
    //actual paint
    tft.setTextSize(1);
    if (painted_overtemp != overtemp) {
        paintOvertemp(overtemp);
    }
    if (!painted_overtemp && painted_standby != standby) {
        paintStandby(standby);
    }
    if (painted_limited_a != limited_a) {
        painted_limited_a = limited_a;
        if (painted_limited_a) {
            tft.setTextColor(ILI9341_RED);
        } else {
            tft.setTextColor(ILI9341_BLACK);
        }
        tft.setCursor(10, PT18_IN_PXH*4+5*2);
        tft.print("Limited");
    }
    if (painted_limited_p != limited_p) {
        painted_limited_p = limited_p;
        if (painted_limited_p) {
            tft.setTextColor(ILI9341_RED);
        } else {
            tft.setTextColor(ILI9341_BLACK);
        }
        tft.setCursor(10, PT18_IN_PXH*7+5*3);
        tft.print("Limited");
    }
    yield();
    //optimized hybrid
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setFont(&FreeMono18pt7b);
    //TODO improvement
    //     speed can be improved by reducing overdraw.
    //     if we render the bg color char and the new char in memory
    //     and only send the resulting pixels without bg color to the display
    tft.setCursor(60+PT18_IN_PXW*6, PT18_IN_PXH+5);
    formatMilliNumber(buffer, milli_volts_setpoint, ROW_VOLTS, true);
    fastStringPrint(buffer, buffer_volts_setp, PT18_IN_PXW, ROW_VOLTS);
    tft.setCursor(60+PT18_IN_PXW*6, PT18_IN_PXH*4+5*2);
    formatMilliNumber(buffer, milli_amps_limit, ROW_AMPS, true);
    fastStringPrint(buffer, buffer_amps_limit, PT18_IN_PXW, ROW_AMPS);
    tft.setCursor(60+PT18_IN_PXW*6, PT18_IN_PXH*7+5*3);
    formatCentiNumber(buffer, centi_watts_limit, ROW_WATTS, true);
    fastStringPrint(buffer, buffer_watts_limit, PT18_IN_PXW, ROW_WATTS);
    painted_selected_pos = selected_pos;
    tft.setTextSize(2);
    tft.setCursor(60, PT18_IN_PXH*3+5);
    formatMilliNumber(buffer, milli_volts, ROW_VOLTS);
    fastStringPrint(buffer, buffer_volts, PT18_IN_PXW*2, ROW_NULL);
    tft.setCursor(60, PT18_IN_PXH*6+5*2);
    formatMilliNumber(buffer, milli_amps, ROW_AMPS);
    fastStringPrint(buffer, buffer_amps, PT18_IN_PXW*2, ROW_NULL);
    tft.setCursor(60, PT18_IN_PXH*9+5*3);
    formatCentiNumber(buffer, centi_watts, ROW_WATTS);
    fastStringPrint(buffer, buffer_watts, PT18_IN_PXW*2, ROW_NULL);
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

void PsDisplay::setLimitedV(bool limited_v) {
    this->limited_v = limited_v;
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

void PsDisplay::setMilliVoltsSetpoint(int16_t voltage_setpoint) {
    this->milli_volts_setpoint = voltage_setpoint;
}

void PsDisplay::setMilliAmpsLimit(int16_t amps_limit) {
    this->milli_amps_limit = amps_limit;
}

void PsDisplay::setCentiWattsLimit(int16_t watts_limit) {
    this->centi_watts_limit = watts_limit;
}

void PsDisplay::setMilliVolts(int16_t voltage) {
    this->milli_volts = voltage;
}

void PsDisplay::setMilliAmps(int16_t amps) {
    this->milli_amps = amps;
}

void PsDisplay::setCentiWatts(int16_t watts) {
    this->centi_watts = watts;
}

void PsDisplay::setCurser(row_t row, uint8_t pos) {
    this->selected_pos = row << 4 | pos;
    if (pos >= 3 || (row != ROW_WATTS && pos >= 2)) {
        //correct position for decimal '.'
        this->selected_pos++;
    }
}

#include "ps_display.h"
#include "Toolbox/toolbox_logo_only.xbm"
#include "Toolbox/toolbox_logo_only_small.xbm"
#include "Toolbox/toolbox_logo_font_toolbox.xbm"
#include "Toolbox/toolbox_logo_font_bodensee.xbm"
#include "Fonts/courier-prime-code-regular.h"

//display state flags
#define PS_DISPLAY_STATE_LOCKED 0b00000001
#define PS_DISPLAY_STATE_MEMORY 0b00000010
#define PS_DISPLAY_STATE_REMOTE 0b00000100
#define PS_DISPLAY_STATE_STANDBY 0b00001000
#define PS_DISPLAY_STATE_LIMITED_V 0b00010000
#define PS_DISPLAY_STATE_LIMITED_A 0b00100000
#define PS_DISPLAY_STATE_LIMITED_P 0b01000000
#define PS_DISPLAY_STATE_OVERTEMP 0b10000000

//i hate to write it like this but functions as macros get better optimized
#define setState(value, mask) this->state = value ? this->state | mask : this->state & mask;
#define setPaintedState(value, mask) this->painted_state = value ? this->painted_state | mask : this->painted_state & mask;
#define isState(mask) (this->state & mask)
#define isPaintedState(mask) (this->painted_state & mask)

PsDisplay::PsDisplay( Adafruit_ILI9341 & tft) :
    tft(tft) {
}

void PsDisplay::init() {
    if (!init_done) {
        tft.begin();
        tft.setRotation(3);
        tft.setTextWrap(0);
        tft.setFont(&courier_prime_code_regular18pt7b);
        init_done = true;
    }
}

void PsDisplay::clear() {
    init();
    tft.fillScreen(DEFAULT_BACKGROUND_COLOR);
    painted_state = 0;
    painted_selected_pos = 0;
    const char clear_string[PS_DISPLAY_BUFFER_LENGTH] = {0};
    strncpy(buffer_volts, clear_string, PS_DISPLAY_BUFFER_LENGTH);
    strncpy(buffer_volts_setp, clear_string, PS_DISPLAY_BUFFER_LENGTH);
    strncpy(buffer_amps, clear_string, PS_DISPLAY_BUFFER_LENGTH);
    strncpy(buffer_amps_limit, clear_string, PS_DISPLAY_BUFFER_LENGTH);
    strncpy(buffer_watts, clear_string, PS_DISPLAY_BUFFER_LENGTH);
    strncpy(buffer_watts_limit, clear_string, PS_DISPLAY_BUFFER_LENGTH);
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
    paintLogo(0, (PS_DISPLAY_HEIGHT-toolbox_logo_only_height)/2, toolbox_logo_only_width, toolbox_logo_only_height, toolbox_logo_only_bits);
    //render toolbox bodensee e.v. logo text
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

void PsDisplay::paintLogo(uint8_t x, uint8_t y, uint16_t size_x, uint16_t size_y, const unsigned char* picture, uint16_t color_override) {
    tft.drawXBitmap(x, y,
#if defined(__AVR__) || defined(ESP8266)
        picture,
#else
        // Some non-AVR MCU's have a "flat" memory model and don't
        // distinguish between flash and RAM addresses.  In this case,
        // the RAM-resident-optimized drawRGBBitmap in the ILI9341
        // library can be invoked by forcibly type-converting the
        // PROGMEM bitmap pointer to a non-const uint16_t *.
        (uint16_t *)picture,
#endif
        size_x, size_y/3,
        color_override != 0x1337 ? color_override : TOOLBOX_LOGO_LIGHT_RED);
    uint8_t pixel_per_iteration = size_y*2/3/11;
    for (uint8_t i = 0; i<11; i++) {
        //red upper part 11101 0x1d << 0xB
        //red middle part 11000 0x18 << 0xB
        //red lower part 10010 0x12 << 0xB
        //green bit 8 stuck on 0x100 or 0x8 << 5
        //blue bit 0 on light
        uint16_t color = 0x100 | ((0x1c - i) << 0xB);
        //looping 10 times to get from the bright red to the dark red + 12th time to render the rest dark
        drawXBitmapPartial(x, y+size_y/3+pixel_per_iteration*i,
    #if defined(__AVR__) || defined(ESP8266)
            picture,
    #else
            // Some non-AVR MCU's have a "flat" memory model and don't
            // distinguish between flash and RAM addresses.  In this case,
            // the RAM-resident-optimized drawRGBBitmap in the ILI9341
            // library can be invoked by forcibly type-converting the
            // PROGMEM bitmap pointer to a non-const uint16_t *.
            (uint16_t *)picture,
    #endif
            0, size_y/3+pixel_per_iteration*i,
            size_x, size_y/3+pixel_per_iteration*(i+1),
            color_override != 0x1337 ? color_override : color);
    }
    drawXBitmapPartial(x, y+size_y/3+pixel_per_iteration*11,
    #if defined(__AVR__) || defined(ESP8266)
            picture,
    #else
            // Some non-AVR MCU's have a "flat" memory model and don't
            // distinguish between flash and RAM addresses.  In this case,
            // the RAM-resident-optimized drawRGBBitmap in the ILI9341
            // library can be invoked by forcibly type-converting the
            // PROGMEM bitmap pointer to a non-const uint16_t *.
            (uint16_t *)picture,
    #endif
            0, size_y/3+pixel_per_iteration*11,
            size_x, size_y,
            color_override != 0x1337 ? color_override : TOOLBOX_LOGO_DARK_RED);
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
        strcpy(format, PSTR("%02d.%02d%c"));
    } else {
        strcpy(format, PSTR("%2d.%02d%c"));
    }
    formatNumber(buffer, format, value/1000, (value%1000)/10, row);
}

void PsDisplay::formatCentiNumber(char * buffer, int16_t value, row_t row, bool zero_padding) {
    char format[11];
    if (zero_padding) {
        strcpy(format, PSTR("%03d.%1d%c"));
    } else {
        strcpy(format, PSTR("%3d.%1d%c"));
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

void PsDisplay::paintSmallLogo(bool visible) {
    if (visible) {
        paintLogo(0, 5, toolbox_logo_only_small_width, toolbox_logo_only_small_height, toolbox_logo_only_small_bits);
    } else {
        paintLogo(0, 5, toolbox_logo_only_small_width, toolbox_logo_only_small_height, toolbox_logo_only_small_bits, DEFAULT_BACKGROUND_COLOR);
    }
}

void PsDisplay::paintStandby(bool visible) {
    setPaintedState(visible, PS_DISPLAY_STATE_STANDBY);
    if (visible) {
        tft.setTextColor(DEFAULT_ATTENTION_COLOR);
    } else {
        tft.setTextColor(DEFAULT_BACKGROUND_COLOR);
    }
    tft.setCursor(10, PT18_IN_PXH+5+3);
    tft.print(F("Standby"));
}

void PsDisplay::paintOvertemp(bool visible) {
    setPaintedState(visible, PS_DISPLAY_STATE_OVERTEMP);
    if (visible) {
        tft.setTextColor(DEFAULT_ATTENTION_COLOR);
    } else {
        tft.setTextColor(DEFAULT_BACKGROUND_COLOR);
    }
    tft.setCursor(10, PT18_IN_PXH+5+3);
    tft.print(F("Overtemp"));
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
    //actual paint
    tft.setTextSize(1);
    if (isPaintedState(PS_DISPLAY_STATE_OVERTEMP) != isState(PS_DISPLAY_STATE_OVERTEMP)) {
        if (isState(PS_DISPLAY_STATE_OVERTEMP)) {
            paintSmallLogo(false);
        }
        if (isPaintedState(PS_DISPLAY_STATE_STANDBY)) {
            paintStandby(false);
        }
        paintOvertemp(isState(PS_DISPLAY_STATE_OVERTEMP));
        if (!isState(PS_DISPLAY_STATE_OVERTEMP)) {
            paintSmallLogo(true);
        }
    }
    if (!isPaintedState(PS_DISPLAY_STATE_OVERTEMP) && isPaintedState(PS_DISPLAY_STATE_STANDBY) != isState(PS_DISPLAY_STATE_STANDBY)) {
        if (isState(PS_DISPLAY_STATE_STANDBY)) {
            paintSmallLogo(false);
        }
        paintStandby(isState(PS_DISPLAY_STATE_STANDBY));
        if (!isState(PS_DISPLAY_STATE_STANDBY)) {
            paintSmallLogo(true);
        }
    }
    if (isPaintedState(PS_DISPLAY_STATE_LIMITED_A) != isState(PS_DISPLAY_STATE_LIMITED_A)) {
        setPaintedState(isState(PS_DISPLAY_STATE_LIMITED_A), PS_DISPLAY_STATE_LIMITED_A);
        if (isPaintedState(PS_DISPLAY_STATE_LIMITED_A)) {
            tft.setTextColor(DEFAULT_ATTENTION_COLOR);
        } else {
            tft.setTextColor(DEFAULT_BACKGROUND_COLOR);
        }
        tft.setCursor(10, PT18_IN_PXH*4+5*2+3*4);
        tft.print(F("Limited"));
    }
    if (isPaintedState(PS_DISPLAY_STATE_LIMITED_P) != isState(PS_DISPLAY_STATE_LIMITED_P)) {
        setPaintedState(isState(PS_DISPLAY_STATE_LIMITED_P), PS_DISPLAY_STATE_LIMITED_P);
        if (isPaintedState(PS_DISPLAY_STATE_LIMITED_P)) {
            tft.setTextColor(DEFAULT_ATTENTION_COLOR);
        } else {
            tft.setTextColor(DEFAULT_BACKGROUND_COLOR);
        }
        tft.setCursor(10, PT18_IN_PXH*7+5*3+3*7);
        tft.print(F("Limited"));
    }
    yield();
    //optimized hybrid
    //TODO improvement
    //     speed can be improved by reducing overdraw.
    //     if we render the bg color char and the new char in memory
    //     and only send the resulting pixels without bg color to the display
    tft.setTextSize(1);
    tft.setCursor(60+PT18_IN_PXW*6, PT18_IN_PXH+5+3);
    formatMilliNumber(buffer, milli_volts_setpoint, ROW_VOLTS, true);
    fastStringPrint(buffer, buffer_volts_setp, PT18_IN_PXW, ROW_VOLTS);
    tft.setCursor(60+PT18_IN_PXW*6, PT18_IN_PXH*4+5*2+3*4);
    formatMilliNumber(buffer, milli_amps_limit, ROW_AMPS, true);
    fastStringPrint(buffer, buffer_amps_limit, PT18_IN_PXW, ROW_AMPS);
    tft.setCursor(60+PT18_IN_PXW*6, PT18_IN_PXH*7+5*3+3*7);
    formatCentiNumber(buffer, centi_watts_limit, ROW_WATTS, true);
    fastStringPrint(buffer, buffer_watts_limit, PT18_IN_PXW, ROW_WATTS);
    painted_selected_pos = selected_pos;
    tft.setTextSize(2);
    tft.setCursor(60, PT18_IN_PXH*3+5+3*3);
    formatMilliNumber(buffer, milli_volts, ROW_VOLTS);
    fastStringPrint(buffer, buffer_volts, PT18_IN_PXW*2);
    tft.setCursor(60, PT18_IN_PXH*6+5*2+3*6);
    formatMilliNumber(buffer, milli_amps, ROW_AMPS);
    fastStringPrint(buffer, buffer_amps, PT18_IN_PXW*2);
    tft.setCursor(60, PT18_IN_PXH*9+5*3+3*9);
    formatCentiNumber(buffer, centi_watts, ROW_WATTS);
    fastStringPrint(buffer, buffer_watts, PT18_IN_PXW*2);
    yield();
}

void PsDisplay::renderTest() {
    tft.startWrite();
    for (int16_t i = 0; i < PS_DISPLAY_HEIGHT; i++) {
        for (int16_t j = 0; j < PS_DISPLAY_WIDTH; j++) {
            tft.writePixel(j, i, TOOLBOX_LOGO_LIGHT_RED);
        }
    }
    tft.endWrite();
}

void PsDisplay::renderHistory(const uint8_t* history_data, uint16_t history_pos, uint8_t thickness) {
    tft.startWrite();
    for (int16_t i = 0; i < HISTORY_LENGTH; i++) {
        int16_t current_pos = (history_pos+i) % HISTORY_LENGTH;
        int16_t last_pos = ((current_pos-1) % HISTORY_LENGTH) < 0 ? HISTORY_LENGTH-1 : (current_pos-1) % HISTORY_LENGTH;
        if (history_data[current_pos] == 0) {
            continue;
        }
        if (history_data[current_pos] != history_data[last_pos]) {
            for (uint8_t j=0; j < thickness; j++) {
                tft.writePixel(i, PS_DISPLAY_HEIGHT-history_data[last_pos]+j, DEFAULT_BACKGROUND_COLOR);
            }
            if (i > 1) { //not the best workaround for the sticky edge due to wrap and missing old data
                for (uint8_t j=0; j < thickness; j++) {
                    tft.writePixel(i, PS_DISPLAY_HEIGHT-history_data[current_pos]+j, TOOLBOX_LOGO_LIGHT_RED);
                }
            }
        }
    }
    tft.endWrite();
}

void PsDisplay::renderVolts() {
    char buffer[PS_DISPLAY_BUFFER_LENGTH];
    tft.setTextSize(2);
    tft.setCursor(60, PT18_IN_PXH*2+4);
    formatMilliNumber(buffer, milli_volts, ROW_VOLTS);
    //tft.setTextSize(2);
    fastStringPrint(buffer, buffer_volts, PT18_IN_PXW*2, ROW_NULL);
    renderHistory(history_volts, history_volts_pos);
}

void PsDisplay::renderAmps() {
    char buffer[PS_DISPLAY_BUFFER_LENGTH];
    tft.setTextSize(2);
    tft.setCursor(60, PT18_IN_PXH*2+4);
    formatMilliNumber(buffer, milli_amps, ROW_AMPS);
    fastStringPrint(buffer, buffer_amps, PT18_IN_PXW*2, ROW_NULL);
    renderHistory(history_amps, history_apms_pos);
}

void PsDisplay::renderWatts() {
    char buffer[PS_DISPLAY_BUFFER_LENGTH];
    tft.setTextSize(2);
    tft.setCursor(60, PT18_IN_PXH*2+4);
    formatCentiNumber(buffer, centi_watts, ROW_WATTS);
    fastStringPrint(buffer, buffer_watts, PT18_IN_PXW*2, ROW_NULL);
    renderHistory(history_watts, history_watts_pos);
}

void PsDisplay::setStandby(bool standby) {
    setState(state, PS_DISPLAY_STATE_STANDBY);
}

void PsDisplay::setLocked(bool locked) {
    setState(state, PS_DISPLAY_STATE_LOCKED);
}

void PsDisplay::setMemory(bool memory) {
    setState(state, PS_DISPLAY_STATE_MEMORY);
}

void PsDisplay::setRemote(bool remote) {
    setState(state, PS_DISPLAY_STATE_REMOTE);
}

void PsDisplay::setLimitedV(bool limited_v) {
    setState(state, PS_DISPLAY_STATE_LIMITED_V);
}

void PsDisplay::setLimitedA(bool limited_a) {
    setState(state, PS_DISPLAY_STATE_LIMITED_A);
}

void PsDisplay::setLimitedP(bool limited_p) {
    setState(state, PS_DISPLAY_STATE_LIMITED_P);
}

void PsDisplay::setOvertemp(bool overtemp) {
    setState(state, PS_DISPLAY_STATE_OVERTEMP);
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
    history_volts_pos = (history_volts_pos+1) % HISTORY_LENGTH;
    history_volts[history_volts_pos] = (voltage / 10000.f) *(PS_DISPLAY_HEIGHT-24*2-5-1) +1;
}

void PsDisplay::setMilliAmps(int16_t amps) {
    this->milli_amps = amps;
    history_apms_pos = (history_apms_pos+1) % HISTORY_LENGTH;
    history_amps[history_apms_pos] = (amps / 10000.f) *(PS_DISPLAY_HEIGHT-24*2-5-1) +1;
}

void PsDisplay::setCentiWatts(int16_t watts) {
    this->centi_watts = watts;
    history_watts_pos = (history_watts_pos+1) % HISTORY_LENGTH;
    history_watts[history_watts_pos] = (watts / 30000.f) *(PS_DISPLAY_HEIGHT-24*2-5-1) +1;
}

void PsDisplay::setCurser(row_t row, uint8_t pos) {
    this->selected_pos = row << 4 | pos;
    if (pos >= 3 || (row != ROW_WATTS && pos >= 2)) {
        //correct position for decimal '.'
        this->selected_pos++;
    }
}

#include "ps_display.h"
#include "Toolbox/toolbox_logo_only.xbm"
#include "Toolbox/toolbox_logo_only_small.xbm"
#include "Toolbox/toolbox_logo_font_toolbox.xbm"
#include "Toolbox/toolbox_logo_font_bodensee.xbm"
#include "Fonts/courier-prime-code-regular.h"
#include "Icons/lock.xbm"

//display state flags
#define PS_DISPLAY_STATE_LOCKED 0b00000001
#define PS_DISPLAY_STATE_MEMORY 0b00000010
#define PS_DISPLAY_STATE_REMOTE 0b00000100
#define PS_DISPLAY_STATE_STANDBY 0b00001000
#define PS_DISPLAY_STATE_LIMITED_V 0b00010000
#define PS_DISPLAY_STATE_LIMITED_A 0b00100000
#define PS_DISPLAY_STATE_LIMITED_P 0b01000000
#define PS_DISPLAY_STATE_OVERTEMP 0b10000000

//rows
#define PS_DISPLAY_VIRTUAL_ROW_COUNT 9
#define PS_DISPLAY_ROW_COUNT 6
//just a helper - starting with 1
#define getVirtualRow(row) (row + row/2)

//i hate to write it like this but functions as macros get better optimized
#define setState(value, mask) value ? this->state |= mask : this->state &= ~(mask);
#define setPaintedState(value, mask) value ? this->painted_state |= mask : this->painted_state &= ~(mask);
#define isState(mask) (this->state & (mask))
#define isPaintedState(mask) (this->painted_state & (mask))

static constexpr uint8_t rowSpacing = (PS_DISPLAY_HEIGHT-PS_DISPLAY_VIRTUAL_ROW_COUNT*PT18_IN_PXH)/(PS_DISPLAY_ROW_COUNT+1.f);

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
    memset(buffer_volts, '\0', PS_DISPLAY_BUFFER_LENGTH);
    memset(buffer_volts_setp, '\0', PS_DISPLAY_BUFFER_LENGTH);
    memset(buffer_amps, '\0', PS_DISPLAY_BUFFER_LENGTH);
    memset(buffer_amps_limit, '\0', PS_DISPLAY_BUFFER_LENGTH);
    memset(buffer_watts, '\0', PS_DISPLAY_BUFFER_LENGTH);
    memset(buffer_watts_limit, '\0', PS_DISPLAY_BUFFER_LENGTH);
    yield();
}

void PsDisplay::drawXBitmapPartial(const int16_t x, int16_t y, const uint8_t bitmap[], const int16_t sw, const int16_t sh,
                               const int16_t w, const int16_t h, const uint16_t color) {

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

void PsDisplay::paintLogo(const uint8_t x, const uint8_t y, const uint16_t size_x, const uint16_t size_y, const unsigned char* picture, const uint16_t color_override) {
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

void PsDisplay::formatNumber(char * buffer, const char * format, const int16_t value_a, const int16_t value_b, const row_t row) {
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

void PsDisplay::formatMilliNumber(char * buffer, const int16_t value, const row_t row, const bool zero_padding) {
    char format[12];
    if (zero_padding) {
        strcpy_P(format, PSTR("%02d.%02d%c"));
    } else {
        strcpy_P(format, PSTR("%2d.%02d%c"));
    }
    formatNumber(buffer, format, value/1000, (value%1000)/10, row);
}

void PsDisplay::formatCentiNumber(char * buffer, const int16_t value, const row_t row, const bool zero_padding) {
    char format[11];
    if (zero_padding) {
        strcpy_P(format, PSTR("%03d.%1d%c"));
    } else {
        strcpy_P(format, PSTR("%3d.%1d%c"));
    }
    formatNumber(buffer, format, value/100, (value%100)/10, row);
}

void PsDisplay::fastStringPrint(const char * buffer, char * old_buffer, const uint8_t font_width, const row_t row, const uint16_t fg_color, const uint16_t se_color, const uint16_t bg_color) {
    for (uint8_t i=0; i<PS_DISPLAY_BUFFER_LENGTH-1; i++) {
        uint16_t char_color = fg_color;
        bool force_paint = false;
        if (row != ROW_NULL) {
            // valid row
            if (selected_pos != painted_selected_pos) {
                // changed since last time
                if ((selected_pos & 0x0f) == i && (selected_pos >> 4) == row) {
                    //correct highlight position, correct highlight row
                    char_color = se_color;
                    force_paint = true;
                } else if ((painted_selected_pos & 0x0f) == i && (painted_selected_pos >> 4) == row) {
                    //old highlight position, old highlight row
                    force_paint = true;
                }
            } else if (old_buffer[i] != buffer[i] && (selected_pos & 0x0f) == i && (selected_pos >> 4) == row) {
                char_color = se_color;
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

void PsDisplay::paintSmallLogo(const bool visible) {
    if (visible) {
        paintLogo(0, 5, toolbox_logo_only_small_width, toolbox_logo_only_small_height, toolbox_logo_only_small_bits);
    } else {
        paintLogo(0, 5, toolbox_logo_only_small_width, toolbox_logo_only_small_height, toolbox_logo_only_small_bits, DEFAULT_BACKGROUND_COLOR);
    }
}

void PsDisplay::paintFlag(const bool visible, const uint8_t flag, const uint8_t y) {
    setPaintedState(visible, flag);
    if (visible) {
        tft.setTextColor(DEFAULT_ATTENTION_COLOR);
    } else {
        tft.setTextColor(DEFAULT_BACKGROUND_COLOR);
    }
    tft.setCursor(10, y);
    if (flag == PS_DISPLAY_STATE_STANDBY) {
        tft.print(F("Standby"));
    } else if (flag == PS_DISPLAY_STATE_OVERTEMP) {
        tft.print(F("Overtemp"));
    } else {
        tft.print(F("Limited"));
    }
}

constexpr uint8_t PsDisplay::getRowYPos(const uint8_t row) {
    return getVirtualRow(row)*PT18_IN_PXH + rowSpacing*row;
}

void PsDisplay::render() {
    bool mode_changed = screen_mode != painted_screen_mode;
    if (mode_changed) {
        // clear screen on mode change
        clear();
    }
    switch (screen_mode)
    {
        case SCREEN_MODE_MAINSCREEN:
            renderMainscreen();
            break;
        case SCREEN_MODE_VOLTS:
            renderVolts(mode_changed);
            break;
        case SCREEN_MODE_AMPS:
            renderAmps(mode_changed);
            break;
        case SCREEN_MODE_WATTS:
            renderWatts(mode_changed);
            break;
        case SCREEN_MODE_FULL_GRAPH:
            renderFullGraph(mode_changed);
            break;
        case SCREEN_MODE_TEST:
            renderTest();
            break;
        case SCREEN_MODE_LOGO:
            if (mode_changed) {
                // no dynamic update
                renderLogo();
            }
            break;
        case SCREEN_MODE_NONE:
        default:
            break;
    }
    painted_screen_mode = screen_mode;
}

void PsDisplay::renderMainscreen() {
    //layout is:
    //---------------------
    // voltage setpoint 21px
    // VOLTAGE 42px
    // amps limit 21px
    // AMPS 42px
    // watts limit 21px
    // WATTS 42px
    //---------------------
    char buffer[PS_DISPLAY_BUFFER_LENGTH];
    //actual paint
    tft.setTextSize(1);
    uint8_t changedStates = this->state ^ this->painted_state;
    if (changedStates & (PS_DISPLAY_STATE_OVERTEMP | PS_DISPLAY_STATE_STANDBY)) {
        //as long as we are in overtemp mode overtemp flag is king
        if (!(isPaintedState(PS_DISPLAY_STATE_OVERTEMP) && !(changedStates & PS_DISPLAY_STATE_OVERTEMP))) {
            //paint old state in bg color
            if (!isPaintedState(PS_DISPLAY_STATE_OVERTEMP | PS_DISPLAY_STATE_STANDBY)) {
                paintSmallLogo(false);
            } else if (isPaintedState(PS_DISPLAY_STATE_OVERTEMP)) {
                paintFlag(false, PS_DISPLAY_STATE_OVERTEMP, getRowYPos(1));
            } else {
                paintFlag(false, PS_DISPLAY_STATE_STANDBY, getRowYPos(1));
            }
            //paint new state
            if (!isState(PS_DISPLAY_STATE_OVERTEMP | PS_DISPLAY_STATE_STANDBY)) {
                paintSmallLogo(true);
            } else if (isState(PS_DISPLAY_STATE_OVERTEMP)) {
                paintFlag(true, PS_DISPLAY_STATE_OVERTEMP, getRowYPos(1));
            } else {
                paintFlag(true, PS_DISPLAY_STATE_STANDBY, getRowYPos(1));
            }
        }
    }
    if (changedStates & PS_DISPLAY_STATE_LIMITED_A) {
        paintFlag(isState(PS_DISPLAY_STATE_LIMITED_A), PS_DISPLAY_STATE_LIMITED_A, getRowYPos(3));
    }
    if (changedStates & PS_DISPLAY_STATE_LIMITED_P) {
        paintFlag(isState(PS_DISPLAY_STATE_LIMITED_P), PS_DISPLAY_STATE_LIMITED_P, getRowYPos(5));
    }
    if (changedStates & PS_DISPLAY_STATE_LOCKED) {
        setPaintedState(isState(PS_DISPLAY_STATE_LOCKED), PS_DISPLAY_STATE_LOCKED);
        tft.drawXBitmap(10, getRowYPos(6) - lock_height,
#if defined(__AVR__) || defined(ESP8266)
        lock_bits,
#else
        // Some non-AVR MCU's have a "flat" memory model and don't
        // distinguish between flash and RAM addresses.  In this case,
        // the RAM-resident-optimized drawRGBBitmap in the ILI9341
        // library can be invoked by forcibly type-converting the
        // PROGMEM bitmap pointer to a non-const uint16_t *.
        (uint16_t *)lock_bits,
#endif
        lock_width, lock_height,
        isPaintedState(PS_DISPLAY_STATE_LOCKED) ? TOOLBOX_LOGO_LIGHT_RED : ILI9341_BLACK);
    }
    yield();
    //optimized hybrid
    tft.setTextSize(1);
    tft.setCursor(60+PT18_IN_PXW*6, getRowYPos(1));
    formatMilliNumber(buffer, milli_volts_setpoint, ROW_VOLTS, true);
    fastStringPrint(buffer, buffer_volts_setp, PT18_IN_PXW, ROW_VOLTS);
    tft.setCursor(60+PT18_IN_PXW*6, getRowYPos(3));
    formatMilliNumber(buffer, milli_amps_limit, ROW_AMPS, true);
    fastStringPrint(buffer, buffer_amps_limit, PT18_IN_PXW, ROW_AMPS);
    tft.setCursor(60+PT18_IN_PXW*6, getRowYPos(5));
    formatCentiNumber(buffer, centi_watts_limit, ROW_WATTS, true);
    fastStringPrint(buffer, buffer_watts_limit, PT18_IN_PXW, ROW_WATTS);
    painted_selected_pos = selected_pos;
    tft.setTextSize(2);
    tft.setCursor(60, getRowYPos(2));
    formatMilliNumber(buffer, milli_volts, ROW_VOLTS);
    fastStringPrint(buffer, buffer_volts, PT18_IN_PXW*2);
    tft.setCursor(60, getRowYPos(4));
    formatMilliNumber(buffer, milli_amps, ROW_AMPS);
    fastStringPrint(buffer, buffer_amps, PT18_IN_PXW*2);
    tft.setCursor(60, getRowYPos(6));
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

void PsDisplay::renderHistory(const uint8_t* history_data, const uint16_t history_pos, const bool force_paint, const uint8_t thickness, const uint16_t color_override) {
    tft.startWrite();
    for (uint16_t i = 0; i < HISTORY_LENGTH; i++) {
        uint16_t current_pos = (history_pos+i) % HISTORY_LENGTH;
        uint16_t last_pos = ((current_pos-1) % HISTORY_LENGTH) < 0 ? HISTORY_LENGTH-1 : (current_pos-1) % HISTORY_LENGTH;
        if (history_data[current_pos] != history_data[last_pos]) {
            for (uint8_t j=0; j < thickness; j++) {
                tft.writePixel(i, PS_DISPLAY_HEIGHT-1-history_data[last_pos]+j, DEFAULT_BACKGROUND_COLOR);
            }
            if (i > 1) { //not the best workaround for the sticky edge due to wrap and missing old data
                for (uint8_t j=0; j < thickness; j++) {
                    tft.writePixel(i, PS_DISPLAY_HEIGHT-1-history_data[current_pos]+j, color_override != 0x1337 ? color_override : TOOLBOX_LOGO_LIGHT_RED);
                }
            }
        }
        if (force_paint) {
            // paint if nothing is painted yet
            if (i > 1) { //not the best workaround for the sticky edge due to wrap and missing old data
                for (uint8_t j=0; j < thickness; j++) {
                    tft.writePixel(i, PS_DISPLAY_HEIGHT-1-history_data[current_pos]+j, color_override != 0x1337 ? color_override : TOOLBOX_LOGO_LIGHT_RED);
                }
            }
        }
    }
    tft.endWrite();
}

void PsDisplay::clearText(char * old_buffer) {
    tft.setTextColor(ILI9341_BLACK);
    tft.print(old_buffer);
    memset(old_buffer, '\0', PS_DISPLAY_BUFFER_LENGTH);
}

void PsDisplay::renderSingleGraph(const uint16_t value1, char * old_buffer1, const uint16_t value2, char * old_buffer2, const uint8_t * history, const uint16_t history_pos, const row_t row, const bool mode_changed) {
    char buffer[PS_DISPLAY_BUFFER_LENGTH];
    tft.setTextSize(2);
    tft.setCursor(60, PT18_IN_PXH*2+4);
    if ((selected_pos >> 4) == row) {
        if ((painted_selected_pos >> 4) != row) {
            clearText(old_buffer1);
            tft.setCursor(60, PT18_IN_PXH*2+4);
        }
        formatMilliNumber(buffer, value2, row, true);
        fastStringPrint(buffer, old_buffer2, PT18_IN_PXW*2, row);
    } else {
        if ((painted_selected_pos >> 4) == row) {
            clearText(old_buffer2);
            tft.setCursor(60, PT18_IN_PXH*2+4);
        }
        formatMilliNumber(buffer, value1, row);
        fastStringPrint(buffer, old_buffer1, PT18_IN_PXW*2, ROW_NULL);
    }
    renderHistory(history, history_pos, mode_changed);
    painted_selected_pos = selected_pos;
}

inline void PsDisplay::renderVolts(const bool mode_changed) {
    renderSingleGraph(milli_volts, buffer_volts, milli_volts_setpoint, buffer_volts_setp, history_volts, history_volts_pos, ROW_VOLTS, mode_changed);
}

inline void PsDisplay::renderAmps(const bool mode_changed) {
    renderSingleGraph(milli_amps, buffer_amps, milli_amps_limit, buffer_amps_limit, history_amps, history_apms_pos, ROW_AMPS, mode_changed);
}

inline void PsDisplay::renderWatts(const bool mode_changed) {
    renderSingleGraph(centi_watts, buffer_watts, centi_watts_limit, buffer_watts_limit, history_watts, history_watts_pos, ROW_WATTS, mode_changed);
}

void PsDisplay::renderFullGraph(const bool mode_changed) {
    char buffer[PS_DISPLAY_BUFFER_LENGTH];
    tft.setTextSize(1);
    tft.setCursor(10, getRowYPos(1));
    if ((selected_pos >> 4) == ROW_VOLTS) {
        if ((painted_selected_pos >> 4) != ROW_VOLTS) {
            clearText(buffer_volts);
            tft.setCursor(10, getRowYPos(1));
        }
        formatMilliNumber(buffer, milli_volts_setpoint, ROW_VOLTS, true);
        fastStringPrint(buffer, buffer_volts_setp, PT18_IN_PXW, ROW_VOLTS);
    } else {
        if ((painted_selected_pos >> 4) == ROW_VOLTS) {
            clearText(buffer_volts_setp);
            tft.setCursor(10, getRowYPos(1));
        }
        formatMilliNumber(buffer, milli_volts, ROW_VOLTS);
        fastStringPrint(buffer, buffer_volts, PT18_IN_PXW, ROW_NULL, ILI9341_GREENYELLOW);
    }
    tft.setCursor(60+PT18_IN_PXW*6, getRowYPos(1));
    if ((selected_pos >> 4) == ROW_AMPS) {
        if ((painted_selected_pos >> 4) != ROW_AMPS) {
            clearText(buffer_amps);
            tft.setCursor(60+PT18_IN_PXW*6, getRowYPos(1));
        }
        formatMilliNumber(buffer, milli_amps_limit, ROW_AMPS, true);
        fastStringPrint(buffer, buffer_amps_limit, PT18_IN_PXW, ROW_AMPS);
    } else {
        if ((painted_selected_pos >> 4) == ROW_AMPS) {
            clearText(buffer_amps_limit);
            tft.setCursor(60+PT18_IN_PXW*6, getRowYPos(1));
        }
        formatMilliNumber(buffer, milli_amps, ROW_AMPS);
        fastStringPrint(buffer, buffer_amps, PT18_IN_PXW, ROW_NULL, ILI9341_YELLOW);
    }
    renderHistory(history_watts, history_watts_pos, mode_changed, 2);
    renderHistory(history_amps, history_apms_pos, mode_changed, 2, ILI9341_YELLOW);
    renderHistory(history_volts, history_volts_pos, mode_changed, 2, ILI9341_GREENYELLOW);
    painted_selected_pos = selected_pos;
}

const PsDisplay::screen_mode_t PsDisplay::getScreenMode() const {
    return screen_mode;
}

void PsDisplay::setScreenMode(const screen_mode_t mode) {
    screen_mode = mode;
}

void PsDisplay::setStandby(const bool standby) {
    setState(standby, PS_DISPLAY_STATE_STANDBY);
}

void PsDisplay::setLocked(const bool locked) {
    setState(locked, PS_DISPLAY_STATE_LOCKED);
}

void PsDisplay::setMemory(const bool memory) {
    setState(memory, PS_DISPLAY_STATE_MEMORY);
}

void PsDisplay::setRemote(const bool remote) {
    setState(remote, PS_DISPLAY_STATE_REMOTE);
}

void PsDisplay::setLimitedV(const bool limited_v) {
    setState(limited_v, PS_DISPLAY_STATE_LIMITED_V);
}

void PsDisplay::setLimitedA(const bool limited_a) {
    setState(limited_a, PS_DISPLAY_STATE_LIMITED_A);
}

void PsDisplay::setLimitedP(const bool limited_p) {
    setState(limited_p, PS_DISPLAY_STATE_LIMITED_P);
}

const bool PsDisplay::isLimitedV() const {
    return isState(PS_DISPLAY_STATE_LIMITED_V);
}

const bool PsDisplay::isLimitedA() const {
    return isState(PS_DISPLAY_STATE_LIMITED_A);
}

const bool PsDisplay::isLimitedP() const {
    return isState(PS_DISPLAY_STATE_LIMITED_P);
}

void PsDisplay::setOvertemp(const bool overtemp) {
    setState(overtemp, PS_DISPLAY_STATE_OVERTEMP);
}

void PsDisplay::setMilliVoltsSetpoint(const int16_t voltage_setpoint) {
    this->milli_volts_setpoint = voltage_setpoint;
}

void PsDisplay::setMilliAmpsLimit(const int16_t amps_limit) {
    this->milli_amps_limit = amps_limit;
}

void PsDisplay::setCentiWattsLimit(const int16_t watts_limit) {
    this->centi_watts_limit = watts_limit;
}

void PsDisplay::setMilliVolts(const int16_t voltage) {
    this->milli_volts = voltage;
    history_volts_pos = (history_volts_pos+1) % HISTORY_LENGTH;
    // normalize to (PS_DISPLAY_HEIGHT-(PT18_IN_PXH*2+6*2)) => ~ 187 pixel
    history_volts[history_volts_pos] = (voltage >> 8) * 1.589f;
}

void PsDisplay::setMilliAmps(const int16_t amps) {
    this->milli_amps = amps;
    history_apms_pos = (history_apms_pos+1) % HISTORY_LENGTH;
    // normalize to (PS_DISPLAY_HEIGHT-(PT18_IN_PXH*2+6*2)) => ~ 187 pixel
    history_amps[history_apms_pos] = (amps >> 6) * 1.192f;
}

void PsDisplay::setCentiWatts(const int16_t watts) {
    this->centi_watts = watts;
    history_watts_pos = (history_watts_pos+1) % HISTORY_LENGTH;
    // normalize to (PS_DISPLAY_HEIGHT-(PT18_IN_PXH*2+6*2)) => ~ 187 pixel
    history_watts[history_watts_pos] = (watts >> 8) * 1.589f;
}

void PsDisplay::setCurser(const row_t row, const uint8_t pos) {
    this->selected_pos = row << 4 | pos;
    if (pos >= 3 || (row != ROW_WATTS && pos >= 2)) {
        //correct position for decimal '.'
        this->selected_pos++;
    }
}

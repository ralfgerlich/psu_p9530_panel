#ifndef PS_DISPLAY
#define PS_DISPLAY

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>
#include "Toolbox/toolbox_colors.h"
#include "canvas1.h"

#define TFT_DC 9
#define TFT_CS 10

//resolution 320x240
#define PS_DISPLAY_WIDTH 320
#define PS_DISPLAY_HEIGHT 240

#define PT18_IN_PXH 21
#define PT18_IN_PXW 21

#define PS_DISPLAY_BUFFER_LENGTH 7

#define HISTORY_LENGTH PS_DISPLAY_WIDTH

// colors
#define DEFAULT_BACKGROUND_COLOR ILI9341_BLACK
#define DEFAULT_TEXT_COLOR ILI9341_WHITE
#define DEFAULT_HIGHLIGHT_COLOR ILI9341_GREEN
#define DEFAULT_ATTENTION_COLOR TOOLBOX_LOGO_MEDIUM_RED

class PsDisplay {
    public:
    PsDisplay(Adafruit_ILI9341 & tft);

    enum row_t {
        ROW_NULL,
        ROW_VOLTS,
        ROW_AMPS,
        ROW_WATTS
    };
    enum screen_mode_t {
        SCREEN_MODE_NONE,
        SCREEN_MODE_LOGO,
        SCREEN_MODE_MAINSCREEN,
        SCREEN_MODE_VOLTS,
        SCREEN_MODE_AMPS,
        SCREEN_MODE_WATTS,
        SCREEN_MODE_FULL_GRAPH,
        SCREEN_MODE_TEST
    };

    void init(void);

    void render(void);

    void setScreenMode(const screen_mode_t mode);
    const screen_mode_t getScreenMode() const;
    void setStandby(const bool standby);
    void setLocked(const bool locked);
    void setMemory(const bool memory);
    void setRemote(const bool remote);
    void setLimitedV(const bool limited_v);
    void setLimitedA(const bool limited_a);
    void setLimitedP(const bool limited_p);
    const bool isLimitedV() const;
    const bool isLimitedA() const;
    const bool isLimitedP() const;
    void setOvertemp(const bool overtemp);
    void setMilliVoltsSetpoint(const int16_t voltage_setpoint);
    void setMilliAmpsLimit(const int16_t amps_limit);
    void setCentiWattsLimit(const int16_t watts_limit);
    void setMilliVolts(const int16_t voltage);
    void setMilliAmps(const int16_t amps);
    void setCentiWatts(const int16_t watts);
    void setCurser(const row_t row, const uint8_t pos);
    
    private:
    void clear(void);
    void renderLogo(void);
    void renderMainscreen(void);
    inline void renderVolts(const bool mode_changed);
    inline void renderAmps(const bool mode_changed);
    inline void renderWatts(const bool mode_changed);
    void renderFullGraph(const bool mode_changed);
    void renderTest(void);
    void renderSingleGraph(const uint16_t value1, char * old_buffer1, const uint16_t value2, char * old_buffer2, const uint8_t * history, const uint16_t history_pos, const row_t row, const bool mode_changed);
    void clearText(char * old_buffer);
    void paintLogo(const uint8_t x, const uint8_t y, const uint16_t size_x, const uint16_t size_y, const unsigned char* picture, const uint16_t color_override = 0x1337);
    void drawXBitmapPartial(const int16_t x, int16_t y, const uint8_t bitmap[], const int16_t sw, const int16_t sh, const int16_t w, const int16_t h, const uint16_t color);
    void fastStringPrint(const char * buffer, char * old_buffer, const uint8_t font_width, const row_t row = ROW_NULL, const uint16_t fg_color = DEFAULT_TEXT_COLOR, const uint16_t se_color = DEFAULT_HIGHLIGHT_COLOR, const uint16_t bg_color = DEFAULT_BACKGROUND_COLOR);
    void formatNumber(char * buffer, const char * format, const int16_t value_a, const int16_t value_b, const row_t row);
    void formatMilliNumber(char * buffer, const int16_t value, const row_t row, const bool zero_padding = false);
    void formatCentiNumber(char * buffer, const int16_t value, const row_t row, const bool zero_padding = false);
    void paintFlag(const bool visible, const uint8_t flag, const uint8_t y);
    void paintSmallLogo(const bool visible);
    void renderHistory(const uint8_t* history_data, const uint16_t history_pos, const bool force_paint, const uint8_t thickness = 2, const uint16_t color_override = 0x1337);
    constexpr uint8_t getRowYPos(const uint8_t row);

    Adafruit_ILI9341& tft;
    Canvas1<PT18_IN_PXW, PT18_IN_PXH> canvas;
    bool init_done = 0;
    screen_mode_t screen_mode = SCREEN_MODE_NONE;
    screen_mode_t painted_screen_mode = SCREEN_MODE_NONE;
    //temp values from outside world
    uint8_t state = 0;
    int16_t milli_volts_setpoint = 0;
    int16_t milli_amps_limit = 0;
    int16_t centi_watts_limit = 0;
    int16_t milli_volts = 0;
    int16_t milli_amps = 0;
    int16_t centi_watts = 0;
    uint8_t selected_pos = 0;
    //actually painted values
    uint8_t painted_state = 0;
    uint8_t painted_selected_pos = 0;
    char buffer_volts[PS_DISPLAY_BUFFER_LENGTH] = {0};
    char buffer_amps[PS_DISPLAY_BUFFER_LENGTH] = {0};
    char buffer_watts[PS_DISPLAY_BUFFER_LENGTH] = {0};
    char buffer_volts_setp[PS_DISPLAY_BUFFER_LENGTH] = {0};
    char buffer_amps_limit[PS_DISPLAY_BUFFER_LENGTH] = {0};
    char buffer_watts_limit[PS_DISPLAY_BUFFER_LENGTH] = {0};
    uint16_t history_volts_pos = 0;
    uint16_t history_apms_pos = 0;
    uint16_t history_watts_pos = 0;
    uint8_t history_volts[HISTORY_LENGTH] = {0};
    uint8_t history_amps[HISTORY_LENGTH] = {0};
    uint8_t history_watts[HISTORY_LENGTH] = {0};
};

#endif

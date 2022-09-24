#ifndef PS_DISPLAY
#define PS_DISPLAY

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>
#include "Toolbox/toolbox_colors.h"

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

    void setScreenMode(screen_mode_t mode);
    screen_mode_t getScreenMode();
    void setStandby(bool standby);
    void setLocked(bool locked);
    void setMemory(bool memory);
    void setRemote(bool remote);
    void setLimitedV(bool limited_v);
    void setLimitedA(bool limited_a);
    void setLimitedP(bool limited_p);
    bool isLimitedV();
    bool isLimitedA();
    bool isLimitedP();
    void setOvertemp(bool overtemp);
    void setMilliVoltsSetpoint(int16_t voltage_setpoint);
    void setMilliAmpsLimit(int16_t amps_limit);
    void setCentiWattsLimit(int16_t watts_limit);
    void setMilliVolts(int16_t voltage);
    void setMilliAmps(int16_t amps);
    void setCentiWatts(int16_t watts);
    void setCurser(row_t row, uint8_t pos);
    
    private:
    void clear(void);
    void renderLogo(void);
    void renderMainscreen(void);
    inline void renderVolts(void);
    inline void renderAmps(void);
    inline void renderWatts(void);
    void renderFullGraph(void);
    void renderTest(void);
    void renderSingleGraph(uint16_t value1, char * old_buffer1, uint16_t value2, char * old_buffer2, uint8_t * history, uint16_t history_pos, row_t row);
    void clearText(char * old_buffer);
    void paintLogo(uint8_t x, uint8_t y, uint16_t size_x, uint16_t size_y, const unsigned char* picture, uint16_t color_override = 0x1337);
    void drawXBitmapPartial(int16_t x, int16_t y, const uint8_t bitmap[], int16_t sw, int16_t sh, int16_t w, int16_t h, uint16_t color);
    void fastStringPrint(char * buffer, char * old_buffer, uint8_t font_width, row_t row = ROW_NULL, uint16_t fg_color = DEFAULT_TEXT_COLOR, uint16_t se_color = DEFAULT_HIGHLIGHT_COLOR, uint16_t bg_color = DEFAULT_BACKGROUND_COLOR);
    void formatNumber(char * buffer, char * format, int16_t value_a, int16_t value_b, row_t row);
    void formatMilliNumber(char * buffer, int16_t value, row_t row, bool zero_padding = false);
    void formatCentiNumber(char * buffer, int16_t value, row_t row, bool zero_padding = false);
    void paintFlag(bool visible, uint8_t flag, uint8_t y);
    void paintSmallLogo(bool visible);
    void renderHistory(const uint8_t* history_data, uint16_t history_pos, uint8_t thickness = 2, uint16_t color_override = 0x1337);
    uint8_t getRowYPos(uint8_t row);

    Adafruit_ILI9341& tft;
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

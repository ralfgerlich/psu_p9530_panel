#ifndef PS_DISPLAY
#define PS_DISPLAY

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>

#define TFT_DC 9
#define TFT_CS 10

//resolution 320x240
#define PS_DISPLAY_WIDTH 320
#define PS_DISPLAY_HEIGHT 240

#define PT18_IN_PXH 24
#define PT18_IN_PXW 21

#define PS_DISPLAY_BUFFER_LENGTH 7

class PsDisplay {
    public:
    PsDisplay(Adafruit_ILI9341 & tft);

    enum row_t {
        ROW_NULL,
        ROW_VOLTS,
        ROW_AMPS,
        ROW_WATTS
    };

    void init(void);

    void clear(void);
    void renderLogo(void);
    void renderMainscreen(void);

    void setStandby(bool standby);
    void setLocked(bool locked);
    void setMemory(bool memory);
    void setRemote(bool remote);
    void setLimitedV(bool limited_v);
    void setLimitedA(bool limited_a);
    void setLimitedP(bool limited_p);
    void setOvertemp(bool overtemp);
    void setMilliVoltsSetpoint(int16_t voltage_setpoint);
    void setMilliAmpsLimit(int16_t amps_limit);
    void setCentiWattsLimit(int16_t watts_limit);
    void setMilliVolts(int16_t voltage);
    void setMilliAmps(int16_t amps);
    void setCentiWatts(int16_t watts);
    void setCurser(row_t row, uint8_t pos);
    
    private:
    void drawXBitmapPartial(int16_t x, int16_t y, const uint8_t bitmap[], int16_t sw, int16_t sh, int16_t w, int16_t h, uint16_t color);
    void fastStringPrint(char * buffer, char * old_buffer, uint8_t font_width, row_t row, uint16_t fg_color = ILI9341_WHITE, uint16_t se_color = ILI9341_GREEN, uint16_t bg_color = ILI9341_BLACK);
    void formatNumber(char * buffer, char * format, int16_t value_a, int16_t value_b, row_t row);
    void formatMilliNumber(char * buffer, int16_t value, row_t row, bool zero_padding = false);
    void formatCentiNumber(char * buffer, int16_t value, row_t row, bool zero_padding = false);
    void paintStandby(bool visible);
    void paintOvertemp(bool visible);

    Adafruit_ILI9341& tft;
    bool init_done;
    //temp values from outside world
    bool locked; //ignored
    bool memory; //ignored
    bool remote; //ignored
    bool standby;
    bool limited_v; //ignored
    bool limited_a;
    bool limited_p;
    bool overtemp;
    int16_t milli_volts_setpoint;
    int16_t milli_amps_limit;
    int16_t centi_watts_limit;
    int16_t milli_volts;
    int16_t milli_amps;
    int16_t centi_watts;
    uint8_t selected_pos;
    //actually painted values
    bool painted_standby;
    bool painted_limited_a;
    bool painted_limited_p;
    bool painted_overtemp;
    uint8_t painted_selected_pos;
    char buffer_volts[PS_DISPLAY_BUFFER_LENGTH];
    char buffer_amps[PS_DISPLAY_BUFFER_LENGTH];
    char buffer_watts[PS_DISPLAY_BUFFER_LENGTH];
    char buffer_volts_setp[PS_DISPLAY_BUFFER_LENGTH];
    char buffer_amps_limit[PS_DISPLAY_BUFFER_LENGTH];
    char buffer_watts_limit[PS_DISPLAY_BUFFER_LENGTH];
};

#endif

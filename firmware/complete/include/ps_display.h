#ifndef PS_DISPLAY
#define PS_DISPLAY

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>

//resolution 320x240
#define PS_DISPLAY_WIDTH 320
#define PS_DISPLAY_HEIGHT 240

#define PT18_IN_PX 24

class PsDisplay {
    public:
    PsDisplay(Adafruit_ILI9341 & tft);

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
    void setMVoltsSetpoint(int16_t voltage_setpoint);
    void setMAmpsLimit(int16_t amps_limit);
    void setCWattsLimit(int16_t watts_limit);
    void setMVolts(int16_t voltage);
    void setMAmps(int16_t amps);
    void setCWatts(int16_t watts);
    
    private:
    void fastPrintNumber(char * buffer, int16_t value);
    void fastStringPrint(char * buffer, char * old_buffer, uint8_t font_width);
    void formatMilliNumber(char * buffer, int16_t value, char unit);
    void formatCentiNumber(char * buffer, int16_t value, char unit);

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
    //actually painted values
    bool painted_standby;
    bool painted_limited_a;
    bool painted_limited_p;
    char buffer_volts[8];
    char buffer_amps[8];
    char buffer_watts[8];
    char buffer_volts_setp[8];
    char buffer_amps_limit[8];
    char buffer_watts_limit[8];
};

#endif

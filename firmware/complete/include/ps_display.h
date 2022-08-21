#ifndef PS_DISPLAY
#define PS_DISPLAY

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

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
    void setLimitedA(bool limited_a);
    void setLimitedP(bool limited_p);
    void setOvertemp(bool overtemp);
    void setMVoltsSetpoint(int voltage_setpoint);
    void setMAmpsLimit(int amps_limit);
    void setCWattsLimit(int watts_limit);
    void setMVolts(int voltage);
    void setMAmps(int amps);
    void setCWatts(int watts);
    
    private:
    void fastPrintNumber(char * buffer, int value);
    void fastStringPrint(char * buffer, char * old_buffer, int font_width);
    void formatMilliNumber(char * buffer, int value, char unit);
    void formatCentiNumber(char * buffer, int value, char unit);

    Adafruit_ILI9341& tft;
    bool init_done;
    //temp values from outside world
    bool locked;
    bool memory;
    bool remote;
    bool standby;
    bool limited_a;
    bool limited_p;
    bool overtemp;
    int milli_volts_setpoint;
    int milli_amps_limit;
    int centi_watts_limit;
    int milli_volts;
    int milli_amps;
    int centi_watts;
    //actually painted values
    bool painted_locked;
    bool painted_memory;
    bool painted_remote;
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

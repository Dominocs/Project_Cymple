#ifndef LED_HPP
#define LED_HPP
#include<Arduino.h>
#include "../config.h"
enum LED_MODE{
    LED_MODE_ALWAYS_OFF,
    LED_MODE_ALWAYS_ON,
    LED_MODE_BLINK,
};
class ledClass{
private:
    uint16_t uimode = LED_MODE_ALWAYS_OFF;
    unsigned long uiLEDTimer = 0;
    unsigned long uiLEDOnTimer = 0;
    unsigned long uiLEDOffTimer = 0;
    
public:
    ledClass(){
        pinMode(LED_PIN, OUTPUT);
        ledMode(LED_MODE_ALWAYS_ON, 0, 0);
    }
    void ledMode(LED_MODE mode, unsigned long Ondelay, unsigned long Offdelay);
    void runFrame(unsigned long currentT);
};
extern ledClass *pledObj;
#endif
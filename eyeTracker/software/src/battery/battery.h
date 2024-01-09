#ifndef _BATTERY_H
#define _BATTERY_H
#include <Arduino.h>
class batteryClass{
private:
    unsigned long ulTimer = 0;
    uint8_t ucBattery = 0;
public:
    void updateBattery();
    uint8_t getBattery(){
        return ucBattery;
    }
};
extern batteryClass *pbattery;
#endif
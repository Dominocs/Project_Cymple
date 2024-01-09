#include <Arduino.h>
#include "battery.h"
#include "../config.h"

batteryClass *pbattery = NULL;
void batteryClass::updateBattery()
{
    unsigned long currentTime = millis();
    if(currentTime - ulTimer > BATTERY_UPDATE_TIME){
        int sensorValue = analogRead(BATTERY_PIN);
        float battery = (float)sensorValue / BATEERY_ADC_MAX;  //分压电压
        battery = battery / BATTERY_DOT;                //电源实际电压
        battery = battery < BATTERY_MAX?battery:BATTERY_MAX;
        battery = battery > BATTERY_MIN?battery:BATTERY_MIN;
        ucBattery = ceil((battery - BATTERY_MIN) / (BATTERY_MAX - BATTERY_MIN) * 100);
        ulTimer = currentTime;
    }
}


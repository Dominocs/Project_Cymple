#include <Arduino.h>
#include "led.h"
ledClass *pledObj = NULL;


void ledClass::ledMode(LED_MODE mode, unsigned long Ondelay, unsigned long Offdelay){
    if((uimode == mode) && (Ondelay == uiLEDOnTimer) && (uiLEDOffTimer == Offdelay)){
        return;
    }
    
    switch(mode){
        case LED_MODE_ALWAYS_ON:
            digitalWrite(LED_PIN, LOW);
            break;
        case LED_MODE_ALWAYS_OFF:
            digitalWrite(LED_PIN, HIGH);
            break;
        case LED_MODE_BLINK:
            uiLEDTimer = millis(); 
            uiLEDOnTimer = Ondelay;
            uiLEDOffTimer = Offdelay;
            digitalWrite(LED_PIN, HIGH);
            break;
    }
    uimode = mode;
}

void ledClass::runFrame(unsigned long currentT){
    if(LED_MODE_BLINK != uimode){
        return;
    }
    if(currentT - uiLEDTimer < uiLEDOnTimer){
        digitalWrite(LED_PIN, LOW);
    }
    else if(currentT - uiLEDTimer < (uiLEDOnTimer + uiLEDOffTimer)){
        /* 超过一个周期 */
        digitalWrite(LED_PIN, HIGH);
    }else{
        uiLEDTimer = currentT;
        digitalWrite(LED_PIN, LOW);
    }
}
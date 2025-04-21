#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include "drv/eeprom.h"
#include "wlanMsg.h"
#include "serialMsg.h"
#include "esp32cam.h"
unsigned long currentT;
void setup() {
  delay(1000);
  EEPROM.begin(sizeof(EEPROM_DATA_S));
  pserialObj = new serialClass();
  pwlanMsgObj = new wlanMsgClass();
  Serial.println("CympleEye WLAN Init!");
  pCamera = new cameraClass();
  Serial.println("CympleEye Camera Init!");
  delay(1000);
}

void loop() {
  currentT = millis();
  pserialObj->runFrame(currentT);
  if(pwlanMsgObj->runFrame(currentT)){
    return;
  }
  if(pCamera->runFrame()){
    delay(1000);
    return;
  }
  delay(5);
}

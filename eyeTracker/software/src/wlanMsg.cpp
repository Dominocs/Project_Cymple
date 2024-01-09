#include <AsyncUDP.h>
#include "common.h"
#include "drv/led.h"
#include "drv/eeprom.h"
#include "drv/network.h"
#include "esp32cam.h"
#include "wlanMsg.h"
#include <esp_wifi.h>
#define WIFI_MAX_TX_POWER 80
wlanMsgClass *pwlanMsgObj = NULL;
IPAddress peerAddr;
bool bHeartbeatTimeout = true;
unsigned long heartBeatTimer = 0;

static void onPacketCallBack(AsyncUDPPacket packet){
    if(!packet.available()){
        return;
    }
    TLV_S *pstMsgHdr = (TLV_S *)packet.data();
    int msgLen = packet.length();
    switch(pstMsgHdr->uiType){
        case MSG_SERVER_HEARTBEAT_E:
            heartBeatTimer = millis();
            bHeartbeatTimeout = false;
            peerAddr = packet.remoteIP();
            pledObj->ledMode(LED_MODE_BLINK, 1000, 2000);
            break;
        case MSG_POSITION_CFG_E:{
            if(msgLen == sizeof(MSG_WLAN_POSIOTN_CONFIG_S)){
                MSG_WLAN_POSIOTN_CONFIG_S *pstMsg = (MSG_WLAN_POSIOTN_CONFIG_S *)packet.data();
                eepromApi::write(&pstMsg->ucPosition, OFFSET(EEPROM_DATA_S, ucFlags), sizeof(pstMsg->ucPosition));
                pCamera->ucFlags = pstMsg->ucPosition;
            }else{
                Serial.printf("Mismatch: sizeof(MSG_WLAN_PARACONFIG_S):%u, msg len:%d\n", sizeof(MSG_WLAN_POSIOTN_CONFIG_S), msgLen);
            }
            break;
        }
        default:
            Serial.printf("Invalid msg type%u, msg len:%d\n", pstMsgHdr->uiType, msgLen);
            break;
    }
}

wlanMsgClass::wlanMsgClass(){
    memset(acSSID, 0 , sizeof(acSSID));
    memset(acPassword, 0 , sizeof(acPassword));
    eepromApi::read(acSSID, OFFSET(EEPROM_DATA_S, acSSID), sizeof(acSSID));
    eepromApi::read(acPassword, OFFSET(EEPROM_DATA_S, acPassword), sizeof(acPassword));
    acSSID[SSID_LENGTH - 1] = '\0';
    acPassword[WIFI_PASSWORD_LENGTH - 1] = '\0';
    peerAddr = INADDR_NONE;
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    if(ESP_OK != esp_wifi_set_max_tx_power(WIFI_MAX_TX_POWER)){
        Serial.println("Failed to config wifi max tx power.");
    }
    while (!udpClient.listen(CYMPLEFACE_CAM_PORT)) //等待udp监听设置成功
    {
    }
    udpClient.onPacket(onPacketCallBack);
    if(acSSID[0] != 0){
        connect(acSSID, acPassword);
    }
}
void wlanMsgClass::connect(const char *SSID, const char *password){
    bool ret = false;
    if(0 == SSID[0]){
        Serial.println("SSID missing");
    }
    if(0 != strcmp(SSID, acSSID)){
        ret = true;
        strncpy(acSSID, SSID, SSID_LENGTH);
        eepromApi::write((void *)SSID, OFFSET(EEPROM_DATA_S, acSSID), SSID_LENGTH);
    }
    if(0 != strcmp(password, acPassword)){
        ret = true;
        strncpy(acPassword, password, WIFI_PASSWORD_LENGTH);
        eepromApi::write((void *)password, OFFSET(EEPROM_DATA_S, acPassword), WIFI_PASSWORD_LENGTH);
    }
    if(ret){
        Serial.printf("Connecting to %s\n", acSSID);
        networkApi::connect(acSSID, acPassword);
    }
}
void wlanMsgClass::connect(){
    Serial.printf("Connecting to %s\n", acSSID);
    networkApi::connect(acSSID, acPassword);
}

void wlanMsgClass::send(uint8_t *data, size_t len, IPAddress ip, uint16_t port){
    if(len > CONFIG_TCP_MSS){
        Serial.printf("wlanMsgClass::send: data too large:%u\n", len);
        return;
    }
    udpClient.writeTo(data, len, ip, port);
}

void wlanMsgClass::send(uint8_t *data, size_t len){
    send(data, len, peerAddr, CYMPLEFACE_SERVER_PORT);
}

int wlanMsgClass::runFrame(unsigned long currentT){
    long long deltaT = (long long)currentT - heartBeatTimer;
    deltaT = abs(deltaT);
    if((!bHeartbeatTimeout) && (deltaT > WLAN_HEARTBEAT_TIMEOUT)){
        bHeartbeatTimeout = true;
        pledObj->ledMode(LED_MODE_ALWAYS_ON, 0, 0); 
    }
    if(bHeartbeatTimeout){
        if(!WiFi.isConnected()){
            Serial.println("Wlan disconnected");
            connect();
            delay(3000);
            return 1;
        }else{
            Serial.println("Heartbeat timeout");
            delay(1000);
            return 1;
        }
    }
    return 0;
}


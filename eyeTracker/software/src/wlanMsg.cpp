#include <AsyncUDP.h>
#include "common.h"
#include "drv/eeprom.h"
#include "drv/network.h"
#include "esp32cam.h"
#include "wlanMsg.h"
#include <esp_wifi.h>
#include "wifiUser.h"
#include "serialMsg.h"

#define WIFI_MAX_TX_POWER 80
wlanMsgClass *pwlanMsgObj = NULL;
IPAddress brocastAddr;
IPAddress unicastAddr;
bool bHeartbeatTimeout = true;
unsigned long heartBeatTimer = 0;
#define AP_NAME "Cymple_Face"
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
            brocastAddr = packet.remoteIP();
            break;
        case MSG_SERVER_UNICAST_HEARTBEAT_E:
            heartBeatTimer = millis();
            bHeartbeatTimeout = false;
            unicastAddr = packet.remoteIP();
            break;
        case MSG_CONFIG_WIFI_E:{
            if(msgLen != sizeof(MSG_WLAN_WIFI_CONFIG_S)){
                serial_writelog("Sizeof MSG_WLAN_WIFI_CONFIG_S dismatch: rcv: %u, local %u\r\n", msgLen, sizeof(MSG_WLAN_WIFI_CONFIG_S));
            }else{
                MSG_WLAN_WIFI_CONFIG_S *tmp = (MSG_WLAN_WIFI_CONFIG_S *)pstMsgHdr;
                tmp->SSID[SSID_LENGTH - 1] = '\0';
                tmp->password[WIFI_PASSWORD_LENGTH - 1] = '\0';
                WiFi.mode(WIFI_STA);
                WiFi.persistent(false);
                pwlanMsgObj->tryConCount = 0;
                pwlanMsgObj->connect(tmp->SSID, tmp->password);
            }
            break;
        }
            
        default:
            serial_writelog("Invalid msg type%u, msg len:%d\n", pstMsgHdr->uiType, msgLen);
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
    unicastAddr = INADDR_NONE;
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    // if(ESP_OK != esp_wifi_set_max_tx_power(WIFI_MAX_TX_POWER)){
    //     serial_writelog("Failed to config wifi max tx power.");
    // }
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
        serial_writelog("SSID missing\r\n");
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
        serial_writelog("Connecting to %s\n", acSSID);
        networkApi::connect(acSSID, acPassword);
    }
}


void wlanMsgClass::connect(){
    serial_writelog("Connecting to %s\n", acSSID);
    networkApi::connect(acSSID, acPassword);
}

void wlanMsgClass::APMode(){
    WiFi.mode(WIFI_AP);
    WiFi.persistent(false);
    WiFi.softAP(AP_NAME);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   //设置AP热点IP和子网掩码
    wifiConfig();
}

void wlanMsgClass::send(uint8_t *data, size_t len, IPAddress ip, uint16_t port){
    if(len > CONFIG_TCP_MSS){
        serial_writelog("wlanMsgClass::send: data too large:%u\n", len);
        return;
    }
    udpClient.writeTo(data, len, ip, port);
}

void wlanMsgClass::send(uint8_t *data, size_t len){
    if(INADDR_NONE !=unicastAddr){
        send(data, len, unicastAddr, CYMPLEFACE_SERVER_PORT);
    }else{
        send(data, len, brocastAddr, CYMPLEFACE_SERVER_PORT);
    }
    
}

int wlanMsgClass::runFrame(unsigned long currentT){
    if(tryConCount > 2){
        if(3 == tryConCount){
            serial_writelog("Wating to config WIFI\r\n");
            APMode();
            tryConCount++;
        }
        checkDNS_HTTP();                  //检测客户端DNS&HTTP请求，也就是检查配网页面那部分
        delay(30);
        return 1;
    }
    long long deltaT = (long long)currentT - heartBeatTimer;
    deltaT = abs(deltaT);
    if((!bHeartbeatTimeout) && (deltaT > WLAN_HEARTBEAT_TIMEOUT)){
        bHeartbeatTimeout = true;
    }
    if(bHeartbeatTimeout){
        if(!WiFi.isConnected()){
            serial_writelog("Wlan disconnected\r\n");
            tryConCount++;
            connect();
            delay(3000);
            return 1;
        }else{
            serial_writelog("Heartbeat timeout\r\n");
            delay(1000);
            return 1;
        }
    }
    return 0;
}


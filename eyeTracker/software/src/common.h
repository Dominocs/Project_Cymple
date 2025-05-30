#ifndef COMMON_H
#define COMMON_H
#include <Arduino.h>
#define CYMPLEFACE_CAM_PORT      23302
#define CYMPLEFACE_SERVER_PORT   23303
#define SSID_LENGTH             32
#define WIFI_PASSWORD_LENGTH    64
#define WLAN_HEARTBEAT_TIMEOUT  5000
#define OFFSET(structure, member) ((int32_t)&((structure*)0)->member) 
typedef struct{
    uint16_t uiType;
    uint16_t uiLength;
}TLV_S;

#endif
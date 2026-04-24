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

#define STREAM_PREFIX 0xFFDDEDFE
typedef struct{
    uint32_t uiPrefix;
    uint16_t uiType;
    uint16_t uiLength;
}STREAM_TLV_S;

static inline void setStreamTLV(STREAM_TLV_S &tlv, uint16_t type, uint16_t len){
    tlv.uiPrefix = STREAM_PREFIX;
    tlv.uiType = type;
    tlv.uiLength = len;
}

enum{
    TRANSMIT_MODE_WIRELESS_E,
    TRANSMIT_MODE_WIRE_E,
    TRANSMIT_MODE_MAX_E,
};
extern uint8_t g_transmitMode;
#define VERSION_STR "DIY_1_4_0"
#endif
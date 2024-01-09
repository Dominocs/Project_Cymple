#ifndef IMUMSG_H
#define IMUMSG_H
#include "common.h"
#include "config.h"
#define SERIAL_RX_BUFF_SIZE 1024
enum{
    SERIAL_MSG_DEVICEINFO_E,
    SERIAL_MSG_WIFI_CONFIG_E,
    SERIAL_MSG_MAX_E
};

typedef struct{
    uint8_t hdr[4];
    TLV_S tlv;
}SERIAL_MSG_HDR_S, SERIAL_MSG_CONNECT_S;

typedef struct{
    SERIAL_MSG_HDR_S stHdr;
    char acSSID[SSID_LENGTH];
    char acPassword[WIFI_PASSWORD_LENGTH];
}SERIAL_MSG_WIFICONFIG_S;

class serialClass{
public:
    serialClass();
    void runFrame(unsigned long currentT);
private:
    void serialMsgCallback(uint16_t type, uint16_t len);
    bool bRcvSerialHdr = false;
    uint16_t usSerialRxDataLen = 0;
    uint16_t usSerialRxDataOffset = 0;
    char acSerialRxBuffer[SERIAL_RX_BUFF_SIZE];
    unsigned long timer = 0;
    bool getSerialMsgHead();
    inline bool isValidSerialHdr(const char *pData){
        return (0xff == pData[0])&&(0xfe == pData[1])&&(0xfd == pData[2])&&(0xfc == pData[3]);
    }
};

#define SERIAL_MIN_SIZE sizeof(SERIAL_MSG_HDR_S)

extern serialClass *pserialObj;
#endif /* IMUMSG_H */

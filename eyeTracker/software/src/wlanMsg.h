#ifndef WLANMSG_H
#define WLANMSG_H
#include <AsyncUDP.h>
#include "common.h"
enum{
    MSG_SERVER_HEARTBEAT_E,
    MSG_IMAGE_E,
    MSG_POSITION_CFG_E,
    MSG_REFRESH_RATE_CFT_E,
    MSG_CONFIG_WIFI_E,
    MSG_SERVER_UNICAST_HEARTBEAT_E,
    MSG_MAX_E,
};

/* 标定参数结构体，发送给tracker */
typedef TLV_S MSG_WLAN_HEARTBEAT_S;
typedef struct {
    TLV_S tlv;
    uint32_t uiOffset;
    uint32_t uiTotalLen;
    uint8_t ucFlags;
    uint8_t ucFrameIndex;
    uint8_t ucBattery;
    uint8_t ucReserved;
    uint8_t aucData[0];
}MSG_WLAN_IMAGE_S;

typedef struct{
    TLV_S tlv;
    char SSID[SSID_LENGTH];
    char password[WIFI_PASSWORD_LENGTH];
}MSG_WLAN_WIFI_CONFIG_S;

class wlanMsgClass{
private:
    char acSSID[SSID_LENGTH];
    char acPassword[WIFI_PASSWORD_LENGTH];
    uint32_t uiTxBufferSize = 0;
    AsyncUDP udpClient;
    bool isConnected = false;
    
public:
    uint8_t tryConCount = 0;
    wlanMsgClass();
    void connect(const char *SSID, const char *password);
    void connect();
    void send(uint8_t *data, size_t len, IPAddress ip, uint16_t port);
    void send(uint8_t *data, size_t len);
    int runFrame(unsigned long currentT);
    void APMode();
};
static IPAddress apIP(192, 168, 4, 1);            //设置AP的IP地址
extern wlanMsgClass *pwlanMsgObj;
#endif
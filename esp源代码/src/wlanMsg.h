#ifndef WLANMSG_H
#define WLANMSG_H
#include <AsyncUDP.h>
#include "common.h"
enum{
    MSG_SERVER_HEARTBEAT_E,
    MSG_IMAGE_E,
    MSG_REQ_CERTIFICATION_E,
    MSG_REPLY_CERTIFICATION_E,
    MSG_SERVER_UNICAST_HEARTBEAT_E,
    MSG_IMAGE_V2_E,
    MSG_REQ_FTV2_PARA_E,
    MSG_SET_FTV2_PARA_E,
    MSG_FTV2_PARA_E,
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

typedef struct {
    // tlv头部，uiType为MSG_IMAGE_V2_E，uiLength总长度(包含该头部)
    TLV_S tlv;
    // 偏移
    uint16_t uiOffset;
    // jpg图片总长度
    uint16_t uiTotalLen;
    // 当前包总长度（包含该头部）
    uint16_t uiDataLen;
    // 0 为左眼，1为右眼，2为单目
    uint8_t ucDeviceFlag;
    // 帧自增索引，每发送一帧图像加1
    uint8_t ucFrameIndex;
    // JPG图像数据
    uint8_t aucData[0];
}MSG_WLAN_IMAGE_V2_S;

// 如果是在电脑端编程，可以将图像resize到合适的大小或者压缩到整个包小于一个udp报文大小（65535），
// 无需手动分片（uiOffset == 0, uiTotalLen == uiDataLen）就可以直接发送
// ESP由于强制使用TCP MTU为1460，所以需要手动分片


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
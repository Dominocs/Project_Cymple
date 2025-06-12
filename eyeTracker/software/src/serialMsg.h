#ifndef IMUMSG_H
#define IMUMSG_H
#include "common.h"
#include "config.h"
#define SERIAL_RX_BUFF_SIZE 1024
// 消息类型枚举
typedef enum {
    SERIAL_MSG_LOG_E,
    SERIAL_MSG_REQ_DEVICEINFO_E,
    SERIAL_MSG_REPLY_DEVICEINFO_E,
    SERIAL_MSG_WIFI_CONFIG_E,
    SERIAL_MSG_IMAGE_E = 5,
    SERIAL_MSG_POSITION_CFG_E,
    SERIAL_MSG_MAX_E
} SERIAL_MSG_TYPE_E;


// TLV结构
typedef struct {
    uint32_t uiPrefix;
    uint16_t uiType;
    uint16_t uiLength;
} STREAM_TLV_S;

static inline void set_stream_tlv(STREAM_TLV_S *tlv, uint16_t type, uint16_t length) {
    // 设置TLV
    #define STREAM_PREFIX 0xFFDDEDFE
    tlv->uiPrefix = STREAM_PREFIX;
    tlv->uiType = type;
    tlv->uiLength = length;
}

// WiFi配置消息
typedef struct {
    STREAM_TLV_S tlv;
    char acSSID[32];
    char acPassword[64];
} SERIAL_MSG_WIFICONFIG_S;

typedef struct {
    STREAM_TLV_S tlv;
    uint8_t ucPosition;
    int8_t reserved[3];
}SERIAL_MSG_POSITION_CFG_S;

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

#define SERIAL_MIN_SIZE sizeof(STREAM_TLV_S)
extern void serial_writelog(const char *format, ...);
extern serialClass *pserialObj;
#endif /* IMUMSG_H */

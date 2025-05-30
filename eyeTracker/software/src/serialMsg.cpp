#include <WiFi.h>
#include "serialMsg.h"
#include "wlanMsg.h"
#define SERIAL_TIMEOUT 200
serialClass *pserialObj = NULL;
#define LOG_BUFF_SIZE 256
static uint8_t acSerialLogbuffer[LOG_BUFF_SIZE];

// 日志消息
typedef struct {
    STREAM_TLV_S tlv;
    uint16_t logLength;
    uint8_t logLevel;
    uint8_t ucDeviceFlag;
    char data[0];  // 柔性数组成员
} SERIAL_MSG_LOG_S;


void serial_writelog(const char *format, ...) {
    SERIAL_MSG_LOG_S *pTmp = (SERIAL_MSG_LOG_S *)acSerialLogbuffer;
    pTmp->logLevel = 0;
    pTmp->ucDeviceFlag = 0;
    va_list args;
    va_start(args, format);
    int16_t logSize = vsnprintf(pTmp->data, LOG_BUFF_SIZE - sizeof(SERIAL_MSG_LOG_S), format, args);
    va_end(args);
    set_stream_tlv(&pTmp->tlv, SERIAL_MSG_LOG_E, logSize + sizeof(SERIAL_MSG_LOG_S) - sizeof(STREAM_TLV_S));
    pTmp->logLength = logSize;
    Serial.write(acSerialLogbuffer, logSize + sizeof(SERIAL_MSG_LOG_S));
}

serialClass::serialClass(){
    memset(acSerialRxBuffer, 0, sizeof(acSerialRxBuffer));
    Serial.begin(115200);
    serial_writelog("CympleEye Serial Init!\r\n");
}

bool serialClass::getSerialMsgHead()
{
    // 确保有足够的数据来检查消息头
    if (usSerialRxDataLen < sizeof(STREAM_TLV_S)) {
        return false;
    }
    
    STREAM_TLV_S *pTmp = (STREAM_TLV_S *)(acSerialRxBuffer + usSerialRxDataOffset);
    
    // 检查前缀是否匹配
    if (pTmp->uiPrefix != STREAM_PREFIX) {
        return false;
    }
    
    // 检查类型是否有效
    if (pTmp->uiType >= SERIAL_MSG_MAX_E) {
        return false;
    }
    
    // 检查长度是否合理
    if (pTmp->uiLength > SERIAL_RX_BUFF_SIZE - sizeof(STREAM_TLV_S)) {
        return false;
    }
    
    return true;
}


void serialClass::serialMsgCallback(uint16_t type, uint16_t len){
    switch(type){
        case SERIAL_MSG_REQ_DEVICEINFO_E:
            serial_writelog("\n\nProject cympleEye\nContributor: Dominocs\nGithub & BiliBili & Vrc: Dominocs\nEmail: 1030487127@qq.com Version:DIY_1_0_0\n");
            if(WiFi.isConnected()){
                serial_writelog("Ip address:%s\n", WiFi.localIP().toString().c_str());
            }
            break;
        case SERIAL_MSG_WIFI_CONFIG_E:
            if(len != sizeof(SERIAL_MSG_WIFICONFIG_S)){
                serial_writelog("Sizeof SERIAL_MSG_WIFICONFIG_S dismatch: rcv: %u, local %u\r\n", len, sizeof(SERIAL_MSG_WIFICONFIG_S));
            }else{
                SERIAL_MSG_WIFICONFIG_S *tmp = (SERIAL_MSG_WIFICONFIG_S *)(acSerialRxBuffer + usSerialRxDataOffset);
                pwlanMsgObj->connect(tmp->acSSID, tmp->acPassword);
            }
            break;
        default:
            serial_writelog("Unrecgnized serial msg type: %u\n", type);
    }
}

void serialClass::runFrame(unsigned long currentT){
    if(Serial.available() <= 0){
        return;
    }
    
    /* serial timeout */
    if((currentT - timer) > 1000){
        usSerialRxDataLen = 0;
        usSerialRxDataOffset = 0;
        bRcvSerialHdr = false;
    }
    timer = currentT;
    usSerialRxDataLen  += Serial.read(acSerialRxBuffer + usSerialRxDataLen, SERIAL_RX_BUFF_SIZE - usSerialRxDataLen);
    while(usSerialRxDataLen >= SERIAL_MIN_SIZE){
        if(!bRcvSerialHdr){
            bRcvSerialHdr = getSerialMsgHead();
            if(!bRcvSerialHdr){
                usSerialRxDataOffset++;
                usSerialRxDataLen--;
                continue;
            }
        }
        if(bRcvSerialHdr){
            STREAM_TLV_S *pTmp = (STREAM_TLV_S *)(void *)(acSerialRxBuffer + usSerialRxDataOffset);
            int len = pTmp->uiLength + sizeof(STREAM_TLV_S);
            bRcvSerialHdr = false;
            /* 长度异常 */
            if(len > SERIAL_RX_BUFF_SIZE){
                usSerialRxDataOffset += sizeof(uint8_t) * 4;
                usSerialRxDataLen -= sizeof(uint8_t) * 4; /* 向后偏移头标识符 */
                continue;
            }
            /* 正常解析 */
            else if(usSerialRxDataLen >= len){
                serialMsgCallback(pTmp->uiType, pTmp->uiLength + sizeof(STREAM_TLV_S));
                usSerialRxDataLen -= len;
                usSerialRxDataOffset += len;
            }else{
                break;
            }
        }
    }
    /* 复制到头部 */
    if(usSerialRxDataLen > 0){
        memcpy(acSerialRxBuffer, acSerialRxBuffer + usSerialRxDataOffset, usSerialRxDataLen);
        usSerialRxDataOffset = 0;
    }
}
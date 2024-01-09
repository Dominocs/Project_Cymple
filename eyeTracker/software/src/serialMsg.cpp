#include <WiFi.h>
#include "serialMsg.h"
#include "wlanMsg.h"
#define SERIAL_TIMEOUT 200
serialClass *pserialObj = NULL;
serialClass::serialClass(){
    memset(acSerialRxBuffer, 0, sizeof(acSerialRxBuffer));
    Serial.begin(115200);
    Serial.println("CympleEye Serial Init!");
}
bool inline serialClass::getSerialMsgHead(){
    if(isValidSerialHdr(acSerialRxBuffer + usSerialRxDataOffset)){
        return true;
    }
    return false;
}

void serialClass::serialMsgCallback(uint16_t type, uint16_t len){
    switch(type){
        case SERIAL_MSG_DEVICEINFO_E:
            if(len != sizeof(SERIAL_MSG_CONNECT_S)){
                Serial.printf("Sizeof SERIAL_MSG_CONNECT_S dismatch: rcv: %u, local %u", len, sizeof(SERIAL_MSG_CONNECT_S));
            }else{
                Serial.printf("\n\nProject cympleEye\nContributor: Dominocs\nGithub & BiliBili & Vrc: Dominocs\nEmail: 1030487127@qq.com\n");
                if(WiFi.isConnected()){
                    Serial.printf("Ip address:%s\n", WiFi.localIP().toString().c_str());
                }
                Serial.println();
            }
            break;
        case SERIAL_MSG_WIFI_CONFIG_E:
            if(len != sizeof(SERIAL_MSG_WIFICONFIG_S)){
                Serial.printf("Sizeof SERIAL_MSG_WIFICONFIG_S dismatch: rcv: %u, local %u", len, sizeof(SERIAL_MSG_WIFICONFIG_S));
            }else{
                SERIAL_MSG_WIFICONFIG_S *tmp = (SERIAL_MSG_WIFICONFIG_S *)(acSerialRxBuffer + usSerialRxDataOffset);
                pwlanMsgObj->connect(tmp->acSSID, tmp->acPassword);
            }
            break;
        default:
            Serial.printf("Unrecgnized serial msg type: %u\n", type);
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
            SERIAL_MSG_HDR_S *pTmp = (SERIAL_MSG_HDR_S *)(void *)(acSerialRxBuffer + usSerialRxDataOffset);
            int len = pTmp->tlv.uiLength;
            bRcvSerialHdr = false;
            /* 长度异常 */
            if(len > SERIAL_RX_BUFF_SIZE){
                usSerialRxDataOffset += sizeof(uint8_t) * 4;
                usSerialRxDataLen -= sizeof(uint8_t) * 4; /* 向后偏移头标识符 */
                continue;
            }
            /* 正常解析 */
            else if(usSerialRxDataLen >= len){
                serialMsgCallback(pTmp->tlv.uiType, pTmp->tlv.uiLength);
                usSerialRxDataLen -= len;
                usSerialRxDataOffset += len;
            }
        }
    }
    /* 复制到头部 */
    if(usSerialRxDataLen > 0){
        memcpy(acSerialRxBuffer, acSerialRxBuffer + usSerialRxDataOffset, usSerialRxDataLen);
        usSerialRxDataOffset = 0;
    }
}
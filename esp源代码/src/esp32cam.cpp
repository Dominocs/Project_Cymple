#include <Arduino.h>
#include <esp_camera.h>
#include "common.h"
#include "esp32cam.h"
#include "camCfg.h"
#include "config.h"
#include "serialMsg.h"
#include "wlanMsg.h"
#include "drv/eeprom.h"
cameraClass *pCamera = NULL;
static const char *TAG = "cympleFace";
static const size_t MAX_DATA_LEN = CONFIG_TCP_MSS - sizeof(MSG_WLAN_IMAGE_S);

uint8_t sanitizeDeviceFlag(uint8_t ucFlag){
    if(ucFlag >= FLAG_MAX_E){
        return FLAG_LEFT_EYE_E;
    }
    return ucFlag;
}

void updateDeviceFlag(uint8_t ucFlag){
    ucFlag = sanitizeDeviceFlag(ucFlag);
    eepromApi::write(&ucFlag, OFFSET(EEPROM_DATA_S, ucFlags), sizeof(ucFlag));
    if(pCamera){
        pCamera->ucFlags = ucFlag;
    }
}

cameraClass::cameraClass(){
    uint8_t storedFlag = FLAG_LEFT_EYE_E;
    eepromApi::read(&storedFlag, OFFSET(EEPROM_DATA_S, ucFlags), sizeof(storedFlag));
    ucFlags = sanitizeDeviceFlag(storedFlag);
    if(ucFlags != storedFlag){
        eepromApi::write(&ucFlags, OFFSET(EEPROM_DATA_S, ucFlags), sizeof(ucFlags));
    }
    memset(aucTxBuffer, 0, sizeof(aucTxBuffer));
    camera_config_t config = camconfig;
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        serial_writelog("ERROR:%d\r\n", err);
        ESP_LOGE(TAG, "Camera Init Failed");
    }else{
        serial_writelog("Camera succeed to init\r\n");
        sensor_t * s = esp_camera_sensor_get();
        s->set_brightness(s, 2);     // -2 to 2
        s->set_contrast(s, 0);       // -2 to 2
        s->set_saturation(s, 0);     // -2 to 2
        s->set_special_effect(s, 2); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
        s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
        s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
        s->set_wb_mode(s, 1);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
        s->set_aec2(s, 1);           // 0 = disable , 1 = enable
        s->set_ae_level(s, 0);       // -2 to 2
        s->set_aec_value(s, 800);    // 0 to 1200
        s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
        s->set_agc_gain(s, 10);       // 0 to 30
        s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
        s->set_bpc(s, 1);            // 0 = disable , 1 = enable
        s->set_wpc(s, 1);            // 0 = disable , 1 = enable
        s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
        s->set_lenc(s, 0);           // 0 = disable , 1 = enable
        s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
        s->set_vflip(s, 0);          // 0 = disable , 1 = enable
        s->set_dcw(s, 1);            // 0 = disable , 1 = enable
        s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
        s->set_framesize(s, FRAMESIZE_QVGA);
        s->set_quality(s, 7);
        bValid = true;
    }
}
int cameraClass::runFrame(){
    if(!bValid){
        serial_writelog("Camera failed to init\r\n");
        return 1;
    }
    MSG_WLAN_IMAGE_S *pstImgMsg = (MSG_WLAN_IMAGE_S *)aucTxBuffer;
    camera_fb_t *pic = esp_camera_fb_get();
    if(NULL == pic){
        serial_writelog("NULL == pic in cameraClass::runFrame\r\n");
        return 1;
    }
    size_t imgLen = pic->len;
    
    if(TRANSMIT_MODE_WIRELESS_E == g_transmitMode){
        MSG_WLAN_IMAGE_V2_S *pstImgMsg = (MSG_WLAN_IMAGE_V2_S *)aucTxBuffer;
        if(TX_BUFF_SIZE < sizeof(MSG_WLAN_IMAGE_V2_S) + imgLen){
            serial_writelog("TX buffer is not enough, img len:%u\n", imgLen);
            return 1;
        }
        memcpy(pstImgMsg->aucData,  pic->buf, imgLen);
        esp_camera_fb_return(pic);
        sendByWireless(pstImgMsg, imgLen, ucFlags);
    }else if(TRANSMIT_MODE_WIRE_E == g_transmitMode){
        SERIAL_MSG_WLAN_PACKET_S *pstWlanPacketHdr = (SERIAL_MSG_WLAN_PACKET_S *)aucTxBuffer;
        MSG_WLAN_IMAGE_V2_S *pstImagePacket = (MSG_WLAN_IMAGE_V2_S *)pstWlanPacketHdr->aucData;
        memcpy(pstImagePacket->aucData,  pic->buf, imgLen);
        esp_camera_fb_return(pic);
        sendByUsb(pstWlanPacketHdr, imgLen, ucFlags);
    }
    ucFrameIndex++;
    return 0;
}

void cameraClass::sendByUsb(SERIAL_MSG_WLAN_PACKET_S *pstImgMsg, size_t imgLen, uint8_t ucDeviceFlag){
    setStreamTLV(pstImgMsg->tlv, SERIAL_MSG_WLAN_PACKET_E, imgLen + sizeof(MSG_WLAN_IMAGE_V2_S));
    MSG_WLAN_IMAGE_V2_S *pstwlanPacket = (MSG_WLAN_IMAGE_V2_S *)pstImgMsg->aucData;
    pstwlanPacket->tlv.uiType = MSG_IMAGE_V2_E;
    pstwlanPacket->tlv.uiLength = imgLen + sizeof(MSG_WLAN_IMAGE_V2_S) - sizeof(TLV_S);
    pstwlanPacket->ucDeviceFlag = ucDeviceFlag;
    pstwlanPacket->uiTotalLen = imgLen;
    pstwlanPacket->ucFrameIndex = ucFrameIndex;
    pstwlanPacket->uiOffset = 0;
    pstwlanPacket->uiDataLen = imgLen;
    Serial.write((const char*)pstImgMsg, imgLen + sizeof(SERIAL_MSG_WLAN_PACKET_S) + sizeof(MSG_WLAN_IMAGE_V2_S));
    return;
}
void cameraClass::sendByWireless(MSG_WLAN_IMAGE_V2_S *pstImgMsg, size_t imgLen, uint8_t ucDeviceFlag){
    if(!pwlanMsgObj){
        return;
    }
    size_t copyLen = 0;
    int dataOffset = 0;
    while(imgLen > dataOffset){
        copyLen = min(MAX_DATA_LEN, imgLen - dataOffset);
        pstImgMsg = (MSG_WLAN_IMAGE_V2_S *)(aucTxBuffer + dataOffset);
        pstImgMsg->tlv.uiType = MSG_IMAGE_V2_E;
        pstImgMsg->tlv.uiLength = sizeof(MSG_WLAN_IMAGE_V2_S) + copyLen - sizeof(TLV_S);
        pstImgMsg->ucDeviceFlag = ucDeviceFlag;
        pstImgMsg->uiTotalLen = imgLen;
        pstImgMsg->ucFrameIndex = ucFrameIndex;
        pstImgMsg->uiOffset = dataOffset;
        pstImgMsg->uiDataLen = copyLen;
        pwlanMsgObj->send(aucTxBuffer + dataOffset, sizeof(MSG_WLAN_IMAGE_V2_S) + copyLen);
        dataOffset += copyLen;
    }
}
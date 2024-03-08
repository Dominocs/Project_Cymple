#include <Arduino.h>
#include <esp_camera.h>
#include "common.h"
#include "esp32cam.h"
#include "camCfg.h"
#include "config.h"
#include "wlanMsg.h"
#include "drv/eeprom.h"
cameraClass *pCamera = NULL;
static const char *TAG = "cympleFace";
static const size_t MAX_DATA_LEN = CONFIG_TCP_MSS - sizeof(MSG_WLAN_IMAGE_S);
cameraClass::cameraClass(){
    eepromApi::read(&ucFlags, OFFSET(EEPROM_DATA_S, ucFlags), sizeof(ucFlags));
    if(ucFlags >= FLAG_MAX_E){
        ucFlags = FLAG_LEFT_EYE_E;
    }
    memset(aucTxBuffer, 0, sizeof(aucTxBuffer));
    camera_config_t config = camconfig;
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("ERROR:%d", err);
        ESP_LOGE(TAG, "Camera Init Failed");
    }else{
        Serial.println("Camera succeed to init");
        sensor_t * s = esp_camera_sensor_get();
        s->set_brightness(s, 0);     // -2 to 2
        s->set_contrast(s, 0);       // -2 to 2
        s->set_saturation(s, 0);     // -2 to 2
        s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
        s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
        s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
        s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
        s->set_aec2(s, 0);           // 0 = disable , 1 = enable
        s->set_ae_level(s, 0);       // -2 to 2
        s->set_aec_value(s, 400);    // 0 to 1200
        s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
        s->set_agc_gain(s, 0);       // 0 to 30
        s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
        s->set_bpc(s, 0);            // 0 = disable , 1 = enable
        s->set_wpc(s, 1);            // 0 = disable , 1 = enable
        s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
        s->set_lenc(s, 1);           // 0 = disable , 1 = enable
        s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
        s->set_vflip(s, 0);          // 0 = disable , 1 = enable
        s->set_dcw(s, 1);            // 0 = disable , 1 = enable
        s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
        bValid = true;
    }
}
int cameraClass::runFrame(){
    if(!bValid){
        Serial.println("Camera failed to init");
        return 1;
    }
    MSG_WLAN_IMAGE_S *pstImgMsg = (MSG_WLAN_IMAGE_S *)aucTxBuffer;
    camera_fb_t *pic = esp_camera_fb_get();
    if(NULL == pic){
        Serial.println("NULL == pic in cameraClass::runFrame");
        return 1;
    }
    size_t imgLen = pic->len;
    size_t copyLen = 0;
    memset(pstImgMsg, 0, sizeof(MSG_WLAN_IMAGE_S));
    pstImgMsg->tlv.uiType = MSG_IMAGE_E;
    pstImgMsg->ucFlags = ucFlags;
    pstImgMsg->uiTotalLen = imgLen;
    pstImgMsg->ucFrameIndex = ucFrameIndex;
    while(imgLen > 0){
        copyLen = min(MAX_DATA_LEN, imgLen);
        pstImgMsg->tlv.uiLength = sizeof(MSG_WLAN_IMAGE_S) + copyLen;
        memcpy(pstImgMsg->aucData, pic->buf + pstImgMsg->uiOffset, copyLen);
        pwlanMsgObj->send(aucTxBuffer, pstImgMsg->tlv.uiLength);
        pstImgMsg->uiOffset += copyLen;
        imgLen -= copyLen;
    }
    esp_camera_fb_return(pic);
    ucFrameIndex++;
    return 0;
}
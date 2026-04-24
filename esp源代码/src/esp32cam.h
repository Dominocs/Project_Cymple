#ifndef _ESP32CAM_H_
#define _ESP32CAM_H_
#include "common.h"
#include "wlanMsg.h"
#include "serialMsg.h"
enum{
  FLAG_LEFT_EYE_E,
  FLAG_RIGHT_EYE_E,
  FLAG_MOUTH_E,
  FLAG_MAX_E
};
uint8_t sanitizeDeviceFlag(uint8_t ucFlag);
void updateDeviceFlag(uint8_t ucFlag);
class cameraClass
{
public:
  cameraClass();
  int runFrame();
  uint8_t ucFlags;
private:
  #define TX_BUFF_SIZE 40960
  uint8_t aucTxBuffer[TX_BUFF_SIZE];
  bool bValid = false;
  bool bParaInit = false;
  uint8_t ucFrameIndex = 0;
  void sendByUsb(SERIAL_MSG_WLAN_PACKET_S *pstImgMsg, size_t imgLen, uint8_t ucDeviceFlag);
  void sendByWireless(MSG_WLAN_IMAGE_V2_S *pstImgMsg, size_t imgLen, uint8_t ucDeviceFlag);
};
extern cameraClass *pCamera;
#endif
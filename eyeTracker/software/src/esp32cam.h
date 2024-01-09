#ifndef _ESP32CAM_H_
#define _ESP32CAM_H_
#include "common.h"
#include "wlanMsg.h"
enum{
  FLAG_LEFT_EYE_E,
  FLAG_RIGHT_EYE_E,
  FLAG_MOUTH_E,
  FLAG_MAX_E
};
class cameraClass
{
public:
  cameraClass();
  int runFrame();
  uint8_t ucFlags;
private:
  uint8_t aucTxBuffer[CONFIG_TCP_MSS];
  bool bValid = false;
  bool bParaInit = false;
  uint8_t ucFrameIndex = 0;
};
extern cameraClass *pCamera;
#endif
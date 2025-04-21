#ifndef EEPROM_HPP
#define EEPROM_HPP
#include <EEPROM.h>
#include "../common.h"
#define FLAG2_FAST_MODE 0x01
typedef struct{
    char acSSID[SSID_LENGTH];
    char acPassword[WIFI_PASSWORD_LENGTH];
    uint8_t ucFlags;
}EEPROM_DATA_S;
namespace eepromApi{
    static void read(void *pData, uint32_t uiOffset, uint32_t length){
        uint8_t *pcData = (uint8_t *)pData;
        for(uint32_t uiAddr = 0; uiAddr < length; uiAddr++)
        {
            pcData[uiAddr] = EEPROM.read(uiAddr + uiOffset);
        }
    }
    static void write(void *pData, uint32_t uiOffset, uint32_t length){
        uint8_t *pcData = (uint8_t *)pData;
        for(uint32_t uiAddr = 0; uiAddr < length; uiAddr++)
        {
            EEPROM.write(uiAddr + uiOffset, pcData[uiAddr]);
        }
        EEPROM.commit();
    }
}
#endif
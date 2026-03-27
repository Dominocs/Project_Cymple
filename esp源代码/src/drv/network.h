#ifndef _NETWORK_HPP
#define _NETWORK_HPP
#include <Arduino.h>
#include <WiFi.h>
#include "../serialMsg.h"
namespace networkApi
{
    static bool connect(const char *pSSID, const char *pPassword)
    {
        WiFi.begin(pSSID, pPassword);
        if(WL_CONNECTED != WiFi.waitForConnectResult())
        {
            serial_writelog("Fail to connect to %s\n", pSSID);
        }
        else{
            serial_writelog("Success to connect to WIFI\r\n");
        }
        return WiFi.isConnected();
    }
};


#endif 
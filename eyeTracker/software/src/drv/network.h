#ifndef _NETWORK_HPP
#define _NETWORK_HPP
#include <Arduino.h>
#include <WiFi.h>

namespace networkApi
{
    static bool connect(const char *pSSID, const char *pPassword)
    {
        WiFi.begin(pSSID, pPassword);
        if(WL_CONNECTED != WiFi.waitForConnectResult())
        {
            Serial.printf("Fail to connect to %s\n", pSSID);
        }
        else{
            Serial.println("Success to connect to WIFI");
        }
        return WiFi.isConnected();
    }
};


#endif 
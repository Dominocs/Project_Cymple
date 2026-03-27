#ifndef __WIFIUSER_H__
#define __WIFIUSER_H__
 
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>      //用于设备域名 MDNS.begin("esp32")
#include <esp_wifi.h>     //用于esp_wifi_restore() 删除保存的wifi信息
 
//===========需要调用的函数===========
void checkDNS_HTTP();                 //检测客户端DNS&HTTP请求
 
//===========内部函数===========
void handleRoot();                    //处理网站根目录的访问请求
void handleConfigWifi() ;             //提交数据后的提示页面
void handleNotFound();                //处理404情况的函数'handleNotFound'
void initDNS();                       //开启DNS服务器
void initWebServer();                 //初始化WebServer
bool scanWiFi();                      //扫描附近的WiFi，为了显示在配网界面
void wifiConfig();                    //配置配网功能

#endif
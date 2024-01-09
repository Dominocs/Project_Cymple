#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <Arduino.h>
enum board{
    esp32cam,
    cympleeye,
};
#define BOARD_DEF cympleeye
/* LED配置 */
 #define LED_PIN 33 /* GPIO33 for espcam*/
#define BATTERY_UPDATE_TIME  10000//10S刷新一次  
#define BATTERY_PIN          A0    //A0
#define BATEERY_ADC_MAX      1024.0   //ADC读出值范围为0-1024
#define BATTERY_MAX          4.2    //电池满电电压 
#define BATTERY_MIN          3.7    //电池零电电压
#define BATTERY_DOT          (10.0/(47.0 + 10.0))  //分压
#endif /* CONFIG_HPP */

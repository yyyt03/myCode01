#include "network_manager.h"
#include "esp8266.h"
#include "onenet.h"
#include "usart.h"
#include "display_manager.h"
#include "delay.h"
#include <stddef.h>  // 为NULL提供定义

uint8_t NetworkManager_Initialize(void)
{
    // 初始化ESP8266
    ESP8266_Init();

    // NTP时间同步
    Display_ShowConnectionStatus("NTP Syncing...");
    uint8_t ntp_tries = 3;
    while(ntp_tries--) {
        if(ESP8266_GetNTPTime() == 0) {
            UsartPrintf(USART_DEBUG, "[NTP] Initial sync success\r\n");
            break;
        }
        delayms(1000);
        UsartPrintf(USART_DEBUG, "[NTP] Retry initial sync...\r\n");
    }
    
    // 连接OneNET平台
    Display_ShowConnectionStatus("Conn OneNET...");
    while(OneNet_DevLink()) {
        delayms(500);
        OLED_ShowStr(0, 2, (uint8_t *)"Retrying...", 2);
    }
    OneNET_Subscribe();
    
    Display_ShowConnectionStatus("SmartLight OK");
    
    // 显示初始时间
    char time_str[20];
    sprintf(time_str, "%04d-%02d-%02d", ntp_time.year, ntp_time.month, ntp_time.day);
    OLED_ShowStr(0, 0, (uint8_t *)time_str, 1);
    sprintf(time_str, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
    OLED_ShowStr(64, 0, (uint8_t *)time_str, 1);
    
    delayms(200);
    OLED_CLS();
    
    return 0;
}

void NetworkManager_SendData(void)
{
    UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
    OneNet_SendData();
    ESP8266_Clear();
}

void NetworkManager_ProcessIncoming(void)
{
    unsigned char *dataPtr = ESP8266_GetIPD(0);
    if(dataPtr != NULL)
        OneNet_RevPro(dataPtr);
}
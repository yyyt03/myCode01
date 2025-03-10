#include "display_manager.h"
#include "oled.h"
#include <stdio.h>

// 显示行号
#define LINE_MODE   1
#define LINE_ANGLE  3
#define LINE_LIGHT  5
#define LINE_SEND   7

// 初始化显示
void Display_Init(void) {
    OLED_Init();
    OLED_CLS();
}

// 更新时钟显示
void Display_UpdateClock(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t is_valid) {
    char time_str[20];
    
    if(is_valid && year >= 2000) {
        sprintf(time_str, "%04d-%02d-%02d", year, month, day);
        OLED_ShowStr(0, 0, (uint8_t *)time_str, 1);
        
        sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);
        OLED_ShowStr(64, 0, (uint8_t *)time_str, 1);
    } else {
        OLED_ShowStr(0, 0, (uint8_t *)"NTP Time Invalid", 1);
    }
}

// 更新模式显示
void Display_UpdateMode(uint8_t mode) {
    char mode_str[16];
    
    if(mode == 0)
        sprintf(mode_str, "Mode:Auto   ");
    else if(mode == 1)
        sprintf(mode_str, "Mode:Manual ");
    else
        sprintf(mode_str, "Mode:Timer  ");
        
    OLED_ShowStr(0, LINE_MODE, (uint8_t *)mode_str, 2);
}

// 更新角度显示
void Display_UpdateAngle(uint8_t angle) {
    char angle_str[16];
    sprintf(angle_str, "Angle:%3d   ", angle);
    OLED_ShowStr(0, LINE_ANGLE, (uint8_t *)angle_str, 2);
}

// 更新光照值显示
void Display_UpdateLightValue(float light_value) {
    char light_info[16];
    sprintf(light_info, "Lux:%-5.0f   ", light_value);
    OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)light_info, 2);
}

// 更新定时设置显示和光标
void Display_UpdateTimerSettings(uint8_t start_hour, uint8_t start_minute, uint8_t end_hour, uint8_t end_minute, uint8_t angle, uint8_t setting_index, uint8_t prev_index) {
    // 显示定时设置字符串
    char timer_str[16];
    sprintf(timer_str, "S:%02d:%02d E:%02d:%02d", start_hour, start_minute, end_hour, end_minute);
    OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)timer_str, 2);
    
    // 显示角度设置
    char angle_str[16];
    sprintf(angle_str, "Set:%3d", angle);
    OLED_ShowStr(83, LINE_ANGLE, (uint8_t *)angle_str, 1);
    
    // 只有当索引变化时才更新光标
    if(prev_index != setting_index) {
        // 清除所有可能位置的光标
        OLED_ShowStr(20, LINE_SEND, (uint8_t *)" ", 1); 
        OLED_ShowStr(41, LINE_SEND, (uint8_t *)" ", 1);
        OLED_ShowStr(82, LINE_SEND, (uint8_t *)" ", 1);
        OLED_ShowStr(105, LINE_SEND, (uint8_t *)" ", 1);
        OLED_ShowStr(112, 4, (uint8_t *)" ", 1);
        
        // 显示新位置的光标
        switch(setting_index) {
            case 0: OLED_ShowStr(20, LINE_SEND, (uint8_t *)"^", 1); break;
            case 1: OLED_ShowStr(41, LINE_SEND, (uint8_t *)"^", 1); break;
            case 2: OLED_ShowStr(82, LINE_SEND, (uint8_t *)"^", 1); break;
            case 3: OLED_ShowStr(105, LINE_SEND, (uint8_t *)"^", 1); break;
            case 4: OLED_ShowStr(112, 4, (uint8_t *)"^", 1); break;
        }
    }
}

// 显示连接状态
void Display_ShowConnectionStatus(const char* status) {
    OLED_CLS();
    OLED_ShowStr(0, 0, (uint8_t *)status, 2);
}

// 添加函数实现
void Display_ShowError(const char* error_message)
{
    OLED_CLS();
    OLED_ShowStr(0, 2, (uint8_t *)error_message, 1);
}
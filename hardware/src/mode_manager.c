#include "mode_manager.h"
#include "usart.h"
#include "oled.h"
#include <stdio.h>

// 阈值定义
#define LIGHT_THRESHOLD_LOW       50
#define LIGHT_THRESHOLD_MID_LOW  100
#define LIGHT_THRESHOLD_MID      200
#define LIGHT_THRESHOLD_MID_HIGH 500
#define LIGHT_THRESHOLD_HIGH    1000

// 角度定义
#define ANGLE_ULTRA_LOW     0
#define ANGLE_LOW          30
#define ANGLE_MID          90
#define ANGLE_HIGH        150
#define ANGLE_ULTRA_HIGH  180
#define ANGLE_STEP        30

// 显示行号
#define LINE_MODE   1
#define LINE_ANGLE  3
#define LINE_LIGHT  5
#define LINE_SEND   7

// 模式管理的全局变量
SystemMode work_mode = AUTO_MODE;
SystemMode prev_work_mode = AUTO_MODE;
uint8_t manual_angle = 90;
uint8_t angle = 0;
uint8_t timer_setting_index = 0;
uint8_t prev_timer_setting_index = 0xFF;


TimerControl timer_setting = {8, 0, 19, 0, 150};

// 初始化模式管理
void ModeManager_Init(void) {
    work_mode = AUTO_MODE;
    prev_work_mode = AUTO_MODE;
    manual_angle = 90;
    angle = 0;
    timer_setting_index = 0;
    prev_timer_setting_index = 0xFF;
}

// 模式切换
void ModeManager_SwitchMode(void) {
    if(work_mode == AUTO_MODE) 
        work_mode = MANUAL_MODE;
    else if(work_mode == MANUAL_MODE) 
        work_mode = TIMER_MODE;
    else 
        work_mode = AUTO_MODE;
        
    // 模式切换时的设置
    if(work_mode == MANUAL_MODE) manual_angle = 90;
    timer_setting_index = 0;
}

// 处理模式变更的显示清理等操作
void ModeManager_HandleModeChange(void) {
    if(prev_work_mode != work_mode) {
        UsartPrintf(USART_DEBUG, "[MODE] Mode changed from %d to %d\r\n", prev_work_mode, work_mode);
        
        if(prev_work_mode == TIMER_MODE && work_mode != TIMER_MODE) {
            // 清除显示内容
            OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)"                    ", 2);
            OLED_ShowStr(0, LINE_SEND, (uint8_t *)"                    ", 1);
            OLED_ShowStr(112, 4, (uint8_t *)"  ", 1);
            OLED_ShowStr(83, LINE_ANGLE, (uint8_t *)"        ", 1);
            
            prev_timer_setting_index = 0xFF;
            timer_setting_index = 0;
        }
        
        prev_work_mode = work_mode;
    }
}

// 光照强度到角度的映射
uint8_t LightToFixedAngle(float light_value) {
    if(light_value < LIGHT_THRESHOLD_LOW)        
        return ANGLE_ULTRA_LOW;
    else if(light_value < LIGHT_THRESHOLD_MID_LOW) 
        return ANGLE_LOW;
    else if(light_value < LIGHT_THRESHOLD_MID)     
        return ANGLE_MID;
    else if(light_value < LIGHT_THRESHOLD_MID_HIGH)
        return ANGLE_HIGH;
    else                                            
        return ANGLE_ULTRA_HIGH;
}

// 根据当前模式和光照强度更新角度
void ModeManager_UpdateAngle(float light_value) {
    if(work_mode == AUTO_MODE) {
        angle = LightToFixedAngle(light_value);
    }
    else if(work_mode == MANUAL_MODE) {
        angle = manual_angle;
    }
    // TIMER_MODE下角度由定时器处理
}

// 获取当前角度值
uint8_t ModeManager_GetCurrentAngle(void) {
    return angle;
}

// 处理定时模式的时间判断和角度设置
void ModeManager_HandleTimerMode(uint8_t curr_hour, uint8_t curr_minute) {
    if(work_mode == TIMER_MODE) {
        // 将时间转换为分钟计数
        uint16_t curr_time_mins = curr_hour * 60 + curr_minute;
        uint16_t start_time_mins = timer_setting.start_hour * 60 + timer_setting.start_minute;
        uint16_t end_time_mins = timer_setting.end_hour * 60 + timer_setting.end_minute;
        
        // 判断是否在时间段内（处理跨天情况）
        uint8_t is_in_time_range;
        if(start_time_mins <= end_time_mins) {
            is_in_time_range = (curr_time_mins >= start_time_mins && curr_time_mins <= end_time_mins);
        } else {
            is_in_time_range = (curr_time_mins >= start_time_mins || curr_time_mins <= end_time_mins);
        }
        
        // 根据时间段设置角度
        if(is_in_time_range) {
            angle = timer_setting.angle;
        } else {
            angle = 0;
        }
        
        UsartPrintf(USART_DEBUG, "[TIMER] Time %02d:%02d, in range: %d, angle=%d\r\n",
                    curr_hour, curr_minute, is_in_time_range, angle);
    }
}

// 处理定时模式下的设置调整
void ModeManager_AdjustTimerSettings(uint8_t key_num) {
    switch(key_num) {
        case 2: // 功能键 - 循环切换设置项
            if(work_mode == MANUAL_MODE) {
                manual_angle = (manual_angle >= 180) ? 0 : manual_angle + ANGLE_STEP;
            }
            else if(work_mode == TIMER_MODE) {
                timer_setting_index = (timer_setting_index + 1) % 5;
            }
            break;
            
        case 3: // 增加当前设置项的值
            if(work_mode == MANUAL_MODE) {
                manual_angle = (manual_angle <= 0) ? 180 : manual_angle - ANGLE_STEP;
            }
            else if(work_mode == TIMER_MODE) {
                switch(timer_setting_index) {
                    case 0: timer_setting.start_hour = (timer_setting.start_hour + 1) % 24; break;
                    case 1: timer_setting.start_minute = (timer_setting.start_minute + 1) % 60; break;
                    case 2: timer_setting.end_hour = (timer_setting.end_hour + 1) % 24; break;
                    case 3: timer_setting.end_minute = (timer_setting.end_minute + 1) % 60; break;
                    case 4: timer_setting.angle = (timer_setting.angle + ANGLE_STEP > 180) ? 0 : timer_setting.angle + ANGLE_STEP; break;
                }
            }
            break;
            
        case 4: // 减少当前设置项的值
            if(work_mode == TIMER_MODE) {
                switch(timer_setting_index) {
                    case 0: timer_setting.start_hour = (timer_setting.start_hour == 0) ? 23 : timer_setting.start_hour - 1; break;
                    case 1: timer_setting.start_minute = (timer_setting.start_minute == 0) ? 59 : timer_setting.start_minute - 1; break;
                    case 2: timer_setting.end_hour = (timer_setting.end_hour == 0) ? 23 : timer_setting.end_hour - 1; break;
                    case 3: timer_setting.end_minute = (timer_setting.end_minute == 0) ? 59 : timer_setting.end_minute - 1; break;
                    case 4: timer_setting.angle = (timer_setting.angle <= 0) ? 180 : timer_setting.angle - ANGLE_STEP; break;
                }
            }
            break;
    }
}

// 获取当前模式
SystemMode ModeManager_GetCurrentMode(void) {
    return work_mode;
}

// 获取手动模式角度
uint8_t ModeManager_GetManualAngle(void) {
    return manual_angle;
}

// 获取定时设置索引
uint8_t ModeManager_GetTimerSettingIndex(void) {
    return timer_setting_index;
}

// 修改函数返回正确的变量名（将timer_settings改为timer_setting）
TimerControl *ModeManager_GetTimerSettings(void) {
    return &timer_setting;  // 修正变量名，使用正确的timer_setting而非timer_settings
}

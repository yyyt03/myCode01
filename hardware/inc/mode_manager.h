#ifndef __MODE_MANAGER_H
#define __MODE_MANAGER_H

#include "stm32f10x.h"
#include "sys.h"

// 添加TimerControl结构体定义
typedef struct {
    uint8_t start_hour;
    uint8_t start_minute;
    uint8_t end_hour;
    uint8_t end_minute;
    uint8_t angle;
} TimerControl;

// 模式控制函数
void ModeManager_Init(void);
void ModeManager_SwitchMode(void);
void ModeManager_HandleModeChange(void);
void ModeManager_UpdateAngle(float light_value);
uint8_t ModeManager_GetCurrentAngle(void);
void ModeManager_HandleTimerMode(uint8_t curr_hour, uint8_t curr_minute);
void ModeManager_AdjustTimerSettings(uint8_t key_num);

// 获取模式和设置值的函数
SystemMode ModeManager_GetCurrentMode(void);
uint8_t ModeManager_GetManualAngle(void);
TimerControl* ModeManager_GetTimerSettings(void);
uint8_t ModeManager_GetTimerSettingIndex(void);

extern uint8_t angle;

#endif
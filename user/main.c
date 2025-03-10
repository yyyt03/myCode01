/* 头文件包含分区说明 */
// 系统和标准库
#include "stm32f10x.h"
#include "sys.h"
#include <string.h>
#include <stdio.h>


// 硬件驱动
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "oled.h"  // 添加OLED头文件

// 网络通信
#include "onenet.h"
#include "esp8266.h"

// 自定义模块
#include "hw_control.h"
#include "mode_manager.h"
#include "display_manager.h"
#include "network_manager.h"  // 新增网络管理模块头文件

// 外部变量声明
extern volatile uint8_t clock_update_flag;
extern void UpdateClock(void);
extern NTP_Time ntp_time;  // 原来是 extern struct NTP_TIME ntp_time;

// 全局变量定义
uint8_t index_bh1750 = 0;
float light_value = 0;

// 本地函数声明
static void ProcessLightSensor(float light_value);
static void ProcessUserInput(void);

int main(void)
{    
    uint32_t tick = 0;
    uint8_t index_duoji = 0;
    unsigned short timeCount = 0;
    unsigned char *dataPtr = NULL;
    uint8_t prev_timer_setting_index = 0;
    
    // 硬件初始化
    Hardware_Init();
    ModeManager_Init();
    
    // 网络初始化
    if(NetworkManager_Initialize() != 0) {
        Display_ShowError("Network Init Failed");
        // 错误处理
    }
    
    // 主循环
    while(1)
    {
        // 时钟更新
        if(clock_update_flag)
        {
            clock_update_flag = 0;
            UpdateClock();
            
            Display_UpdateClock(ntp_time.year, ntp_time.month, ntp_time.day, 
                               ntp_time.hour, ntp_time.minute, ntp_time.second, 
                               ntp_time.is_valid);
            
            // 处理定时模式
            ModeManager_HandleTimerMode(ntp_time.hour, ntp_time.minute);
        }

        // 检查模式变化
        ModeManager_HandleModeChange();
        
        // 处理用户输入
        ProcessUserInput();

        // 光照传感器读取
        if(index_bh1750)
        {
            light_value = HW_ReadLightSensor();
            ProcessLightSensor(light_value);
            index_bh1750 = 0;
        }
        
        // 舵机控制
        if(index_duoji)
        {
            HW_UpdateServos(ModeManager_GetCurrentAngle());
            index_duoji = 0;
        }
        
        // 云平台数据处理
        if(++timeCount >= 100)    // 5秒间隔
        {
            NetworkManager_SendData();
            timeCount = 0;
        }

        // 接收数据处理
        NetworkManager_ProcessIncoming();
        
        // 定时器
        if(++tick % 50 == 0) index_bh1750 = 1;
        if(tick % 10 == 0) index_duoji = 1;
        delayms(10);
    }
}

// 处理用户输入
static void ProcessUserInput(void)
{
    key_Functoin();
    if(key_num != 0)
    {
        if(key_num == 1) {
            ModeManager_SwitchMode();
        } else {
            ModeManager_AdjustTimerSettings(key_num);
        }
        key_num = 0;
    }
}

// 处理光照传感器数据
static void ProcessLightSensor(float light_value)
{
    UsartPrintf(USART_DEBUG, "[DEBUG] light=%.0f\n", light_value);
    
    // 更新角度
    ModeManager_UpdateAngle(light_value);
    
    // 更新显示
    SystemMode mode = ModeManager_GetCurrentMode();
    Display_UpdateMode(mode);
    Display_UpdateAngle(ModeManager_GetCurrentAngle());
    
    if(mode == AUTO_MODE || mode == MANUAL_MODE) {
        Display_UpdateLightValue(light_value);
    }
    else if(mode == TIMER_MODE) {
        TimerControl* settings = ModeManager_GetTimerSettings();
        static uint8_t prev_timer_setting_index = 0;
        
        Display_UpdateTimerSettings(
            settings->start_hour, settings->start_minute,
            settings->end_hour, settings->end_minute,
            settings->angle, 
            ModeManager_GetTimerSettingIndex(), 
            prev_timer_setting_index
        );
        prev_timer_setting_index = ModeManager_GetTimerSettingIndex();
    }
}
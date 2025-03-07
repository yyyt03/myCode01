#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm32f10x.h"

// 菜单状态枚举 - 添加这个定义
typedef enum {
    MENU_NONE,             // 非菜单状态
    MENU_TIME_START_HOUR,  // 设置开始小时
    MENU_TIME_START_MIN,   // 设置开始分钟
    MENU_TIME_END_HOUR,    // 设置结束小时
    MENU_TIME_END_MIN,     // 设置结束分钟
    MENU_ANGLE_PRESET      // 角度预设选择
} MenuState;

// 时间计划结构体
typedef struct {
    uint8_t start_hour;    // 开始小时
    uint8_t start_minute;  // 开始分钟
    uint8_t end_hour;      // 结束小时
    uint8_t end_minute;    // 结束分钟
    uint8_t preset_angle;  // 预设角度
    uint8_t is_active;     // 是否激活
} TimeSchedule;

// 全局标志位声明
extern uint8_t index_led, index_bh1750, index_ds18b20, index_step, index_duoji;
extern uint8_t index_key;      // 新增目标函数需要的按键标志
extern uint8_t usart1_Rx_ok;   // 新增串口接收完成标志

// 函数声明
extern void PA12_Init(void);
extern void key_Functoin(void); // 注意原函数名拼写（可能是拼写错误）
void DisplayMenu(MenuState menu, uint8_t value);
void DisplayTimeSchedule(void);
void ProcessMenuOperation(uint8_t key_id, uint8_t press_type);
uint8_t IsInTimeSchedule(void);

#endif
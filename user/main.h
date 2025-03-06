#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"

// 全局标志位声明
extern uint8_t index_led, index_bh1750, index_ds18b20, index_step, index_duoji;
extern uint8_t index_key;      // 新增目标函数需要的按键标志
extern uint8_t usart1_Rx_ok;   // 新增串口接收完成标志


// 函数声明
extern void PA12_Init(void);
extern void key_Functoin(void); // 注意原函数名拼写（可能是拼写错误）

#endif
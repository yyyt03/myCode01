#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

//KEY端口宏定义，方便移植
//KEY1--PA11  KEY2--PA15  KEY3--PB3  KEY4--PB4  KEY5--PB5  BEEP--PA12
#define BEEP_PORT GPIOA
#define KEY1_PORT GPIOA
#define KEY2_PORT GPIOA
#define KEY3_PORT GPIOB
#define KEY4_PORT GPIOB
#define KEY5_PORT GPIOB
#define BEEP_PIN  GPIO_Pin_12
#define KEY1_PIN  GPIO_Pin_11
#define KEY2_PIN  GPIO_Pin_15
#define KEY3_PIN  GPIO_Pin_3
#define KEY4_PIN  GPIO_Pin_4
#define KEY5_PIN  GPIO_Pin_5
#define RCC_APB2Periph_key12 RCC_APB2Periph_GPIOA
#define RCC_APB2Periph_key345 RCC_APB2Periph_GPIOB

// 原有按键定义保持不变
#define KEY1 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)
#define KEY2 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)
#define KEY3 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)

// 按键值扩展定义
extern u8 key_num;
extern u8 index_key;

// 添加按键类型定义
#define KEY_SHORT_PRESS  0x00    // 短按(低4位)
#define KEY_LONG_PRESS   0x10    // 长按(高4位)

// 长按阈值定义(单位:10ms)
// 修改常量定义
#define KEY_LONG_THRESHOLD  100  // 1秒(100*10ms)

// 按键编号掩码
#define KEY_NUM_MASK       0x0F  // 低4位为按键编号
#define KEY_TYPE_MASK      0xF0  // 高4位为按键类型

//声明全局变量
extern uint8_t key_num;
extern uint8_t index_key;

//外部函数声明
extern void key_Init(void);
extern void beep_Init(void);
extern void key_Functoin(void); // 保持原有拼写，兼容现有代码

#endif

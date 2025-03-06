#ifndef __DELAY_H
#define __DELAY_H 

#include "stm32f10x.h"


//声明全局变量
extern uint16_t nMsCount;
extern uint8_t  delay_Start;

//声明全局函数
extern void delay_Init(void);
extern void delay_us(uint32_t nus);
extern void delayms(uint16_t ms);

#endif






























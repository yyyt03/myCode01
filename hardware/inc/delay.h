#ifndef __DELAY_H
#define __DELAY_H 

#include "stm32f10x.h"


//����ȫ�ֱ���
extern uint16_t nMsCount;
extern uint8_t  delay_Start;

//����ȫ�ֺ���
extern void delay_Init(void);
extern void delay_us(uint32_t nus);
extern void delayms(uint16_t ms);

#endif






























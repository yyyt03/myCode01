#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"
//����ȫ�ֱ���
extern uint16_t flag_100ms;

//����ȫ�ֱ���---���ڲ���
extern uint16_t  timer4update_count;
extern uint16_t  timer4capture_time;
extern uint32_t  keypress_time;
extern uint8_t   capture_status;

//��������
extern void tiemr1_Init(uint16_t psc,uint16_t arr);
extern void tiemr3_Init(uint16_t psc,uint16_t arr);
extern void tiemr4_Capture_Init(uint16_t psc,uint16_t arr);
extern void tiemr2_Ch1_Pwm_Init(uint16_t psc,uint16_t arr,uint16_t pwm_val);
extern void tiemr3_Ch1_Pwm_Init(uint16_t psc,uint16_t arr,uint16_t pwm_val);
extern void tiemr2_DJ_Pwm_Init(uint16_t psc,uint16_t arr,uint16_t pwm_val);

#endif

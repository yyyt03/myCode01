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
	//声明全局变量
	extern uint8_t key_num;
	extern uint8_t index_key;
  //外部函数声明
	extern void key_Init(void);
	extern void beep_Init(void);
	extern void key_Functoin(void);
	
#endif

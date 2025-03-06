//delay头文件
#include "delay.h"

 
//定义全局变量
uint16_t nMsCount = 0;
uint8_t  delay_Start=0;
 
/*
************************************************************
*	函数名称：	Delay_Init
*
*	函数功能：	systick初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：设置systick时钟源为系统时钟72MHZ,
				开启systick中断，
				开启systick嘀答定时器		
************************************************************
*/
void delay_Init(void)
{
	SysTick_Config(SystemCoreClock/1000); 
}
 
/*
************************************************************
*	函数名称：	delayUs
*
*	函数功能：	微秒级延时
*
*	入口参数：	us：延时的时长（0~65535）
*
*	返回参数：	无
*
*	说    明：软件延时
************************************************************
*/
//void delayUs(uint16_t us)
//{   
//		while( us--){
//				__nop();__nop();__nop();
//		}
//}

/************************************************************
*	函数名称：	delay_us
*
*	函数功能：	微秒级延时
*
*	入口参数：	us：延时的时长（0~65535）
*
*	返回参数：	无
*
*	说    明：采用系统嘀答时钟，时间片抽取法延时，非常精准
************************************************************/
void delay_us(uint32_t nus)
{
  uint32_t tickStart, tickCur, tickCnt;
  uint32_t tickMax = SysTick->LOAD;   	// SysTick->LOAD=72000000/1000
  uint32_t udelay_value = (SysTick->LOAD/1000)*nus;

  tickStart = SysTick->VAL;
  while(1)
  {
    tickCur = SysTick->VAL;
    tickCnt = (tickStart < tickCur) ? (tickMax+tickStart-tickCur) : (tickStart-tickCur);
    if (tickCnt > udelay_value)
      break;
  }
}

/************************************************************
*	函数名称：	DelayMs
*
*	函数功能：	毫秒级长延时
*
*	入口参数：	ms：延时的时长(0~65536)
*
*	返回参数：	无
*
*	说明：		调用SYSTICK中断，实现软件延时
************************************************************
*/
void delayms(uint16_t ms)
{ 
	nMsCount = ms;
	while(nMsCount != 0);	
}




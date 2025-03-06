//delayͷ�ļ�
#include "delay.h"

 
//����ȫ�ֱ���
uint16_t nMsCount = 0;
uint8_t  delay_Start=0;
 
/*
************************************************************
*	�������ƣ�	Delay_Init
*
*	�������ܣ�	systick��ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵��������systickʱ��ԴΪϵͳʱ��72MHZ,
				����systick�жϣ�
				����systick�ִ�ʱ��		
************************************************************
*/
void delay_Init(void)
{
	SysTick_Config(SystemCoreClock/1000); 
}
 
/*
************************************************************
*	�������ƣ�	delayUs
*
*	�������ܣ�	΢�뼶��ʱ
*
*	��ڲ�����	us����ʱ��ʱ����0~65535��
*
*	���ز�����	��
*
*	˵    ���������ʱ
************************************************************
*/
//void delayUs(uint16_t us)
//{   
//		while( us--){
//				__nop();__nop();__nop();
//		}
//}

/************************************************************
*	�������ƣ�	delay_us
*
*	�������ܣ�	΢�뼶��ʱ
*
*	��ڲ�����	us����ʱ��ʱ����0~65535��
*
*	���ز�����	��
*
*	˵    ��������ϵͳ�ִ�ʱ�ӣ�ʱ��Ƭ��ȡ����ʱ���ǳ���׼
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
*	�������ƣ�	DelayMs
*
*	�������ܣ�	���뼶����ʱ
*
*	��ڲ�����	ms����ʱ��ʱ��(0~65536)
*
*	���ز�����	��
*
*	˵����		����SYSTICK�жϣ�ʵ�������ʱ
************************************************************
*/
void delayms(uint16_t ms)
{ 
	nMsCount = ms;
	while(nMsCount != 0);	
}




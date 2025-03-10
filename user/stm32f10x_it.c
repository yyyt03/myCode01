/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "usart.h"
#include "delay.h"
#include "key.h"
#include "main.h"
#include "esp8266.h"  // 包含NTP时间定义和UpdateClock函数声明

uint32_t SystickTime;
extern __IO uint32_t TimeDisplay;
extern unsigned char index_led;

// 添加全局时钟更新标志
volatile uint8_t clock_update_flag = 0;

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
	//USART_printf(USART1, "\r\n*******************HardFault_Handler*******************\r\n");
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
	//USART_printf(USART1, "\r\n*******************MemManage_Handler*******************\r\n");
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
	//USART_printf(USART1, "\r\n*******************BusFault_Handler*******************\r\n");
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
	//USART_printf(USART1, "\r\n*******************UsageFault_Handler*******************\r\n");
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void) //UCOS系统中不写在这里
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    static uint16_t nms = 0;  // 静态变量，记录毫秒数
    nms++;  // 每次中断自增

    // 处理毫秒延时计数器
    if (nMsCount != 0) {
        nMsCount--;
    }

    // 10ms定时任务：按键轮询
    if (nms % 10 == 0) {
        index_key = 1;
    }

    // 500ms定时任务：光照传感器
    if (nms % 500 == 0) {
        index_bh1750 = 1;
    }

    // 1000ms定时任务：更新时钟（新增）
    if (nms >= 1000) {
        nms = 0;  // 防止溢出
        clock_update_flag = 1;  // 设置时钟更新标志
    }
    
    // USART接收超时检测（现有代码保持不变）
    #if USART1_RNE_IT_ENABLE
        if (usart1_Rx_Start != 0) {
            usart1_timeout++;
            if (usart1_timeout > 100) {  // 超时100ms
                usart1_Rx_ok = 0x01;
                // 可选：重置超时变量
                // usart1_timeout = 0;
                // usart1_Rx_Start = 0;
            }
        }
    #endif

    #if USART2_RNE_IT_ENABLE
        if (usart2_Rx_Start != 0) {
            usart2_timeout++;
            if (usart2_timeout > 100) {
                usart2_Rx_ok = 0x01;
                // 可选：重置超时变量
            }
        }
    #endif
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/


void EXTI0_IRQHandler(void)
{
    
}

/**
  * @brief  This function handles usart1 global interrupt request.
  * @param  None
  * @retval : None
  */
//void USART1_IRQHandler(void)
//{
//		unsigned int data;

//    if(USART1->SR & 0x0F)
//    {
//        // See if we have some kind of error
//        // Clear interrupt (do nothing about it!)
//        data = USART1->DR;
//    }
//    else if(USART1->SR & USART_FLAG_RXNE)      //Receive Data Reg Full Flag
//    {		
//        data = USART1->DR;
//				//usart1_putrxchar(data);     //Insert received character into buffer                     
//    }
//		else
//		{;}
//}

/**
  * @brief  This function handles usart2 global interrupt request.
  * @param  None
  * @retval : None
  */
//void USART2_IRQHandler(void)
//{
//		unsigned int data;

//    if(USART2->SR & 0x0F)
//    {
//        // See if we have some kind of error
//        // Clear interrupt (do nothing about it!)
//        data = USART2->DR;
//    }
//		else if(USART2->SR & USART_FLAG_RXNE)   //Receive Data Reg Full Flag
//    {		
//        data = USART2->DR;
//				usart2_rcv_buf[usart2_rcv_len++]=data;
//				
//				if(data=='{') //约定平台下发的控制命令以'{'为开始符，‘}’为控制命令结束符，读者可以自定义自己的开始符合结束符
//				{
//						rcv_cmd_start=1;
//				}
//				if(rcv_cmd_start==1)
//				{
//						usart2_cmd_buf[usart2_cmd_len++]=data;
//						if((data=='}')||(usart2_cmd_len>=MAX_CMD_LEN-1))
//						{
//								rcv_cmd_start=0;
//								LED_CmdCtl();
//								memset(usart2_cmd_buf,0,usart2_cmd_len);
//        				usart2_cmd_len=0;
//						}
//				}	  
//    }
//		else
//		{
//				;
//		}
//}

/**
  * @brief  This function handles RTC global interrupt request.
  * @param  None
  * @retval : None
  */
void RTC_IRQHandler(void)
{
   
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

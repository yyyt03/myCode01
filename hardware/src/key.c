#include "key.h"

//定义全局变量
uint8_t index_key;//按键扫描时间片
uint8_t key_num; //按键值0~5

//beep端口初始化
void beep_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = BEEP_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BEEP_PORT,&GPIO_InitStruct);	
	GPIO_SetBits(BEEP_PORT,BEEP_PIN);
}

//key端口初始化
void key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	
	beep_Init();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_key12,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_key345,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	//PB3 上电后默认JTDO，用于JTAG，要把JTAG关闭，使用SW方式！！！
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = KEY1_PIN|KEY2_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(KEY1_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin =  KEY3_PIN|KEY4_PIN|KEY5_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(KEY5_PORT,&GPIO_InitStruct);	
	
}

/*****************************************************************************
函数功能：按键扫描
输入变量：无
返回变量：unsigned char型按键值，当有按键按下时，并且松手后，返回键值
         当没有按键按下时，返回0；
外部变量：extern uint8_t index_key
函数算法：采用状态机消抖。
*****************************************************************************/
uint8_t key_Scan(void)
{
	static uint8_t key_status = 0,tmp=0;
	if(index_key == 0x01){
    index_key = 0;		
		switch(key_status){
			case 0 :				//状态0-检测是否有按键按下
					if((GPIO_ReadInputDataBit(KEY1_PORT,KEY1_PIN) == 0)||
						(GPIO_ReadInputDataBit(KEY2_PORT,KEY2_PIN) == 0) ||
						(GPIO_ReadInputDataBit(KEY3_PORT,KEY3_PIN) == 0) ||
					  (GPIO_ReadInputDataBit(KEY5_PORT,KEY5_PIN) == 0) ||
						(GPIO_ReadInputDataBit(KEY4_PORT,KEY4_PIN) == 0)) 
					{						
						 key_status = 1;tmp=0; 
					}else{ 
						key_status = 0;tmp=0;return 0;
					}					
			case 1 :		//状态1-确认有按键按下
					if(GPIO_ReadInputDataBit(KEY1_PORT,KEY1_PIN) == 0){
						 key_status = 2;tmp=1; return 0;} 					 
					else if(GPIO_ReadInputDataBit(KEY2_PORT,KEY2_PIN) == 0){
						 key_status = 2;tmp=2; return 0;}
					else if(GPIO_ReadInputDataBit(KEY3_PORT,KEY3_PIN) == 0){
						 key_status = 2;tmp=3; return 0;}
					else if(GPIO_ReadInputDataBit(KEY4_PORT,KEY4_PIN) == 0){
						 key_status = 2;tmp=4; return 0;}
					else if(GPIO_ReadInputDataBit(KEY5_PORT,KEY5_PIN) == 0){
						 key_status = 2;tmp=5; return 0;}	
					else{key_status = 0;tmp=0;return 0;}										
			case 2 :		//状态2-松手检测
					if((GPIO_ReadInputDataBit(KEY1_PORT,KEY1_PIN) == 0)||
						(GPIO_ReadInputDataBit(KEY2_PORT,KEY2_PIN) == 0) ||
						(GPIO_ReadInputDataBit(KEY3_PORT,KEY3_PIN) == 0) ||
						(GPIO_ReadInputDataBit(KEY5_PORT,KEY5_PIN) == 0) ||
						(GPIO_ReadInputDataBit(KEY4_PORT,KEY4_PIN) == 0)) 
					{						
						key_status = 2; GPIO_ResetBits(BEEP_PORT,BEEP_PIN);return 0;	
					}else{ 
					  key_status = 0;GPIO_SetBits(BEEP_PORT,BEEP_PIN);return tmp;
					} 					
				default :return 0;			  
		}	
	}else return 0;  
}

/*
函数功能：按键功能解析
输入参数：无
输出参数：无
外部变量：extern uint8_t key_num;
相关函数：调key_Scan()
*/
void key_Functoin(void)
{ unsigned char temp1; 	
	temp1=key_Scan();
   if(temp1!=0)//当有按键按下时
	 {	    
			switch(temp1)//根据不同的按键进行不同的功能处理
			{
			case 1:{
						key_num=1;
						break;		
						}
			case 2:{
						key_num=2;
						break;		
						}
			 case 3:{
						key_num=3;
						break;		
						}
			 case 4:{
						key_num=4;
						break;		
						}
			 case 5:{
						key_num=5;
						break;		
						}
			default:key_num=0; break;
			}	
	}
} 

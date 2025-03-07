#include "key.h"
#include "delay.h"
#include "usart.h"
//定义全局变量
uint8_t index_key = 0;  // 按键扫描标志
uint8_t key_num = 0;    // 按键编号

// 新增变量：按键持续按下时间计数
static uint16_t key1_press_time = 0;
static uint16_t key2_press_time = 0;
static uint16_t key3_press_time = 0;

// 新增变量：按键当前状态
static uint8_t key1_state = 0;  // 0=释放，1=按下未处理
static uint8_t key2_state = 0;
static uint8_t key3_state = 0;

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
{
    if(!index_key) return;  // 只在定时器中断后处理
    index_key = 0;          // 复位标志
    
    // 按键1处理(模式切换键)
    if(GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == 0) {  // 低电平表示按下
        if(key1_state == 0) {  // 首次按下
            key1_state = 1;
            key1_press_time = 0;
            UsartPrintf(USART_DEBUG, "[KEY] KEY1 Pressed\r\n");
            //GPIO_ResetBits(BEEP_PORT, BEEP_PIN);  // 蜂鸣器提示
        } else {
            key1_press_time++;
            
            // 调试输出计时器值，帮助诊断
            if(key1_press_time % 20 == 0) {  
                UsartPrintf(USART_DEBUG, "[KEY] KEY1 Holding: %d/%d\r\n", 
                           key1_press_time, KEY_LONG_THRESHOLD);
            }
            
            if(key1_press_time == KEY_LONG_THRESHOLD) {
                key_num = 1 | KEY_LONG_PRESS;  // 按键1长按
                UsartPrintf(USART_DEBUG, "[KEY] KEY1 Long Press TRIGGERED\r\n");
                
                // 立即处理长按事件，不等待释放
                if(key_num != 0) {
                    uint8_t key_id = key_num & KEY_NUM_MASK;
                    uint8_t press_type = key_num & KEY_TYPE_MASK;
                    UsartPrintf(USART_DEBUG, "[KEY] Processing now: %d %s\r\n", 
                               key_id, (press_type == KEY_LONG_PRESS) ? "LONG" : "SHORT");
                }
            }
        }
    } else {  // 按键释放
        if(key1_state == 1) {  // 之前是按下状态
            if(key1_press_time < KEY_LONG_THRESHOLD) {
                // 未达到长按阈值，触发短按事件
                key_num = 1 | KEY_SHORT_PRESS;  // 按键1短按
                UsartPrintf(USART_DEBUG, "[KEY] KEY1 Short Press\r\n");
            }
            key1_state = 0;  // 复位状态
            key1_press_time = 0; // 重置计时器
            GPIO_SetBits(BEEP_PORT, BEEP_PIN);  // 蜂鸣器关闭
        }
    }
    
    // 按键2处理(角度增加/时段设置) - 类似逻辑
    if(GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == 0) {
        if(key2_state == 0) {
            key2_state = 1;
            key2_press_time = 0;
            UsartPrintf(USART_DEBUG, "[KEY] KEY2 Pressed\r\n");
        } else {
            key2_press_time++;
            
            // 调试输出
            if(key2_press_time % 20 == 0) {  
                UsartPrintf(USART_DEBUG, "[KEY] KEY2 Holding: %d/%d\r\n", 
                           key2_press_time, KEY_LONG_THRESHOLD);
            }
            
            if(key2_press_time == KEY_LONG_THRESHOLD) {
                key_num = 2 | KEY_LONG_PRESS;
                UsartPrintf(USART_DEBUG, "[KEY] KEY2 Long Press TRIGGERED\r\n");
            }
        }
    } else {
        if(key2_state == 1) {
            if(key2_press_time < KEY_LONG_THRESHOLD) {
                key_num = 2 | KEY_SHORT_PRESS;
                UsartPrintf(USART_DEBUG, "[KEY] KEY2 Short Press\r\n");
            }
            key2_state = 0;
            key2_press_time = 0;
        }
    }
    
    // 按键3处理(角度减小/角度预设) - 类似逻辑
    if(GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN) == 0) {
        if(key3_state == 0) {
            key3_state = 1;
            key3_press_time = 0;
            UsartPrintf(USART_DEBUG, "[KEY] KEY3 Pressed\r\n");
        } else {
            key3_press_time++;
            
            // 调试输出
            if(key3_press_time % 20 == 0) {  
                UsartPrintf(USART_DEBUG, "[KEY] KEY3 Holding: %d/%d\r\n", 
                           key3_press_time, KEY_LONG_THRESHOLD);
            }
            
            if(key3_press_time == KEY_LONG_THRESHOLD) {
                key_num = 3 | KEY_LONG_PRESS;
                UsartPrintf(USART_DEBUG, "[KEY] KEY3 Long Press TRIGGERED\r\n");
            }
        }
    } else {
        if(key3_state == 1) {
            if(key3_press_time < KEY_LONG_THRESHOLD) {
                key_num = 3 | KEY_SHORT_PRESS;
                UsartPrintf(USART_DEBUG, "[KEY] KEY3 Short Press\r\n");
            }
            key3_state = 0;
            key3_press_time = 0;
        }
    }
}
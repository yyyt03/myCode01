#include "key.h"
#include "delay.h"
#include "usart.h"
//����ȫ�ֱ���
uint8_t index_key = 0;  // ����ɨ���־
uint8_t key_num = 0;    // �������

// ����������������������ʱ�����
static uint16_t key1_press_time = 0;
static uint16_t key2_press_time = 0;
static uint16_t key3_press_time = 0;

// ����������������ǰ״̬
static uint8_t key1_state = 0;  // 0=�ͷţ�1=����δ����
static uint8_t key2_state = 0;
static uint8_t key3_state = 0;

//beep�˿ڳ�ʼ��
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

//key�˿ڳ�ʼ��
void key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	
	beep_Init();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_key12,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_key345,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	//PB3 �ϵ��Ĭ��JTDO������JTAG��Ҫ��JTAG�رգ�ʹ��SW��ʽ������
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
�������ܣ�����ɨ��
�����������
���ر�����unsigned char�Ͱ���ֵ�����а�������ʱ���������ֺ󣬷��ؼ�ֵ
         ��û�а�������ʱ������0��
�ⲿ������extern uint8_t index_key
�����㷨������״̬��������
*****************************************************************************/
uint8_t key_Scan(void)
{
	static uint8_t key_status = 0,tmp=0;
	if(index_key == 0x01){
    index_key = 0;		
		switch(key_status){
			case 0 :				//״̬0-����Ƿ��а�������
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
			case 1 :		//״̬1-ȷ���а�������
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
			case 2 :		//״̬2-���ּ��
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
�������ܣ��������ܽ���
�����������
�����������
�ⲿ������extern uint8_t key_num;
��غ�������key_Scan()
*/
void key_Functoin(void)
{
    if(!index_key) return;  // ֻ�ڶ�ʱ���жϺ���
    index_key = 0;          // ��λ��־
    
    // ����1����(ģʽ�л���)
    if(GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == 0) {  // �͵�ƽ��ʾ����
        if(key1_state == 0) {  // �״ΰ���
            key1_state = 1;
            key1_press_time = 0;
            UsartPrintf(USART_DEBUG, "[KEY] KEY1 Pressed\r\n");
            //GPIO_ResetBits(BEEP_PORT, BEEP_PIN);  // ��������ʾ
        } else {
            key1_press_time++;
            
            // ���������ʱ��ֵ���������
            if(key1_press_time % 20 == 0) {  
                UsartPrintf(USART_DEBUG, "[KEY] KEY1 Holding: %d/%d\r\n", 
                           key1_press_time, KEY_LONG_THRESHOLD);
            }
            
            if(key1_press_time == KEY_LONG_THRESHOLD) {
                key_num = 1 | KEY_LONG_PRESS;  // ����1����
                UsartPrintf(USART_DEBUG, "[KEY] KEY1 Long Press TRIGGERED\r\n");
                
                // �����������¼������ȴ��ͷ�
                if(key_num != 0) {
                    uint8_t key_id = key_num & KEY_NUM_MASK;
                    uint8_t press_type = key_num & KEY_TYPE_MASK;
                    UsartPrintf(USART_DEBUG, "[KEY] Processing now: %d %s\r\n", 
                               key_id, (press_type == KEY_LONG_PRESS) ? "LONG" : "SHORT");
                }
            }
        }
    } else {  // �����ͷ�
        if(key1_state == 1) {  // ֮ǰ�ǰ���״̬
            if(key1_press_time < KEY_LONG_THRESHOLD) {
                // δ�ﵽ������ֵ�������̰��¼�
                key_num = 1 | KEY_SHORT_PRESS;  // ����1�̰�
                UsartPrintf(USART_DEBUG, "[KEY] KEY1 Short Press\r\n");
            }
            key1_state = 0;  // ��λ״̬
            key1_press_time = 0; // ���ü�ʱ��
            GPIO_SetBits(BEEP_PORT, BEEP_PIN);  // �������ر�
        }
    }
    
    // ����2����(�Ƕ�����/ʱ������) - �����߼�
    if(GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == 0) {
        if(key2_state == 0) {
            key2_state = 1;
            key2_press_time = 0;
            UsartPrintf(USART_DEBUG, "[KEY] KEY2 Pressed\r\n");
        } else {
            key2_press_time++;
            
            // �������
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
    
    // ����3����(�Ƕȼ�С/�Ƕ�Ԥ��) - �����߼�
    if(GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN) == 0) {
        if(key3_state == 0) {
            key3_state = 1;
            key3_press_time = 0;
            UsartPrintf(USART_DEBUG, "[KEY] KEY3 Pressed\r\n");
        } else {
            key3_press_time++;
            
            // �������
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
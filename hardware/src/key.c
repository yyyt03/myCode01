#include "key.h"

//����ȫ�ֱ���
uint8_t index_key;//����ɨ��ʱ��Ƭ
uint8_t key_num; //����ֵ0~5

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
{ unsigned char temp1; 	
	temp1=key_Scan();
   if(temp1!=0)//���а�������ʱ
	 {	    
			switch(temp1)//���ݲ�ͬ�İ������в�ͬ�Ĺ��ܴ���
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

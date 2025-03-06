#include "timer.h"

//����ȫ�ֱ���
uint16_t flag_100ms;
//Timer1 ��ʱ�����ܳ�ʼ������
void tiemr1_Init(uint16_t psc,uint16_t arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;	
	NVIC_InitTypeDef 				NVIC_InitStruct;	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);	
	
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = arr;
	TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);	
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);	
	
  TIM_Cmd(TIM1,ENABLE);	

}
//Timer3 ��ʱ�����ܳ�ʼ������
void tiemr3_Init(uint16_t psc,uint16_t arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;	
	NVIC_InitTypeDef 				NVIC_InitStruct;	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	
	
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = arr;
	TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);	
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
  TIM_Cmd(TIM3,ENABLE);	

}
//Timer3-chanel-1&2_pwm��ʼ������
void tiemr3_Ch1_Pwm_Init(uint16_t psc,uint16_t arr,uint16_t pwm_val)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		
	TIM_OCInitTypeDef       TIM_OCInitStruct;
	GPIO_InitTypeDef        GPIO_InitStruct;
	//ʹ�ܶ˿�ʱ�Ӽ�����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);  	
	//GPIO�˿����� PA6--Timer3 ch1     PA7--Timer3 ch2
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);	
	//��ʱ��������ʼ��
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = arr;
	TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);	
	//PWM��ʼ��	
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = pwm_val;
	TIM_OC1Init(TIM3,&TIM_OCInitStruct);
	TIM_OC2Init(TIM3,&TIM_OCInitStruct);  
	//������ʱ��
  TIM_Cmd(TIM3,ENABLE);	
}
//Timer2-chanel-1&2_pwm��ʼ������
void tiemr2_Ch1_Pwm_Init(uint16_t psc,uint16_t arr,uint16_t pwm_val)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		
	TIM_OCInitTypeDef       TIM_OCInitStruct;
	GPIO_InitTypeDef        GPIO_InitStruct;
	//ʹ�ܶ˿�ʱ�Ӽ�����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); 
	
	//GPIO�˿����� PA0--Timer2 ch1     PA1--Timer2 ch2
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);	
	//��ʱ��������ʼ��
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = arr;
	TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);	
	//PWM��ʼ��	
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = pwm_val;
	TIM_OC1Init(TIM2,&TIM_OCInitStruct);
	TIM_OC2Init(TIM2,&TIM_OCInitStruct);  
	//������ʱ��
  TIM_Cmd(TIM2,ENABLE);	
}

//Timer2-���_pwm��ʼ������
void tiemr2_DJ_Pwm_Init(uint16_t psc,uint16_t arr,uint16_t pwm_val)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		
	TIM_OCInitTypeDef       TIM_OCInitStruct;
	GPIO_InitTypeDef        GPIO_InitStruct;
	//ʹ�ܶ˿�ʱ�Ӽ�����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE); //�ر�JTAG��ʹ��SW���ϵ��PA15Ĭ��JTDI,PB3Ĭ��JTDO
	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2,ENABLE);		//��ӳ��CH1---PA15  CH2---PB3 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);		//����PBʱ��
	
	//GPIO�˿����� PA15--Timer2 ch1     PB3--Timer2 ch2
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);	
	//��ʱ��������ʼ��
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = arr;
	TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);	
	//PWM��ʼ��	
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = pwm_val;
	TIM_OC1Init(TIM2,&TIM_OCInitStruct);
	TIM_OC2Init(TIM2,&TIM_OCInitStruct);  
	//������ʱ��
  TIM_Cmd(TIM2,ENABLE);	
}

//����ȫ�ֱ���---���ڲ���
uint16_t  timer4update_count;
uint16_t  timer4capture_time;
uint32_t  keypress_time;
uint8_t   capture_status;

//TIM4��ʱ�����벶���ʼ������
void tiemr4_Capture_Init(uint16_t psc,uint16_t arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;	
	NVIC_InitTypeDef 				NVIC_InitStruct;
	TIM_ICInitTypeDef       TIM_ICInitStruct;
	GPIO_InitTypeDef        GPIO_InitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);  //PB6 TIM4-CH1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	//�������ų�ʼ��Ϊ�������������øߵ�ƽ
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_6;	
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOB,GPIO_Pin_6,Bit_SET);
	
	//TIM4������ʼ��
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = arr;
	TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStruct);
	//TIM4���벶���ʼ��
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICFilter = 0x00;
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInit(TIM4,&TIM_ICInitStruct);
	
	//�ж����ȼ�����
	NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);	
	
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC1,ENABLE); 	//ʹ�ܸ��º�����ͨ��1�����ж�
	
  TIM_Cmd(TIM4,ENABLE);	

}




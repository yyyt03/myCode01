/* ͷ�ļ���������˵�� --------------------------------------------------------*/
//��Ƭ�����Ŀ�
#include "stm32f10x.h"          // STM32F10xϵ�б�׼�����

//Ӳ���豸������
#include "delay.h"              // SysTick��ʱ��������ȷ��ʱ1us/10us/1ms��
#include "usart.h"             // USART1����������115200bps, �жϽ��գ�
#include "oled.h"              // 0.96��OLED��ʾ��������I2C�ӿڣ�
#include "bh1750.h"            // BH1750FVI���մ�����������I2C�ӿڣ�
#include "timer.h"             // ��ʱ��������TIM2 PWM������ƣ�
#include "key.h"               // ��������������֧�ֵ�����⣩

//ϵͳ�����
#include "sys.h"               // ϵͳ�����壨ʱ�����á������ض��壩

//����Э���
#include "onenet.h"             // OneNET������ƽ̨ͨ��Э��

//�����豸
#include "esp8266.h"           // ESP8266 WiFiģ������

//C��׼��
#include <string.h>            // �ַ�����������memcpy/memset�ȣ�
#include <stdio.h>             // ��׼�������������sprintf��ʽ����

//ö�������ƶ���sys.h
//typedef enum {
//    AUTO_MODE,
//    MANUAL_MODE
//} SystemMode;

/* ȫ�ֱ������� -----------------------------------------------------------*/
uint8_t index_bh1750 = 0, index_duoji = 0; // �������Ͷ����������־
SystemMode work_mode = AUTO_MODE;         // ϵͳ����ģʽ���Զ�/�ֶ���
uint8_t manual_angle = 90;               // �ֶ�ģʽ�µ�Ŀ��Ƕ�
unsigned short timeCount = 0;             // ���ͼ����ʱ��
char light_info[16];                      // ���ն���ʾ����
float light_value = 0;                    // ʵʱ����ǿ��ֵ
uint8_t angle = 0;                       // ��ǰĿ��Ƕ�
char time_str[20]; // ʱ����ʾ����

// ��main.c�ļ���ͷ�����ⲿ��������
extern volatile uint8_t clock_update_flag;

/* ��ֵ˵������λ��lux�� --------------------------------------------------*/
#define LIGHT_THRESHOLD_LOW       50   // ���ڻ谵���գ�0�ȣ�
#define LIGHT_THRESHOLD_MID_LOW  100   // ��������������30�ȣ�
#define LIGHT_THRESHOLD_MID      200   // �������ڻ�����90�ȣ�
#define LIGHT_THRESHOLD_MID_HIGH 500   // ǿ���ջ�����150�ȣ�
#define LIGHT_THRESHOLD_HIGH    1000   // ֱ�����⻷����180�ȣ�

/* ����Ƕȶ��壨��λ���ȣ� --------------------------------------------------*/
#define ANGLE_ULTRA_LOW     0    // ��ȫ�պ�״̬
#define ANGLE_LOW          30    // С�Ƕȿ���
#define ANGLE_MID          90    // �еȿ����Ƕ�
#define ANGLE_HIGH        150    // ��Ƕȿ���
#define ANGLE_ULTRA_HIGH  180    // ��ȫչ��״̬
#define ANGLE_STEP        30     // �Ƕȵ�������

/* �̶��кŷ��� ------------------------------------------------------------*/
#define LINE_MODE   1   // ��2�У���ʾģʽ��
#define LINE_ANGLE  3   // ��4�У���ʾ�Ƕȣ�
#define LINE_LIGHT  5   // ��6�У���ʾ����ǿ�ȣ�
#define LINE_SEND   7   // ��8�У���ʾ����״̬��

/*
************************************************************
*	�������ƣ�	Hardware_Init
*
*	�������ܣ�	Ӳ���ۺϳ�ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		���γ�ʼ������ģ�飺
*				[1] ����NVIC�жϷ���Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ���
*				[2] ��ʼ��SysTick��ʱ����
*				[3] ��ʼ��USART1���ڣ�115200bps�����ڵ��������
*				[4] ��ʼ��USART2���ڣ�115200bps������ESP8266ͨ�ţ�
*				[5] ��ʼ��OLED��ʾ��������
*				[6] ��ʼ��BH1750���մ��������ϵ�����
*				[7] ��ʼ���������GPIO
*				[8] ��ʼ��TIM2ͨ��1 PWM�����72MHz/720=100kHz������20ms��
*				[9] ��ʾϵͳ��ʼ����Ϣ
*
*				�ؼ�������
*				- USART1�����ʣ�115200bps�����ڵ�����Ϣ���
*				- USART2�����ʣ�115200bps������ESP8266ͨ��
*				- PWMƵ�ʣ�100kHz������20ms
************************************************************
*/
void Hardware_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    // �����ж����ȼ�����
    delay_Init();                                      // ��ʼ����ʱ����
	Usart1_Init(115200);							//����1����ӡ��Ϣ��	
	Usart2_Init(115200);							//����2������ESP8266��
	
    OLED_Init();                    // ��ʼ��OLED��ʾ��
    OLED_CLS();                     // �����Ļ��ʾ
    
    Init_BH1750();                  // ��ʼ��BH1750���մ�����
    bh_data_send(BHPowOn);          // �����ϵ�ָ��
    key_Init();                     // ��ʼ���������
    
    // ��ʼ�����PWM��72MHz/(720+1) = 100kHz������20ms��100kHz * 20ms = 2000��
    tiemr2_Ch1_Pwm_Init(720-1, 2000, 100); 
    delayms(1000);                  // �ȴ������ʼ�����
    
    // ��ʾ��ʼ����Ϣ
    OLED_ShowStr(0,0,(uint8_t *)"IoT Init...",2);
    UsartPrintf(USART_DEBUG, "Hardware init OK\r\n");
}	

/*
************************************************************
*	�������ƣ�	AngleToPWM
*
*	�������ܣ�	����Ƕ�תPWMռ�ձ�
*
*	��ڲ�����	angle - Ŀ��Ƕ�ֵ (0~180��)
*
*	���ز�����	uint8_t - ��Ӧ��PWMռ�ձ�
*
*	˵����		���ݶ�����Խ�������ת����
*				���㹫ʽ��PWM = 50 + (angle * 190) / 180
*				��Ӧ������
*				- 0�� �� 50/2000 = 2.5% ռ�ձ�
*				- 180�� �� 240/2000 = 12% ռ�ձ�
*				ȷ������Ƕ�����Ч��Χ�ڣ�0��angle��180��
************************************************************
*/
uint16_t AngleToPWM(uint8_t angle) {
    return 50 + angle * 190 / 180;
}

/*
************************************************************
*	�������ƣ�	LightToFixedAngle
*
*	�������ܣ�	����ǿ�ȵ��̶��Ƕ�ӳ��
*
*	��ڲ�����	light_value - BH1750�����Ĺ���ǿ��ֵ����λ��lux��
*
*	���ز�����	uint8_t - ӳ���Ĺ̶��Ƕ�ֵ��0~180�ȣ�
*
*	˵����		����Ԥ����ֵ���н���ʽ�Ƕȷ��䣺
*				[0,50)    �� 0�ȣ���ȫ�պϣ�
*				[50,100)  �� 30��
*				[100,200) �� 90�ȣ��м�λ�ã�
*				[200,500) �� 150��
*				[500,��)   �� 180�ȣ���ȫչ����
*				��ֵ�ɸ���ʵ�ʴ���͸���������
************************************************************
*/
uint8_t LightToFixedAngle(float light_value)
{
    if(light_value < LIGHT_THRESHOLD_LOW)        
        return ANGLE_ULTRA_LOW;  // 0�� (������<50)
    else if(light_value < LIGHT_THRESHOLD_MID_LOW) 
        return ANGLE_LOW;        // 30�� (50�ܹ���<100)
    else if(light_value < LIGHT_THRESHOLD_MID)     
        return ANGLE_MID;        // 90�� (100�ܹ���<200)
    else if(light_value < LIGHT_THRESHOLD_MID_HIGH)
        return ANGLE_HIGH;       // 150�� (200�ܹ���<500)
    else                                            
        return ANGLE_ULTRA_HIGH; // 180�� (���ա�500)
}


/*
************************************************************
*	�������ƣ�	main
*
*	�������ܣ�	���������
*
*	��ڲ�����	��
*
*	���ز�����	int - ʵ��δʹ�ã����ֱ�׼��ʽ��
*
*	˵����		ϵͳ������ѭ�����������¹��ܣ�
*				[1] Ӳ����ʼ��
*				[2] ��ʼ��ESP8266 WiFiģ��
*				[3] ����OneNETƽ̨�����Ի��ƣ�
*				[4] ������ѭ����
*					- ����ɨ�輰����ģʽ�л�/�Ƕȵ��ڣ�
*					- ��ʱ�ɼ��������ݣ�500ms�����
*					- ����ģʽ����Ŀ��Ƕ�
*					- ����OLED��ʾ��ģʽ/�Ƕ�/����ֵ��
*					- PWM������ƶ���Ƕ�
*					- ��ʱ���»��ƣ�delaymsʵ�ּ򵥵��ȣ�
*					- ���ݲɼ��������Ϣ���
*					- �������ݷ�������մ���
*
*				�ؼ�������
*				- ���ͼ����5�루100*50ms��
*				- ���ݲɼ����ڣ�500ms
************************************************************
*/
int main(void)
{	
    unsigned short timeCount = 0; // ���ͼ����ʱ��
    unsigned char *dataPtr = NULL; // ��������ָ��
    
    // ��tick������ǰ��������ͷ�����Ƴ�clock_tick
    uint32_t tick = 0;
    uint32_t last_second = 0;
    uint8_t pwm = 100;
    
    Hardware_Init();        // Ӳ����ʼ���������裩
    ESP8266_Init();         // ��ʼ��WiFiģ��

    // ����OneNETƽ̨�����Ŀ���ָ��
    OLED_ShowStr(0,0,(uint8_t *)"Conn OneNET...",2);
    while(OneNet_DevLink()) {
        delayms(500);
        OLED_ShowStr(0,2,(uint8_t *)"Retrying...",2);
    }
    OneNET_Subscribe();     // ���Ŀ���ָ��
    
    // ���ӳɹ���ʾ
    OLED_ShowStr(0,0,(uint8_t *)"SmartLight OK",2);
    // ��ʾ��ʼʱ��
    sprintf(time_str, "%04d-%02d-%02d", ntp_time.year, ntp_time.month, ntp_time.day);
    OLED_ShowStr(0,0,(uint8_t *)time_str,1);
    sprintf(time_str, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
    OLED_ShowStr(64,0,(uint8_t *)time_str,1);
    
    delayms(200);
    OLED_CLS();
    
    while(1)
    {
        // ʹ���ж����õı�־����ʱ�ӣ���׼ȷ
        if(clock_update_flag)
        {
            clock_update_flag = 0;  // �����־
            UpdateClock();          // ����ʱ��
            
            // ����ʱ����ʾ
            if(ntp_time.is_valid && ntp_time.year >= 2000) {
                sprintf(time_str, "%04d-%02d-%02d", ntp_time.year, ntp_time.month, ntp_time.day);
                OLED_ShowStr(0,0,(uint8_t *)time_str,1);
                sprintf(time_str, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
                OLED_ShowStr(64,0,(uint8_t *)time_str,1);
                
                // ���������ǰʱ��
                UsartPrintf(USART_DEBUG, "[TIME] %02d:%02d:%02d\r\n", 
                            ntp_time.hour, ntp_time.minute, ntp_time.second);
            } else {
                // ���ʱ����Ч����ʾ�ȴ�NTPͬ������Ϣ
                OLED_ShowStr(0,0,(uint8_t *)"Wait NTP Sync",1);
            }
        }
        
        // ������뱣�ֲ���...
        key_Functoin();  
        if(key_num != 0)
        {
            switch(key_num)
            {
            case 1: // ģʽ�л�
                work_mode = (work_mode == AUTO_MODE) ? MANUAL_MODE : AUTO_MODE;
                if(work_mode == MANUAL_MODE) manual_angle = 90;
                break;
                
            case 2: // �Ƕ�+ANGLE_STEP
                if(work_mode == MANUAL_MODE) 
                {
                    manual_angle = (manual_angle >= 180) ? 0 : manual_angle + ANGLE_STEP;
                }
                break;

            case 3: // �����Ƕ�-ANGLE_STEP
                if(work_mode == MANUAL_MODE)
                {
                    manual_angle = (manual_angle <= 0) ? 180 : manual_angle - ANGLE_STEP;
                }
                break;
            }
            key_num = 0;
        }

        if(index_bh1750)
        {
            bh_data_send(BHReset);
            bh_data_send(BHModeH1);     
            delayms(120);
			light_value = bh_data_read() / 1.2;
			
			// ���ݲɼ�����ӵ�����Ϣ
            UsartPrintf(USART_DEBUG, "[DEBUG] light=%.0f -> angle=%d\n", 
                        light_value, LightToFixedAngle(light_value));
			
           angle = (work_mode == AUTO_MODE) ? LightToFixedAngle(light_value) : manual_angle;
            
            /* ��ʾ�Ż� */
            // ģʽ��ʾ
            char mode_str[16];
            sprintf(mode_str, "Mode:%s", (work_mode == AUTO_MODE) ? "Auto" : "Manu");
            OLED_ShowStr(0, LINE_MODE, (uint8_t *)mode_str, 2);

            // �Ƕ���ʾ���̶�λ�ã�
            char angle_str[16];
            sprintf(angle_str, "Angle:%3d", angle); 
            OLED_ShowStr(0, LINE_ANGLE, (uint8_t *)angle_str, 2);

            // ����ǿ����ʾ������룩
            char light_info[16];
            sprintf(light_info, "Lux:%-5.0f", light_value);
            OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)light_info, 2);

            // ����״̬����ʱ��ʾ��С���壩
            if(timeCount % 10 == 0) {
                char send_status[16];
                sprintf(send_status, "%-8s", (timeCount % 10 == 0) ? "Sending" : ""); // �����8�ַ�
                OLED_ShowStr(64, LINE_SEND, (uint8_t *)send_status, 1);
            }
	
            pwm = AngleToPWM(angle);
            index_bh1750 = 0;
        }
        
        if(index_duoji)
        {
            TIM_SetCompare1(TIM2, pwm); //PA0��
            TIM_SetCompare2(TIM2, pwm);	//PA1��
            index_duoji = 0;
        }
        
		if(++timeCount >= 100)    // 5�뷢�ͼ����100*50ms��
		{
			// �������ݵ�ƽ̨
			UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
			OneNet_SendData();
			
			timeCount = 0;
			ESP8266_Clear();
		}

		// �������ݴ���
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
		
        // ʹ������ǰ�涨���tick�������Ƴ�static�ؼ���
        if(++tick % 50 == 0) index_bh1750 = 1;
        if(tick % 10 == 0)   index_duoji = 1;
        delayms(10);
    }
}
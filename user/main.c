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

//��Ŀͷ�ļ�
#include "main.h"              // �����Ŀ��ͷ�ļ�������MenuState��TimeSchedule����

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
// ����ȫ�ֱ���
MenuState current_menu = MENU_NONE;  // ��ǰ�˵�״̬
TimeSchedule time_schedule = {9, 0, 18, 0, 90, 0};  // Ĭ��9:00-18:00��90��
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
    delayms(200); // �ȴ��������ȶ�
    float init_light = bh_data_read() / 1.2;
    if(init_light > 0) {
        UsartPrintf(USART_DEBUG, "[INIT] BH1750 OK, Initial light=%.0f lux\r\n", init_light);
    } else {
        UsartPrintf(USART_DEBUG, "[INIT] WARNING: BH1750 may not working\r\n");
    }
    
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
*   �������ƣ�   DisplayMenu
*
*   �������ܣ�   ��ʾ��ǰ�˵�״̬������ֵ
*
*   ��ڲ�����   menu - ��ǰ�˵�״̬
*               value - ��ǰ���õ�ֵ
*
*   ���ز�����   ��
*
*   ˵����       ���ݲ�ͬ�Ĳ˵�״̬��ʾ��Ӧ�����ý���
************************************************************
*/
void DisplayMenu(MenuState menu, uint8_t value)
{
    char menu_title[32];
    char menu_value[32];
    char menu_hint[32];
    
    // �����ʾ���򣨱���ʱ����ʾ��
    OLED_CLS();  // ʹ��OLED_CLS���OLED_Fill(0)
    
    // �ڵ�һ��������ʾʱ��
    if(ntp_time.is_valid) {
        sprintf(time_str, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
        OLED_ShowStr(0, 0, (uint8_t *)time_str, 1);
    }
    
    // ���ݲ�ͬ�˵�״̬��ʾ��Ӧ�����ֵ
    switch(menu)
    {
    case MENU_TIME_START_HOUR:
        sprintf(menu_title, "Set Start Hour");
        sprintf(menu_value, "%02d:00", value);
        break;
        
    case MENU_TIME_START_MIN:
        sprintf(menu_title, "Set Start Min");
        sprintf(menu_value, "%02d:%02d", time_schedule.start_hour, value);
        break;
        
    case MENU_TIME_END_HOUR:
        sprintf(menu_title, "Set End Hour");
        sprintf(menu_value, "%02d:00", value);
        break;
        
    case MENU_TIME_END_MIN:
        sprintf(menu_title, "Set End Min");
        sprintf(menu_value, "%02d:%02d", time_schedule.end_hour, value);
        break;
        
    case MENU_ANGLE_PRESET:
        sprintf(menu_title, "Set Angle");
        sprintf(menu_value, "%d Degrees", value);
        break;
        
    default:
        return;  // �ǲ˵�״̬������
    }
    
    // ���õĲ�����ʾ
    sprintf(menu_hint, "K1:OK K2:+ K3:-");
    
    // ��ʾ�˵����⣨��2�У������壩
    OLED_ShowStr(0, 2, (uint8_t *)menu_title, 2);
    
    // ��ʾ��ǰֵ����4�У������壩
    OLED_ShowStr(0, 4, (uint8_t *)menu_value, 2);
    
    // ��ʾ������ʾ����6�У�С���壩
    OLED_ShowStr(0, 6, (uint8_t *)menu_hint, 1);
}

/*
************************************************************
*   �������ƣ�   DisplayTimeSchedule
*
*   �������ܣ�   ��ʾ��ǰʱ��ƻ�����
*
*   ��ڲ�����   ��
*
*   ���ز�����   ��
*
*   ˵����       ��OLED����ʾ��ǰ��ʱ��ƻ�����
************************************************************
*/
void DisplayTimeSchedule(void)
{
    char schedule_info[32];
    
    // ��OLED��7����ʾʱ��ƻ���Ϣ
    sprintf(schedule_info, "%c%02d:%02d-%02d:%02d %d",
            time_schedule.is_active ? '+' : '-',  // ����״̬ǰ׺
            time_schedule.start_hour, time_schedule.start_minute,
            time_schedule.end_hour, time_schedule.end_minute,
            time_schedule.preset_angle);
    
    OLED_ShowStr(0, LINE_SEND, (uint8_t *)schedule_info, 1);
}

/*
************************************************************
*   �������ƣ�   IsInTimeSchedule
*
*   �������ܣ�   �жϵ�ǰʱ���Ƿ����趨��ʱ�����
*
*   ��ڲ�����   ��
*
*   ���ز�����   uint8_t - 1��ʾ��ʱ����ڣ�0��ʾ����
*
*   ˵����       �Ƚϵ�ǰʱ����趨��ʱ��Σ����ǿ������
************************************************************
*/
uint8_t IsInTimeSchedule(void)
{
    // ���ʱ��ƻ�δ�����NTPʱ����Ч������ʱ�����
    if(!time_schedule.is_active || !ntp_time.is_valid)
        return 0;
    
    // ��ʱ��ת��Ϊ���Ӽ��������ڱȽ�
    uint16_t current_mins = ntp_time.hour * 60 + ntp_time.minute;
    uint16_t start_mins = time_schedule.start_hour * 60 + time_schedule.start_minute;
    uint16_t end_mins = time_schedule.end_hour * 60 + time_schedule.end_minute;
    
    // ���������������22:00-08:00������ʱ��Σ�
    if(start_mins > end_mins) {
        // ����ʱ��Σ���ǰʱ��Ҫô��start֮��Ҫô��end֮ǰ
        return (current_mins >= start_mins || current_mins <= end_mins) ? 1 : 0;
    } else {
        // ��ͨʱ��Σ���ǰʱ����start��end֮��
        return (current_mins >= start_mins && current_mins <= end_mins) ? 1 : 0;
    }
}

/*
************************************************************
*   �������ƣ�   ProcessMenuOperation
*
*   �������ܣ�   ����˵�״̬�µİ�������
*
*   ��ڲ�����   key_id - �������
*               press_type - ��������(�̰�/����)
*
*   ���ز�����   ��
*
*   ˵����       ���ݵ�ǰ�˵�״̬�Ͱ�������������Ӧ����
************************************************************
*/
void ProcessMenuOperation(uint8_t key_id, uint8_t press_type)
{
    static uint8_t temp_value = 0; // ��ʱ�洢�������õ�ֵ
    static uint8_t init_done = 0;  // ����Ƿ��ѳ�ʼ��
    
    // ֻ����̰�
    if(press_type != KEY_SHORT_PRESS) return;
    
        // �˵���ʼ��ֵ�趨
        if(key_id == 1 && !init_done) {
            // ��һ�ν���˵������ݵ�ǰ�˵�״̬��ʼ��ֵ
            init_done = 1; // �������ɳ�ʼ��
            
            switch(current_menu) {
                case MENU_TIME_START_HOUR:
                    temp_value = time_schedule.start_hour;
                    break;
                case MENU_TIME_START_MIN:
                    temp_value = time_schedule.start_minute;
                    break;
                case MENU_TIME_END_HOUR:
                    temp_value = time_schedule.end_hour;
                    break;
                case MENU_TIME_END_MIN:
                    temp_value = time_schedule.end_minute;
                    break;
                case MENU_ANGLE_PRESET:
                    temp_value = time_schedule.preset_angle;
                    break;
                default:
                    break;
            }
            DisplayMenu(current_menu, temp_value);
            return;
        }
    
    // ���ݵ�ǰ�˵�״̬�Ͱ���ID���д���
    switch(current_menu)
    {
    case MENU_TIME_START_HOUR:
        switch(key_id)
        {
        case 1: // ȷ�ϣ�������һ�˵�
            time_schedule.start_hour = temp_value;
            current_menu = MENU_TIME_START_MIN;
            temp_value = time_schedule.start_minute;
            break;
        case 2: // ����Сʱ
            temp_value = (temp_value >= 23) ? 0 : temp_value + 1;
            break;
        case 3: // ����Сʱ
            temp_value = (temp_value <= 0) ? 23 : temp_value - 1;
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    case MENU_TIME_START_MIN:
        switch(key_id)
        {
        case 1: // ȷ�ϣ�������һ�˵�
            time_schedule.start_minute = temp_value;
            current_menu = MENU_TIME_END_HOUR;
            temp_value = time_schedule.end_hour;
            break;
        case 2: // ���ӷ���
            temp_value = (temp_value >= 59) ? 0 : temp_value + 1;
            break;
        case 3: // ���ٷ���
            temp_value = (temp_value <= 0) ? 59 : temp_value - 1;
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    case MENU_TIME_END_HOUR:
        switch(key_id)
        {
        case 1: // ȷ�ϣ�������һ�˵�
            time_schedule.end_hour = temp_value;
            current_menu = MENU_TIME_END_MIN;
            temp_value = time_schedule.end_minute;
            break;
        case 2: // ����Сʱ
            temp_value = (temp_value >= 23) ? 0 : temp_value + 1;
            break;
        case 3: // ����Сʱ
            temp_value = (temp_value <= 0) ? 23 : temp_value - 1;
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    case MENU_TIME_END_MIN:
        switch(key_id)
        {
        case 1: // ȷ�ϣ�������һ�˵�
            time_schedule.end_minute = temp_value;
            current_menu = MENU_ANGLE_PRESET;
            temp_value = time_schedule.preset_angle;
            break;
        case 2: // ���ӷ���
            temp_value = (temp_value >= 59) ? 0 : temp_value + 1;
            break;
        case 3: // ���ٷ���
            temp_value = (temp_value <= 0) ? 59 : temp_value - 1;
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    case MENU_ANGLE_PRESET:
        switch(key_id)
        {
        case 1: // ȷ�ϲ��˳��˵�
            time_schedule.preset_angle = temp_value;
            time_schedule.is_active = 1; // ����ʱ��ƻ�
            
            // �˳��˵�״̬
            current_menu = MENU_NONE;
            temp_value = 0;
            init_done = 0;
            
            // ����˵���ʾ���ָ���������
            OLED_CLS();
            // ����ʱ��ƻ���ʾ
            DisplayTimeSchedule();
            
            // ��ӳ�ʼ������ʾ
            char mode_str[16];
            sprintf(mode_str, "Mode:%s", "Auto");  // Ĭ���Զ�ģʽ
            OLED_ShowStr(0, LINE_MODE, (uint8_t *)mode_str, 2);

            char angle_str[16];
            sprintf(angle_str, "Angle:%3d", angle); 
            OLED_ShowStr(0, LINE_ANGLE, (uint8_t *)angle_str, 2);

            sprintf(light_info, "Lux:%-5.0f", light_value);
            OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)light_info, 2);

            // ��¼������־
            UsartPrintf(USART_DEBUG, "[SCHEDULE] Set %02d:%02d-%02d:%02d Angle:%d\r\n",
                      time_schedule.start_hour, time_schedule.start_minute,
                      time_schedule.end_hour, time_schedule.end_minute,
                      time_schedule.preset_angle);
            break;
            
        case 2: // ���ӽǶ�
            temp_value = (temp_value >= 180) ? 0 : (temp_value + 10);
            break;
            
        case 3: // ���ٽǶ�
            temp_value = (temp_value <= 0) ? 180 : (temp_value - 10);
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    default:
        // δ֪�˵�״̬���ָ�������ʾ
        current_menu = MENU_NONE;
        temp_value = 0;
        OLED_CLS();
        break;
    }
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
    
    // ��ӵ��Ա������ڼ�ش�����״̬
    static uint16_t bh1750_check_counter = 0;
    
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
    
    // ��ʾ��ʼʱ��
    sprintf(time_str, "%04d-%02d-%02d", ntp_time.year, ntp_time.month, ntp_time.day);
    OLED_ShowStr(0,0,(uint8_t *)time_str,1);
    sprintf(time_str, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
    OLED_ShowStr(64,0,(uint8_t *)time_str,1);

    // ������ȡһ�ι��ղ����½���
    bh_data_send(BHReset);
    bh_data_send(BHModeH1);     
    delayms(120);
    light_value = bh_data_read() / 1.2;
    UsartPrintf(USART_DEBUG, "[INIT] Initial light reading: %.1f lux\r\n", light_value);

    // ��ʾ��ʼ����
    char mode_str[16];
    sprintf(mode_str, "Mode:%s", "Auto");  // Ĭ���Զ�ģʽ
    OLED_ShowStr(0, LINE_MODE, (uint8_t *)mode_str, 2);

    char angle_str[16];
    angle = LightToFixedAngle(light_value);
    sprintf(angle_str, "Angle:%3d", angle); 
    OLED_ShowStr(0, LINE_ANGLE, (uint8_t *)angle_str, 2);

    sprintf(light_info, "Lux:%-5.0f", light_value);
    OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)light_info, 2);

    // ��ʾ��ʼʱ��ƻ�
    DisplayTimeSchedule();

    // ��������PWM
    pwm = AngleToPWM(angle);
    TIM_SetCompare1(TIM2, pwm);
    TIM_SetCompare2(TIM2, pwm);

    // ǿ�ƴ�����һ��������ȡ
    index_bh1750 = 1;

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
        
        // ���а���ɨ��
        key_Functoin();  

        // �������¼�
        if(key_num != 0)
        {
            uint8_t key_id = key_num & KEY_NUM_MASK;       // ��ȡ�������
            uint8_t press_type = key_num & KEY_TYPE_MASK;  // ��ȡ��������
            
            UsartPrintf(USART_DEBUG, "[MAIN] Processing key %d, type %s\r\n", 
                       key_id, (press_type == KEY_LONG_PRESS) ? "LONG" : "SHORT");
            
        // �����Ƿ��ڲ˵�״̬ѡ�����߼�
        if(current_menu != MENU_NONE) {
            // �˵�״̬�µİ�������
            ProcessMenuOperation(key_id, press_type);
        } else {
            // �ǲ˵�״̬�µİ�������               
            if(press_type == KEY_SHORT_PRESS) {
                // �̰�����
                switch(key_id)
                {
                    case 1: // ģʽ�л�
                    // ����ģʽѭ���л�
                    if(work_mode == AUTO_MODE) {
                        work_mode = MANUAL_MODE;
                        manual_angle = angle; // ʹ�õ�ǰ�Ƕ���Ϊ��ʼֵ
                    } else if(work_mode == MANUAL_MODE) {
                        work_mode = TIMELOCK_MODE;
                        time_schedule.is_active = 1; // ����ʱ���
                    } else {
                        work_mode = AUTO_MODE;
                    }
                    
                    // ��Ӹ���ϸ�ĵ������
                    UsartPrintf(USART_DEBUG, "[MODE] Changed to %s, current angle=%d\r\n", 
                              (work_mode == AUTO_MODE) ? "AUTO" : 
                              (work_mode == MANUAL_MODE) ? "MANUAL" : "TIMELOCK", angle);
                    break;
                    
                case 2: // �Ƕ�+ANGLE_STEP
                    if(work_mode == MANUAL_MODE) 
                    {
                        manual_angle = (manual_angle >= 180) ? 0 : manual_angle + ANGLE_STEP;
                        pwm = AngleToPWM(manual_angle); // ��������PWMֵ
                        
                        // ����Ӧ�ö���Ƕȱ仯
                        TIM_SetCompare1(TIM2, pwm);
                        TIM_SetCompare2(TIM2, pwm);
                        
                        UsartPrintf(USART_DEBUG, "[ANGLE] Increased to %d, PWM=%d\r\n", manual_angle, pwm);
                    }
                    break;
                
                case 3: // �Ƕ�-ANGLE_STEP
                    if(work_mode == MANUAL_MODE)
                    {
                        manual_angle = (manual_angle <= 0) ? 180 : manual_angle - ANGLE_STEP;
                        pwm = AngleToPWM(manual_angle); // ��������PWMֵ
                        
                        // ����Ӧ�ö���Ƕȱ仯
                        TIM_SetCompare1(TIM2, pwm);
                        TIM_SetCompare2(TIM2, pwm);
                        
                        UsartPrintf(USART_DEBUG, "[ANGLE] Decreased to %d, PWM=%d\r\n", manual_angle, pwm);
                    }
                    break;
                }
            }
            else if(press_type == KEY_LONG_PRESS) {
                // ���������߼�
                UsartPrintf(USART_DEBUG, "[LONG] Long press on key %d detected\r\n", key_id);
                
                switch(key_id)
                {
                case 1: // ����1���� - Ԥ������
                    break;
                    
                case 2: // ����2���� - ����ʱ������
                    current_menu = MENU_TIME_START_HOUR;
                    DisplayMenu(current_menu, 0); // ��ʾ��ʼ�˵�
                    UsartPrintf(USART_DEBUG, "[MENU] Enter time schedule setting\r\n");
                    break;
                    
                case 3: // ����3���� - ����Ƕ�Ԥ��
                    current_menu = MENU_ANGLE_PRESET;
                    DisplayMenu(current_menu, time_schedule.preset_angle); // ��ʾ��ʼ�˵�
                    UsartPrintf(USART_DEBUG, "[MENU] Enter angle preset\r\n");
                    break;
                }
            }
        }
        
        key_num = 0;  // ����������Ƶ����ȷ����������¶����������־

        // ����ѭ����index_bh1750�����֣���Ӳ˵�״̬���
        if(index_bh1750)
        {
            UsartPrintf(USART_DEBUG, "[BH1750] Starting read...\r\n");
    
            // ���ɿ��Ķ�ȡ����
            bh_data_send(BHReset);   // ���ô�����
            delayms(10);
            bh_data_send(BHModeH1);  // ����Ϊ�߾���ģʽ
            delayms(180);           // ��������Ҫʱ����в���
            
            float new_light = bh_data_read() / 1.2;
            
            // ����ȡֵ�Ƿ����
            if(new_light > 0 && new_light < 10000) {
                light_value = new_light;
                UsartPrintf(USART_DEBUG, "[LIGHT] Valid reading: %.1f lux\r\n", light_value);
                
                // ����������ʾ
                if(current_menu == MENU_NONE) {
                    sprintf(light_info, "Lux:%-5.0f", light_value);
                    OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)light_info, 2);
                }
                
                // ����AUTO��TIMELOCKģʽ
                if(work_mode == AUTO_MODE || 
                  (work_mode == TIMELOCK_MODE && !IsInTimeSchedule())) {
                    uint8_t new_angle = LightToFixedAngle(light_value);
                    if(angle != new_angle) {
                        angle = new_angle;
                        pwm = AngleToPWM(angle);
                        TIM_SetCompare1(TIM2, pwm);
                        TIM_SetCompare2(TIM2, pwm);
                        
                        // ���½Ƕ���ʾ
                        if(current_menu == MENU_NONE) {
                            char angle_str[16];
                            sprintf(angle_str, "Angle:%3d", angle); 
                            OLED_ShowStr(0, LINE_ANGLE, (uint8_t *)angle_str, 2);
                        }
                    }
                }
            } else {
                UsartPrintf(USART_DEBUG, "[ERROR] Failed to get valid BH1750 readings!\r\n");
            }
            
            index_bh1750 = 0;  // ���۳ɹ�ʧ�ܣ�����λ��־
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
		
        // �򻯶�ʱ�������߼�
        // ÿ50ms����һ�μ�����
        tick++;

        // ֱ�Ӵ������ն�ȡ����ʹ�������ж�
        if(tick % 20 == 0 && !index_bh1750) { // ÿ200ms
            index_bh1750 = 1;
            UsartPrintf(USART_DEBUG, "[TRIGGER] BH1750 read triggered\r\n");
        }

        // ÿ500ms����һ�ε�����Ϣ
        if(tick % 50 == 0) {
            UsartPrintf(USART_DEBUG, "[STATUS] Tick=%lu light=%.1f mode=%d\r\n", 
                    tick, light_value, work_mode);
        }

        delayms(10); // 10ms��ʱ

        }
    }
}
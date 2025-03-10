/* ͷ�ļ���������˵�� */
// ϵͳ�ͱ�׼��
#include "stm32f10x.h"
#include "sys.h"
#include <string.h>
#include <stdio.h>


// Ӳ������
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "oled.h"  // ���OLEDͷ�ļ�

// ����ͨ��
#include "onenet.h"
#include "esp8266.h"

// �Զ���ģ��
#include "hw_control.h"
#include "mode_manager.h"
#include "display_manager.h"
#include "network_manager.h"  // �����������ģ��ͷ�ļ�

// �ⲿ��������
extern volatile uint8_t clock_update_flag;
extern void UpdateClock(void);
extern NTP_Time ntp_time;  // ԭ���� extern struct NTP_TIME ntp_time;

// ȫ�ֱ�������
uint8_t index_bh1750 = 0;
float light_value = 0;

// ���غ�������
static void ProcessLightSensor(float light_value);
static void ProcessUserInput(void);

int main(void)
{    
    uint32_t tick = 0;
    uint8_t index_duoji = 0;
    unsigned short timeCount = 0;
    unsigned char *dataPtr = NULL;
    uint8_t prev_timer_setting_index = 0;
    
    // Ӳ����ʼ��
    Hardware_Init();
    ModeManager_Init();
    
    // �����ʼ��
    if(NetworkManager_Initialize() != 0) {
        Display_ShowError("Network Init Failed");
        // ������
    }
    
    // ��ѭ��
    while(1)
    {
        // ʱ�Ӹ���
        if(clock_update_flag)
        {
            clock_update_flag = 0;
            UpdateClock();
            
            Display_UpdateClock(ntp_time.year, ntp_time.month, ntp_time.day, 
                               ntp_time.hour, ntp_time.minute, ntp_time.second, 
                               ntp_time.is_valid);
            
            // ����ʱģʽ
            ModeManager_HandleTimerMode(ntp_time.hour, ntp_time.minute);
        }

        // ���ģʽ�仯
        ModeManager_HandleModeChange();
        
        // �����û�����
        ProcessUserInput();

        // ���մ�������ȡ
        if(index_bh1750)
        {
            light_value = HW_ReadLightSensor();
            ProcessLightSensor(light_value);
            index_bh1750 = 0;
        }
        
        // �������
        if(index_duoji)
        {
            HW_UpdateServos(ModeManager_GetCurrentAngle());
            index_duoji = 0;
        }
        
        // ��ƽ̨���ݴ���
        if(++timeCount >= 100)    // 5����
        {
            NetworkManager_SendData();
            timeCount = 0;
        }

        // �������ݴ���
        NetworkManager_ProcessIncoming();
        
        // ��ʱ��
        if(++tick % 50 == 0) index_bh1750 = 1;
        if(tick % 10 == 0) index_duoji = 1;
        delayms(10);
    }
}

// �����û�����
static void ProcessUserInput(void)
{
    key_Functoin();
    if(key_num != 0)
    {
        if(key_num == 1) {
            ModeManager_SwitchMode();
        } else {
            ModeManager_AdjustTimerSettings(key_num);
        }
        key_num = 0;
    }
}

// ������մ���������
static void ProcessLightSensor(float light_value)
{
    UsartPrintf(USART_DEBUG, "[DEBUG] light=%.0f\n", light_value);
    
    // ���½Ƕ�
    ModeManager_UpdateAngle(light_value);
    
    // ������ʾ
    SystemMode mode = ModeManager_GetCurrentMode();
    Display_UpdateMode(mode);
    Display_UpdateAngle(ModeManager_GetCurrentAngle());
    
    if(mode == AUTO_MODE || mode == MANUAL_MODE) {
        Display_UpdateLightValue(light_value);
    }
    else if(mode == TIMER_MODE) {
        TimerControl* settings = ModeManager_GetTimerSettings();
        static uint8_t prev_timer_setting_index = 0;
        
        Display_UpdateTimerSettings(
            settings->start_hour, settings->start_minute,
            settings->end_hour, settings->end_minute,
            settings->angle, 
            ModeManager_GetTimerSettingIndex(), 
            prev_timer_setting_index
        );
        prev_timer_setting_index = ModeManager_GetTimerSettingIndex();
    }
}
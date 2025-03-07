#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm32f10x.h"

// �˵�״̬ö�� - ����������
typedef enum {
    MENU_NONE,             // �ǲ˵�״̬
    MENU_TIME_START_HOUR,  // ���ÿ�ʼСʱ
    MENU_TIME_START_MIN,   // ���ÿ�ʼ����
    MENU_TIME_END_HOUR,    // ���ý���Сʱ
    MENU_TIME_END_MIN,     // ���ý�������
    MENU_ANGLE_PRESET      // �Ƕ�Ԥ��ѡ��
} MenuState;

// ʱ��ƻ��ṹ��
typedef struct {
    uint8_t start_hour;    // ��ʼСʱ
    uint8_t start_minute;  // ��ʼ����
    uint8_t end_hour;      // ����Сʱ
    uint8_t end_minute;    // ��������
    uint8_t preset_angle;  // Ԥ��Ƕ�
    uint8_t is_active;     // �Ƿ񼤻�
} TimeSchedule;

// ȫ�ֱ�־λ����
extern uint8_t index_led, index_bh1750, index_ds18b20, index_step, index_duoji;
extern uint8_t index_key;      // ����Ŀ�꺯����Ҫ�İ�����־
extern uint8_t usart1_Rx_ok;   // �������ڽ�����ɱ�־

// ��������
extern void PA12_Init(void);
extern void key_Functoin(void); // ע��ԭ������ƴд��������ƴд����
void DisplayMenu(MenuState menu, uint8_t value);
void DisplayTimeSchedule(void);
void ProcessMenuOperation(uint8_t key_id, uint8_t press_type);
uint8_t IsInTimeSchedule(void);

#endif
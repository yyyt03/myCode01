#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"

// ȫ�ֱ�־λ����
extern uint8_t index_led, index_bh1750, index_ds18b20, index_step, index_duoji;
extern uint8_t index_key;      // ����Ŀ�꺯����Ҫ�İ�����־
extern uint8_t usart1_Rx_ok;   // �������ڽ�����ɱ�־


// ��������
extern void PA12_Init(void);
extern void key_Functoin(void); // ע��ԭ������ƴд��������ƴд����

#endif
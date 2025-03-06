#ifndef __bh1750_H
#define __bh1750_H	 

#include "myiic.h"
#include "delay.h"

#define ADDR 0x23//0100011
#define BHAddWrite     0x46      //�ӻ���ַ+���д����λ
#define BHAddRead      0x47      //�ӻ���ַ+��������λ
#define BHPowDown      0x00      //�ر�ģ��
#define BHPowOn        0x01      //��ģ��ȴ�����ָ��
#define BHReset        0x07      //�������ݼĴ�����poweronģʽ����Ч
#define BHModeH1       0x10      //�߷ֱ��� ��λ1lx ����ʱ��120ms
#define BHModeH2       0x11      //�߷ֱ���2 ��λ0.5lx ����ʱ��120ms
#define BHModeL        0x13      //�ͷֱ��� ��λ4lx ����ʱ��16ms
#define BHSigModeH     0x20      //һ�θ߷ֱ��ʲ���  ��ģ��ת��powerdownģʽ
#define BHSigModeH2    0x21      //ͬ������
#define BHSigModeL     0x23      // ͬ��

void Single_Write_BH1750(unsigned char REG_Address);
void Init_BH1750(void);
void bh_data_send(u8 command);
u16 bh_data_read(void);

#endif



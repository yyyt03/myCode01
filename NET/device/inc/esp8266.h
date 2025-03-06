#ifndef _ESP8266_H_
#define _ESP8266_H_


#define ESP8266_WIFI_INFO		"AT+CWJAP=\"DESKTOP-MJVRCN3 8391\",\"TEST8888\"\r\n"

#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"


#define REV_OK		0	//������ɱ�־
#define REV_WAIT	1	//����δ��ɱ�־



// ����NTPʱ��ṹ��
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    char weekday[4];
    uint8_t is_valid;  // ʱ���Ƿ���Ч
} NTP_Time;

// ȫ��ʱ���������
extern NTP_Time ntp_time;

// ��������
void ESP8266_Init(void);
void ESP8266_Clear(void);
_Bool ESP8266_WaitRecive(void);  // ȷ������ʹ��_Bool����
_Bool ESP8266_SendCmd(char *cmd, char *res);
void ESP8266_SendData(unsigned char *data, unsigned short len);
unsigned char *ESP8266_GetIPD(unsigned short timeOut);
uint8_t ESP8266_GetNTPTime(void);
void UpdateClock(void); // ���±���ʱ�ӣ�ÿ�����һ��

#endif

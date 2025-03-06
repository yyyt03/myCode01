#ifndef _ESP8266_H_
#define _ESP8266_H_


#define ESP8266_WIFI_INFO		"AT+CWJAP=\"DESKTOP-MJVRCN3 8391\",\"TEST8888\"\r\n"

#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"


#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志



// 新增NTP时间结构体
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    char weekday[4];
    uint8_t is_valid;  // 时间是否有效
} NTP_Time;

// 全局时间变量声明
extern NTP_Time ntp_time;

// 函数声明
void ESP8266_Init(void);
void ESP8266_Clear(void);
_Bool ESP8266_WaitRecive(void);  // 确保这里使用_Bool类型
_Bool ESP8266_SendCmd(char *cmd, char *res);
void ESP8266_SendData(unsigned char *data, unsigned short len);
unsigned char *ESP8266_GetIPD(unsigned short timeOut);
uint8_t ESP8266_GetNTPTime(void);
void UpdateClock(void); // 更新本地时钟，每秒调用一次

#endif

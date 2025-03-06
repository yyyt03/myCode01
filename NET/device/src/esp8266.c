/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	esp8266.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-08
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		ESP8266�ļ�����
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸����
#include "esp8266.h"

//Ӳ������
#include "delay.h"
#include "usart.h"

//C��
#include <string.h>
#include <stdio.h>
#include "oled.h"





unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
// ���ļ���ͷ��ȫ�ֱ����������
NTP_Time ntp_time = {0};

// ����NTPʱ���ַ���
void ParseNTPTime(char *timeStr) 
{
    // ��ʽ���磺"Mon Nov 20 15:30:45 2023"
    char month[4];
    if(sscanf(timeStr, "%s %s %d %d:%d:%d %d", 
           ntp_time.weekday, month, &ntp_time.day, 
           &ntp_time.hour, &ntp_time.minute, &ntp_time.second,
           &ntp_time.year) == 7)
    {
        // ת��Ӣ���·�Ϊ����
        if(strstr(month, "Jan")) ntp_time.month = 1;
        else if(strstr(month, "Feb")) ntp_time.month = 2;
        else if(strstr(month, "Mar")) ntp_time.month = 3;
        else if(strstr(month, "Apr")) ntp_time.month = 4;
        else if(strstr(month, "May")) ntp_time.month = 5;
        else if(strstr(month, "Jun")) ntp_time.month = 6;
        else if(strstr(month, "Jul")) ntp_time.month = 7;
        else if(strstr(month, "Aug")) ntp_time.month = 8;
        else if(strstr(month, "Sep")) ntp_time.month = 9;
        else if(strstr(month, "Oct")) ntp_time.month = 10;
        else if(strstr(month, "Nov")) ntp_time.month = 11;
        else if(strstr(month, "Dec")) ntp_time.month = 12;
        
        // ����ʱ����Ч��־
        ntp_time.is_valid = 1;
    }
    else {
        // ����ʧ�ܣ�������Ч��־
        ntp_time.is_valid = 0;
    }
}

// NTPʱ���ȡ����
uint8_t ESP8266_GetNTPTime(void)
{
    ESP8266_Clear();
    
    // ����SNTP������ (���ã�ʱ��+8��NTP��������ַ)
    UsartPrintf(USART_DEBUG, "Setting SNTP config...\r\n");
    if(ESP8266_SendCmd("AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n", "OK"))
    {
        UsartPrintf(USART_DEBUG, "SNTP config failed\r\n");
        ntp_time.is_valid = 0;  // ���ʱ����Ч
        return 1;
    }
    
    delayms(500); // �ȴ�SNTP������Ч
    ESP8266_Clear();
    
    // ��ѯSNTPʱ��
    UsartPrintf(USART_DEBUG, "Querying SNTP time...\r\n");
    Usart_SendString(USART2, (unsigned char *)"AT+CIPSNTPTIME?\r\n", strlen("AT+CIPSNTPTIME?\r\n"));
    
    // �ȴ�ʱ����Ӧ
    uint8_t timeOut = 50;  // 5�볬ʱ
    while(timeOut--)
    {
        if(ESP8266_WaitRecive() == REV_OK)
        {
            // ��鷵�ص�ʱ���ַ���
            char *timeStr = strstr((char *)esp8266_buf, "+CIPSNTPTIME:");
            if(timeStr != NULL)
            {
                // ��������ͷ
                timeStr += strlen("+CIPSNTPTIME:");
                // �����ո�ͻس�����
                while(*timeStr == ' ' || *timeStr == '\r' || *timeStr == '\n')
                    timeStr++;
                
                ParseNTPTime(timeStr);
                ESP8266_Clear();
                
                // ���������
                if(ntp_time.is_valid && ntp_time.year >= 2000) {
                    return 0;  // �ɹ���ȡʱ��
                }
            }
        }
        delayms(100);
    }
    
    UsartPrintf(USART_DEBUG, "Get SNTP time failed\r\n");
    ntp_time.is_valid = 0;  // ���ʱ����Ч
    return 1;  // ��ȡʱ��ʧ��
}

//==========================================================
//	�������ƣ�	ESP8266_Clear
//
//	�������ܣ�	��ջ���
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

//==========================================================
//	�������ƣ�	ESP8266_WaitRecive
//
//	�������ܣ�	�ȴ��������
//
//	��ڲ�����	��
//
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//
//	˵����		ѭ�����ü���Ƿ�������
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		esp8266_cnt = 0;							//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	esp8266_cntPre = esp8266_cnt;					//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־

}

//==========================================================
//	�������ƣ�	ESP8266_SendCmd
//
//	�������ܣ�	��������
//
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	Usart_SendString(USART2, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//����յ�����
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//����������ؼ���
			{
				ESP8266_Clear();									//��ջ���
				
				return 0;
			}
		}
		
		delayms(10);
	}
	
	return 1;

}

//==========================================================
//	�������ƣ�	ESP8266_SendData
//
//	�������ܣ�	��������
//
//	��ڲ�����	data������
//				len������
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();								//��ս��ջ���
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//��������
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//�յ���>��ʱ���Է�������
	{
		Usart_SendString(USART2, data, len);		//�����豸������������
	}

}

//==========================================================
//	�������ƣ�	ESP8266_GetIPD
//
//	�������ܣ�	��ȡƽ̨���ص�����
//
//	��ڲ�����	�ȴ���ʱ��(����10ms)
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//
//	˵����		��ͬ�����豸���صĸ�ʽ��ͬ����Ҫȥ����
//				��ESP8266�ķ��ظ�ʽΪ	"+IPD,x:yyy"	x�������ݳ��ȣ�yyy����������
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//����������
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,");				//������IPD��ͷ
			if(ptrIPD == NULL)											//���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
			{
				//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//�ҵ�':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
		}
		
		delayms(5);													//��ʱ�ȴ�
	} while(timeOut--);
	
	return NULL;														//��ʱ��δ�ҵ������ؿ�ָ��

}

//==========================================================
//	�������ƣ�	ESP8266_Init
//
//	�������ܣ�	��ʼ��ESP8266
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_Init(void)
{	
	ESP8266_Clear();
	
	UsartPrintf(USART_DEBUG, "1. AT\r\n");
	OLED_ShowStr(0,2,(uint8_t *)"1. AT        ",2);
	while(ESP8266_SendCmd("AT\r\n", "OK"))
		delayms(500);
	
	UsartPrintf(USART_DEBUG, "2. AT+RST\r\n");
	OLED_ShowStr(0,2,(uint8_t *)"2. AT+RST      ",2);
	while(ESP8266_SendCmd("AT+RST\r\n", "OK"))
		delayms(500);
	
	UsartPrintf(USART_DEBUG, "3. CWMODE\r\n");
	OLED_ShowStr(0,2,(uint8_t *)"3. CWMODE    ",2);
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		delayms(500);
	
	UsartPrintf(USART_DEBUG, "4. AT+CWDHCP\r\n");
	OLED_ShowStr(0,2,(uint8_t *)"4. AT+CWDHCP     ",2);
	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		delayms(500);
	
	UsartPrintf(USART_DEBUG, "5. CWJAP\r\n");
	OLED_ShowStr(0,2,(uint8_t *)"5. CWJAP        ",2);
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
		delayms(500);
	
    // ��ȡNTPʱ�� (��TCP����֮ǰ)
    UsartPrintf(USART_DEBUG, "6. NTP Time\r\n");
    OLED_ShowStr(0,2,(uint8_t *)"6. NTP Time    ",2);
    if(ESP8266_GetNTPTime() == 0) {
        UsartPrintf(USART_DEBUG, "NTP Time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
                    ntp_time.year, ntp_time.month, ntp_time.day,
                    ntp_time.hour, ntp_time.minute, ntp_time.second);
        
        // ��ʾ��ȡ����ʱ��
        char timeStr[20];
        sprintf(timeStr, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
        OLED_ShowStr(0,4,(uint8_t *)timeStr,2);
    } else {
        UsartPrintf(USART_DEBUG, "Failed to get NTP time\r\n");
        OLED_ShowStr(0,4,(uint8_t *)"NTP Failed",2);
    }
    
    // ����OneNET (TCP����)
    UsartPrintf(USART_DEBUG, "7. CIPSTART\r\n");
    OLED_ShowStr(0,2,(uint8_t *)"7. CIPSTART     ",2);
    while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
        delayms(500);
    
    UsartPrintf(USART_DEBUG, "8. ESP8266 Init OK\r\n");
    OLED_ShowStr(0,2,(uint8_t *)"ESP8266 Init OK ",2);

}

//==========================================================
//	�������ƣ�	USART2_IRQHandler
//
//	�������ܣ�	����2�շ��ж�
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //�����ж�
	{
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //��ֹ���ڱ�ˢ��
		esp8266_buf[esp8266_cnt++] = USART2->DR;
		
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

}


// ���±���ʱ�ӣ�ÿ�����һ��
void UpdateClock(void)
{
    // ֻ�гɹ���ȡ��NTPʱ��Ÿ���
    if(!ntp_time.is_valid || ntp_time.year < 2000) return;
    
    // ������
    ntp_time.second++;
    
    // �����λ
    if(ntp_time.second >= 60)
    {
        ntp_time.second = 0;
        ntp_time.minute++;
        
        // ������� - ÿ�������һ��
        UsartPrintf(USART_DEBUG, "[TIME] New minute: %02d:%02d:00\r\n", 
                  ntp_time.hour, ntp_time.minute);
        
        if(ntp_time.minute >= 60)
        {
            ntp_time.minute = 0;
            ntp_time.hour++;
            
            // ������� - ÿСʱ���һ��
            UsartPrintf(USART_DEBUG, "[TIME] New hour: %02d:00:00\r\n", ntp_time.hour);
            
            if(ntp_time.hour >= 24)
            {
                ntp_time.hour = 0;
                ntp_time.day++;
                
                // �򻯵��·ݴ���ʵ����Ҫ����ÿ���µ�����
                uint8_t days_in_month;
                switch(ntp_time.month)
                {
                    case 2: // ����
                        // �������ж�
                        days_in_month = ((ntp_time.year % 4 == 0 && ntp_time.year % 100 != 0) || 
                                        (ntp_time.year % 400 == 0)) ? 29 : 28;
                        break;
                    case 4: case 6: case 9: case 11: // 30����·�
                        days_in_month = 30;
                        break;
                    default: // 31����·�
                        days_in_month = 31;
                        break;
                }
                
                if(ntp_time.day > days_in_month)
                {
                    ntp_time.day = 1;
                    ntp_time.month++;
                    
                    if(ntp_time.month > 12)
                    {
                        ntp_time.month = 1;
                        ntp_time.year++;
                    }
                }
            }
        }
    }
}
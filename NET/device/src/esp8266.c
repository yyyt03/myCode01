/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	esp8266.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-08
	*
	*	版本： 		V1.0
	*
	*	说明： 		ESP8266的简单驱动
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络设备驱动
#include "esp8266.h"

//硬件驱动
#include "delay.h"
#include "usart.h"

//C库
#include <string.h>
#include <stdio.h>
#include "oled.h"





unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
// 在文件开头的全局变量区域添加
NTP_Time ntp_time = {0};

// 解析NTP时间字符串
void ParseNTPTime(char *timeStr) 
{
    // 格式例如："Mon Nov 20 15:30:45 2023"
    char month[4];
    if(sscanf(timeStr, "%s %s %d %d:%d:%d %d", 
           ntp_time.weekday, month, &ntp_time.day, 
           &ntp_time.hour, &ntp_time.minute, &ntp_time.second,
           &ntp_time.year) == 7)
    {
        // 转换英文月份为数字
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
        
        // 设置时间有效标志
        ntp_time.is_valid = 1;
    }
    else {
        // 解析失败，设置无效标志
        ntp_time.is_valid = 0;
    }
}

// NTP时间获取函数
uint8_t ESP8266_GetNTPTime(void)
{
    ESP8266_Clear();
    
    // 配置SNTP服务器 (启用，时区+8，NTP服务器地址)
    UsartPrintf(USART_DEBUG, "Setting SNTP config...\r\n");
    if(ESP8266_SendCmd("AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n", "OK"))
    {
        UsartPrintf(USART_DEBUG, "SNTP config failed\r\n");
        ntp_time.is_valid = 0;  // 标记时间无效
        return 1;
    }
    
    delayms(500); // 等待SNTP配置生效
    ESP8266_Clear();
    
    // 查询SNTP时间
    UsartPrintf(USART_DEBUG, "Querying SNTP time...\r\n");
    Usart_SendString(USART2, (unsigned char *)"AT+CIPSNTPTIME?\r\n", strlen("AT+CIPSNTPTIME?\r\n"));
    
    // 等待时间响应
    uint8_t timeOut = 50;  // 5秒超时
    while(timeOut--)
    {
        if(ESP8266_WaitRecive() == REV_OK)
        {
            // 检查返回的时间字符串
            char *timeStr = strstr((char *)esp8266_buf, "+CIPSNTPTIME:");
            if(timeStr != NULL)
            {
                // 跳过命令头
                timeStr += strlen("+CIPSNTPTIME:");
                // 跳过空格和回车换行
                while(*timeStr == ' ' || *timeStr == '\r' || *timeStr == '\n')
                    timeStr++;
                
                ParseNTPTime(timeStr);
                ESP8266_Clear();
                
                // 检查解析结果
                if(ntp_time.is_valid && ntp_time.year >= 2000) {
                    return 0;  // 成功获取时间
                }
            }
        }
        delayms(100);
    }
    
    UsartPrintf(USART_DEBUG, "Get SNTP time failed\r\n");
    ntp_time.is_valid = 0;  // 标记时间无效
    return 1;  // 获取时间失败
}

//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志

}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	Usart_SendString(USART2, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		delayms(10);
	}
	
	return 1;

}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();								//清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//收到‘>’时可以发送数据
	{
		Usart_SendString(USART2, data, len);		//发送设备连接请求数据
	}

}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,");				//搜索“IPD”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//找到':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
		}
		
		delayms(5);													//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
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
	
    // 获取NTP时间 (在TCP连接之前)
    UsartPrintf(USART_DEBUG, "6. NTP Time\r\n");
    OLED_ShowStr(0,2,(uint8_t *)"6. NTP Time    ",2);
    if(ESP8266_GetNTPTime() == 0) {
        UsartPrintf(USART_DEBUG, "NTP Time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
                    ntp_time.year, ntp_time.month, ntp_time.day,
                    ntp_time.hour, ntp_time.minute, ntp_time.second);
        
        // 显示获取到的时间
        char timeStr[20];
        sprintf(timeStr, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
        OLED_ShowStr(0,4,(uint8_t *)timeStr,2);
    } else {
        UsartPrintf(USART_DEBUG, "Failed to get NTP time\r\n");
        OLED_ShowStr(0,4,(uint8_t *)"NTP Failed",2);
    }
    
    // 连接OneNET (TCP连接)
    UsartPrintf(USART_DEBUG, "7. CIPSTART\r\n");
    OLED_ShowStr(0,2,(uint8_t *)"7. CIPSTART     ",2);
    while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
        delayms(500);
    
    UsartPrintf(USART_DEBUG, "8. ESP8266 Init OK\r\n");
    OLED_ShowStr(0,2,(uint8_t *)"ESP8266 Init OK ",2);

}

//==========================================================
//	函数名称：	USART2_IRQHandler
//
//	函数功能：	串口2收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断
	{
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
		esp8266_buf[esp8266_cnt++] = USART2->DR;
		
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

}


// 更新本地时钟，每秒调用一次
void UpdateClock(void)
{
    // 只有成功获取过NTP时间才更新
    if(!ntp_time.is_valid || ntp_time.year < 2000) return;
    
    // 更新秒
    ntp_time.second++;
    
    // 处理进位
    if(ntp_time.second >= 60)
    {
        ntp_time.second = 0;
        ntp_time.minute++;
        
        // 调试输出 - 每分钟输出一次
        UsartPrintf(USART_DEBUG, "[TIME] New minute: %02d:%02d:00\r\n", 
                  ntp_time.hour, ntp_time.minute);
        
        if(ntp_time.minute >= 60)
        {
            ntp_time.minute = 0;
            ntp_time.hour++;
            
            // 调试输出 - 每小时输出一次
            UsartPrintf(USART_DEBUG, "[TIME] New hour: %02d:00:00\r\n", ntp_time.hour);
            
            if(ntp_time.hour >= 24)
            {
                ntp_time.hour = 0;
                ntp_time.day++;
                
                // 简化的月份处理，实际需要考虑每个月的天数
                uint8_t days_in_month;
                switch(ntp_time.month)
                {
                    case 2: // 二月
                        // 简单闰年判断
                        days_in_month = ((ntp_time.year % 4 == 0 && ntp_time.year % 100 != 0) || 
                                        (ntp_time.year % 400 == 0)) ? 29 : 28;
                        break;
                    case 4: case 6: case 9: case 11: // 30天的月份
                        days_in_month = 30;
                        break;
                    default: // 31天的月份
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
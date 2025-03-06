/* 头文件包含分区说明 --------------------------------------------------------*/
//单片机核心库
#include "stm32f10x.h"          // STM32F10x系列标准外设库

//硬件设备驱动层
#include "delay.h"              // SysTick延时函数（精确延时1us/10us/1ms）
#include "usart.h"             // USART1串口驱动（115200bps, 中断接收）
#include "oled.h"              // 0.96寸OLED显示屏驱动（I2C接口）
#include "bh1750.h"            // BH1750FVI光照传感器驱动（I2C接口）
#include "timer.h"             // 定时器驱动（TIM2 PWM输出控制）
#include "key.h"               // 独立按键驱动（支持单击检测）

//系统抽象层
#include "sys.h"               // 系统级定义（时钟配置、类型重定义）

//网络协议层
#include "onenet.h"             // OneNET物联网平台通信协议

//网络设备
#include "esp8266.h"           // ESP8266 WiFi模块驱动

//C标准库
#include <string.h>            // 字符串处理函数（memcpy/memset等）
#include <stdio.h>             // 标准输入输出（用于sprintf格式化）

//枚举类型移动到sys.h
//typedef enum {
//    AUTO_MODE,
//    MANUAL_MODE
//} SystemMode;

/* 全局变量声明 -----------------------------------------------------------*/
uint8_t index_bh1750 = 0, index_duoji = 0; // 传感器和舵机的索引标志
SystemMode work_mode = AUTO_MODE;         // 系统工作模式（自动/手动）
uint8_t manual_angle = 90;               // 手动模式下的目标角度
unsigned short timeCount = 0;             // 发送间隔计时器
char light_info[16];                      // 光照度显示缓存
float light_value = 0;                    // 实时光照强度值
uint8_t angle = 0;                       // 当前目标角度
char time_str[20]; // 时间显示缓存

// 在main.c文件开头加入外部变量声明
extern volatile uint8_t clock_update_flag;

/* 阈值说明（单位：lux） --------------------------------------------------*/
#define LIGHT_THRESHOLD_LOW       50   // 室内昏暗光照（0度）
#define LIGHT_THRESHOLD_MID_LOW  100   // 正常室内照明（30度）
#define LIGHT_THRESHOLD_MID      200   // 明亮室内环境（90度）
#define LIGHT_THRESHOLD_MID_HIGH 500   // 强光照环境（150度）
#define LIGHT_THRESHOLD_HIGH    1000   // 直射阳光环境（180度）

/* 舵机角度定义（单位：度） --------------------------------------------------*/
#define ANGLE_ULTRA_LOW     0    // 完全闭合状态
#define ANGLE_LOW          30    // 小角度开启
#define ANGLE_MID          90    // 中等开启角度
#define ANGLE_HIGH        150    // 大角度开启
#define ANGLE_ULTRA_HIGH  180    // 完全展开状态
#define ANGLE_STEP        30     // 角度调整步长

/* 固定行号分配 ------------------------------------------------------------*/
#define LINE_MODE   1   // 第2行（显示模式）
#define LINE_ANGLE  3   // 第4行（显示角度）
#define LINE_LIGHT  5   // 第6行（显示光照强度）
#define LINE_SEND   7   // 第8行（显示发送状态）

/*
************************************************************
*	函数名称：	Hardware_Init
*
*	函数功能：	硬件综合初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		依次初始化以下模块：
*				[1] 设置NVIC中断分组为组2（2位抢占优先级，2位响应优先级）
*				[2] 初始化SysTick延时函数
*				[3] 初始化USART1串口（115200bps，用于调试输出）
*				[4] 初始化USART2串口（115200bps，用于ESP8266通信）
*				[5] 初始化OLED显示屏并清屏
*				[6] 初始化BH1750光照传感器并上电启动
*				[7] 初始化按键检测GPIO
*				[8] 初始化TIM2通道1 PWM输出（72MHz/720=100kHz，周期20ms）
*				[9] 显示系统初始化信息
*
*				关键参数：
*				- USART1波特率：115200bps，用于调试信息输出
*				- USART2波特率：115200bps，用于ESP8266通信
*				- PWM频率：100kHz，周期20ms
************************************************************
*/
void Hardware_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    // 设置中断优先级分组
    delay_Init();                                      // 初始化延时函数
	Usart1_Init(115200);							//串口1，打印信息用	
	Usart2_Init(115200);							//串口2，驱动ESP8266用
	
    OLED_Init();                    // 初始化OLED显示屏
    OLED_CLS();                     // 清空屏幕显示
    
    Init_BH1750();                  // 初始化BH1750光照传感器
    bh_data_send(BHPowOn);          // 发送上电指令
    key_Init();                     // 初始化按键检测
    
    // 初始化舵机PWM：72MHz/(720+1) = 100kHz，周期20ms（100kHz * 20ms = 2000）
    tiemr2_Ch1_Pwm_Init(720-1, 2000, 100); 
    delayms(1000);                  // 等待舵机初始化完成
    
    // 显示初始化信息
    OLED_ShowStr(0,0,(uint8_t *)"IoT Init...",2);
    UsartPrintf(USART_DEBUG, "Hardware init OK\r\n");
}	

/*
************************************************************
*	函数名称：	AngleToPWM
*
*	函数功能：	舵机角度转PWM占空比
*
*	入口参数：	angle - 目标角度值 (0~180度)
*
*	返回参数：	uint8_t - 对应的PWM占空比
*
*	说明：		根据舵机特性进行线性转换：
*				计算公式：PWM = 50 + (angle * 190) / 180
*				对应参数：
*				- 0度 → 50/2000 = 2.5% 占空比
*				- 180度 → 240/2000 = 12% 占空比
*				确保输入角度在有效范围内（0≤angle≤180）
************************************************************
*/
uint16_t AngleToPWM(uint8_t angle) {
    return 50 + angle * 190 / 180;
}

/*
************************************************************
*	函数名称：	LightToFixedAngle
*
*	函数功能：	光照强度到固定角度映射
*
*	入口参数：	light_value - BH1750测量的光照强度值（单位：lux）
*
*	返回参数：	uint8_t - 映射后的固定角度值（0~180度）
*
*	说明：		根据预设阈值进行阶梯式角度分配：
*				[0,50)    → 0度（完全闭合）
*				[50,100)  → 30度
*				[100,200) → 90度（中间位置）
*				[200,500) → 150度
*				[500,∞)   → 180度（完全展开）
*				阈值可根据实际窗帘透光需求调整
************************************************************
*/
uint8_t LightToFixedAngle(float light_value)
{
    if(light_value < LIGHT_THRESHOLD_LOW)        
        return ANGLE_ULTRA_LOW;  // 0度 (当光照<50)
    else if(light_value < LIGHT_THRESHOLD_MID_LOW) 
        return ANGLE_LOW;        // 30度 (50≤光照<100)
    else if(light_value < LIGHT_THRESHOLD_MID)     
        return ANGLE_MID;        // 90度 (100≤光照<200)
    else if(light_value < LIGHT_THRESHOLD_MID_HIGH)
        return ANGLE_HIGH;       // 150度 (200≤光照<500)
    else                                            
        return ANGLE_ULTRA_HIGH; // 180度 (光照≥500)
}


/*
************************************************************
*	函数名称：	main
*
*	函数功能：	主程序入口
*
*	入口参数：	无
*
*	返回参数：	int - 实际未使用（保持标准格式）
*
*	说明：		系统主控制循环，包含以下功能：
*				[1] 硬件初始化
*				[2] 初始化ESP8266 WiFi模块
*				[3] 连接OneNET平台（重试机制）
*				[4] 进入主循环：
*					- 按键扫描及处理（模式切换/角度调节）
*					- 定时采集光照数据（500ms间隔）
*					- 根据模式计算目标角度
*					- 更新OLED显示（模式/角度/光照值）
*					- PWM输出控制舵机角度
*					- 定时更新机制（delayms实现简单调度）
*					- 数据采集与调试信息输出
*					- 网络数据发送与接收处理
*
*				关键参数：
*				- 发送间隔：5秒（100*50ms）
*				- 数据采集周期：500ms
************************************************************
*/
int main(void)
{	
    unsigned short timeCount = 0; // 发送间隔计时器
    unsigned char *dataPtr = NULL; // 接收数据指针
    
    // 将tick声明提前到函数开头，并移除clock_tick
    uint32_t tick = 0;
    uint32_t last_second = 0;
    uint8_t pwm = 100;
    
    Hardware_Init();        // 硬件初始化（含外设）
    ESP8266_Init();         // 初始化WiFi模块

    // 连接OneNET平台并订阅控制指令
    OLED_ShowStr(0,0,(uint8_t *)"Conn OneNET...",2);
    while(OneNet_DevLink()) {
        delayms(500);
        OLED_ShowStr(0,2,(uint8_t *)"Retrying...",2);
    }
    OneNET_Subscribe();     // 订阅控制指令
    
    // 连接成功提示
    OLED_ShowStr(0,0,(uint8_t *)"SmartLight OK",2);
    // 显示初始时间
    sprintf(time_str, "%04d-%02d-%02d", ntp_time.year, ntp_time.month, ntp_time.day);
    OLED_ShowStr(0,0,(uint8_t *)time_str,1);
    sprintf(time_str, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
    OLED_ShowStr(64,0,(uint8_t *)time_str,1);
    
    delayms(200);
    OLED_CLS();
    
    while(1)
    {
        // 使用中断设置的标志更新时钟，更准确
        if(clock_update_flag)
        {
            clock_update_flag = 0;  // 清除标志
            UpdateClock();          // 更新时钟
            
            // 更新时间显示
            if(ntp_time.is_valid && ntp_time.year >= 2000) {
                sprintf(time_str, "%04d-%02d-%02d", ntp_time.year, ntp_time.month, ntp_time.day);
                OLED_ShowStr(0,0,(uint8_t *)time_str,1);
                sprintf(time_str, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
                OLED_ShowStr(64,0,(uint8_t *)time_str,1);
                
                // 调试输出当前时间
                UsartPrintf(USART_DEBUG, "[TIME] %02d:%02d:%02d\r\n", 
                            ntp_time.hour, ntp_time.minute, ntp_time.second);
            } else {
                // 如果时间无效，显示等待NTP同步的信息
                OLED_ShowStr(0,0,(uint8_t *)"Wait NTP Sync",1);
            }
        }
        
        // 其余代码保持不变...
        key_Functoin();  
        if(key_num != 0)
        {
            switch(key_num)
            {
            case 1: // 模式切换
                work_mode = (work_mode == AUTO_MODE) ? MANUAL_MODE : AUTO_MODE;
                if(work_mode == MANUAL_MODE) manual_angle = 90;
                break;
                
            case 2: // 角度+ANGLE_STEP
                if(work_mode == MANUAL_MODE) 
                {
                    manual_angle = (manual_angle >= 180) ? 0 : manual_angle + ANGLE_STEP;
                }
                break;

            case 3: // 新增角度-ANGLE_STEP
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
			
			// 数据采集后添加调试信息
            UsartPrintf(USART_DEBUG, "[DEBUG] light=%.0f -> angle=%d\n", 
                        light_value, LightToFixedAngle(light_value));
			
           angle = (work_mode == AUTO_MODE) ? LightToFixedAngle(light_value) : manual_angle;
            
            /* 显示优化 */
            // 模式显示
            char mode_str[16];
            sprintf(mode_str, "Mode:%s", (work_mode == AUTO_MODE) ? "Auto" : "Manu");
            OLED_ShowStr(0, LINE_MODE, (uint8_t *)mode_str, 2);

            // 角度显示（固定位置）
            char angle_str[16];
            sprintf(angle_str, "Angle:%3d", angle); 
            OLED_ShowStr(0, LINE_ANGLE, (uint8_t *)angle_str, 2);

            // 光照强度显示（左对齐）
            char light_info[16];
            sprintf(light_info, "Lux:%-5.0f", light_value);
            OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)light_info, 2);

            // 发送状态（临时显示，小字体）
            if(timeCount % 10 == 0) {
                char send_status[16];
                sprintf(send_status, "%-8s", (timeCount % 10 == 0) ? "Sending" : ""); // 左对齐8字符
                OLED_ShowStr(64, LINE_SEND, (uint8_t *)send_status, 1);
            }
	
            pwm = AngleToPWM(angle);
            index_bh1750 = 0;
        }
        
        if(index_duoji)
        {
            TIM_SetCompare1(TIM2, pwm); //PA0用
            TIM_SetCompare2(TIM2, pwm);	//PA1用
            index_duoji = 0;
        }
        
		if(++timeCount >= 100)    // 5秒发送间隔（100*50ms）
		{
			// 发送数据到平台
			UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
			OneNet_SendData();
			
			timeCount = 0;
			ESP8266_Clear();
		}

		// 接收数据处理
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
		
        // 使用我们前面定义的tick变量，移除static关键字
        if(++tick % 50 == 0) index_bh1750 = 1;
        if(tick % 10 == 0)   index_duoji = 1;
        delayms(10);
    }
}
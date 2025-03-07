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

//项目头文件
#include "main.h"              // 添加项目主头文件，包含MenuState和TimeSchedule定义

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
// 新增全局变量
MenuState current_menu = MENU_NONE;  // 当前菜单状态
TimeSchedule time_schedule = {9, 0, 18, 0, 90, 0};  // 默认9:00-18:00，90°
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
    delayms(200); // 等待传感器稳定
    float init_light = bh_data_read() / 1.2;
    if(init_light > 0) {
        UsartPrintf(USART_DEBUG, "[INIT] BH1750 OK, Initial light=%.0f lux\r\n", init_light);
    } else {
        UsartPrintf(USART_DEBUG, "[INIT] WARNING: BH1750 may not working\r\n");
    }
    
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
*   函数名称：   DisplayMenu
*
*   函数功能：   显示当前菜单状态和设置值
*
*   入口参数：   menu - 当前菜单状态
*               value - 当前设置的值
*
*   返回参数：   无
*
*   说明：       根据不同的菜单状态显示对应的设置界面
************************************************************
*/
void DisplayMenu(MenuState menu, uint8_t value)
{
    char menu_title[32];
    char menu_value[32];
    char menu_hint[32];
    
    // 清空显示区域（保留时间显示）
    OLED_CLS();  // 使用OLED_CLS替代OLED_Fill(0)
    
    // 在第一行重新显示时间
    if(ntp_time.is_valid) {
        sprintf(time_str, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
        OLED_ShowStr(0, 0, (uint8_t *)time_str, 1);
    }
    
    // 根据不同菜单状态显示对应标题和值
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
        return;  // 非菜单状态不处理
    }
    
    // 共用的操作提示
    sprintf(menu_hint, "K1:OK K2:+ K3:-");
    
    // 显示菜单标题（第2行，大字体）
    OLED_ShowStr(0, 2, (uint8_t *)menu_title, 2);
    
    // 显示当前值（第4行，大字体）
    OLED_ShowStr(0, 4, (uint8_t *)menu_value, 2);
    
    // 显示操作提示（第6行，小字体）
    OLED_ShowStr(0, 6, (uint8_t *)menu_hint, 1);
}

/*
************************************************************
*   函数名称：   DisplayTimeSchedule
*
*   函数功能：   显示当前时间计划设置
*
*   入口参数：   无
*
*   返回参数：   无
*
*   说明：       在OLED上显示当前的时间计划设置
************************************************************
*/
void DisplayTimeSchedule(void)
{
    char schedule_info[32];
    
    // 在OLED第7行显示时间计划信息
    sprintf(schedule_info, "%c%02d:%02d-%02d:%02d %d",
            time_schedule.is_active ? '+' : '-',  // 激活状态前缀
            time_schedule.start_hour, time_schedule.start_minute,
            time_schedule.end_hour, time_schedule.end_minute,
            time_schedule.preset_angle);
    
    OLED_ShowStr(0, LINE_SEND, (uint8_t *)schedule_info, 1);
}

/*
************************************************************
*   函数名称：   IsInTimeSchedule
*
*   函数功能：   判断当前时间是否在设定的时间段内
*
*   入口参数：   无
*
*   返回参数：   uint8_t - 1表示在时间段内，0表示不在
*
*   说明：       比较当前时间和设定的时间段，考虑跨天情况
************************************************************
*/
uint8_t IsInTimeSchedule(void)
{
    // 如果时间计划未激活或NTP时间无效，不在时间段内
    if(!time_schedule.is_active || !ntp_time.is_valid)
        return 0;
    
    // 将时间转换为分钟计数，便于比较
    uint16_t current_mins = ntp_time.hour * 60 + ntp_time.minute;
    uint16_t start_mins = time_schedule.start_hour * 60 + time_schedule.start_minute;
    uint16_t end_mins = time_schedule.end_hour * 60 + time_schedule.end_minute;
    
    // 处理跨天的情况（如22:00-08:00这样的时间段）
    if(start_mins > end_mins) {
        // 跨天时间段：当前时间要么在start之后，要么在end之前
        return (current_mins >= start_mins || current_mins <= end_mins) ? 1 : 0;
    } else {
        // 普通时间段：当前时间在start和end之间
        return (current_mins >= start_mins && current_mins <= end_mins) ? 1 : 0;
    }
}

/*
************************************************************
*   函数名称：   ProcessMenuOperation
*
*   函数功能：   处理菜单状态下的按键操作
*
*   入口参数：   key_id - 按键编号
*               press_type - 按键类型(短按/长按)
*
*   返回参数：   无
*
*   说明：       根据当前菜单状态和按键操作进行相应处理
************************************************************
*/
void ProcessMenuOperation(uint8_t key_id, uint8_t press_type)
{
    static uint8_t temp_value = 0; // 临时存储正在设置的值
    static uint8_t init_done = 0;  // 标记是否已初始化
    
    // 只处理短按
    if(press_type != KEY_SHORT_PRESS) return;
    
        // 菜单初始化值设定
        if(key_id == 1 && !init_done) {
            // 第一次进入菜单，根据当前菜单状态初始化值
            init_done = 1; // 标记已完成初始化
            
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
    
    // 根据当前菜单状态和按键ID进行处理
    switch(current_menu)
    {
    case MENU_TIME_START_HOUR:
        switch(key_id)
        {
        case 1: // 确认，进入下一菜单
            time_schedule.start_hour = temp_value;
            current_menu = MENU_TIME_START_MIN;
            temp_value = time_schedule.start_minute;
            break;
        case 2: // 增加小时
            temp_value = (temp_value >= 23) ? 0 : temp_value + 1;
            break;
        case 3: // 减少小时
            temp_value = (temp_value <= 0) ? 23 : temp_value - 1;
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    case MENU_TIME_START_MIN:
        switch(key_id)
        {
        case 1: // 确认，进入下一菜单
            time_schedule.start_minute = temp_value;
            current_menu = MENU_TIME_END_HOUR;
            temp_value = time_schedule.end_hour;
            break;
        case 2: // 增加分钟
            temp_value = (temp_value >= 59) ? 0 : temp_value + 1;
            break;
        case 3: // 减少分钟
            temp_value = (temp_value <= 0) ? 59 : temp_value - 1;
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    case MENU_TIME_END_HOUR:
        switch(key_id)
        {
        case 1: // 确认，进入下一菜单
            time_schedule.end_hour = temp_value;
            current_menu = MENU_TIME_END_MIN;
            temp_value = time_schedule.end_minute;
            break;
        case 2: // 增加小时
            temp_value = (temp_value >= 23) ? 0 : temp_value + 1;
            break;
        case 3: // 减少小时
            temp_value = (temp_value <= 0) ? 23 : temp_value - 1;
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    case MENU_TIME_END_MIN:
        switch(key_id)
        {
        case 1: // 确认，进入下一菜单
            time_schedule.end_minute = temp_value;
            current_menu = MENU_ANGLE_PRESET;
            temp_value = time_schedule.preset_angle;
            break;
        case 2: // 增加分钟
            temp_value = (temp_value >= 59) ? 0 : temp_value + 1;
            break;
        case 3: // 减少分钟
            temp_value = (temp_value <= 0) ? 59 : temp_value - 1;
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    case MENU_ANGLE_PRESET:
        switch(key_id)
        {
        case 1: // 确认并退出菜单
            time_schedule.preset_angle = temp_value;
            time_schedule.is_active = 1; // 激活时间计划
            
            // 退出菜单状态
            current_menu = MENU_NONE;
            temp_value = 0;
            init_done = 0;
            
            // 清除菜单显示，恢复正常界面
            OLED_CLS();
            // 更新时间计划显示
            DisplayTimeSchedule();
            
            // 添加初始界面显示
            char mode_str[16];
            sprintf(mode_str, "Mode:%s", "Auto");  // 默认自动模式
            OLED_ShowStr(0, LINE_MODE, (uint8_t *)mode_str, 2);

            char angle_str[16];
            sprintf(angle_str, "Angle:%3d", angle); 
            OLED_ShowStr(0, LINE_ANGLE, (uint8_t *)angle_str, 2);

            sprintf(light_info, "Lux:%-5.0f", light_value);
            OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)light_info, 2);

            // 记录调试日志
            UsartPrintf(USART_DEBUG, "[SCHEDULE] Set %02d:%02d-%02d:%02d Angle:%d\r\n",
                      time_schedule.start_hour, time_schedule.start_minute,
                      time_schedule.end_hour, time_schedule.end_minute,
                      time_schedule.preset_angle);
            break;
            
        case 2: // 增加角度
            temp_value = (temp_value >= 180) ? 0 : (temp_value + 10);
            break;
            
        case 3: // 减少角度
            temp_value = (temp_value <= 0) ? 180 : (temp_value - 10);
            break;
        }
        DisplayMenu(current_menu, temp_value);
        break;
        
    default:
        // 未知菜单状态，恢复正常显示
        current_menu = MENU_NONE;
        temp_value = 0;
        OLED_CLS();
        break;
    }
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
    
    // 添加调试变量用于监控传感器状态
    static uint16_t bh1750_check_counter = 0;
    
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
    
    // 显示初始时间
    sprintf(time_str, "%04d-%02d-%02d", ntp_time.year, ntp_time.month, ntp_time.day);
    OLED_ShowStr(0,0,(uint8_t *)time_str,1);
    sprintf(time_str, "%02d:%02d:%02d", ntp_time.hour, ntp_time.minute, ntp_time.second);
    OLED_ShowStr(64,0,(uint8_t *)time_str,1);

    // 立即读取一次光照并更新界面
    bh_data_send(BHReset);
    bh_data_send(BHModeH1);     
    delayms(120);
    light_value = bh_data_read() / 1.2;
    UsartPrintf(USART_DEBUG, "[INIT] Initial light reading: %.1f lux\r\n", light_value);

    // 显示初始界面
    char mode_str[16];
    sprintf(mode_str, "Mode:%s", "Auto");  // 默认自动模式
    OLED_ShowStr(0, LINE_MODE, (uint8_t *)mode_str, 2);

    char angle_str[16];
    angle = LightToFixedAngle(light_value);
    sprintf(angle_str, "Angle:%3d", angle); 
    OLED_ShowStr(0, LINE_ANGLE, (uint8_t *)angle_str, 2);

    sprintf(light_info, "Lux:%-5.0f", light_value);
    OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)light_info, 2);

    // 显示初始时间计划
    DisplayTimeSchedule();

    // 立即设置PWM
    pwm = AngleToPWM(angle);
    TIM_SetCompare1(TIM2, pwm);
    TIM_SetCompare2(TIM2, pwm);

    // 强制触发第一次完整读取
    index_bh1750 = 1;

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
        
        // 进行按键扫描
        key_Functoin();  

        // 处理按键事件
        if(key_num != 0)
        {
            uint8_t key_id = key_num & KEY_NUM_MASK;       // 获取按键编号
            uint8_t press_type = key_num & KEY_TYPE_MASK;  // 获取按键类型
            
            UsartPrintf(USART_DEBUG, "[MAIN] Processing key %d, type %s\r\n", 
                       key_id, (press_type == KEY_LONG_PRESS) ? "LONG" : "SHORT");
            
        // 根据是否处于菜单状态选择处理逻辑
        if(current_menu != MENU_NONE) {
            // 菜单状态下的按键处理
            ProcessMenuOperation(key_id, press_type);
        } else {
            // 非菜单状态下的按键处理               
            if(press_type == KEY_SHORT_PRESS) {
                // 短按处理
                switch(key_id)
                {
                    case 1: // 模式切换
                    // 三种模式循环切换
                    if(work_mode == AUTO_MODE) {
                        work_mode = MANUAL_MODE;
                        manual_angle = angle; // 使用当前角度作为初始值
                    } else if(work_mode == MANUAL_MODE) {
                        work_mode = TIMELOCK_MODE;
                        time_schedule.is_active = 1; // 激活时间表
                    } else {
                        work_mode = AUTO_MODE;
                    }
                    
                    // 添加更详细的调试输出
                    UsartPrintf(USART_DEBUG, "[MODE] Changed to %s, current angle=%d\r\n", 
                              (work_mode == AUTO_MODE) ? "AUTO" : 
                              (work_mode == MANUAL_MODE) ? "MANUAL" : "TIMELOCK", angle);
                    break;
                    
                case 2: // 角度+ANGLE_STEP
                    if(work_mode == MANUAL_MODE) 
                    {
                        manual_angle = (manual_angle >= 180) ? 0 : manual_angle + ANGLE_STEP;
                        pwm = AngleToPWM(manual_angle); // 立即更新PWM值
                        
                        // 立即应用舵机角度变化
                        TIM_SetCompare1(TIM2, pwm);
                        TIM_SetCompare2(TIM2, pwm);
                        
                        UsartPrintf(USART_DEBUG, "[ANGLE] Increased to %d, PWM=%d\r\n", manual_angle, pwm);
                    }
                    break;
                
                case 3: // 角度-ANGLE_STEP
                    if(work_mode == MANUAL_MODE)
                    {
                        manual_angle = (manual_angle <= 0) ? 180 : manual_angle - ANGLE_STEP;
                        pwm = AngleToPWM(manual_angle); // 立即更新PWM值
                        
                        // 立即应用舵机角度变化
                        TIM_SetCompare1(TIM2, pwm);
                        TIM_SetCompare2(TIM2, pwm);
                        
                        UsartPrintf(USART_DEBUG, "[ANGLE] Decreased to %d, PWM=%d\r\n", manual_angle, pwm);
                    }
                    break;
                }
            }
            else if(press_type == KEY_LONG_PRESS) {
                // 长按处理逻辑
                UsartPrintf(USART_DEBUG, "[LONG] Long press on key %d detected\r\n", key_id);
                
                switch(key_id)
                {
                case 1: // 按键1长按 - 预留功能
                    break;
                    
                case 2: // 按键2长按 - 进入时段设置
                    current_menu = MENU_TIME_START_HOUR;
                    DisplayMenu(current_menu, 0); // 显示初始菜单
                    UsartPrintf(USART_DEBUG, "[MENU] Enter time schedule setting\r\n");
                    break;
                    
                case 3: // 按键3长按 - 进入角度预设
                    current_menu = MENU_ANGLE_PRESET;
                    DisplayMenu(current_menu, time_schedule.preset_angle); // 显示初始菜单
                    UsartPrintf(USART_DEBUG, "[MENU] Enter angle preset\r\n");
                    break;
                }
            }
        }
        
        key_num = 0;  // 将按键清除移到这里，确保所有情况下都清除按键标志

        // 在主循环的index_bh1750处理部分，添加菜单状态检查
        if(index_bh1750)
        {
            UsartPrintf(USART_DEBUG, "[BH1750] Starting read...\r\n");
    
            // 更可靠的读取方法
            bh_data_send(BHReset);   // 重置传感器
            delayms(10);
            bh_data_send(BHModeH1);  // 配置为高精度模式
            delayms(180);           // 传感器需要时间进行测量
            
            float new_light = bh_data_read() / 1.2;
            
            // 检查读取值是否合理
            if(new_light > 0 && new_light < 10000) {
                light_value = new_light;
                UsartPrintf(USART_DEBUG, "[LIGHT] Valid reading: %.1f lux\r\n", light_value);
                
                // 立即更新显示
                if(current_menu == MENU_NONE) {
                    sprintf(light_info, "Lux:%-5.0f", light_value);
                    OLED_ShowStr(0, LINE_LIGHT, (uint8_t *)light_info, 2);
                }
                
                // 处理AUTO和TIMELOCK模式
                if(work_mode == AUTO_MODE || 
                  (work_mode == TIMELOCK_MODE && !IsInTimeSchedule())) {
                    uint8_t new_angle = LightToFixedAngle(light_value);
                    if(angle != new_angle) {
                        angle = new_angle;
                        pwm = AngleToPWM(angle);
                        TIM_SetCompare1(TIM2, pwm);
                        TIM_SetCompare2(TIM2, pwm);
                        
                        // 更新角度显示
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
            
            index_bh1750 = 0;  // 无论成功失败，都复位标志
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
		
        // 简化定时器触发逻辑
        // 每50ms递增一次计数器
        tick++;

        // 直接触发光照读取，不使用条件判断
        if(tick % 20 == 0 && !index_bh1750) { // 每200ms
            index_bh1750 = 1;
            UsartPrintf(USART_DEBUG, "[TRIGGER] BH1750 read triggered\r\n");
        }

        // 每500ms更新一次调试信息
        if(tick % 50 == 0) {
            UsartPrintf(USART_DEBUG, "[STATUS] Tick=%lu light=%.1f mode=%d\r\n", 
                    tick, light_value, work_mode);
        }

        delayms(10); // 10ms延时

        }
    }
}
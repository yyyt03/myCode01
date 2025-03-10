#include "hw_control.h"
#include "usart.h"
#include "delay.h"
#include "oled.h"
#include "bh1750.h"
#include "timer.h"
#include "key.h"
#include "display_manager.h"

// 硬件初始化
void Hardware_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_Init();
    Usart1_Init(115200);
    Usart2_Init(115200);
    
    Display_Init();
    
    Init_BH1750();
    bh_data_send(BHPowOn);
    key_Init();
    
    // 初始化舵机PWM
    tiemr2_Ch1_Pwm_Init(720-1, 2000, 100); 
    delayms(1000);
    
    UsartPrintf(USART_DEBUG, "Hardware init OK\r\n");
}

// 角度转换为PWM值
uint16_t AngleToPWM(uint8_t angle) {
    return 50 + angle * 190 / 180;
}

// 更新舵机角度
void HW_UpdateServos(uint8_t angle) {
    uint16_t pwm = AngleToPWM(angle);
    TIM_SetCompare1(TIM2, pwm);
    TIM_SetCompare2(TIM2, pwm);
}

// 读取光照传感器
float HW_ReadLightSensor(void) {
    bh_data_send(BHReset);
    bh_data_send(BHModeH1);     
    delayms(120);
    return bh_data_read() / 1.2;
}
#ifndef __DISPLAY_MANAGER_H
#define __DISPLAY_MANAGER_H

#include "stm32f10x.h"

void Display_Init(void);
void Display_UpdateClock(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t is_valid);
void Display_UpdateMode(uint8_t mode);
void Display_UpdateAngle(uint8_t angle);
void Display_UpdateLightValue(float light_value);
void Display_UpdateTimerSettings(uint8_t start_hour, uint8_t start_minute, uint8_t end_hour, uint8_t end_minute, uint8_t angle, uint8_t setting_index, uint8_t prev_index);
void Display_ShowConnectionStatus(const char* status);
void Display_ShowError(const char* error_message);

#endif
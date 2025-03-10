#ifndef __HW_CONTROL_H
#define __HW_CONTROL_H

#include "stm32f10x.h"

void Hardware_Init(void);
uint16_t AngleToPWM(uint8_t angle);
void HW_UpdateServos(uint8_t angle);
float HW_ReadLightSensor(void);

#endif
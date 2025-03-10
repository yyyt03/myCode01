#ifndef __NETWORK_MANAGER_H
#define __NETWORK_MANAGER_H

#include "stm32f10x.h"

// 网络管理模块初始化
// 返回0表示成功，非0表示失败
uint8_t NetworkManager_Initialize(void);

// 发送数据到云平台
void NetworkManager_SendData(void);

// 处理接收到的数据
void NetworkManager_ProcessIncoming(void);

#endif
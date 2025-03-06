#ifndef _ONENET_H_
#define _ONENET_H_
#include "sys.h"  // 确保能访问SystemMode类型


_Bool OneNET_RegisterDevice(void);

_Bool OneNet_DevLink(void);

void OneNet_SendData(void);

void OneNET_Subscribe(void);

void Send_Property_Response(int code, const char *msg, int mode_value);

void OneNet_RevPro(unsigned char *cmd);
	
#endif

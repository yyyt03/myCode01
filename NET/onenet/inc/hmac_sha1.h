#ifndef _HAMC_SHA1_H_
#define _HAMC_SHA1_H_

//这个值由原来的4096改为1024，同时，startup_stm32f103_ms.s中栈的值也要改！！！！！！
//Stack_Size      EQU     0x00000800
#define MAX_MESSAGE_LENGTH		1024     	



void hmac_sha1(
	unsigned char *key,
	int key_length,
	unsigned char *data,
	int data_length,
	unsigned char *digest
);


#endif

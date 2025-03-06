#ifndef _HAMC_SHA1_H_
#define _HAMC_SHA1_H_

//���ֵ��ԭ����4096��Ϊ1024��ͬʱ��startup_stm32f103_ms.s��ջ��ֵҲҪ�ģ�����������
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

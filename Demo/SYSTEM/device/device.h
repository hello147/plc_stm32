#ifndef __DEVICE_H
#define __DEVICE_H 			   
#include "stm32F10x.h"




typedef struct
{
	u8 status;  //����״̬   1:��ȡplc  2������  3��
	u8 netstatus;//����״̬
	u8 needreset;//��Ҫ����
	u8 updatime;//��Ҫ����ʱ��
	u8 mod_data;  //mod��������
	u8 msg_recv;//sim��������
	u8 period;//��ȡ���
	u8 machineid[8]; //������
	u8 ipaddress[15];//ip��ַ
	u8 port[4]; //ip�˿�
	u8 ack1[10];
	u8 ack2[10];
	u8 msg_rec;
	char sim_data[10];
	char simid[11];
} Device_Info;



void Device_Timer_Status(char *buf);

#endif


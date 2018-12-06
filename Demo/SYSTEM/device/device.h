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
	u8 period[3];//��ȡ���

	u8 machineid[10];//������
	
	u8 ipaddress[16];//ip��ַ
	u8 simid[12];
	u8 port[5];      //ip�˿�
	u8 ack1[10];
	u8 ack2[10];
	u8 msg_rec;
	u8 sim_data[10];
} Device_Info;



void Device_Timer_Status(char *buf);

#endif


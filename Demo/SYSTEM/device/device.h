#ifndef __DEVICE_H
#define __DEVICE_H 			   
#include "stm32F10x.h"




typedef struct
{
	u8 status;  //整机状态   1:读取plc  2：发送  3：
	u8 netstatus;//网络状态
	u8 needreset;//需要重启
	u8 updatime;//需要更新时间
	u8 mod_data;  //mod接收数据
	u8 msg_recv;//sim接收数据
	u8 period;//读取间隔
	u8 machineid[8]; //机器码
	u8 ipaddress[15];//ip地址
	u8 port[4]; //ip端口
	u8 ack1[10];
	u8 ack2[10];
	u8 msg_rec;
	char sim_data[10];
	char simid[11];
} Device_Info;



void Device_Timer_Status(char *buf);

#endif


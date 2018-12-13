#ifndef __DEVICE_H
#define __DEVICE_H 			   
#include "stm32F10x.h"




typedef struct Device_Info
{
	u8 status;  //整机状态   1:读取plc  2：发送  3：
	u8 netstatus;//网络状态
	u8 needreset;//需要重启
	u8 updatime;//需要更新时间
	u8 mod_data;  //mod接收数据
	u8 msg_recv;//sim接收数据
	u8 period[2];//存放从rom中取出来的间隔

	u8 machineid[10];//机器码
	u8 SimNumber[12];
	u8 ipaddress[16];//ip地址
	u8 simid[12];
	
	u8 port[6];      //ip端口
	u8 ack1[10];
	u8 ack2[10];
	u8 msg_rec;
	u8 sim_data[10];
	u8 rstime[3];
	int rstart_time;   //转化后的重启时间
	int period_time;  //转化后的时间间隔
	u8 message[100];
} Device_Info;



void Device_Timer_Status(char *buf);

#endif


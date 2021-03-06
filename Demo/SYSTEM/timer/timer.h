#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
	

//用来设置心跳包的发送间隔（单位是分钟）
//常规的心跳包间隔是1分钟到10分钟
#define TIME_HEART 1
#define NUMBER_TIME_HEART_50_MS (TIME_HEART*60*1000/50)  	  //1MIN
#define HB_1_MIN (TIME_HEART*60)  	  //1MIN

#define NUMBER_TIMER_1_MINUTE       60
#define NUMBER_MSG_MAX_RETRY        5

#define OFFSET_Connec 30                      	//TRVBP00,000,UBICOM_AUTH_INFOR,0001,070,X,# 
#define OFFSET_TDS    35                      	//TRVBP00,000,UBICOM_AUTH_INFOR,0001,070,X,# 


void TIM5_Int_Init(u16 arr,u16 psc);
void TIM6_Int_Init(u16 arr,u16 psc);
void TIM7_Int_Init(u16 arr,u16 psc);

#endif

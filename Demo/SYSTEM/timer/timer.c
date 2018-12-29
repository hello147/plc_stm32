#include "timer.h"
#include "usart2.h"
#include "usart3.h"
//#include "SIM800.h"
#include "string.h"  
#include "stdlib.h"  
#include "device.h"
#include "readconfig.h"
#include "stm32f10x_tim.h"

extern Device_Info device;  //代表本机
	int readcount=0;
	int timecount=0;
	int seccount=0;//秒数
	int mincount=0;//分钟数
	

//定时器6中断服务程序		    
void TIM6_IRQHandler(void)
{
	u8 index;

	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)					  //是更新中断
	{	
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  					//清除TIM6更新中断标志
	   readcount++;
		timecount++;
		if(readcount==device.period_time)
			{
				device.status=1;//每隔10s读取一次
			}else
 			if(readcount>device.period_time)
				{
					readcount=0;
				}
	}
}

//定时器7中断服务程序		    
void TIM7_IRQHandler(void)
{ 	
	u16 length = 0; 
	u8 result = 0;
	u8 result_temp = 0;
	char *uart_data_left;
	char *p, *p1;
	char *p_temp = NULL;
	

	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//是更新中断
	{	 		
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);  //清除TIM7更新中断标志    
		USART3_RX_STA|=1<<15;	//标记接收完成
		TIM_Cmd(TIM7, DISABLE);  //关闭TIM7
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;	//添加结束符 
		BSP_Printf("USART BUF:%s\r\n",USART3_RX_BUF); 
		if((strstr((const char*)USART3_RX_BUF,"CLOSED")!=NULL) || (strstr((const char*)USART3_RX_BUF,"PDP: DEACT")!=NULL) || (strstr((const char*)USART3_RX_BUF,"CONNECT FAIL")!=NULL))
		{
			device.needreset=1;
		}else{
		  if((strstr(device.ack1, ">")!=NULL) && (strstr((const char*)USART3_RX_BUF,"ERROR")!=NULL))
					{
						device.needreset=1; 
					}
//				if((device.ack1!=0)&&(device.ack2!=0))
//				{
//					if((strstr((const char*)USART3_RX_BUF, device.ack1) != NULL)&&(strstr((const char*)USART3_RX_BUF, device.ack2) != NULL))
//							{
//								device.msg_rec=1;
//							}
//				}
				
			if(strstr((const char*)USART3_RX_BUF, device.ack1) != NULL)
			{	
				memset(device.sim_data,0,sizeof(device.sim_data));
				strcpy(device.sim_data,(const char *)USART3_RX_BUF);
				device.msg_rec=1;
			} 
			 if(strstr((const char*)USART3_RX_BUF, "+CMT") != NULL)
			{
				   
				    memset(device.message,0,sizeof(device.message));
           	strcpy(device.message,(const char *)USART3_RX_BUF);
				    device.msg_rec=1;
				   device.status=3;
      }			
			
	    Clear_Usart3();		    	//清零
		
//				if(device.ack2!=0)
//				{
//					if(strstr((const char*)USART3_RX_BUF, device.ack1) != NULL)
//						device.msg_rec=1;
//				}
		}
	}
}


void TIM5_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)					  //是更新中断
	{	
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);  
		if(seccount<120)
		{
		 seccount++;
		}
		else if(seccount>=device.rstart_time)
		{
		  mincount++;
			seccount=0;
			//device.status=4;//设置为连接服务器
		  device.needreset=1;
			BSP_Printf("2分钟过去了");
			TIM_SetCounter(TIM5,0); 
			TIM_Cmd(TIM5,DISABLE);
		}
//		if(mincount>5)
//		{
//		  mincount=0;
//			
//		}
		
		
	}


}
//通用定时器6中断初始化
//arr：自动重装值。
//psc：时钟预分频数	
void TIM6_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);				//TIM6时钟使能    
	
	//定时器TIM6初始化
	TIM_TimeBaseStructure.TIM_Period = arr;                     //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x00;     //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);             //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE );                   //使能指定的TIM6中断,允许更新中断
	
	//TIM_Cmd(TIM6,ENABLE);//开启定时器6
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}

//TIMER7的初始化 用在USART3（对接SIM800）的中断接收程序/////////
//通用定时器7中断初始化
//这里选择为APB1的1倍，而APB1为24M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz 
//arr：自动重装值。
//psc：时钟预分频数		 
void TIM7_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7时钟使能    
	
	//定时器TIM7初始化
	TIM_TimeBaseStructure.TIM_Period = arr;                     //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);             //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE );                   //使能指定的TIM7中断,允许更新中断
	
	TIM_Cmd(TIM7,ENABLE);//开启定时器7
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =2;	//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		    	//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      	//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}

//TIM5初始化
void TIM5_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);				//TIM5时钟使能    
	
	//定时器TIM6初始化
	TIM_TimeBaseStructure.TIM_Period = arr;                     //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x00;     //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);             //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE );                   //使能指定的TIM5中断,允许更新中断
	
	//TIM_Cmd(TIM5,ENABLE);//开启定时器5
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}
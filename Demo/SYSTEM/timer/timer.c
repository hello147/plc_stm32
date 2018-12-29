#include "timer.h"
#include "usart2.h"
#include "usart3.h"
//#include "SIM800.h"
#include "string.h"  
#include "stdlib.h"  
#include "device.h"
#include "readconfig.h"
#include "stm32f10x_tim.h"

extern Device_Info device;  //������
	int readcount=0;
	int timecount=0;
	int seccount=0;//����
	int mincount=0;//������
	

//��ʱ��6�жϷ������		    
void TIM6_IRQHandler(void)
{
	u8 index;

	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)					  //�Ǹ����ж�
	{	
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  					//���TIM6�����жϱ�־
	   readcount++;
		timecount++;
		if(readcount==device.period_time)
			{
				device.status=1;//ÿ��10s��ȡһ��
			}else
 			if(readcount>device.period_time)
				{
					readcount=0;
				}
	}
}

//��ʱ��7�жϷ������		    
void TIM7_IRQHandler(void)
{ 	
	u16 length = 0; 
	u8 result = 0;
	u8 result_temp = 0;
	char *uart_data_left;
	char *p, *p1;
	char *p_temp = NULL;
	

	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{	 		
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);  //���TIM7�����жϱ�־    
		USART3_RX_STA|=1<<15;	//��ǽ������
		TIM_Cmd(TIM7, DISABLE);  //�ر�TIM7
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;	//��ӽ����� 
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
			
	    Clear_Usart3();		    	//����
		
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

	if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)					  //�Ǹ����ж�
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
			//device.status=4;//����Ϊ���ӷ�����
		  device.needreset=1;
			BSP_Printf("2���ӹ�ȥ��");
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
//ͨ�ö�ʱ��6�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��	
void TIM6_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);				//TIM6ʱ��ʹ��    
	
	//��ʱ��TIM6��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr;                     //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x00;     //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);             //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE );                   //ʹ��ָ����TIM6�ж�,��������ж�
	
	//TIM_Cmd(TIM6,ENABLE);//������ʱ��6
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}

//TIMER7�ĳ�ʼ�� ����USART3���Խ�SIM800�����жϽ��ճ���/////////
//ͨ�ö�ʱ��7�жϳ�ʼ��
//����ѡ��ΪAPB1��1������APB1Ϊ24M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz 
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��		 
void TIM7_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7ʱ��ʹ��    
	
	//��ʱ��TIM7��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr;                     //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);             //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE );                   //ʹ��ָ����TIM7�ж�,��������ж�
	
	TIM_Cmd(TIM7,ENABLE);//������ʱ��7
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =2;	//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		    	//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      	//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}

//TIM5��ʼ��
void TIM5_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);				//TIM5ʱ��ʹ��    
	
	//��ʱ��TIM6��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr;                     //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                   //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x00;     //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);             //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE );                   //ʹ��ָ����TIM5�ж�,��������ж�
	
	//TIM_Cmd(TIM5,ENABLE);//������ʱ��5
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}
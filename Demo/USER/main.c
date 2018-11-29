#include "stm32f10x.h"
#include "timer.h"
#include "SIM800.h"
#include "usart2.h"
#include "delay.h"
#include "mod.h"
#include "device.h"
#include "usart3.h"
#include "24cxx.h" 
Device_Info device;  //������

 int main(void)
 {	
	 delay_init();
	 delay_ms(5000);
	 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  	usart2_init(115200);  //��������   
	  Mod_USARTx_Config(); //�Խ�plc
  	TIM6_Int_Init(9999,7199);	 // 1s�ж�
	 TIM5_Int_Init(9999,7199);		
	 TIM_SetCounter(TIM6,0); 
	 TIM_Cmd(TIM6,ENABLE);
	 Init_RTC();  //��ʼ��ʱ��
	 
	 usart3_init(115200); //�Խ�sim800  
	 Clear_Usart3();      //������3
	 device.msg_rec=0;
	 device.status=0;
	 device.updatime=0;
	 SIM800_ntpserver();
	 SIM800_Link_Server_AT();//���ӷ�����
	 while(1)
	 {   //����������൱�ڰ���������
	    if(device.needreset==1)
			{
				__set_FAULTMASK(1);//�ر����ж�
       NVIC_SystemReset();//����Ƭ������
			}
			//����ʱ��
	    if(device.updatime==1)
			{
				 SIM800_ntpserver();
			}
			switch(device.status)
			{
				case 1:
					 read_hold_register_03(1,0,40);
           delay_ms(200);	
						if(modbus_rev_deal()==1)
							{
								device.status=2;
							}
						else
							{
								device.status=1;
							}
					break;
				case 2:
					if(device.netstatus==ERR_DISCONNECT)
					{
				  	//TIM_Cmd(TIM5,ENABLE);
						BSP_Printf("����Ӧ��rom\r\n"); 
						device.status=0;
					}
					else{
					SendPost_Server(); 		
					 BSP_Printf("�ϴ���\r\n"); 
					clear();
				   device.status=0;
					}
					
					break;
				case 3:
					break;
				case 4:
					break;
			
			}
	 }	 
 }


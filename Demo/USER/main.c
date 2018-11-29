#include "stm32f10x.h"
#include "timer.h"
#include "SIM800.h"
#include "usart2.h"
#include "delay.h"
#include "mod.h"
#include "device.h"
#include "usart3.h"
#include "24cxx.h" 
Device_Info device;  //代表本机

 int main(void)
 {	
	 delay_init();
	 delay_ms(5000);
	 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  	usart2_init(115200);  //用来调试   
	  Mod_USARTx_Config(); //对接plc
  	TIM6_Int_Init(9999,7199);	 // 1s中断
	 TIM5_Int_Init(9999,7199);		
	 TIM_SetCounter(TIM6,0); 
	 TIM_Cmd(TIM6,ENABLE);
	 Init_RTC();  //初始化时钟
	 
	 usart3_init(115200); //对接sim800  
	 Clear_Usart3();      //清理串口3
	 device.msg_rec=0;
	 device.status=0;
	 device.updatime=0;
	 SIM800_ntpserver();
	 SIM800_Link_Server_AT();//连接服务器
	 while(1)
	 {   //软件重启，相当于按下重启键
	    if(device.needreset==1)
			{
				__set_FAULTMASK(1);//关闭总中断
       NVIC_SystemReset();//请求单片机重启
			}
			//更新时间
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
						BSP_Printf("存入应急rom\r\n"); 
						device.status=0;
					}
					else{
					SendPost_Server(); 		
					 BSP_Printf("上传了\r\n"); 
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


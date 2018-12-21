#include "stm32f10x.h"
#include "timer.h"
#include "SIM800.h"
#include "usart2.h"
#include "delay.h"
#include "mod.h"
#include "device.h"
#include "usart3.h"

Device_Info device;  //代表本机
char coredata[200];
char namenum[20*4]={0};
char str[200]={0};

uint8_t IsRead=0;
char gbkstr[200]={0};
u16 ascstr[50]={0};
char strings[5];
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
	 TIM_SetCounter(TIM5,0); 
	 TIM_Cmd(TIM6,ENABLE);
	 Init_RTC();  //初始化时钟
	 I2C_Config();
	 usart3_init(115200); //对接sim800  
	 Clear_Usart3();      //清理串口3
	 device.msg_rec=0;
	 device.status=0;
	 device.updatime=0;
	 
	 SIM800_ntpserver();
	 SIM800_Link_Server_AT();//连接服务器
	 //delay_ms(2000);
	 r_defalutconfig();
	 readbak();
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
				  	TIM_Cmd(TIM5,ENABLE);
						//截取需要的数据
						packagedata();
						write();
						//BSP_Printf("存入应急rom\r\n"); 
						device.status=0;
					}
					else{
						//测试同步
					 packagedata();//打包实时数据
						SendPost_Server(); 		//打包通用数据并发送
					 //BSP_Printf("上传了\r\n"); 
						printf("上传完成\r\n");
				   clear();
				   device.status=0;
					}
					
					break;
				case 3:
					//读取短信
				  	IsRead=readmessage(str);		
				if(IsRead)
				{
				
					BSP_Printf("短信内容:  %s",str);
					sim800_unnicode2asiicc(str,ascstr);       
					//sim800c_unigbk_exchange(str,gbkstr,0);
					BSP_Printf("转换后内容: %s %s\r\n",str,ascstr);
					sprintf(strings,"%c%c%c",ascstr[12],ascstr[13],ascstr[14]);
					BSP_Printf("收到了%s\r\n",strings);
					IsRead=0;
					device.status=0;
				}
					break;
				
			
			}
	 }	 
 }


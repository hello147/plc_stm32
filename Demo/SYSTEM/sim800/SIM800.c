#include "sim800.h"	 	 	
#include "string.h"  
#include "stdio.h"	
#include "usart2.h"
#include "usart3.h" 
#include "timer.h"
#include "delay.h"
#include <stdlib.h>
#include "device.h"
#include "send.h"
#include "rtc.h"
#include "stm32f10x.h"
#include "integer.h"
#include "ff.h"//firecc936.c文件中：ff_convert()函数实现unicode与gbk互转功能
#include "stdint.h"
#define COUNT_AT 3
static uint8_t MaxMessAdd=50;
extern Device_Info device;  //代表本机
u8 mode = 0;				//0,TCP连接;1,UDP连接
const char *modetbl[2] = {"TCP","UDP"};//连接模式
extern char postdata[200];
extern  struct NEW_Time newtimer;
char  *ipaddr = "111.14.212.26";
char  *port = "5000";
u8 step_configure=0;
int sim800_conf=0;
const char delim=',';
const char ending='#';
extern char coredata[200];
char post[]={
"test"
};
struct  STRUCT_USARTx_sim_Fram strSIM800_Fram_Record = { 0 };


//存储PCB_ID的数组（也就是SIM卡的ICCID）
char ICCID_BUF[LENGTH_ICCID_BUF+1] = {0};
char location_BUF[24] = {0};
char Number_BUF[11];
t_DEV dev={0};
const char *msg_device="000";

//SIM800发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//
//waittime:等待时间(单位:10ms)
//正确回复0，错误回复1
u8 SIM800_Send_Cmd(char * cmd, char* reply1, char* reply2, u32 waittime)
{
	char ret = 1; 
  //strSIM800_Fram_Record.InfBit.FramLength=0;
  
//	if((reply1==0)&&(reply2==0))
//		ret=0;
//		delay_ms ( waittime ); 
//	 USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;	//添加结束符 
	
		if(reply1!=NULL)
		{
		 memset(device.ack1,0,sizeof(device.ack1));
			strcpy(device.ack1,(char*)reply1);
		} 
//		if(reply2!=NULL)
//		{
//			memset(device.ack2,0,sizeof(device.ack1));
//			strcpy(device.ack2,(char*)reply2);
//		}
	
			if((u32)cmd <= 0XFF)
			{
				while((USART3->SR&0X40)==0);//等待上一次数据发送完成  
				USART3->DR = (u32)cmd;
			}
			else 
			{
				u3_printf("%s\r\n",cmd);//发送命令
			}
		
			 if(reply1&&waittime)
			 {
				 while(--waittime)
				 {
				   delay_ms(10);
					 if(device.msg_rec&1)
					 {
					   ret=0;
						 device.msg_rec=0;
						 device.netstatus=0;
						 break;
					 }
				 }
			 }
	//	BSP_Printf("USART BUF:%s\r\n",USART3_RX_BUF);		 
	  return ret;
} 
  
u8 Get_Number(void)
{
	u8 index = 0;
	char *p_temp = NULL;
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	
	while(count != 0)
	{
		ret = SIM800_Send_Cmd(" AT+CNUM","CNUM",0,200);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		
		else if(ret == CMD_ACK_OK)
		{
			if(strstr(device.sim_data, "AT+CNUM")==NULL)
				break;
			else
				Disable_Echo();
		}
		else if(ret == CMD_ACK_DISCONN)
			break;
		 count--;
	}

	if(ret == CMD_ACK_OK)	
	{
		//AT指令已经指令完成，下面对返回值进行处理
		p_temp = device.sim_data;
		memset(ICCID_BUF, 0, sizeof(Number_BUF));
		//提取Number_BUF信息到全局变量Number_BUF
		for(index = 0;index < 11;index++)
		{
			Number_BUF[index] = *(p_temp+OFFSET_Num+index);
			 strcpy(device.simid,Number_BUF);
		}
		BSP_Printf("Number_BUF:%s\r\n",Number_BUF);
	}
	//AT指令的回文已经处理完成，清零
	
	return ret;
}


u8 Check_Module(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT","OK",0,100);
		if(ret == CMD_ACK_NONE) 
		{
			delay_ms(2000);
		}
		
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))  //在正常AT 命令里基本上不可能返回"CLOSED"，仅放在这里
			break;
		
		count--;
	}
	
	//Clear_Usart3();	
	return ret;
	
}

//关闭回显
u8 Disable_Echo(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("ATE0","OK",0,200);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		
		count--;
	}
	
	//Clear_Usart3();	
	delay_ms(2000);	
	return ret;
	
}

//检测注册网络
u8 Check_Network(void)
{
	u8 count = 20;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CREG?","+CREG: 0,1",0,500);
		if(ret == CMD_ACK_NONE) 
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))  //在正常AT 命令里基本上不可能返回"CLOSED" 吧 ，仅放在这里
			break;
		
		count--;
	}
	
	//Clear_Usart3();	
	return ret;
	
}

//查看SIM是否正确检测到
u8 Check_SIM_Card(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;

	delay_ms(10000);	
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CPIN?","OK",0,1000);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		
		count--;
	}

	//Clear_Usart3();
	delay_ms(2000);	
	return ret;
}

u8 Check_OPS(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+COPS?","CHINA",0,500);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		
		count--;
	}
	
	//Clear_Usart3();	
	return ret;
}


//查看天线质量
u8 Check_CSQ(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	u8 *p1 = NULL; 
	u8 *p2 = NULL;
	u8 p[50] = {0}; 
  	u8 signal=0;

	while(signal < 5&&count!=0)
	{
		delay_ms(2000);
	
		
			ret = SIM800_Send_Cmd("AT+CSQ","+CSQ:",0,200);
			if(ret == 3)
			{
				delay_ms(2000);
			}
			else if(ret == 1)
			{
				device.netstatus=ERR_DISCONNECT;
			  break;
			}	
		
		if(ret == 0)
		{
			//AT指令已经指令完成，下面对返回值进行处理
			p1=(u8*)strstr((const char*)(device.sim_data),":");
			p2=(u8*)strstr((const char*)(p1),",");
			p2[0]=0;//加入结束符
			signal = atoi((const char *)(p1+2));
			//判断信号质量，为零时直接改状态
			if(signal==0)
			{
			device.netstatus=ERR_DISCONNECT;
			}
			sprintf((char*)p,"信号质量:%d",signal);
			BSP_Printf("%s\r\n",p);
		}
		//AT指令的回文已经处理完成，清零
		count--;
			
	}	
	return ret;
}

//获取SIM卡的ICCID
//SIM卡的ICCID,全球唯一性，可以用作PCB的身份ID
//打印USART3_RX_BUF的內容 調試用途
		/*****  注意+号前面有两个空格
  +CCID: 1,"898602B8191650216485"

		OK
		****/
//这个函数还没有最终确认....
u8 Get_ICCID(void)
{
	u8 index = 0;
	char *p_temp = NULL;
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CCID","OK",0,200);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if(ret == CMD_ACK_OK)
		{
			if(strstr(device.sim_data, "AT+CCID")==NULL)
				break;
			else
				Disable_Echo();
		}
		else if(ret == CMD_ACK_DISCONN)
			break;
		
		count--;
	}

	if(ret == CMD_ACK_OK)	
	{
	//AT指令已经指令完成，下面对返回值进行处理
		p_temp = device.sim_data;
		memset(ICCID_BUF, 0, sizeof(ICCID_BUF));
		//提取ICCID信息到全局变量ICCID_BUF
		for(index = 0;index < LENGTH_ICCID_BUF;index++)
		{
			ICCID_BUF[index] = *(p_temp+OFFSET_ICCID+index);
		}
		BSP_Printf("ICCID_BUF:%s\r\n",ICCID_BUF);
		
	}
	//AT指令的回文已经处理完成，清零
	//Clear_Usart3();
	return ret;
}

u8 SIM800_GPRS_ON(void)
{
	u8 ret = CMD_ACK_NONE;	
	if((ret = Link_Server_AT(0, ipaddr, port)) == CMD_ACK_OK)
		device.needreset = 0;
	
	//Clear_Usart3();	
	return ret;

}

//关闭GPRS的链接
u8 SIM800_GPRS_OFF(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CIPCLOSE=1","CLOSE OK",0,500);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		
		count--;
	}
	return ret;
}

//附着GPRS
u8 SIM800_GPRS_Adhere(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CGATT=1","OK",0,1000);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		
		count--;
	}
	
	//Clear_Usart3();	
	delay_ms(2000);
	return ret;
}

//设置为GPRS链接模式
u8 SIM800_GPRS_Set(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CIPCSGP=1,\"CM\"","OK",0,600);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		count--;
	}
	
	//Clear_Usart3();	
	delay_ms(2000);	
	return ret;
}

//设置接收数据显示IP头(方便判断数据来源)	
u8 SIM800_GPRS_Dispaly_IP(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CIPHEAD=1","OK",0,300);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		
		count--;
	}
	
	//Clear_Usart3();	
	return ret;
}

//关闭移动场景 
u8 SIM800_GPRS_CIPSHUT(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CIPSHUT","SHUT OK",0,1000);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		
		count--;
	}
	
	//Clear_Usart3();	
	delay_ms(2000);	
	return ret;
}

//设置GPRS移动台类别为B,支持包交换和数据交换 
u8 SIM800_GPRS_CGCLASS(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CGCLASS=\"B\"","OK",0,300);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		  count--;
	}
	
	//Clear_Usart3();	
	delay_ms(2000);	
	return ret;
}


//设置PDP上下文,互联网接协议,接入点等信息
u8 SIM800_GPRS_CGDCONT(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",0,600);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		count--;
	}
	
	//Clear_Usart3();	
	delay_ms(2000);	
	return ret;
}
//配置承载场景(连接ntp)
u8 SAPBR3()
{
  u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",0,200);
  return ret;
}
//设置接入点
u8 SAPBR2()
{
  u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",0,200);
  return ret;
}
//激活gprs上下文
u8 SAPBR1()
{
  u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+SAPBR=1,1","OK",0,200);
  return ret;
}
//建立ntp连接
u8 Link_ntp_srver()
{
u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+CNTP=\"ntp1.aliyun.com\",32","OK",0,200);
  return ret;
}
//同步ntp时间
u8 ntptime()
{
  u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+CNTP","+CNTP: 1",0,200);
  return ret;
}
//获取经纬度
u8 Get_location()
{
 u8 ret = CMD_ACK_NONE;
	char *p_temp = NULL;
	int index=0;
	ret=SIM800_Send_Cmd("AT+CLBS=1,1","+CLBS",0,700);
  	if(ret == CMD_ACK_OK)	
		{
			memcpy(location_BUF,device.sim_data+11,20);
		  	//p_temp = device.sim_data;
//			for(index = 0;index < sizeof(location_BUF);index++)
//			{
//				location_BUF[index] = *(p_temp+9+index);
//			}
			 memcpy(device.lng,location_BUF,9);
			 memcpy(device.lat,location_BUF+11,9);
					//memset(location_BUF, 0, sizeof(location_BUF));
			BSP_Printf("lng:%s \r\nlat:%s\r\n",device.lng,device.lat);
			
		}
	 return ret;
}


u8 closebearer()
{
u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+SAPBR=0,1","OK",0,200);
  return ret;
	
}
u8 OPENbearer()
{
u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+SAPBR=1,1","OK",0,200);
  return ret;
	
}
u8 qureybearer()
{
u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+SAPBR=2,1","0.0.0.0",0,200);
  return ret;
	
}

//建立tcp连接
u8 Link_Server_AT(u8 mode,const char* ipaddr,const char *port)
{
	u8 count = COUNT_AT;
	u8 ret = 3;
	u8 p[100]={0};
	
//	if(mode)
//		;
//	else 
//		;
		
  	sprintf((char*)p,"AT+CIPSTART=\"%s\",\"%s\",\"%s\"",modetbl[mode],ipaddr,port);	

	//发起连接
	//AT+CIPSTART指令可能的回文是：CONNECT OK 和ALREADY CONNECT和CONNECT FAIL
	//这里先取三种可能回文的公共部分来作为判断该指令有正确回文的依据
	while(count != 0)
	{
		ret = SIM800_Send_Cmd(p,"CONNECT",0,15000);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		
		count--;
		
		ret = SIM800_Send_Cmd("AT+CIPSTATUS","OK",0,500);
		if(ret == CMD_ACK_OK)
		{
			if(strstr((const char*)(device.sim_data),"CONNECT OK") != NULL)
				return ret;
			if(strstr((const char*)(device.sim_data),"CLOSED") != NULL)
			{
				ret = SIM800_Send_Cmd("AT+CIPCLOSE=1","CLOSE OK",0,500);
				ret = SIM800_Send_Cmd("AT+CIPSHUT","SHUT OK",0,500);
			}
		}
	}
		
	return ret;
}

u8 Send_Data_To_Server(char* data)
{
	u8 ret = CMD_ACK_NONE;
		BSP_Printf("准备开始发送数据\r\n");
		device.msg_rec = 0;		
		//dev.msg_expect = 0;
		memset(device.ack1, 0, sizeof(device.ack1));
		memset(dev.device_on_cmd_string, 0, sizeof(dev.device_on_cmd_string));
		ret = SIM800_Send_Cmd("AT+CIPSEND",">",0,500);
	if(ret==1) //如果发送失败
		{
		//dev.need_reset=1;
		}
	
	if(ret == CMD_ACK_OK)		//发送数据
	{ 
		//Clear_Usart3();   //成功发送"AT+CIPSEND" 之后，才使能串口接收
		u3_printf("%s",data);
		delay_ms(100);
		ret = SIM800_Send_Cmd((u8*)0x1A,"SEND OK",0,3000);
		BSP_Printf("已完成一次发送  %d\r\n", ret);
	}
	else
	{
			device.netstatus=ERR_DISCONNECT;
			BSP_Printf("发送失败，状态: %d\r\n", ret);
	  	TIM_Cmd(TIM5,ENABLE);	//打开断网计时
		 //return ret;
	}
	
	
	return ret;
}

#if 0
u8 Check_Link_Status(void)
{
	u8 count = 0;

	while(SIM800_Send_Cmd("AT+CMSTATE","CONNECTED",500))//检测是否应答AT指令 
	{
		if(count < COUNT_AT)
		{
			count += 1;
			delay_ms(2000);			
		}
		else
		{
//AT指令已经尝试了COUNT_AT次，仍然失败，断电SIM800，开启TIME_AT分钟的定时，定时时间到，再次链接服务器
//目前代码中没有调用本函数，也没有对Flag_TIM6_2_S的判定代码，所以暂时屏蔽掉Flag_TIM6_2_S的赋值
			//Flag_TIM6_2_S = 0xAA;
			return 1;		
		}
	}	

		//AT指令的回文不需要处理，清零
	Clear_Usart3();
	return 0;

}
#endif

//设置文本模式 
u8 SIM800_CMGF_Set(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CMGF=1","OK",0,1000);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		
		count--;
	}

	delay_ms(2000);	
	return ret;
}

//设置短消息文本模式参数 
u8 SIM800_CSMP_Set(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CSMP=17,167,0,0","OK",0,200);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		count--;
	}
	
	delay_ms(2000);	
	return ret;
}

u8 SIM800_CSCS_Set(void)
{
	u8 count = COUNT_AT;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Send_Cmd("AT+CSCS=\"GSM\"","OK",0,200);
		if(ret == CMD_ACK_NONE)
		{
			delay_ms(2000);
		}
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))
			break;
		count--;
	}
	
	delay_ms(2000);	
	return ret;
}
 //创建一封短信
char *SIM800_SMS_Create(char *sms_data, char *raw)
{
	sprintf((char*)sms_data,"Reset Type: %d, Dev Status: %d, Msg expect: %d, Msg recv: %d, HB: %d, HB TIMER: %d, Msg TIMEOUT: %d, Msg: \"%s\", AT-ACK: %s\r\n", dev.need_reset, 
		dev.status, dev.msg_expect, dev.msg_recv, dev.hb_count, dev.hb_timer, dev.msg_timeout, raw, dev.atcmd_ack); 
	return sms_data;
}
  //发送短信
u8 SIM800_SMS_Notif(char *phone, char *sms)
{
	u8 ret = CMD_ACK_NONE;
	u8 sms_cmd[100]={0};
	//u8 sms_data[100]={0};

	if((ret = Check_Module()) == CMD_ACK_OK)
		if((ret = Disable_Echo()) == CMD_ACK_OK)
			if((ret = Check_SIM_Card()) == CMD_ACK_OK)	
				if((SIM800_CSCS_Set()) == CMD_ACK_OK)
					if((ret = SIM800_CMGF_Set()) == CMD_ACK_OK)
						if((ret = SIM800_CSMP_Set()) == CMD_ACK_OK)
						{		
							sprintf((char*)sms_cmd,"AT+CMGS=\"%s\"\r\n",phone); 
							if(SIM800_Send_Cmd(sms_cmd,">",0,200)==CMD_ACK_OK)					//发送短信命令+电话号码
							{
								//sprintf((char*)sms_data,"Dev Status: %d, Msg expect: %d, Msg recv: %d, HB: %d, HB TIMER: %d, Msg TIMEOUT: %d Msg: \"%s\"\r\n", dev.status, dev.msg_expect, dev.msg_recv, dev.hb_count, dev.hb_timer, dev.msg_timeout, current); 
								BSP_Printf("SMS: %s\r\n", sms);
								u3_printf("%s",sms);		 						//发送短信内容到GSM模块 
								delay_ms(500);                                   //必须延时，否则不能发送短信
								ret = SIM800_Send_Cmd((u8*)0X1A,"+CMGS:",0,2000); //发送结束符,等待发送完成(最长等待10秒钟,因为短信长了的话,等待时间会长一些)
							}  			
						}

	return ret;
}


void sim800config()
{  u8 ret = CMD_ACK_NONE;
	switch(step_configure)
	{
			case 0 :
			{
				//memset(usart2Buf, 0, sizeof(usart2Buf));
				//UsartPrintf(USART2, " AT\r\n");
				//检测有无反应
				if((ret = Check_Module()) == CMD_ACK_OK)
				{step_configure = 1;}
				else 
					step_configure=0;
				//delayMs(1000);
			}
			 break;
			case 1:
			{
				//关闭回显
			 if((ret = Disable_Echo()) == CMD_ACK_OK)
			 {step_configure = 2;}
			 else
				 step_configure = 1;
			}
			break;
			case 2:
			{
				//检测网络注册情况
				if((ret = Check_Network()) == CMD_ACK_OK)	
				{step_configure = 3;}
			}
			break;
			case 3:
			{
				//检测有无sim卡
					if((ret = Check_SIM_Card()) == CMD_ACK_OK)	
			
				{step_configure = 4;}
			}
			break;
			case 4:
			{
				//检测信号质量
				if((ret = Check_CSQ()) == CMD_ACK_OK)
			//if( CMD_ACK_OK==Check_CSQ())
				{step_configure = 5;}
			}
			break;
			case 5:
			{
				//获取位置
				if((ret = Get_location()) == CMD_ACK_OK)
			//if( CMD_ACK_OK==Get_location())
				{step_configure = 6;}
			}
			break;
			case 6:
			{
				//关闭数据连接
				if((ret = SIM800_GPRS_CIPSHUT()) == CMD_ACK_OK)
			//if( CMD_ACK_OK==SIM800_GPRS_CIPSHUT())
				{step_configure = 7;}
			}
			break;
			case 7:
			{
				//
				if((ret = SIM800_GPRS_CGCLASS()) == CMD_ACK_OK)
			//if( CMD_ACK_OK==SIM800_GPRS_CGCLASS())
				{step_configure = 8;}
			}
			break;
			case 8:
			{
				//
				if((ret = SIM800_GPRS_CGDCONT()) == CMD_ACK_OK)
			//if( CMD_ACK_OK==SIM800_GPRS_CGDCONT())
				{step_configure = 9;}
			}
			break;
			case 9:
			{
				//关闭数据连接
				if((ret = Link_Server_AT(0, ipaddr, port)) == CMD_ACK_OK)
			//if( CMD_ACK_OK==Link_Server_AT(0, ipaddr, port))
			    device.status=0;//联网成功，把状态	
		 { step_configure = 0;}
			}
			break;
			
	}
	
}
//返回1   某条AT指令执行错误
//返回0   成功连接上服务器
u8 SIM800_Link_Server_AT()
{
	u8 ret = CMD_ACK_NONE;
	//操作AT指令进行联网操作
	
	if((ret = Check_Module()) == CMD_ACK_OK)
	{}
		else
		{device.netstatus=ERR_DISCONNECT;}
		if((ret = Disable_Echo()) == CMD_ACK_OK)
			{}
		else
		{device.netstatus=ERR_DISCONNECT;}
			if((ret = Check_Network()) == CMD_ACK_OK)	
    {}
		else
		{device.netstatus=ERR_DISCONNECT;}				
		if((ret = Check_SIM_Card()) == CMD_ACK_OK)
			{}
		else
		{device.netstatus=ERR_DISCONNECT;}	
		if((ret = Check_CSQ()) == CMD_ACK_OK)
			{}
		else
		{device.netstatus=ERR_DISCONNECT;}				
		if((ret = Get_ICCID()) == CMD_ACK_OK)
			{}
		else
		{device.netstatus=ERR_DISCONNECT;}	
     	if((ret = Get_location()) == CMD_ACK_OK)
			{}
		else
		{device.netstatus=ERR_DISCONNECT;}		
		
							
		if((ret = SIM800_GPRS_CIPSHUT()) == CMD_ACK_OK)
		{}
		else
		{device.netstatus=ERR_DISCONNECT;}	
		if((ret = SIM800_GPRS_CGCLASS()) == CMD_ACK_OK)
		{}
		else
		{device.netstatus=ERR_DISCONNECT;}										
		if((ret = SIM800_GPRS_CGDCONT()) == CMD_ACK_OK)
		{}
		else
		{device.netstatus=ERR_DISCONNECT;}										
												//if((ret = SIM800_GPRS_Adhere()) == CMD_ACK_OK)
													//if((ret = SIM800_GPRS_Set()) == CMD_ACK_OK)
														//if((ret = SIM800_GPRS_Dispaly_IP()) == CMD_ACK_OK)
		if((ret = Link_Server_AT(0, ipaddr, port)) == CMD_ACK_OK)
		{}
		else
		{device.netstatus=ERR_DISCONNECT;}	
															 device.status=0;//联网成功，把状态
																//dev.status=CMD_SEND_SERVER;



}

u8 SIM800_Link_Server_Powerkey(void)
{
	u8 count = 5;
	u8 ret = CMD_ACK_NONE;	
	while(count != 0)
	{
		ret = SIM800_Link_Server_AT();
		if(ret != CMD_ACK_OK)
		{
			
			
		}
		else
			break;
		count--;
	}
	return ret;
}
u8 SIM800_Link_Server(void)
{
	u8 count = 5;
	u8 ret = CMD_ACK_NONE;
	while(count != 0)
	{
		ret = SIM800_Link_Server_Powerkey();
		if(ret != CMD_ACK_OK)
		{
			
		}
		else
			break;
		count--;
	}
	return ret;
}



//////////////异或校验和函数///////
u8 Check_Xor_Sum(char* pBuf, u16 len)
{
	u8 Sum = 0;
	u8 i = 0;
	Sum = pBuf[0];
	
	for (i = 1; i < len; i++ )
	{
		Sum = (Sum ^ pBuf[i]);
	}
	
	return Sum;
}
//向服务器提交
u8 SendPost_Server()
{
   u8 ret = CMD_ACK_NONE;
	 packagepost(ipaddr,port,coredata);
	 ret = Send_Data_To_Server(postdata);
	 return ret;
}


//通过at指令与ntp服务器建立连接
void SIM800_ntpserver()
{
	 u8 ret=0;
	 char *timePtr = NULL;
	if((ret = Disable_Echo()) == CMD_ACK_OK)
		  
	if((ret = SAPBR3()) == CMD_ACK_OK)
    if((ret = SAPBR2()) == CMD_ACK_OK)
			if(ret=(qureybearer())==CMD_ACK_OK)
			{
			  SIM800_Send_Cmd("AT+CNTPCID=1","OK",0,200);
				SIM800_Send_Cmd("AT+SAPBR=1,1","OK",0,200);
			}
			
			if((ret = Link_ntp_srver()) == CMD_ACK_OK)
			if((ret = ntptime()) == CMD_ACK_OK)
			if(SIM800_Send_Cmd("AT+CCLK?","+CCLK:",0,200)==0)
					 { newtimer.newyear=(device.sim_data[10]-'0')*10+(device.sim_data[11]-'0')+2000;
						 newtimer.newmonth=(device.sim_data[13]-'0')*10+(device.sim_data[14]-'0');
						 newtimer.newdate=(device.sim_data[16]-'0')*10+(device.sim_data[17]-'0');
						
						 newtimer.newhour=(device.sim_data[19]-'0')*10+(device.sim_data[20]-'0');
						 newtimer.newmin=(device.sim_data[22]-'0')*10+(device.sim_data[23]-'0');
						 newtimer.newsec=(device.sim_data[25]-'0')*10+(device.sim_data[26]-'0');		
             BSP_Printf("时间更新\r\n");						 
					 }
			Time_Update(newtimer.newyear,newtimer.newmonth,newtimer.newdate,newtimer.newhour,newtimer.newmin,newtimer.newsec);
				 
}
//设置接到短信发到串口
u8 setsmsmodel()
{
   u8 ret=0;
	 ret = SIM800_Send_Cmd("AT+CNMI=2,2","OK","",1000);
	 return ret;
}

//设置短信格式为text
u8 setsms_layout()
{
   u8 ret=0;
	 ret = SIM800_Send_Cmd("AT+CMGF=1","OK","",1000);
	 return ret;
}

//读取短信内容
//形参：	
//			num：		保存发件人号码(unicode编码格式的字符串)
//			str：		保存短信内容(unicode编码格式的字符串)
//返回值：	0表示失败
//			1表示成功读取到短信，该短信未读（此处是第一次读，读完后会自动被标准为已读）
//			2表示成功读取到短信，该短信已读
uint8_t readmessage(char *str)
{
    uint8_t len;
		char *redata;
    char cmd[100]={0};
	  char result=0;
		 redata=device.message;
		 len=strlen(redata);
			
		while(*redata != '"')
	{
		len--;
		if(len==0)
		{
			return 0;
		}
		redata++;
	}
//	  redata+=4;
//			while(*redata != '"')
//			{
//				*num++ = *redata++;
//				len--;
//			}
//			*num = 0; 
//  	
		
	  redata+=46;   
    	//发送方号码为11位时，从44开始截取。
	    //发送放号码为11+X位时，从44+X开始截取
		while(*redata!='\r')
		{
		*str++=*redata++;
				len--;
		}
	  *str=0;
			return 1;
}


   //将1个字符转换为16进制数字
//chr:字符,0~9/A~F/a~F
//返回值:chr对应的16进制数值
u8 sim800c_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//将1个16进制数字转换为字符
//hex:16进制数字,0~15;
//返回值:字符
u8 sim800c_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}
//unicode gbk 转换函数
//src:输入字符串
//dst:输出(uni2gbk时为gbk内码,gbk2uni时,为unicode字符串)
//mode:0,unicode到gbk转换;
//     1,gbk到unicode转换;
void sim800c_unigbk_exchange(u8 *src,u8 *dst,u8 mode)
{
	u16 temp; 
	u8 buf[2];
	if(mode)//gbk 2 unicode
	{
		while(*src!=0)
		{
			if(*src<0X81)	//非汉字
			{
				temp=(u16)ff_convert((WCHAR)*src,1);
				src++;
			}else 			//汉字,占2个字节
			{
				buf[1]=*src++;
				buf[0]=*src++; 
				temp=(u16)ff_convert((WCHAR)*(u16*)buf,1); 
			}
			*dst++=sim800c_hex2chr((temp>>12)&0X0F);
			*dst++=sim800c_hex2chr((temp>>8)&0X0F);
			*dst++=sim800c_hex2chr((temp>>4)&0X0F);
			*dst++=sim800c_hex2chr(temp&0X0F);
		}
	}else	//unicode 2 gbk
	{ 
		while(*src!=0)
		{
			buf[1]=sim800c_chr2hex(*src++)*16;
			buf[1]+=sim800c_chr2hex(*src++);
			buf[0]=sim800c_chr2hex(*src++)*16;
			buf[0]+=sim800c_chr2hex(*src++);
 			temp=(u16)ff_convert((WCHAR)*(u16*)buf,0);
			if(temp<0X80){*dst=temp;dst++;}
			else {*(u16*)dst=swap16(temp);dst+=2;}
		} 
	}
	*dst=0;//添加结束符
} 
void sim800_unnicode2asiicc(u8 *src,u16 *dst)
{
	u16 temp; 
	u16 buf[2];
while(*src!=0)
		{
			buf[1]=sim800c_chr2hex(*src++)*16;
			buf[1]+=sim800c_chr2hex(*src++);
			buf[0]=sim800c_chr2hex(*src++)*16;
			buf[0]+=sim800c_chr2hex(*src++);
 		  //dst[i]=buf;
			if(temp<0xFFFF)	
			{  
				temp=(buf[1]<<8)+buf[0];
				*dst=temp;dst++;
			}
			
		} 
}




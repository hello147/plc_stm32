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
#include "ff.h"//firecc936.c�ļ��У�ff_convert()����ʵ��unicode��gbk��ת����
#define COUNT_AT 3
static uint8_t MaxMessAdd=50;
extern Device_Info device;  //������
u8 mode = 0;				//0,TCP����;1,UDP����
const char *modetbl[2] = {"TCP","UDP"};//����ģʽ
extern char postdata[200];
extern  struct NEW_Time newtimer;
char  *ipaddr = "111.14.212.26";
char  *port = "5000";
const char delim=',';
const char ending='#';
extern char coredata[200];
char post[]={
"test"
};
struct  STRUCT_USARTx_sim_Fram strSIM800_Fram_Record = { 0 };


//�洢PCB_ID�����飨Ҳ����SIM����ICCID��
char ICCID_BUF[LENGTH_ICCID_BUF+1] = {0};
char location_BUF[24] = {0};
char Number_BUF[11];
t_DEV dev={0};
const char *msg_device="000";

//SIM800��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//
//waittime:�ȴ�ʱ��(��λ:10ms)
//��ȷ�ظ�0������ظ�1
u8 SIM800_Send_Cmd(char * cmd, char* reply1, char* reply2, u32 waittime)
{
	char ret = 1; 
  //strSIM800_Fram_Record.InfBit.FramLength=0;
  
//	if((reply1==0)&&(reply2==0))
//		ret=0;
//		delay_ms ( waittime ); 
//	 USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;	//��ӽ����� 
	
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
				while((USART3->SR&0X40)==0);//�ȴ���һ�����ݷ������  
				USART3->DR = (u32)cmd;
			}
			else 
			{
				u3_printf("%s\r\n",cmd);//��������
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
		//ATָ���Ѿ�ָ����ɣ�����Է���ֵ���д���
		p_temp = device.sim_data;
		memset(ICCID_BUF, 0, sizeof(Number_BUF));
		//��ȡNumber_BUF��Ϣ��ȫ�ֱ���Number_BUF
		for(index = 0;index < 11;index++)
		{
			Number_BUF[index] = *(p_temp+OFFSET_Num+index);
			 strcpy(device.simid,Number_BUF);
		}
		BSP_Printf("Number_BUF:%s\r\n",Number_BUF);
	}
	//ATָ��Ļ����Ѿ�������ɣ�����
	
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
		
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))  //������AT ����������ϲ����ܷ���"CLOSED" �� ������������
			break;
		
		count--;
	}
	
	//Clear_Usart3();	
	return ret;
	
}

//�رջ���
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

//���ע������
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
		else if((ret == CMD_ACK_OK) || (ret == CMD_ACK_DISCONN))  //������AT ����������ϲ����ܷ���"CLOSED" �� ������������
			break;
		
		count--;
	}
	
	//Clear_Usart3();	
	return ret;
	
}

//�鿴SIM�Ƿ���ȷ��⵽
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


//�鿴��������
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
			else if((ret == 0) || (ret == 1))
			{
				device.netstatus=ERR_DISCONNECT;
			  break;
			}	
		
		if(ret == 0)
		{
			//ATָ���Ѿ�ָ����ɣ�����Է���ֵ���д���
			p1=(u8*)strstr((const char*)(device.sim_data),":");
			p2=(u8*)strstr((const char*)(p1),",");
			p2[0]=0;//���������
			signal = atoi((const char *)(p1+2));
			//�ж��ź�������Ϊ��ʱֱ�Ӹ�״̬
			if(signal==0)
			{
			device.netstatus=ERR_DISCONNECT;
			}
			sprintf((char*)p,"�ź�����:%d",signal);
			BSP_Printf("%s\r\n",p);
		}
		//ATָ��Ļ����Ѿ�������ɣ�����
		count--;
			
	}	
	return ret;
}

//��ȡSIM����ICCID
//SIM����ICCID,ȫ��Ψһ�ԣ���������PCB�����ID
//��ӡUSART3_RX_BUF�ă��� �{ԇ��;
		/*****  ע��+��ǰ���������ո�
  +CCID: 1,"898602B8191650216485"

		OK
		****/
//���������û������ȷ��....
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
	//ATָ���Ѿ�ָ����ɣ�����Է���ֵ���д���
		p_temp = device.sim_data;
		memset(ICCID_BUF, 0, sizeof(ICCID_BUF));
		//��ȡICCID��Ϣ��ȫ�ֱ���ICCID_BUF
		for(index = 0;index < LENGTH_ICCID_BUF;index++)
		{
			ICCID_BUF[index] = *(p_temp+OFFSET_ICCID+index);
		}
		BSP_Printf("ICCID_BUF:%s\r\n",ICCID_BUF);
		
	}
	//ATָ��Ļ����Ѿ�������ɣ�����
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

//�ر�GPRS������
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

//����GPRS
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

//����ΪGPRS����ģʽ
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

//���ý���������ʾIPͷ(�����ж�������Դ)	
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

//�ر��ƶ����� 
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

//����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
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


//����PDP������,��������Э��,��������Ϣ
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
//���ó��س���(����ntp)
u8 SAPBR3()
{
  u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",0,200);
  return ret;
}
//���ý����
u8 SAPBR2()
{
  u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",0,200);
  return ret;
}
//����gprs������
u8 SAPBR1()
{
  u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+SAPBR=1,1","OK",0,200);
  return ret;
}
//����ntp����
u8 Link_ntp_srver()
{
u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+CNTP=\"ntp1.aliyun.com\",32","OK",0,200);
  return ret;
}
//ͬ��ntpʱ��
u8 ntptime()
{
  u8 ret = CMD_ACK_NONE;
	ret=SIM800_Send_Cmd("AT+CNTP","+CNTP: 1",0,200);
  return ret;
}
//��ȡ��γ��
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

//����tcp����
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

	//��������
	//AT+CIPSTARTָ����ܵĻ����ǣ�CONNECT OK ��ALREADY CONNECT��CONNECT FAIL
	//������ȡ���ֿ��ܻ��ĵĹ�����������Ϊ�жϸ�ָ������ȷ���ĵ�����
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
		BSP_Printf("׼����ʼ��������\r\n");
		device.msg_rec = 0;		
		//dev.msg_expect = 0;
		memset(device.ack1, 0, sizeof(device.ack1));
		memset(dev.device_on_cmd_string, 0, sizeof(dev.device_on_cmd_string));
		ret = SIM800_Send_Cmd("AT+CIPSEND",">",0,500);
	if(ret==1) //�������ʧ��
		{
		//dev.need_reset=1;
		}
	
	if(ret == CMD_ACK_OK)		//��������
	{ 
		//Clear_Usart3();   //�ɹ�����"AT+CIPSEND" ֮�󣬲�ʹ�ܴ��ڽ���
		u3_printf("%s",data);
		delay_ms(100);
		ret = SIM800_Send_Cmd((u8*)0x1A,"SEND OK",0,3000);
		BSP_Printf("�����һ�η���  %d\r\n", ret);
	}
	else
	{
			device.netstatus=ERR_DISCONNECT;
			BSP_Printf("����ʧ�ܣ�״̬: %d\r\n", ret);
	  	TIM_Cmd(TIM5,ENABLE);	//�򿪶�����ʱ
		 //return ret;
	}
	
	
	return ret;
}

#if 0
u8 Check_Link_Status(void)
{
	u8 count = 0;

	while(SIM800_Send_Cmd("AT+CMSTATE","CONNECTED",500))//����Ƿ�Ӧ��ATָ�� 
	{
		if(count < COUNT_AT)
		{
			count += 1;
			delay_ms(2000);			
		}
		else
		{
//ATָ���Ѿ�������COUNT_AT�Σ���Ȼʧ�ܣ��ϵ�SIM800������TIME_AT���ӵĶ�ʱ����ʱʱ�䵽���ٴ����ӷ�����
//Ŀǰ������û�е��ñ�������Ҳû�ж�Flag_TIM6_2_S���ж����룬������ʱ���ε�Flag_TIM6_2_S�ĸ�ֵ
			//Flag_TIM6_2_S = 0xAA;
			return 1;		
		}
	}	

		//ATָ��Ļ��Ĳ���Ҫ��������
	Clear_Usart3();
	return 0;

}
#endif

//�����ı�ģʽ 
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

//���ö���Ϣ�ı�ģʽ���� 
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
 //����һ�����
char *SIM800_SMS_Create(char *sms_data, char *raw)
{
	sprintf((char*)sms_data,"Reset Type: %d, Dev Status: %d, Msg expect: %d, Msg recv: %d, HB: %d, HB TIMER: %d, Msg TIMEOUT: %d, Msg: \"%s\", AT-ACK: %s\r\n", dev.need_reset, 
		dev.status, dev.msg_expect, dev.msg_recv, dev.hb_count, dev.hb_timer, dev.msg_timeout, raw, dev.atcmd_ack); 
	return sms_data;
}
  //���Ͷ���
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
							if(SIM800_Send_Cmd(sms_cmd,">",0,200)==CMD_ACK_OK)					//���Ͷ�������+�绰����
							{
								//sprintf((char*)sms_data,"Dev Status: %d, Msg expect: %d, Msg recv: %d, HB: %d, HB TIMER: %d, Msg TIMEOUT: %d Msg: \"%s\"\r\n", dev.status, dev.msg_expect, dev.msg_recv, dev.hb_count, dev.hb_timer, dev.msg_timeout, current); 
								BSP_Printf("SMS: %s\r\n", sms);
								u3_printf("%s",sms);		 						//���Ͷ������ݵ�GSMģ�� 
								delay_ms(500);                                   //������ʱ�������ܷ��Ͷ���
								ret = SIM800_Send_Cmd((u8*)0X1A,"+CMGS:",0,2000); //���ͽ�����,�ȴ��������(��ȴ�10����,��Ϊ���ų��˵Ļ�,�ȴ�ʱ��᳤һЩ)
							}  			
						}

	return ret;
}

//����1   ĳ��ATָ��ִ�д���
//����0   �ɹ������Ϸ�����
u8 SIM800_Link_Server_AT(void)
{
	u8 ret = CMD_ACK_NONE;
	//����ATָ�������������
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
		
							//if((ret = Check_OPS()) == CMD_ACK_OK)
								//if((ret = SIM800_GPRS_OFF()) == CMD_ACK_OK)
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
															 device.status=0;//�����ɹ�����״̬
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



//////////////���У��ͺ���///////
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
//��������ύ
u8 SendPost_Server()
{
   u8 ret = CMD_ACK_NONE;
	 packagepost(ipaddr,port,coredata);
	 ret = Send_Data_To_Server(postdata);
	 return ret;
}


//ͨ��atָ����ntp��������������
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
      BSP_Printf("ʱ�����\r\n");						 
					 }
			Time_Update(newtimer.newyear,newtimer.newmonth,newtimer.newdate,newtimer.newhour,newtimer.newmin,newtimer.newsec);
				 
}
//���ýӵ����ŷ�������
u8 setsmsmodel()
{
  u8 ret=0;
	 ret = SIM800_Send_Cmd("AT+CNMI=2,2","OK","",1000);
	 return ret;
}

//���ö��Ÿ�ʽΪtext
u8 setsms_layout()
{
   u8 ret=0;
	 ret = SIM800_Send_Cmd("AT+CMGF=1","OK","",1000);
	 return ret;
}

//��ȡ��������
//�βΣ�	
//			num��		���淢���˺���(unicode�����ʽ���ַ���)
//			str��		�����������(unicode�����ʽ���ַ���)
//����ֵ��	0��ʾʧ��
//			1��ʾ�ɹ���ȡ�����ţ��ö���δ�����˴��ǵ�һ�ζ����������Զ�����׼Ϊ�Ѷ���
//			2��ʾ�ɹ���ȡ�����ţ��ö����Ѷ�
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
    	//���ͷ�����Ϊ11λʱ����44��ʼ��ȡ��
	    //���ͷź���Ϊ11+Xλʱ����44+X��ʼ��ȡ
		while(*redata!='\r')
		{
		*str++=*redata++;
				len--;
		}
	  *str=0;
			return 1;
}


   //��1���ַ�ת��Ϊ16��������
//chr:�ַ�,0~9/A~F/a~F
//����ֵ:chr��Ӧ��16������ֵ
u8 sim800c_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//��1��16��������ת��Ϊ�ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
u8 sim800c_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}
//unicode gbk ת������
//src:�����ַ���
//dst:���(uni2gbkʱΪgbk����,gbk2uniʱ,Ϊunicode�ַ���)
//mode:0,unicode��gbkת��;
//     1,gbk��unicodeת��;
void sim800c_unigbk_exchange(u8 *src,u8 *dst,u8 mode)
{
	u16 temp; 
	u8 buf[2];
	if(mode)//gbk 2 unicode
	{
		while(*src!=0)
		{
			if(*src<0X81)	//�Ǻ���
			{
				temp=(u16)ff_convert((WCHAR)*src,1);
				src++;
			}else 			//����,ռ2���ֽ�
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
	*dst=0;//��ӽ�����
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




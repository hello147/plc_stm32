#include "iic.h"
#include "stm32f10x.h"
#include "readconfig.h"
#include "device.h"
#include "usart2.h"
#include "string.h"
#include "sim800.h"
#include "stdint.h"
#include "delay.h"
 dev_configration dev_config={0};
extern struct Device_Info device; 
  char datatemp[72];
 char defalut[20];
 int readedaddr=100;
 char ReadBuf[bakbuffer_len];
 
extern char coredata[200];
//��ȡ����
void r_defalutconfig()
{
	int i=0;
	int a=0,b=0;
	
	if(Get_Number() == 0)
	 {
	  for(i=0;i<11;i++)
		 {
		   E2promWriteByte(i+17,device.simid[i]);
			 Delay(15); 
		 }
	 }
	 if(setsms_layout()==0)
	 {	BSP_Printf("���ö���Ϊtext��ʽ\r\n");	}
	 if(setsmsmodel()==0)
	 {  BSP_Printf("���ö���ֱ�ӷ������ڳɹ�\r\n"); }

	E2promReadBuffer(0,datatemp,72);
	BSP_Printf("��ȡĬ�����óɹ�\r\n");	
	BSP_Printf("\r\n");	
	memset(device.simid,0,12);
	strncpy(device.machineid,datatemp+3,8);//��ȡ������
	strncpy(device.simid,datatemp+17,11);//��ȡ�ֻ�����
	strncpy(device.period,datatemp+35,2);//��ȡʱ����
  strncpy(device.ipaddress,datatemp+42,15);//��ȡip
  strncpy(device.port,datatemp+57,5);//��ȡ�˿�
	strncpy(device.rstime,datatemp+69,3);//��ȡ����ʱ��

	  a=atoi(device.period);
	  b=atoi(device.rstime);
	//device.rstart_time=device.rstime[0]*100+device.rstime[1]*10+device.rstime[2];
	//device.period_time=(int)device.period[0]*10+(int)device.period[1];
	
  //BSP_Printf("period_0%d\r\n",(int)device.period[0]);
  //BSP_Printf("period_1%d\r\n",(int)device.period[1]);

	device.period_time=a;
	device.rstart_time=b;

  
	BSP_Printf("������%s\r\n",device.machineid);
  BSP_Printf("�ֻ���%s \r\n",device.simid);
	BSP_Printf("���%d    %d\r\n",device.period_time,a);
	BSP_Printf("ip%s \r\n",device.ipaddress);
  BSP_Printf("�˿�%s \r\n",device.port);
  BSP_Printf("����ʱ��%d     %d\r\n",device.rstart_time,b);

}

//��ȡ���ڱ��ص�Ӧ������
void readbak()
{
	 int i;
	
	while(readedaddr<1100)
	{
	  
		E2promReadBuffer(readedaddr,ReadBuf,200);
		for(i=0;i<200;i++)
		{
		  E2promWriteByte(readedaddr+i,'0');
			delay_ms(10);
		}
		 BSP_Printf("������\r\n");
		readedaddr+=200;
	//�����������data��־
		if(strstr(ReadBuf,"data")!=NULL)
		 {
			 //Send_Data_To_Server(ReadBuf);
			 sprintf(coredata,"����ʱ������%s\r\n",ReadBuf);
			 SendPost_Server(); 		//���ͨ�����ݲ�����
			 BSP_Printf("�����˱���\r\n");
		 }
	}
	 if(readedaddr>1100)
	{
	  readedaddr=100;
	}
	
	 BSP_Printf("�����˱��ݣ�");
	
	
//		for(i=0;i<sizeof(ReadBuf);i++)
//		{  
//			BSP_Printf("%c",ReadBuf[i]);	
//		}
 
	  //packagepost(ipaddr,port,coredata);
//	 for(i=0;i<sizeof(ReadBuf);i++)
//			{
//			 E2promWriteByte(bakbuffer_len+i+100,"0");
//			}
//			BSP_Printf("������\r\n");
	 
	
}







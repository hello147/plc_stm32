#include "iic.h"
#include "stm32f10x.h"
#include "readconfig.h"
#include "device.h"
#include "usart2.h"
 dev_configration dev_config={0};
extern Device_Info device; 
uint8_t datatemp[65];
 uint8_t defalut[20];
 
 uint8_t ReadBuf[bakbuffer_len];
//��ȡ����
void r_defalutconfig()
{
	int i=0;
	E2promReadBuffer(0,datatemp,63);
	BSP_Printf("��ȡ���óɹ�\r\n");	
	BSP_Printf("\r\n");	
	memset(device.simid,0,11);
	strncpy(device.machineid,datatemp+3,8);//��ȡ������
	strncpy(device.simid,datatemp+17,11);//��ȡ�ֻ�����
	strncpy(device.period,datatemp+35,2);//��ȡʱ����
  strncpy(device.ipaddress,datatemp+42,15);//��ȡip
  strncpy(device.port,datatemp+58,4);//��ȡ�˿�
	BSP_Printf("������%s%s%s%s%s\r\n",device.machineid,device.simid,device.period,device.ipaddress,device.port);
  BSP_Printf("�ֻ���%s\r\n",device.simid);
	BSP_Printf("���%s\r\n",device.period);
	BSP_Printf("ip%s\r\n",device.ipaddress);
  BSP_Printf("�˿�%s\r\n",device.port);
}

//��ȡ���ڱ��ص�Ӧ������
void readbak()
{
	 int i;
	 E2promReadBuffer(100,ReadBuf,sizeof(ReadBuf));
	 BSP_Printf("�����˱��ݣ�");
//		for(i=0;i<sizeof(ReadBuf);i++)
//		{  
//			BSP_Printf("%c",ReadBuf[i]);	
//		}
	//�����������data��־
   if(strstr(ReadBuf,"data")!=NULL)
	 {
	   Send_Data_To_Server(ReadBuf);
		 BSP_Printf("�����˱��ݣ�");
	 }
	  //packagepost(ipaddr,port,coredata);
//	 for(i=0;i<sizeof(ReadBuf);i++)
//			{
//			 E2promWriteByte(bakbuffer_len+i+100,"0");
//			}
//			BSP_Printf("������\r\n");
	 
	
}







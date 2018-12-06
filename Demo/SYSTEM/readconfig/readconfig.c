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
//读取配置
void r_defalutconfig()
{
	int i=0;
	E2promReadBuffer(0,datatemp,63);
	BSP_Printf("读取配置成功\r\n");	
	BSP_Printf("\r\n");	
	memset(device.simid,0,11);
	strncpy(device.machineid,datatemp+3,8);//截取机器码
	strncpy(device.simid,datatemp+17,11);//截取手机号码
	strncpy(device.period,datatemp+35,2);//截取时间间隔
  strncpy(device.ipaddress,datatemp+42,15);//截取ip
  strncpy(device.port,datatemp+58,4);//截取端口
	BSP_Printf("机器码%s%s%s%s%s\r\n",device.machineid,device.simid,device.period,device.ipaddress,device.port);
  BSP_Printf("手机号%s\r\n",device.simid);
	BSP_Printf("间隔%s\r\n",device.period);
	BSP_Printf("ip%s\r\n",device.ipaddress);
  BSP_Printf("端口%s\r\n",device.port);
}

//读取存在本地的应急数据
void readbak()
{
	 int i;
	 E2promReadBuffer(100,ReadBuf,sizeof(ReadBuf));
	 BSP_Printf("读出了备份：");
//		for(i=0;i<sizeof(ReadBuf);i++)
//		{  
//			BSP_Printf("%c",ReadBuf[i]);	
//		}
	//如果读出来有data标志
   if(strstr(ReadBuf,"data")!=NULL)
	 {
	   Send_Data_To_Server(ReadBuf);
		 BSP_Printf("发送了备份：");
	 }
	  //packagepost(ipaddr,port,coredata);
//	 for(i=0;i<sizeof(ReadBuf);i++)
//			{
//			 E2promWriteByte(bakbuffer_len+i+100,"0");
//			}
//			BSP_Printf("擦除了\r\n");
	 
	
}







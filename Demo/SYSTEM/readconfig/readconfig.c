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
//读取配置
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
	 {	BSP_Printf("设置短信为text格式\r\n");	}
	 if(setsmsmodel()==0)
	 {  BSP_Printf("设置短信直接发到串口成功\r\n"); }

	E2promReadBuffer(0,datatemp,72);
	BSP_Printf("读取默认配置成功\r\n");	
	BSP_Printf("\r\n");	
	memset(device.simid,0,12);
	strncpy(device.machineid,datatemp+3,8);//截取机器码
	strncpy(device.simid,datatemp+17,11);//截取手机号码
	strncpy(device.period,datatemp+35,2);//截取时间间隔
  strncpy(device.ipaddress,datatemp+42,15);//截取ip
  strncpy(device.port,datatemp+57,5);//截取端口
	strncpy(device.rstime,datatemp+69,3);//截取重启时间

	  a=atoi(device.period);
	  b=atoi(device.rstime);
	//device.rstart_time=device.rstime[0]*100+device.rstime[1]*10+device.rstime[2];
	//device.period_time=(int)device.period[0]*10+(int)device.period[1];
	
  //BSP_Printf("period_0%d\r\n",(int)device.period[0]);
  //BSP_Printf("period_1%d\r\n",(int)device.period[1]);

	device.period_time=a;
	device.rstart_time=b;

  
	BSP_Printf("机器码%s\r\n",device.machineid);
  BSP_Printf("手机号%s \r\n",device.simid);
	BSP_Printf("间隔%d    %d\r\n",device.period_time,a);
	BSP_Printf("ip%s \r\n",device.ipaddress);
  BSP_Printf("端口%s \r\n",device.port);
  BSP_Printf("重启时间%d     %d\r\n",device.rstart_time,b);

}

//读取存在本地的应急数据
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
		 BSP_Printf("擦除了\r\n");
		readedaddr+=200;
	//如果读出来有data标志
		if(strstr(ReadBuf,"data")!=NULL)
		 {
			 //Send_Data_To_Server(ReadBuf);
			 sprintf(coredata,"断网时的数据%s\r\n",ReadBuf);
			 SendPost_Server(); 		//打包通用数据并发送
			 BSP_Printf("发送了备份\r\n");
		 }
	}
	 if(readedaddr>1100)
	{
	  readedaddr=100;
	}
	
	 BSP_Printf("读出了备份：");
	
	
//		for(i=0;i<sizeof(ReadBuf);i++)
//		{  
//			BSP_Printf("%c",ReadBuf[i]);	
//		}
 
	  //packagepost(ipaddr,port,coredata);
//	 for(i=0;i<sizeof(ReadBuf);i++)
//			{
//			 E2promWriteByte(bakbuffer_len+i+100,"0");
//			}
//			BSP_Printf("擦除了\r\n");
	 
	
}







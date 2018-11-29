#include "send.h"

#include "SysTick.h"
#include <stdio.h>  
#include <string.h>  
#include <stdbool.h>
#include "device.h"
#include "usart2.h"
 #include "readconfig.h" 

#include "rtc.h"
#include "mod.h"
#include "string.h"
//#include "sim800.h"
/************************************************
*数据通过wifi传输到服务器
************************************************/
extern usart_st  usart_data;
extern uint64_t datanum;
extern Device_Info device;
uint16_t address;
char senddata[320];	 
static char rx_buf_str[255];  //串口数据字符串格式
static char body_data_1[160];        //需发送数据
static char body_data_2[50];        //需发送数据
char body_data_len=0;
char jsondata[500];
char postdata[380];

extern  struct Data_Time  timer;
extern  struct NEW_Time newtimer;
 extern dev_configration dev_config; 
volatile uint8_t ucTcpClosedFlag = 0;


 /*字节流转字符串
**参数：  source源数组
**     dest  目标字符串
**     sourceLen长度
*/
 void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen)  
{  
    short i;  
    unsigned char highByte, lowByte;  
  
    for (i = 0; i < sourceLen; i++)  
    {  
        highByte = source[i] >> 4;  
        lowByte = source[i] & 0x0f ;  
  
        highByte += 0x30;  
  
        if (highByte > 0x39)  
                dest[i * 2] = highByte + 0x07;  
        else  
                dest[i * 2] = highByte;  
  
        lowByte += 0x30;  
        if (lowByte > 0x39)  
            dest[i * 2 + 1] = lowByte + 0x07;  
        else  
            dest[i * 2 + 1] = lowByte;  
    }  
    return ;  
}  






//数据内容打包
/* 函数名：packagedata()
** 描述：将需要发送的参数转成json
*/
void packagedata()
{
	 
	 	address=usart_data.tx_buf[3]+40001;
    //字节流转换
     ByteToHexStr(usart_data.rx_buf,rx_buf_str,usart_data.rx_len_ed); 
	  //计算需截取的长度  
	  body_data_len=usart_data.rx_buf[2]*2;
	  //待发送数据截取  
	  strncpy(body_data_1,rx_buf_str+6,body_data_len);
	  Time_Get();  
  	USART2_printf("%d年%02d月%02d日%02d点%02d分%02d秒\r\n",timer.w_year,timer.w_month,timer.w_date,timer.hour,timer.min,timer.sec);		
    r_defalutconfig();
		sprintf(senddata,"{\r\n machineid:\"%s\",\
	  \r\n simid:\"%s\",\
		\r\n data:\"%s\",\
	  \r\n time:\"%d.%02d.%02d %02d:%02d:%02d\",\
		\r\n}",device.machineid,device.simid,body_data_1,timer.w_year,timer.w_month,timer.w_date,timer.hour,timer.min,timer.sec);

}

//打包成post请求
void packagepost(char *ip,char *port,char *page)
{
   char content_page[50];
	 char content_len[300];
	 char content_host[50];
	 char content_type[] = "Content-Type:application/json-patch+json\r\n";
    sprintf(content_page,"POST %s HTTP/1.1\r\n",page);
    sprintf(content_host,"HOST: %s:%d\r\n",ip,port);
    sprintf(content_len,"Content-Length: %d\r\n",strlen(senddata));
    sprintf(postdata,"%s%s%s%s%s",content_page,content_host,content_type,content_len,senddata);
   	//sprintf(postdata,"%s",jsondata);
}

void clear()
{
memset(body_data_1,0,20);
memset(postdata,0,380);
memset(senddata,0,320);
}

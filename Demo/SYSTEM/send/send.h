#ifndef  __SEND_H
#define	 __SEND_H



#include "stm32f10x.h"

/********************************** 外部全局变量 ***************************************/
extern volatile uint8_t ucTcpClosedFlag;
/********************************** 测试函数声明 ***************************************/
void ESP8266_StaTcpClient_UnvarnishTest  ( void );

void packagepost(char *ip,char *port,char *msg);
void Write_To_eeprom(int WriteAddr,u8 *DataToWrite,u8 Len);
#endif


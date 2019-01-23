#ifndef __IIC_H__
#define __IIC_H__	 

#include "stm32f10x.h"



uint8_t E2promWriteByte( uint16_t addr, char data ); 
uint8_t E2promReadBuffer( unsigned  int addr ,unsigned char *buf,unsigned int len) ;
void Delay(u16 speed);


#endif
#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define LOG_ENABLE
#ifdef LOG_ENABLE
/* ���������ڵ��Խ׶��Ŵ� */
#define BSP_Printf	USART2_printf
#else
#define BSP_Printf(...)
#endif

#define USART_REC_LEN  			300  	//�����������ֽ��� 200
#define EN_USART2_RX 			   0		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
void usart1_init(u32 bound);
void usart2_init(u32 bound);
void USART2_printf (char *fmt, ...) ;
#endif



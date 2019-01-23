#ifndef __SIM800C_H__
#define __SIM800C_H__	 
#include "sys.h"
#include <stdio.h>
#include <stdbool.h>
#include "common.h"
#define      macSIM800_USART_BAUD_RATE                       115200
#define      macSIM800_USARTx                                USART3
#define      macSIM800_USART_APBxClock_FUN                   RCC_APB1PeriphClockCmd
#define      macSIM800_USART_CLK                             RCC_APB1Periph_USART3
#define      macSIM800_USART_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define      macSIM800_USART_GPIO_CLK                        RCC_APB2Periph_GPIOB     
#define      macSIM800_USART_TX_PORT                         GPIOB   
#define      macSIM800_USART_TX_PIN                          GPIO_Pin_10
#define      macSIM800_USART_RX_PORT                         GPIOB
#define      macSIM800_USART_RX_PIN                          GPIO_Pin_11
#define      macSIM800_USART_IRQ                             USART3_IRQn
#define      macSIM800_USART_INT_FUN                         USART3_IRQHandler


/******************************* SIM800 外部全局变量声明 ***************************/
#define RX_BUF_MAX_LEN     2000                                     //最大接收缓存字节数
#pragma anon_unions
#endif
extern struct  STRUCT_USARTx_sim_Fram                                  //串口数据帧的处理结构体
{
	char  Data_RX_BUF [ RX_BUF_MAX_LEN ];
	
  union {
    __IO u16 InfAll;
    struct {
		  __IO u16 FramLength       :15;                               // 14:0 
		  __IO u16 FramFinishFlag   :1;                                // 15 
	  } InfBit;
  }; 
	
} strSIM800_Fram_Record;


//存储PCB_ID的数组（也就是SIM卡的ICCID）
#define LENGTH_ICCID_BUF 20     								//ICCID的长度是20个字符
extern char ICCID_BUF[LENGTH_ICCID_BUF+1];
#define OFFSET_ICCID 2 
#define OFFSET_Num   13
#define swap16(x) (x&0XFF)<<8|(x&0XFF00)>>8	//高低字节交换宏定义
//存储设备重发命令的数组
#define LENGTH_RESEND 35
extern char Resend_Buffer[LENGTH_RESEND];

//存储设备登陆命令的数组
#define LENGTH_LOGIN 80
extern char Login_Buffer[LENGTH_LOGIN];

//存储心跳包的数组
#define LENGTH_HEART 50
extern char Heart_Buffer[LENGTH_HEART];

//存储业务指令回文的数组
#define LENGTH_ENABLE 50
extern char Enbale_Buffer[LENGTH_ENABLE];

//存储业务执行完成指令的数组
#define LENGTH_DEVICE_OK 50
extern char Device_OK_Buffer[LENGTH_DEVICE_OK];

#define LENGTH_ATCMD_ACK 50
#define LENGTH_DEVICE_OPEN_CMD        50
#define LENGTH_USART_DATA        100
#define LENGTH_SMS_BACKUP        100

/*********WJ*********/
u8 	SIM800_Send_Cmd(char * cmd, char* reply1, char* reply2, u32 waittime );
u8 Check_Xor_Sum(char* pBuf, u16 len);

u8 	Check_Module(void);
u8 	Disable_Echo(void);
u8 	Check_SIM_Card(void);
u8 	Check_CSQ(void);
u8 	Get_ICCID(void);


u8 	SIM800_GPRS_Adhere(void);
u8	SIM800_GPRS_Set(void);
u8 	SIM800_GPRS_Dispaly_IP(void);
u8 	SIM800_GPRS_CIPSHUT(void);
u8 	SIM800_GPRS_CGCLASS(void);
u8 	SIM800_GPRS_CGDCONT(void);
u8 setsms_layout();
u8 setsmsmodel();
u8 Get_Number(void);
u8 	Link_Server_Echo(void);
u8 	Link_Server_AT(u8 mode,const char* ipaddr,const char *port);

u8 	Send_Data_To_Server(char* data);

u8 	SIM800_GPRS_ON(void);
u8	SIM800_GPRS_OFF(void);
void SIM800_POWER_ON(void);
void SIM800_POWER_OFF(void);
void SIM800_PWRKEY_ON(void);
void SIM800_PWRKEY_OFF(void);

void SIM800_GPRS_Restart(void);


u8 SendPost_Server();
u8 SIM800_Link_Server(void);
u8 SIM800_Link_Server_AT(void);
u8 SIM800_Link_Server_Powerkey(void);


char *SIM800_SMS_Create(char *sms_data, char *raw);
u8 SIM800_SMS_Notif(char *phone, char *sms);
	void SIM800_ntpserver();
#define MSG_STR_LEN_OF_ID                                  7         //strlen("TRVAPXX")
#define MSG_STR_LEN_OF_LENGTH                         3
#define MSG_STR_LEN_OF_SEQ	                               3
#define MSG_STR_LEN_OF_DUP                               2
#define MSG_STR_LEN_OF_DEVICE                          3
#define MSG_STR_LEN_OF_PORTS                           4
#define MSG_STR_LEN_OF_PORTS_PERIOD            (MSG_STR_LEN_OF_PORTS*4)

typedef struct
{
	u8 status;
	u8  netstatus;
	u8 hb_timer;   //hb always running
	u8 reply_timeout;
	u8 need_reset;
	u8 updatetime;
	u16 hb_count;
	u8 msg_seq;
	u8 msg_seq_s;
	u32 msg_timeout;
	u32 msg_recv;
	u32 msg_expect;
	char atcmd_ack[LENGTH_ATCMD_ACK];
	char device_on_cmd_string[LENGTH_DEVICE_OPEN_CMD];
	char usart_data[LENGTH_USART_DATA];
	char sms_backup[LENGTH_SMS_BACKUP];
}t_DEV;

typedef struct
{
	char id[MSG_STR_LEN_OF_ID+1];
	char length[MSG_STR_LEN_OF_LENGTH+1];
	char seq[MSG_STR_LEN_OF_SEQ+1];
	char dup[MSG_STR_LEN_OF_DUP+1];
	char device[MSG_STR_LEN_OF_DEVICE+1];
	char ports[MSG_STR_LEN_OF_PORTS+1];
	char period[MSG_STR_LEN_OF_PORTS_PERIOD+1];
}msg_data;

/*********WJ*********/
extern t_DEV dev;
//#define CMD_LOGIN 1;


enum
{ 
	CMD_ACK_OK = 0,                 //USART3_RX_STA置位，返回的数据正确
	CMD_ACK_DISCONN = 1,       //USART3_RX_STA置位，返回的数据表明掉线
	CMD_ACK_NONE=3,                  //USART3_RX_STA没有置位
};
enum
{
	ERR_NONE = 0,
	ERR_DISCONNECT,	//未连接  
};
enum
{
	
READPLC,
SEND_DATA
};
extern char  *cell;



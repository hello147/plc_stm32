

#include "iic.h"
#include "stm32f10x.h"

void r_defalutconfig(void);

void r_machineid(void);
void r_simid(void);
void r_timeperiod(void);

typedef struct
{
  uint8_t id[8];  //机器码
 	uint8_t simid[11]; //手机号
	int period;//时间间隔
	uint8_t netip[40]; //服务器路径
	uint8_t netport[4];
}dev_configration;





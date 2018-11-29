

#include "iic.h"
#include "stm32f10x.h"

void r_defalutconfig(void);

void r_machineid(void);
void r_simid(void);
void r_timeperiod(void);

typedef struct
{
  uint8_t id[8];  //������
 	uint8_t simid[11]; //�ֻ���
	int period;//ʱ����
	uint8_t netip[40]; //������·��
	uint8_t netport[4];
}dev_configration;





#include "iic.h"
#include "stm32f10x.h"
#include "readconfig.h"
#include "device.h"
#include "usart2.h"
 dev_configration dev_config={0};
extern Device_Info device; 
uint8_t datatemp[65];
 uint8_t defalut[20];

void r_defalutconfig()
{
  //AT24CXX_Read(0,datatemp,65);

	strcpy(device.machineid,"N01-0001");//��ȡ������
	
	strcpy(device.simid,"17854153611");//��ȡ�ֻ���
	dev_config.period=30;
	BSP_Printf("��ȡ���óɹ�\r\n");	
	//strcpy(dev_config.period,"30");//��ȡʱ����
  //strcpy(dev_config.netip,datatemp+45,15);//��ȡip
  //strcpy(dev_config.netport,datatemp+61,4);//��ȡ�˿�
}







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

	strcpy(device.machineid,"N01-0001");//截取机器码
	
	strcpy(device.simid,"17854153611");//截取手机号
	dev_config.period=30;
	BSP_Printf("读取配置成功\r\n");	
	//strcpy(dev_config.period,"30");//截取时间间隔
  //strcpy(dev_config.netip,datatemp+45,15);//截取ip
  //strcpy(dev_config.netport,datatemp+61,4);//截取端口
}







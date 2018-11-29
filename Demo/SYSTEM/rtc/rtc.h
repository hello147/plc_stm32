#ifndef  __RTC_H
#define	 __RTC_H
#include "stm32f10x.h"
struct Data_Time
{
	uint16_t w_year;
	uint8_t  w_month;
	uint8_t  w_date;
	uint8_t  week;
	
	uint8_t hour;
	uint8_t min;
	uint8_t sec;			
	//公历日月年周
	
};  

 struct NEW_Time
{
 uint8_t newhour;
	uint8_t newmin;
	uint8_t newsec;			
	//公历日月年周
	uint16_t newyear;
	uint8_t  newmonth;
	uint8_t  newdate;
	uint8_t  newweek;	
};



	u8 Init_RTC(void);
u8 Is_Leap_Year(u16 year);
u8 Time_Update(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);
u8 Time_Get(void);
u8 RTC_Get_Week(u16 year,u8 month,u8 day);
u8 Usart_Scanf(u32 value,u8 count);
void Time_Set(void);
#endif




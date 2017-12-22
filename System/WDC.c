/**************************************************************
The initial Watch Dog
**************************************************************/
#include "S3C2416.h"

/*---------------------------------------------------------------------------*/


void wt_handler (void)
{
	cy_println( "WatchDog Handler");
}
//初始化看门狗
void watchdog_reset(void)      //看门狗复位函数
{
	cy_println( "WatchDog setup! %dS\n",WatchDogTimer/10000);
	rWTCON=((37<<8)|(3<<3)); 
	/*---------------------------------------------------*/
			//看门狗时钟周期 T = WTCNT * t_watchdog
			//看门狗喂狗
	rWTDAT=WatchDogTimer;
	rWTCNT=WatchDogTimer;
	/*---------------------------------------------------*/
	rWTCON &= ~(2<<1);    //禁止看门狗中断
	//rWTCON |= (2<<1);    //允许看门狗中断
	//IRQ_Register(INT_WDT_AC97, wt_handler);
	//IRQ_EnableInt(INT_WDT_AC97);

	rWTCON|=((1<<5)|(1<<0));   //允许看门狗定时，允许复位
//	while(1);
}
/*---------------------------------------------------------------------------*/
//看门狗喂狗 
void SetWatchDog(void)
{
	rWTDAT=WatchDogTimer;
	rWTCNT=WatchDogTimer;
}
/*---------------------------------------------------------------------------*/

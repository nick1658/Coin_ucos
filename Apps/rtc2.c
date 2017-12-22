///////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经本公司许可，不得用于其它任何商业用途
// 适用开发板型号:Tiny2416、Mini2451、Tiny2451
// 技术论坛:www.arm9home.net
// 修改日期:2013/7/1
// 版权所有，盗版必究。
// Copyright(C) 广州友善之臂计算机科技有限公司
// All rights reserved							
///////////////////////////////////////////////////////////////

#include "s3c2416.h"
#include "rtc.h"

#define printf my_print


#define RTCCON		(*(volatile unsigned short *)0x57000040)	
#define TICNT0		(*(volatile unsigned char *)0x57000044)	
#define TICNT1		(*(volatile unsigned char *)0x5700004c)	
#define TICNT2		(*(volatile unsigned int *)0x57000048)		
#define RTCALM		(*(volatile unsigned char *)0x57000050)	
#define ALMSEC		(*(volatile unsigned char *)0x57000054)	
#define ALMMIN		(*(volatile unsigned char *)0x57000058)	
#define ALMHOUR		(*(volatile unsigned char *)0x5700005c)	
#define ALMDATE		(*(volatile unsigned char *)0x57000060)	
#define ALMMON		(*(volatile unsigned char *)0x57000064)	
#define ALMYEAR		(*(volatile unsigned char *)0x57000068)	
#define RTCRST		(*(volatile unsigned char *)0x5700006c)
#define BCDSEC		(*(volatile unsigned char *)0x57000070)	
#define BCDMIN		(*(volatile unsigned char *)0x57000074)	
#define BCDHOUR		(*(volatile unsigned char *)0x57000078)
#define BCDDATE		(*(volatile unsigned char *)0x5700007c)
#define BCDDAY		(*(volatile unsigned char *)0x57000080)	
#define BCDMON		(*(volatile unsigned char *)0x57000084)	
#define BCDYEAR		(*(volatile unsigned char *)0x57000088)	
#define TICKCNT		(*(volatile unsigned  *)0x57000090)		

#define true  1
#define false 0

char *day[8] = {" ","Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

void delay(volatile int count)
{
	while (count--);
}

void rtc_realtime_display(void)
{
	int counter = 0;
	
	// 使能rtc控制器
	rtc_enable(true);
	// 使能rtc tick timer
	rtc_ticktime_enable(true);

	// 打印5次时间
	while( (counter++) < 5)
	{
		rtc_print();
		delay(0x10000000/3);
	}
	
	// 关闭rtc控制器
	rtc_ticktime_enable(false);
	// 关闭rtc tick timer
	rtc_enable(false);
	
}

// 使能/关闭rtc控制器
void rtc_enable(unsigned char bdata)
{
	unsigned long uread;

	uread = RTCCON;
	RTCCON = (uread&~(1<<0))|(bdata);
}

// 使能/关闭Tick timer
void rtc_ticktime_enable(unsigned char bdata)
{
	unsigned long uread;
	uread = RTCCON;
	RTCCON = ( uread&~(1<<8) ) | (bdata<<8);
}

// 打印时间
void rtc_print(void)
{
	unsigned long uyear,umonth,udate,uday,uhour,umin,usec;

	uyear = BCDYEAR;
	uyear = 0x2000 + uyear;
	umonth= BCDMON;
	udate = BCDDATE;
	uhour = BCDHOUR;
	umin  = BCDMIN;
	usec  = BCDSEC;
	uday  = BCDDAY;

	printf("%2x : %2x : %2x  %10s,  %2x/%2x/%4x\r\n", uhour, umin, usec, day[uday], umonth, udate, uyear);
}

void rtc_settime(void)
{
	// 重置值
	unsigned long year = 17;
	unsigned long month = 6;
	unsigned long	date = 26;
	unsigned long hour = 22;
	unsigned long min = 0;
	unsigned long sec = 0;
	unsigned long weekday= 1;

	//将时间转化为BCD码
	year = ( ((year/100)<<8) +(((year/10)%10)<<4) + (year%10)  );
	month  = ( ((month/10)<<4)+ (month%10) );
	date = ( ((date/10)<<4) + (date%10) );
	weekday = (weekday%10);							
	hour =( ((hour/10)<<4) + (hour%10) );
	min  =( ((min/10)<<4)  + (min%10) );
	sec  =( ((sec/10)<<4)  + (sec%10) );

	rtc_enable(true);
	// 保存BCD码
	BCDSEC  = sec;
	BCDMIN  = min;
	BCDHOUR = hour;
	BCDDATE = date;
	BCDDAY  = weekday;
	BCDMON  = month;
	BCDYEAR = year;
	rtc_enable(false);
	
	printf("reset success\r\n");
}

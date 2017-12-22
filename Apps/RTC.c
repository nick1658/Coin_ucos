#include "s3c2416.h"
#include "RTC.h"

void RTC_GetTime(RTC_Time *pTime)
{
	rRTCCON |= (1 << 0);	
	pTime->Year = 2000 + ((rBCDYEAR & 0xf0) >> 4)*10 + (rBCDYEAR & 0xf);
	pTime->Month = ((rBCDMON>>4) & 0x1)*10 + (rBCDMON & 0xf);
	pTime->Day = ((rBCDDATE>>4) & 0x3)*10 + (rBCDDATE & 0xf);
	pTime->Week = (rBCDDAY & 0x7) - 1; // Sunday = 0
	pTime->Hour = ((rBCDHOUR>>4) & 0x3)*10 + (rBCDHOUR & 0xf);
	pTime->Min = ((rBCDMIN>>4) & 0x7)*10 + (rBCDMIN & 0xf);	
	pTime->Sec = ((rBCDSEC>>4) & 0x7)*10 + (rBCDSEC & 0xf);	
	rRTCCON &= ~(1 << 0);
}

void RTC_SetTime(RTC_Time *pTime)
{
	rRTCCON = (1<<0); // Tick 1hz
	rBCDYEAR = ((pTime->Year%100/10) << 4 | (pTime->Year%100%10) << 0);
	rBCDMON = ((pTime->Month/10) << 4) | ((pTime->Month%10) << 0);
	rBCDDATE = ((pTime->Day/10) << 4) | ((pTime->Day%10) << 0);
	rBCDDAY = (pTime->Week+1); // Sunday = 0;
	rBCDHOUR = ((pTime->Hour/10) << 4) | ((pTime->Hour%10) << 0);
	rBCDMIN = ((pTime->Min/10) << 4) | ((pTime->Min%10) << 0);
	rBCDSEC = ((pTime->Sec/10) << 4) | ((pTime->Sec%10) << 0);
	rRTCCON &= ~(1 << 0);		
}

void RTC_Init(const RTC_Time *pTime)
{
	rRTCCON |= (1 << 0);		
	{ 
		rRTCCON = (5<<5) | (0<<4) | (1<<0); // Tick 1hz
		rBCDYEAR = ((pTime->Year%100/10) << 4 | (pTime->Year%100%10) << 0);
		rBCDMON = ((pTime->Month/10) << 4) | ((pTime->Month%10) << 0);
		rBCDDATE = ((pTime->Day/10) << 4) | ((pTime->Day%10) << 0);
		rBCDDAY = (pTime->Week+1); // Sunday = 0;
		rBCDHOUR = ((pTime->Hour/10) << 4) | ((pTime->Hour%10) << 0);
		rBCDMIN = ((pTime->Min/10) << 4) | ((pTime->Min%10) << 0);
		rBCDSEC = ((pTime->Sec/10) << 4) | ((pTime->Sec%10) << 0);	
	}
	rRTCCON &= ~(1 << 0);	
}

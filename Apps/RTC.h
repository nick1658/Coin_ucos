#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RTC_Time {
	unsigned short Year; 
	unsigned char Month;
	unsigned char Day;
	unsigned char Week;
	unsigned char Hour;
	unsigned char Min;
	unsigned char Sec;
} RTC_Time;	

void RTC_SetTime(RTC_Time *pTime);
void RTC_GetTime(RTC_Time *pTime);
void RTC_Init(const RTC_Time *pTime);
	
#ifdef __cplusplus
}
#endif

#endif /*__RTC_H__*/

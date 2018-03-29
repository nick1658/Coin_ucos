#include "s3c2416.h"



int16_t coin_log (char *fmt, ...)
{
	FATFS fs;
	FIL file;
	FRESULT Res;	
	va_list ap;
	char string[256];
	char log_str[256];
//	RTC_Time Time;

	va_start(ap, fmt);
	vsprintf(string, fmt, ap);
	sprintf((char *)log_str, "%s", string);
		
	if (Hsmmc_exist ()) {
		Res = f_mount(&fs, "" , 0);	
		Res =  f_open(&file, "coin-log.txt", FA_WRITE);
		if (Res != FR_OK)	{
			Res =  f_open(&file, "coin-log.txt", FA_WRITE | FA_CREATE_ALWAYS);
			if (Res != FR_OK) {
				dbg("Open file coin-log.txt failed, Res = %d\n", Res);	
				return -1;
			}
		}
	//	RTC_GetTime(&Time);
	//	
	//	sprintf((char *)log_str, "LOG: %4d-%02d-%02d %02d:%02d:%02d %s\n", 
	//						Time.Year, Time.Month, Time.Day, Time.Hour, Time.Min, Time.Sec,
	//						string);
		f_lseek(&file, file.fsize);
		f_printf (&file, "%s", log_str);	
		f_close(&file);
	}else{
		//cy_println ("SD Card not exist!");
	}
	cy_print ("%s", log_str);	
	
	va_end(ap);
	return 0;
}


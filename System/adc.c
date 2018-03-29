#include "S3C2416.h"


int ad0_value_changed = 0;

int ReadAdc0(void)
{
//	int temp = 0;
	
#ifdef __MY_DEBUG__
	if (sys_env.sim_count_flag)
	{
		return AD_Value_buf[sample_data_buf_index].AD0;
	}
	//#else
#endif
	rADCMUX = 0;		//setup channel
	rADCCON|=0x1;									//start ADC
	ad0_value_changed = 1;
	
//	temp = rTCNTO4;
	while(rADCCON & 0x1);							//check if Enable_start is low
	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
//	temp -= rTCNTO4;
//	if (temp > 0)
//		dbg ("AD convert timer = %d ns\n", (temp * 360));

	return ( (int)rADCDAT0 & 0x3ff );
}
int ReadAdc1(void)
{
//	int temp = 0;
	
	#ifdef __MY_DEBUG__
	if (sys_env.sim_count_flag)
	{
		return AD_Value_buf[sample_data_buf_index].AD1;
	}
	//#else
	#endif
	
	rADCMUX = 0x01;		//setup channel
	rADCCON|=0x1;									//start ADC

//	temp = rTCNTO4;
	while(rADCCON & 0x1);							//check if Enable_start is low
	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
//	temp -= rTCNTO4;
//	if (temp > 0)
//		dbg ("AD convert timer = %d ns\n", (temp * 360));

	return ( (int)rADCDAT0 & 0x3ff );
}
int ReadAdc2(void)
{
//	int temp = 0;
	#ifdef __MY_DEBUG__
	if (sys_env.sim_count_flag)
	{
		return AD_Value_buf[sample_data_buf_index].AD2;
	}
	//#else
	#endif
	
	rADCMUX = 0x02;		//setup channel
	rADCCON|=0x1;									//start ADC

//	temp = rTCNTO4;
	while(rADCCON & 0x1);							//check if Enable_start is low
	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
//	temp -= rTCNTO4;
//	if (temp > 0)
//		dbg ("AD convert timer = %d ns\n", (temp * 360));

	return ( (int)rADCDAT0 & 0x3ff );
}

int ReadAdc3(void) //温度传感器AD采集 LM61
{
	rADCMUX = 0x03;		//setup channel
	rADCCON|=0x1;									//start ADC

	while(rADCCON & 0x1);							//check if Enable_start is low
	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high

	return ( (int)rADCDAT0 & 0x3ff );
}

#define REQCNT 100 
#define ADC_FREQ  2500000   //最大转换时间:1路10BIT是500Ksps 三位的话，500/3=166KSPS 还不算切换时间
//#define ADC_FREQ  5000000   //最大转换时间:1路10BIT是1000Ksps 三位的话，1000/3=333KSPS 还不算切换时间

void adc_init(void)    //初始化ADC 
{
	int m=0; //Initialize variables
//	preScaler = ADC_FREQ;
    uint32_t preScaler = 66;
	preScaler = PCLK/ADC_FREQ -1;   //PCLK =  M 
	cy_println("PCLK/ADC_FREQ - 1 = %d",preScaler);
	rADCCON = (1<<14)|(preScaler<<6)|(0<<3);		//使能 分频 精度

	m=ReadAdc0(); 	
	cy_println( "AIN0: %d", m);
	m=ReadAdc1(); 	
	cy_println( "AIN1: %d", m);
	m=ReadAdc2(); 	
	cy_println( "AIN2: %d", m);
	m=ReadAdc3(); 	
	cy_println( "AIN3: %d", m);

}

void delay_ms(unsigned int t)
{
	tscount =t;
	while (tscount) {;}
//	cy_print("delay is over\r\n");
	
}
////////////////////////////////////////////////



#ifndef __TQ2416_ADC_H__
#define __TQ2416_ADC_H__

#include "yqcoin.h" 

void adc_init(void);    //初始化ADC 
int ReadAdc0(void);
int ReadAdc1(void);
int ReadAdc2(void);
int ReadAdc3(void);

extern  U8 cn0copmaxc0[COINCNUM]; //20 //鉴别值 100元真  200欧  100美元  200英镑 
extern  U8 cn0copminc0[COINCNUM];  //20 
extern  U8 cn0copmaxc1[COINCNUM];  //60
extern  U8 cn0copminc1[COINCNUM]; //30
extern  U8 cn0copmaxc2[COINCNUM]; //40
extern  U8 cn0copminc2[COINCNUM]; //40


#endif




#ifndef __TQ2416_YQFILE_H__
#define __TQ2416_YQFILE_H__

#include "def.h"
extern S8 alertflag;  //报错标志位

//根据币种 决定 各类值
//币种定义
#define CN0 0   // 人民币
#define CN1 1   // 欧元
#define CN2 2   // 美元
#define CN3 3   // 英镑

///////tubiao number of coin name 
#define CNNM0 0   // 1元
#define CNNM1 1		// 5角铜
#define CNNM2 2		// 5角钢 
#define CNNM3 3		// 1角大铝
#define CNNM4 4		// 1角小钢
#define CNNM5 5		// 5分
#define CNNM6 6		// 2分
#define CNNM7 7		// 1分
#define CNNM8 8		// 1角小铝

extern U8 coinchoose;    // 币种选择
void yqi2c_init(void);   //硬币清分机 I2C初始化 
extern U8 dgus_readt[6];  // only need y-m-d h:m  读时间
void print_func(void);   //打印 函数 






#endif

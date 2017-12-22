
#ifndef __TQ2416_YQCOIN_H__
#define __TQ2416_YQCOIN_H__



#include "def.h"
#include "yqsqlite3.h"


typedef struct
{
	U32 total_money;
	U32 total_good;
	U32 total_ng;
	U32 total_coin;
	U32 coinnumber;
	S16 ad2_averaged_value;
}s_processed_coin_info;

extern s_processed_coin_info processed_coin_info;

#define COINCNUM 4     //目前 可以 计数的 国家币种  的总数量 
extern int alertname[COIN_TYPE_NUM];// 币种决定 

extern U32 coin_num[COIN_TYPE_NUM];    //各币种 计数常量
void cy_precoincount(void);	// 鉴伪计数
extern  U16 ccstep;

extern  U32 ch0_counttemp;  // 前一次通道0 通过的硬币计数 
extern  U32 ch1_counttemp;  // 前一次通道1 通过的硬币计数 
extern  U32 ch2_counttemp;  // 前一次通道2 通过的硬币计数 
extern  U32 coinlearnnumber;

void cy_coinlearn(void);   //自学习的程序

extern  S16 coin_maxvalue0;
extern  S16 coin_minvalue0;
extern  S16 coin_maxvalue1;
extern  S16 coin_minvalue1;
extern  S16 coin_maxvalue2;
extern  S16 coin_minvalue2;


extern  U16 coine[COINCNUM][COIN_TYPE_NUM];

void detect_read(void);   

#define FREE_Q_FLAG 0xA0
#define COIN_NG_FLAG 0xA1
#define COIN_FULL_FLAG 0xA2
#define COIN_GOOD_FLAG 0xA3
#define COIN_PROCESSED_FLAG 0xA4

/*****************
报错宏定义    函数
*****************/

#define SWITCHERROR 11      //门没有关上
#define COINNGKICKERROR 12
#define DETECTERROR 13     // 币斗满 
#define KICKCOINFULL 14     //转盘赌币
#define KICK1COINERROR 15
#define KICK2COINERROR 16
#define PRESSMLOCKED  17      //压币带堵币
#define COUNT_FINISHED 18  	// 1 MCU接到 有清分斗满币 停止	
#define ADSTDEEROR    20       //表示传感器下有币
#define RTCREEROR    21       //表示READ time wrong
#define READOUTDATA    22       // 读出  数据  内存满
#define COMPLETE_UPDATE 30


#define ALERT0 0  //向地址ADDR_TTS 发送"请调基准值"
#define ALERT1 1   // //向地址ADDR_TTS 发送"请调基准值"
#define ALERT3 3  //向地址ADDR_TTS 发送"转盘堵币"
#define ALERT4 4   // //向地址ADDR_TTS 发送"压币带堵币"
#define ALERT5 5   // 踢币异常
#define ALERT6 6  //向地址ADDR_TTS 发送"币斗满"
#define ALERT7 7   // //向地址ADDR_TTS 发送"确认需长按1秒生效"
#define ALERT8 8   // //向地址ADDR_TTS 发送"请导出数据，内存已满"
#define ALERT9 9   // //向地址ADDR_TTS 发送"清分斗满币 请取出"


#define ALERT20 20   // //向地址ADDR_TTS 发送"一元预置值已到"
#define ALERT21 21   // //向地址ADDR_TTS 发送"0.5元预置值已到"
#define ALERT22 22   // //向地址ADDR_TTS 发送"0.1元预置值已到"
#define ALERT23 23   // //向地址ADDR_TTS 发送"0.05元预置值已到"
#define ALERT24 24   // //向地址ADDR_TTS 发送"0.02元预置值已到"
#define ALERT25 25   // //向地址ADDR_TTS 发送"0.01元预置值已到"
#define ALERT26 26   // //向地址ADDR_TTS 发送"10元预置值已到"
#define ALERT27 27   // //向地址ADDR_TTS 发送"5元预置值已到"

extern  U16 prepic_prenum;      // 用于记录 报错前的界面 
void alertfuc(U16 alertflag); //报错






#endif


#ifndef __TQ2416_YQCOIN_H__
#define __TQ2416_YQCOIN_H__

#include "def.h"
#include "yqsqlite3.h"



typedef struct
{
	uint32_t total_money;
	uint32_t total_good;
	uint32_t total_ng;
	uint32_t total_coin;
	uint32_t total_coin_old;
	uint32_t coinnumber;
	int16_t ad2_averaged_value;
	int16_t coin_ctr[COIN_TYPE_NUM];
}s_processed_coin_info;

extern s_processed_coin_info processed_coin_info;

#define COINCNUM 4     //目前 可以 计数的 国家币种  的总数量 

void cy_precoincount(void);	// 鉴伪计数
extern  uint16_t ccstep;

extern  uint32_t coinlearnnumber;

void cy_coinlearn(void);   //自学习的程序

extern  int16_t coin_maxvalue0;
extern  int16_t coin_minvalue0;
extern  int16_t coin_maxvalue1;
extern  int16_t coin_minvalue1;
extern  int16_t coin_maxvalue2;
extern  int16_t coin_minvalue2;


extern  uint16_t coine[COINCNUM][COIN_TYPE_NUM];

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
#define KICKCOINNGERROR 19
#define ADSTDEEROR    20       //表示传感器下有币
#define RTCREEROR    21       //表示READ time wrong
#define READOUTDATA    22       // 读出  数据  内存满
#define DETECT_COUNT_ERROR 31
#define COMPLETE_UPDATE 30


#define COIN_KICK_OP_0() { \
		sys_env.coin_ng_flag = 0; \
		if (coin_env.kick_Q[coin_env.kick_Q_index] == 0){ \
			coin_env.coin_Q2[coin_env.coin_Q2_remain] = COIN_NG_FLAG;/*假币标志*/ \
			coin_env.kick_Q[coin_env.kick_Q_index] = para_set_value.data.kick_start_delay_t1; \
			coin_env.kick_Q_index++; \
			coin_env.kick_Q_index %= KICK_Q_LEN; \
		}else{/*剔除工位1队列追尾错误*/ \
			SEND_ERROR(KICK1COINERROR); \
			dbg ("kick1 error alertflag = %d %s, %d", KICK1COINERROR,  __FILE__, __LINE__); \
		} \
}



extern  uint16_t prepic_prenum;      // 用于记录 报错前的界面 
void alertfuc(uint16_t alertflag); //报错






#endif

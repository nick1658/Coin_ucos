
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

#define COINCNUM 4     //Ŀǰ ���� ������ ���ұ���  �������� 

void cy_precoincount(void);	// ��α����
extern  uint16_t ccstep;

extern  uint32_t coinlearnnumber;

void cy_coinlearn(void);   //��ѧϰ�ĳ���

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
����궨��    ����
*****************/

#define SWITCHERROR 11      //��û�й���
#define COINNGKICKERROR 12
#define DETECTERROR 13     // �Ҷ��� 
#define KICKCOINFULL 14     //ת�̶ı�
#define KICK1COINERROR 15
#define KICK2COINERROR 16
#define PRESSMLOCKED  17      //ѹ�Ҵ��±�
#define COUNT_FINISHED 18  	// 1 MCU�ӵ� ����ֶ����� ֹͣ	
#define KICKCOINNGERROR 19
#define ADSTDEEROR    20       //��ʾ���������б�
#define RTCREEROR    21       //��ʾREAD time wrong
#define READOUTDATA    22       // ����  ����  �ڴ���
#define DETECT_COUNT_ERROR 31
#define COMPLETE_UPDATE 30


#define COIN_KICK_OP_0() { \
		sys_env.coin_ng_flag = 0; \
		if (coin_env.kick_Q[coin_env.kick_Q_index] == 0){ \
			coin_env.coin_Q2[coin_env.coin_Q2_remain] = COIN_NG_FLAG;/*�ٱұ�־*/ \
			coin_env.kick_Q[coin_env.kick_Q_index] = para_set_value.data.kick_start_delay_t1; \
			coin_env.kick_Q_index++; \
			coin_env.kick_Q_index %= KICK_Q_LEN; \
		}else{/*�޳���λ1����׷β����*/ \
			SEND_ERROR(KICK1COINERROR); \
			dbg ("kick1 error alertflag = %d %s, %d", KICK1COINERROR,  __FILE__, __LINE__); \
		} \
}



extern  uint16_t prepic_prenum;      // ���ڼ�¼ ����ǰ�Ľ��� 
void alertfuc(uint16_t alertflag); //����






#endif

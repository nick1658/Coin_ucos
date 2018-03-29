
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
#define ADSTDEEROR    20       //��ʾ���������б�
#define RTCREEROR    21       //��ʾREAD time wrong
#define READOUTDATA    22       // ����  ����  �ڴ���
#define COMPLETE_UPDATE 30



extern  uint16_t prepic_prenum;      // ���ڼ�¼ ����ǰ�Ľ��� 
void alertfuc(uint16_t alertflag); //����






#endif

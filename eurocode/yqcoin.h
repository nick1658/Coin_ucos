
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

#define COINCNUM 4     //Ŀǰ ���� ������ ���ұ���  �������� 

void cy_precoincount(void);	// ��α����
extern  U16 ccstep;

extern  U32 coinlearnnumber;

void cy_coinlearn(void);   //��ѧϰ�ĳ���

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



extern  U16 prepic_prenum;      // ���ڼ�¼ ����ǰ�Ľ��� 
void alertfuc(U16 alertflag); //����






#endif


#ifndef __TQ2416_YQADC_H__
#define __TQ2416_YQADC_H__


#include "def.h"
#include "yqsqlite3.h"

#define KICK_Q_LEN 2
#define FULL_KICK_Q_LEN 2
#define COIN_Q_LEN 16
#define SAMPLE_METHOD_1

typedef struct _AD_Value_S
{
	int16_t index;
	int16_t coin_id;
	int16_t use_index;
	int16_t AD0;
	int16_t AD1;
	int16_t AD2;
	int16_t ad_index;
} AD_Value;

typedef struct
{
	int16_t compare_min0;
	int16_t compare_max0;
	int16_t compare_min1;
	int16_t compare_max1;
	int16_t compare_min2;
	int16_t compare_max2;
}s_coin_compare_value;

typedef struct
{
	int16_t std_down_value0;
	int16_t std_up_value0;
	int16_t ad0_averaged_value;
	int16_t ad1_averaged_value;
	int16_t ad2_averaged_value;
	int16_t ad0_step;
	int16_t ad1_step;
	int16_t ad2_step;
	int16_t process_coin_index;
	int16_t cmp_use_index;
	int16_t ad_index;
	uint16_t AD_min_index[3];
	
	uint16_t full_stack_num;
	uint16_t full_coin_stack[COIN_TYPE_NUM]; //���Ҷ�ջ
	
	uint16_t kick_remain;
	uint16_t kick_keep_t1;
	uint16_t kick_Q[KICK_Q_LEN];//�޳�����
	uint16_t kick_Q_index;
	
	uint16_t full_kick_remain;
	uint16_t full_kick_keep_t2;
	uint16_t full_kick_Q[FULL_KICK_Q_LEN];//�޳�����
	uint16_t full_kick_Q_index;

	uint16_t coin_Q1[COIN_Q_LEN];//Ӳ�Ҿ�������
	uint16_t coin_Q1_index;
	uint16_t coin_Q1_remain;
	
	uint16_t coin_Q2[COIN_Q_LEN];//Ӳ�Ҿ�������
	uint16_t coin_Q2_index;
	uint16_t coin_Q2_remain;
	
	uint16_t coin_detect_ctr;
	uint16_t coin_ir_ctr1;
	uint16_t kick1_ctr;
	uint16_t coin_ir_ctr2;
	uint16_t kick2_ctr;
	uint16_t inhibit_coin[COIN_TYPE_NUM]; //����Ӳ������λ
	
	
	uint16_t ir_filter_time;
	uint16_t ir_in_time;
	uint16_t min_ir_in_time;
	uint16_t max_ir_in_time;
	uint16_t ir_interval_time;
	uint16_t min_ir_interval_time;
	uint16_t max_ir_interval_time;
}s_coin_env;

//����������    �����  ������������

//#define AD0STDVALUE  pre_value.country[coinchoose].coin[CNNM0].std0   //800   //3.25    ���ڲ��Բ��ȵĳ���
//#define AD1STDVALUE  pre_value.country[coinchoose].coin[CNNM0].std1  //940   //3.25
//#define AD2STDVALUE  pre_value.country[coinchoose].coin[CNNM0].std2  //940   //3.25
#define ADOFFSET  40


#define ADSAMPNUM0  10    //every ad channel sample number for getting average value 2  8
#define ADSAMPNUM  50    //every ad channel sample number for getting average value 2  8

#define  WAVE_DOWN_TO_STD_N  32   //1  //8ȷ�ϲ����½� ����  4
#define  WAVE_GO_UP_N  32 //1  //8 15   //ȷ�ϲ��η�ֵ  ���� 50 40   65     20
#define  WAVE_UP_TO_STD_N 32 //2 //15  //������ص��ο�ֵ���µļ���20
#define  WAVE_DOWN_TWO_N  32  //3 //4//8   // ȷ��˫����  ����һ ȷ�ϲ�������  ����   10 4      �󵼷�  3

#define WAVE_DOWN_VALUE 50
#define WAVE_UP_VALUE 50
#define WAVE_COIN_TWO_UP_VALUE 15
#define WAVE_COIN_TWO_DOWN_VALUE 20


//��· ��׼ֵ ��׼ֵ ����   ��Ϊд�뵽1Ԫֵ��ȥ�������ο���ֵ ���бҵĳ�ʼ��Χ
#define AD0STDSET 900    //��Ƶ
#define AD1STDSET 900	 //��Ƶ
#define AD2STDSET 900	 //��Ƶ
#define ADADJACTERTIME 50


extern  uint16_t ad0_min;     //����ÿöӲ�ҹ�ȥ��� ���ֵ
extern  uint16_t ad1_min;     //����ÿöӲ�ҹ�ȥ��� ���ֵ
extern  uint16_t ad2_min;     //����ÿöӲ�ҹ�ȥ��� ���ֵ

#define ADBLOCKT  20  //10  //30     //130 00---30     //ʱ�� ��α������ ���� �±�ʱ��
extern volatile uint16_t blockflag;      //�±ұ�־����
extern volatile uint32_t adtime;

extern uint32_t ch0_count;  //ͨ��0 ͨ����Ӳ�Ҽ���

extern uint32_t ch0_coin_come;  //ͨ�� ��Ӳ�� ���
extern uint32_t coin_cross_time;
extern uint32_t ch0_pre_count;  //ͨ��0 ��Ӳ�Ҿ��� ���

extern  uint16_t std_ad0;
extern  uint16_t std_ad1;
extern  uint16_t std_ad2;

void ad2_valueget(void);  //adc function   get value
void ad1_valueget(void);  //adc function   get value
void ad0_valueget(void);  //adc function   get value


extern  int temperstd;   //20��  20*10 +600  = 800MV;  800/3300 *1024 = 248.24  // �����������ѹ��ʽ VO = (10mV/C * T) + 600
uint16_t adstd_test(void);   //ֻ���ڵ�ѹֵ�ڱ�׼״̬ʱ�ſ����������еĵ����ADת��  ������ֵ������ 1010���Ұ�
uint16_t adstd_sample(void);    //��׼ֵ����
uint16_t cy_adstd_adj (void);
void print_std_value(void);
void coin_env_init (void);

#define AD0STDNUM 10    //���׼ֵ ʱ�Ĳ�������
#define AD1STDNUM 10
#define AD2STDNUM 10


uint16_t adstd_offset(void);
#define OFFSETMAX 200
#define OFFSETMIN 200   //990


#define OFFSET1MAX 200
#define OFFSET1MIN 200   //990

#define OFFSET2MAX 200
#define OFFSET2MIN 200   //990


extern  s_coin_compare_value coin_cmp_value[COIN_TYPE_NUM];



/////////////////////////////////
#define AD_BUF_GROUP_LEN 2
#define DATA_BUF_LENGTH	2*1024
#define ADJ_BUF_LENGTH 4096
#define NG_BUF_LENGTH	4096
#define GOOD_BUF_LENGTH	4096
extern AD_Value AD_Value_buf[DATA_BUF_LENGTH];

//extern volatile AD_Value Adj_AD_Value_buf[ADJ_BUF_LENGTH];
extern AD_Value Detect_AD_Value_buf[AD_BUF_GROUP_LEN][DATA_BUF_LENGTH];
extern AD_Value *Detect_AD_Value_buf_p;


extern volatile AD_Value NG_value_buf[NG_BUF_LENGTH];//��Ҳ������ݻ�����
extern volatile AD_Value GOOD_value_buf[GOOD_BUF_LENGTH];//��Ҳ������ݻ�����
extern volatile uint32_t ng_value_index;
extern volatile uint32_t good_value_index;

extern volatile uint32_t sample_data_buf_index;
extern volatile uint32_t detect_sample_data_buf_index;
extern volatile uint32_t adj_data_buf_index;

extern volatile uint32_t start_sample;
//extern int16_t coin_env.ad0_averaged_value;

extern int ad0_value_changed;

extern void send_sample_data (AD_Value ad_value_buf[], int counter);

void setStdValue (void);

void AD_Sample_All (void);

extern int L_H_min;
extern int L_M_min;
extern int M_H_min;

extern int L_H_max;
extern int L_M_max;
extern int M_H_max;


extern s_coin_env coin_env;

void cy_ad0_valueget(void);
void cy_ad1_valueget(void);
void cy_ad2_valueget(void);

#endif


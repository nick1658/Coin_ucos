
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
	uint16_t full_coin_stack[COIN_TYPE_NUM]; //满币堆栈
	
	uint16_t kick_remain;
	uint16_t kick_keep_t1;
	uint16_t kick_Q[KICK_Q_LEN];//剔除队列
	uint16_t kick_Q_index;
	
	uint16_t full_kick_remain;
	uint16_t full_kick_keep_t2;
	uint16_t full_kick_Q[FULL_KICK_Q_LEN];//剔除队列
	uint16_t full_kick_Q_index;

	uint16_t coin_Q1[COIN_Q_LEN];//硬币经过队列
	uint16_t coin_Q1_index;
	uint16_t coin_Q1_remain;
	
	uint16_t coin_Q2[COIN_Q_LEN];//硬币经过队列
	uint16_t coin_Q2_index;
	uint16_t coin_Q2_remain;
	
	uint16_t coin_detect_ctr;
	uint16_t coin_ir_ctr1;
	uint16_t kick1_ctr;
	uint16_t coin_ir_ctr2;
	uint16_t kick2_ctr;
	uint16_t inhibit_coin[COIN_TYPE_NUM]; //拒收硬币设置位
	
	
	uint16_t ir_filter_time;
	uint16_t ir_in_time;
	uint16_t min_ir_in_time;
	uint16_t max_ir_in_time;
	uint16_t ir_interval_time;
	uint16_t min_ir_interval_time;
	uint16_t max_ir_interval_time;
	
	uint16_t motor_test_flag;
	uint16_t motor_test_status;
	uint16_t motor_test_ctr;
}s_coin_env;

//各函数申明    步骤号  参数变量定义

//#define AD0STDVALUE  pre_value.country[coinchoose].coin[CNNM0].std0   //800   //3.25    用于测试波谷的程序
//#define AD1STDVALUE  pre_value.country[coinchoose].coin[CNNM0].std1  //940   //3.25
//#define AD2STDVALUE  pre_value.country[coinchoose].coin[CNNM0].std2  //940   //3.25
#define ADOFFSET  40


#define ADSAMPNUM0  10    //every ad channel sample number for getting average value 2  8
#define ADSAMPNUM  50    //every ad channel sample number for getting average value 2  8

#define  WAVE_DOWN_TO_STD_N  32   //1  //8确认波形下降 次数  4
#define  WAVE_GO_UP_N  32 //1  //8 15   //确认波形峰值  次数 50 40   65     20
#define  WAVE_UP_TO_STD_N 32 //2 //15  //波形落回到参考值以下的计数20
#define  WAVE_DOWN_TWO_N  32  //3 //4//8   // 确认双波峰  方法一 确认波形上升  次数   10 4      求导法  3

#define WAVE_DOWN_VALUE 50
#define WAVE_UP_VALUE 50
#define WAVE_COIN_TWO_UP_VALUE 15
#define WAVE_COIN_TWO_DOWN_VALUE 20


//三路 基准值 标准值 设置   此为写入到1元值里去作基本参考的值 所有币的初始范围
#define AD0STDSET 900    //高频
#define AD1STDSET 900	 //中频
#define AD2STDSET 900	 //低频
#define ADADJACTERTIME 50


extern  uint16_t ad0_min;     //贮存每枚硬币过去后的 最大值
extern  uint16_t ad1_min;     //贮存每枚硬币过去后的 最大值
extern  uint16_t ad2_min;     //贮存每枚硬币过去后的 最大值

#define ADBLOCKT  20  //10  //30     //130 00---30     //时间 鉴伪传感器 鉴别 堵币时间
extern volatile uint16_t blockflag;      //堵币标志变量
extern volatile uint32_t adtime;

extern uint32_t ch0_count;  //通道0 通过的硬币计数

extern uint32_t ch0_coin_come;  //通道 来硬币 标记
extern uint32_t coin_cross_time;
extern uint32_t ch0_pre_count;  //通道0 有硬币经过 标记

extern  uint16_t std_ad0;
extern  uint16_t std_ad1;
extern  uint16_t std_ad2;

void ad2_valueget(void);  //adc function   get value
void ad1_valueget(void);  //adc function   get value
void ad0_valueget(void);  //adc function   get value


extern  int temperstd;   //20度  20*10 +600  = 800MV;  800/3300 *1024 = 248.24  // 传感器输出电压公式 VO = (10mV/C * T) + 600
uint16_t adstd_test(void);   //只有在电压值在标准状态时才可以启动所有的电机与AD转换  本程序值控制在 1010左右吧
uint16_t adstd_sample(void);    //基准值调试
uint16_t cy_adstd_adj (void);
void print_std_value(void);
void coin_env_init (void);

#define AD0STDNUM 10    //求标准值 时的采样个数
#define AD1STDNUM 10
#define AD2STDNUM 10


uint16_t adstd_offset(void);
uint16_t calc_fb_func (uint16_t ch, uint16_t save_base_v, uint16_t new_base_v, uint16_t save_coin_v);
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


extern volatile AD_Value NG_value_buf[NG_BUF_LENGTH];//异币采样数据缓冲区
extern volatile AD_Value GOOD_value_buf[GOOD_BUF_LENGTH];//异币采样数据缓冲区
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


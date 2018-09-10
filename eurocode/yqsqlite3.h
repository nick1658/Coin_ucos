
#ifndef __TQ2416_YQSQLITE3_H__
#define __TQ2416_YQSQLITE3_H__

#include "def.h"

#define APP_NAND_ADDR 10*64*2048 //Ӧ�ó����ŵ�ַ ΪBlock 10 Page 0��
#define APP_NAND_SIZE 256*1024 //Ӧ�ó����С�ݶ�Ϊ256K (��ռ2��BLOCK)

#define COUNTRY0_COIN_PRE_VALUE_START_BLOCK_NUM PUBULIC_DATA_START_BLOCK_NUM
#define COUNTRY0_COIN_PRE_VALUE_START_PAGE_NUM (2 + PUBULIC_DATA_START_PAGE_NUM)
#define COUNTRY0_COIN_PRE_VALUE_START_ADDR (DATA_START_BLOCK + COUNTRY0_COIN_PRE_VALUE_START_PAGE_NUM * 2048)
#define COIN_TYPE_NUM 11
#define NORMAL_COIN_TYPE_NUM 6
#define COUNTRY_NUM 16
#define COIN_NUM 8

#define MAGIC_NUM 0xA55A


#define DBINSERT 1
#define DBDELETE 2
#define DBDISPLAY 3
#define DBDISPLAYBACK 4

#define HISTORYSAVANUM (64)

#define PAGE_BYTE_SIZE (2048)
#define BLOCK_PAGE_SIZE (64)
#define BLOCK_BYTE_SIZE (PAGE_BYTE_SIZE * BLOCK_PAGE_SIZE)

#define ITEM_SIZE HISTORYSAVANUM  //��¼�Ĵ�С
#define PAGE_ITEM_NUM_SIZE (PAGE_BYTE_SIZE / ITEM_SIZE) //һҳ�м�¼������


						//����				//����					   //���� //���				//����				//���
/////////////////////////(BLOCK * (64 * 2048) + PAGE)
#define PUBULIC_DATA_START_BLOCK_NUM 20
#define PUBULIC_DATA_START_PAGE_NUM 0
#define HISTORY_START_BLOCK_NUM (PUBULIC_DATA_START_BLOCK_NUM + 1)
#define HISTORY_START_PAGE_NUM (0)

#define DATA_START_BLOCK  (PUBULIC_DATA_START_BLOCK_NUM	* (64 * 2048) + PUBULIC_DATA_START_PAGE_NUM * 2048)  //���湫����Ϣҳ START BLOCK 20 PAGE 0
#define YQNDHISTORYBLOCK (HISTORY_START_BLOCK_NUM * (64 * 2048))  //nations������ʷ ��Ϣҳ����0-50��-3200ҳ 0-2048  22���ֽ�   4--110��--7040 ������93�Ķ��ٱ��ͻ��Ӷ���ҳ

// ��ʼ�������  ����д��  �ж�   ������ʾ �����Ҳ���� ���� ����дһ���������ݿ�ķ�ʽ  �����  ����д ɾ�����ݿ������
#define YQNDHISTORYADDR (YQNDHISTORYBLOCK + (para_set_value.data.db_total_item_num * ITEM_SIZE) / PAGE_BYTE_SIZE)  //nations������ʷ ��  ��ַ  ������ ���ұ��־��� ҳ

#define YQNDHISTORY_DB_ID_PAGE_ADDR (YQNDHISTORYBLOCK + (db_id * ITEM_SIZE / PAGE_BYTE_SIZE))


#define COUNTRY_ID 0
#define ALL_COIN

#ifdef ALL_COIN
#define MONEY_10_00 1000
#define MONEY_5_00 	500
#define MONEY_1_00 	100
#define MONEY_0_50 	50
#define MONEY_0_10 	10
#define MONEY_0_05 	5
#define MONEY_0_02 	2
#define MONEY_0_01 	1
#else
#define MONEY_10_00 100
#define MONEY_5_00 	50
#define MONEY_1_00 	10
#define MONEY_0_50 	5
#define MONEY_0_10 	1
#define MONEY_0_05 	0.5
#define MONEY_0_02 	0.2
#define MONEY_0_01 	0.1
#endif


typedef struct
{
	char  time[20];
	uint16_t	index;
	uint16_t ID;
//	uint16_t m_1yuan;
//	uint16_t m_5jiao;
//	uint16_t m_1jiao;
//	uint16_t m_1jiao_big;
//	uint16_t m_5fen;
//	uint16_t m_2fen;
//	uint16_t m_1fen;
//	uint16_t m_10yuan;
//	uint16_t m_5yuan;
	uint16_t coin_ctr[COIN_TYPE_NUM];
	uint16_t total_good;
	uint16_t total_ng;
	uint32_t total_money;
	//uint16_t coin_speed;
}s_db_item_info;


typedef union
{
	U8 fill[ITEM_SIZE];
	s_db_item_info data;
}u_db_item_info;

extern u_db_item_info db_item_info;

typedef struct
{
	s_db_item_info item_info_array[PAGE_BYTE_SIZE / ITEM_SIZE];
}s_db_item_block;


typedef struct
{
	uint16_t magic_num;
	uint16_t db_total_item_num;
	uint16_t country_id;
	uint16_t coin_id;
	uint16_t op_id;
	uint16_t rej_level;
	//---------------------------------
	uint32_t precoin_set_num[COIN_TYPE_NUM];
	uint32_t total_money;
	uint32_t total_good;
	uint32_t total_ng;
	uint16_t kick_start_delay_t1;
	uint16_t kick_keep_t1;
	uint16_t kick_start_delay_t2;
	uint16_t kick_keep_t2;
	uint16_t motor_idle_t;
	uint16_t pre_count_stop_n;
	uint16_t coin_full_rej_pos;
	uint16_t adj_offset_position;
	uint16_t system_mode;
	uint16_t system_boot_delay;
	uint16_t coin_min_trig_time;
	uint32_t coin_size;
	uint32_t coin_push_size;
	uint32_t coin_h;
}s_coin_parameter_value;

typedef union
{
	U8 fill[PAGE_BYTE_SIZE];
	s_coin_parameter_value data;
}u_coin_parameter_value;

extern u_coin_parameter_value para_set_value;


typedef struct
{
	int16_t max0;
	int16_t min0;
	int16_t max1;
	int16_t min1;
	int16_t max2;
	int16_t min2;
	int16_t std0;
	int16_t std1;
	int16_t std2;
	int16_t offsetmax0;
	int16_t offsetmin0;
	int16_t offsetmax1;
	int16_t offsetmin1;
	int16_t offsetmax2;
	int16_t offsetmin2;
	uint32_t * p_pre_count_set;
	uint32_t * p_pre_count_cur;
	uint32_t * p_pre_count_full_flag;
	uint32_t * p_coinval;
	uint32_t coin_type;
	uint32_t accept;
	uint16_t hmi_pre_count_set_addr;
	uint16_t money;
}s_coin_cmp_value;

typedef union
{
	U8 fill[64];
	s_coin_cmp_value data;
}u_coin_cmp_value;

typedef union
{
	U8 fill[PAGE_BYTE_SIZE];
	u_coin_cmp_value coin[COIN_TYPE_NUM];

}u_coin_pre_value;

typedef struct
{
	u_coin_pre_value country[COUNTRY_NUM];
}s_country_coin;

extern s_country_coin pre_value;

extern volatile char dbsave[HISTORYSAVANUM];




void Writekick_value(void);		//д�� ��ǰ�߱�ʱ��  �߱ҳ���ʱ�� ת�̶�תʱ��  ���ʹ�����ʱ��

void read_para (void);
void write_para (void);
void read_coin_value(void); 	 // read  COIN  0--8
void write_coin_value (void);

//void Writecoinall_value(uint16_t coin);   	//write coin 0--8




void ini_picaddr(void); //�����л�ʱ�� ��ʼ����ַ����
void ini_screen (void);


void initial_nandflash(void);    //nandflash

void yqsql_exec(uint16_t chos);

void export_record (void);
void export_detect_data (void);

int16_t test_read_r_code (int16_t r_code);
int16_t test_write_r_code (int16_t r_code);
int16_t test_erase_r_code (int16_t r_code);



#endif

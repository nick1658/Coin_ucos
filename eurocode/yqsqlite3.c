/*============================================================================
 Name                : sqlite3.c
 Author              : linxijun
============================================================================*/
#include "s3c2416.h"

//2048块 1块64页 1页2048字节  1块= 64*2048 = 131072 =128K  第N块的地址  64*2048*N + *
//程序 现在为107K  前两块 给程序 可以么？与ID的保存会冲突么？  2*64*2048= 256K 两块 一块128K
//前10块不要碰它。 一个块为一个币种

// 10*64*2048 基地址 第10块存公共内容
//第11块开始 下一个币种的 起始地址： ((11 + 币种名称)*64+页数)*2048 + 页内地址 第一页作 单币种的参数存储  2048个字节
//第二页开始 作历史数据保存 ((11 + 币种名称)*64+页数)*2048+ (2048+页内地址( 即条数*HISTORYSAVANUM))作为 一条基址， 保存22次
//1000提示保存已满，请导出。
//20150407之前是a5 5a
//20150408 SD启动程序卡 为a7 7a
//20150408 无单片机SD启动程序卡为a6 6a
#define FLAG1NANDFLASH	0xa5
#define FLAG2NANDFLASH	0x5a

u_coin_parameter_value para_set_value;
s_country_coin pre_value;
u_db_item_info db_item_info;

int16_t test_read_r_code (int16_t r_code)
{
	if (r_code == 0){
		//cy_println ("Read OP Complete");
		return 0;
	}
	else if (r_code == 1)
	{
		cy_println (" Parameter Error, Buf must be not null!!!");
	}
	else if (r_code == 2)
	{
		cy_println ("ECC Error, Read Op failed!!!");
	}
	return -1;
}
int16_t test_write_r_code (int16_t r_code)
{
	if (r_code == 0)
	{
		//cy_println ("Write OP Complete");
		return 0;
	}
	else if (r_code == 1)
	{
		cy_println (" Parameter Error, Buf must be not null!!!");
	}
	else if (r_code == 2)
	{
		cy_println ("BAD BLOCK, Write failed!!!");
	}
	else if (r_code == 3)
	{
		cy_println ("BAD BLOCK, Mark failed!!!");
	}
	else if (r_code == 4)
	{
		cy_println ("BAD BLOCK, Mark Succeesfull!!!");
	}
	return -1;
}
int16_t test_erase_r_code (int16_t r_code)
{
	if (r_code == 0)
	{
		//cy_println ("Erase OP Complete");
		return 0;
	}
	else if (r_code == 3)
	{
		cy_println ("BAD BLOCK, Mark failed!!!");
	}
	else if (r_code == 4)
	{
		cy_println ("BAD BLOCK, Mark Succeesfull!!!");
	}
	return -1;
}

void initial_nandflash(void)    //nandflash
{

	uint16_t i=0, j;

	cy_println ("Start Check Data ...!");
	cy_println ("sizeof (s_coin_parameter_value)   = %d", sizeof (s_coin_parameter_value));
	cy_println ("sizeof (u_coin_cmp_value) = %d", sizeof (u_coin_cmp_value));

	cy_println ("sizeof (para_set_value)   = %d", sizeof (para_set_value));
	cy_println ("sizeof (u_coin_pre_value) = %d", sizeof (u_coin_pre_value));

	cy_println ("sizeof (s_db_item_info) = %d", sizeof (s_db_item_info));
	cy_println ("sizeof (u_db_item_info) = %d", sizeof (u_db_item_info));

	ASSERT(sizeof (para_set_value) > PAGE_BYTE_SIZE);
	ASSERT(sizeof (u_coin_pre_value) > PAGE_BYTE_SIZE);
	ASSERT(sizeof (u_db_item_info) > ITEM_SIZE);

	test_read_r_code (Nand_ReadPage(PUBULIC_DATA_START_BLOCK_NUM, PUBULIC_DATA_START_PAGE_NUM, (U8 *)&para_set_value));

	if(para_set_value.data.magic_num != MAGIC_NUM){    //不满足条件需要初始化    这里每个国家的信息都 初始化了 在界面更换硬币时不用担心值不确定
		cy_print("#####   FIRST USE ##### \r\n");
		para_set_value.data.magic_num = MAGIC_NUM;
		para_set_value.data.coin_id = 0;
		para_set_value.data.country_id = 0;
		para_set_value.data.db_total_item_num = 0;
		para_set_value.data.op_id = 0;
		para_set_value.data.rej_level = 0;
		para_set_value.data.kick_start_delay_t1 = 1;
		para_set_value.data.kick_start_delay_t2 = 1;
		para_set_value.data.kick_keep_t1 = 80;
		para_set_value.data.kick_keep_t2 = 80;
		para_set_value.data.coin_full_rej_pos = 1;
		para_set_value.data.motor_idle_t = 400;
		para_set_value.data.adj_offset_position = 4096;
		para_set_value.data.pre_count_stop_n = 1;
		para_set_value.data.system_boot_delay = 0;
		para_set_value.data.system_mode = 1;

		for (i = 0; i < COIN_TYPE_NUM; i++){
			para_set_value.data.precoin_set_num[i] = 50;
		}

		test_erase_r_code (Nand_EraseBlock(PUBULIC_DATA_START_BLOCK_NUM ));
		cy_println ("erase block %d completed", PUBULIC_DATA_START_BLOCK_NUM);
		test_write_r_code (Nand_WritePage(PUBULIC_DATA_START_BLOCK_NUM, PUBULIC_DATA_START_PAGE_NUM, (U8 *)&para_set_value));
		cy_println ("write block %d page %d completed", PUBULIC_DATA_START_BLOCK_NUM, PUBULIC_DATA_START_PAGE_NUM);

		for (i = 0; i < COUNTRY_NUM; i++){
			for (j = 0; j < COIN_TYPE_NUM; j++){
				pre_value.country[i].coin[j].data.max0 = 1023;
				pre_value.country[i].coin[j].data.min0 = 1023;
				pre_value.country[i].coin[j].data.max1 = 1023;
				pre_value.country[i].coin[j].data.min1 = 1023;
				pre_value.country[i].coin[j].data.max2 = 1023;
				pre_value.country[i].coin[j].data.min2 = 1023;
				pre_value.country[i].coin[j].data.std0 = 900;
				pre_value.country[i].coin[j].data.std1 = 900;
				pre_value.country[i].coin[j].data.std2 = 900;
				pre_value.country[i].coin[j].data.offsetmax0 = 10;
				pre_value.country[i].coin[j].data.offsetmin0 = -10;
				pre_value.country[i].coin[j].data.offsetmax1 = 10;
				pre_value.country[i].coin[j].data.offsetmin1 = -10;
				pre_value.country[i].coin[j].data.offsetmax2 = 10;
				pre_value.country[i].coin[j].data.offsetmin2 = -10;
			}
			test_write_r_code (Nand_WritePage(COUNTRY0_COIN_PRE_VALUE_START_BLOCK_NUM, (COUNTRY0_COIN_PRE_VALUE_START_PAGE_NUM + i), (U8 *)(&(pre_value.country[i]))));
			cy_println ("write block %d page %d completed",
						(COUNTRY0_COIN_PRE_VALUE_START_BLOCK_NUM ),
						(COUNTRY0_COIN_PRE_VALUE_START_PAGE_NUM + i));
		}
	}else{
		cy_println ("Check Data Completed!");
	}

}


void Writekick_value(void)		//写入 当前踢币时延  踢币持续时间 转盘堵转时间  传送带传送时间
{
	write_para ();
}

void read_kick_value (void)
{
	test_read_r_code (Nand_ReadPage(PUBULIC_DATA_START_BLOCK_NUM, PUBULIC_DATA_START_PAGE_NUM, (U8 *)&para_set_value));
}
void write_kick_value (void)
{
	test_write_r_code (Nand_WritePage(PUBULIC_DATA_START_BLOCK_NUM, PUBULIC_DATA_START_PAGE_NUM, (U8 *)&para_set_value));
}

void read_para(void)  //读出 当前币种 历史数据 总
{
	read_kick_value ();
	read_coin_value ();
}

void write_para (void)	//写入 当前币种 历史数据 总
{
	test_erase_r_code (Nand_EraseBlock(PUBULIC_DATA_START_BLOCK_NUM ));
	write_kick_value ();
	write_coin_value ();
}


void read_coin_value(void) 	 // read  COIN  0--8
{
	uint32_t i;
	for (i = 0; i < COUNTRY_NUM; i++)
	{
		test_read_r_code (Nand_ReadPage(COUNTRY0_COIN_PRE_VALUE_START_BLOCK_NUM, COUNTRY0_COIN_PRE_VALUE_START_PAGE_NUM + i, (U8 *)(&(pre_value.country[i]))));
	}

	//映射硬币金额
	pre_value.country[COUNTRY_ID].coin[0].data.money = MONEY_1_00; //一元
	pre_value.country[COUNTRY_ID].coin[1].data.money = MONEY_0_50; //五角铜
	pre_value.country[COUNTRY_ID].coin[2].data.money = MONEY_0_50; //五角钢
	pre_value.country[COUNTRY_ID].coin[3].data.money = MONEY_0_10; //一角大铝
	pre_value.country[COUNTRY_ID].coin[4].data.money = MONEY_0_10; //一角小钢
	pre_value.country[COUNTRY_ID].coin[5].data.money = MONEY_0_10; //一角小铝
	pre_value.country[COUNTRY_ID].coin[6].data.money = MONEY_0_05; //五分
	pre_value.country[COUNTRY_ID].coin[7].data.money = MONEY_0_02; //两分
	pre_value.country[COUNTRY_ID].coin[8].data.money = MONEY_0_01; //一分
	pre_value.country[COUNTRY_ID].coin[9].data.money = MONEY_10_00; //纪念币10元
	pre_value.country[COUNTRY_ID].coin[10].data.money = MONEY_5_00; //纪念币5元

	//映射预置计数值
	pre_value.country[COUNTRY_ID].coin[0].data.coin_type = 0;
	pre_value.country[COUNTRY_ID].coin[1].data.coin_type = 1;
	pre_value.country[COUNTRY_ID].coin[2].data.coin_type = 1;
	pre_value.country[COUNTRY_ID].coin[3].data.coin_type = 3;//大1角
	pre_value.country[COUNTRY_ID].coin[4].data.coin_type = 4;
	pre_value.country[COUNTRY_ID].coin[5].data.coin_type = 4;
	pre_value.country[COUNTRY_ID].coin[6].data.coin_type = 6;
	pre_value.country[COUNTRY_ID].coin[7].data.coin_type = 7;
	pre_value.country[COUNTRY_ID].coin[8].data.coin_type = 8;
	pre_value.country[COUNTRY_ID].coin[9].data.coin_type = 9;
	pre_value.country[COUNTRY_ID].coin[10].data.coin_type = 10;

	//映射触摸屏预置计数预置设置值显示地址
	pre_value.country[COUNTRY_ID].coin[0].data.hmi_pre_count_set_addr = ADDR_YZS0; //一元
	pre_value.country[COUNTRY_ID].coin[1].data.hmi_pre_count_set_addr = ADDR_YZS1; //五角铜
	pre_value.country[COUNTRY_ID].coin[2].data.hmi_pre_count_set_addr = ADDR_YZS1; //五角钢
	pre_value.country[COUNTRY_ID].coin[3].data.hmi_pre_count_set_addr = ADDR_YZS3; //一角大铝
	pre_value.country[COUNTRY_ID].coin[4].data.hmi_pre_count_set_addr = ADDR_YZS4; //一角小钢
	pre_value.country[COUNTRY_ID].coin[5].data.hmi_pre_count_set_addr = ADDR_YZS4; //一角小铝
	pre_value.country[COUNTRY_ID].coin[6].data.hmi_pre_count_set_addr = ADDR_YZS6; //五分
	pre_value.country[COUNTRY_ID].coin[7].data.hmi_pre_count_set_addr = ADDR_YZS7; //二分
	pre_value.country[COUNTRY_ID].coin[8].data.hmi_pre_count_set_addr = ADDR_YZS8; //一分
	pre_value.country[COUNTRY_ID].coin[9].data.hmi_pre_count_set_addr = ADDR_YZS9; //纪念币10元
	pre_value.country[COUNTRY_ID].coin[10].data.hmi_pre_count_set_addr = ADDR_YZS10; //纪念币5元


	for (i = 0; i < COIN_TYPE_NUM; i++)
	{
		//count_coin_temp[pre_value.country[COUNTRY_ID].coin[i].data.coin_type].pre_count_set = 50;
		//para_set_value.data.precoin_set_num[i] = 50;
		pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set 	= &count_coin_temp[pre_value.country[COUNTRY_ID].coin[i].data.coin_type].pre_count_set;//预置计数设置值
		pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_cur = &count_coin_temp[pre_value.country[COUNTRY_ID].coin[i].data.coin_type].pre_count_cur;//预置计数当前值
		pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_full_flag = &count_coin_temp[pre_value.country[COUNTRY_ID].coin[i].data.coin_type].full_flag;//预置计数到达标志
		pre_value.country[COUNTRY_ID].coin[i].data.p_coinval = &count_coin_temp[pre_value.country[COUNTRY_ID].coin[i].data.coin_type].coinval;//包装卷数

		*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set = para_set_value.data.precoin_set_num[pre_value.country[COUNTRY_ID].coin[i].data.coin_type];//预置计数设置值初始化
		*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_full_flag = 0;//预置计数到达标志清零
	}
}


void write_coin_value (void)
{
	uint16_t i;

	for (i = 0; i < COUNTRY_NUM; i++)
	{
		test_write_r_code (Nand_WritePage(COUNTRY0_COIN_PRE_VALUE_START_BLOCK_NUM, (COUNTRY0_COIN_PRE_VALUE_START_PAGE_NUM + i), (U8 *)(&(pre_value.country[i]))));
	}
}

void ini_screen (void)
{
	int i;
	dgus_tf1word(ADDR_CNCH,coinchoose);  //国家币种图标变量
	dgus_tf1word(ADDR_PGH1,para_set_value.data.coin_full_rej_pos);   // 真币剔除工号
	dgus_tf1word(ADDR_KICK_DELAY_T1, para_set_value.data.kick_start_delay_t1);	//第一个踢币延时时间
	dgus_tf1word(ADDR_KICK_KEEP_T1, para_set_value.data.kick_keep_t1);	//第一个踢币保持时间
	dgus_tf1word(ADDR_KICK_DELAY_T2, para_set_value.data.kick_start_delay_t2);	//第二个踢币延时时间
	dgus_tf1word(ADDR_KICK_KEEP_T2, para_set_value.data.kick_keep_t2);	//第二个踢币保持时间
	dgus_tf1word(ADDR_MOTOR_IDLE_T, para_set_value.data.motor_idle_t);	//无币空转等待时间
	dgus_tf1word(ADDR_PRE_COUNT_STOP_N, para_set_value.data.pre_count_stop_n);	//满币停机数，设置为1则任意一种硬币达到预置数就停机
	dgus_tf1word(ADDR_LEVEL100,cn0copmaxc0[coinchoose]);	//清分等级，暂时没有设置


	//开机 把每个地址的值给初始化赋一下值
	disp_KJAmount(); // initial addr on zhu jiemian ze zs forge
	disp_allcount(); //initial addr on xiandan jiemian
	disp_data(ADDR_CPZE,ADDR_CPZS,ADDR_CPFG);			//initial addr on jishu jiemian  ze zs forge data variable

	dgus_tf1word(ADDR_CNTB,sys_env.coin_index);	//initial addr on zixuexi jiemian coin name tubiao
	disp_preselflearn(pre_value.country[coinchoose].coin[sys_env.coin_index].data.max0,pre_value.country[coinchoose].coin[sys_env.coin_index].data.min0,
					pre_value.country[coinchoose].coin[sys_env.coin_index].data.max1,pre_value.country[coinchoose].coin[sys_env.coin_index].data.min1,
					pre_value.country[coinchoose].coin[sys_env.coin_index].data.max2,pre_value.country[coinchoose].coin[sys_env.coin_index].data.min2);	//initial addr on zixuexi jiemian value


	dgus_tf1word(ADDR_MODE, para_set_value.data.system_mode);
	for (i = 0; i < COIN_TYPE_NUM; i++){
		if (para_set_value.data.system_mode == 0){
			*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set = para_set_value.data.precoin_set_num[pre_value.country[COUNTRY_ID].coin[i].data.coin_type];
		}else{
			*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set = 9999;
		}
		dgus_tf1word(pre_value.country[COUNTRY_ID].coin[i].data.hmi_pre_count_set_addr, *pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set);	//预置值
	}
}

void ini_picaddr(void) //币种切换时的 初始化地址函数
{
	int pi;
	read_para();		 //开机 读入当前 当前累计总额  当前总量 当前异币
	// 各币种 数量	开机初始化为零

	for(pi = 0;pi<COIN_TYPE_NUM;pi++)
	{
		count_coin_temp[pi].pre_count_set = pi;	 //预置值  初始化为0
		count_coin_temp[pi].pre_count_cur = 0;
		count_coin_temp[pi].full_flag = 0;
		count_coin_temp[pi].coinval = 0;
		processed_coin_info.coin_ctr[pi] = 0;
	}
	processed_coin_info.total_good = 0;
	processed_coin_info.total_money = 0;
	processed_coin_info.total_coin = 0;
	processed_coin_info.total_ng = 0;
	processed_coin_info.coinnumber =0;
	sys_env.coin_index = 0;
	return;
}

void yqsql_exec(uint16_t chos)
{
	switch(chos)
	{
		case DBINSERT:
		{
//		  	char dbsave[HISTORYSAVANUM] ={0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00, 0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,};
//			U8 str_db[20];
									      //条数				//日期					   //工号 //金额				//数量				//异币
			s_db_item_info * db_item_info_temp;
			s_db_item_block * db_item_block_temp;
			uint16_t item_index;
			uint16_t i = 0;
			U8 yqnddata[PAGE_BYTE_SIZE];
			for(i = 0; i < PAGE_BYTE_SIZE; i++)
			{
				yqnddata[i] = 0xff;
			}



			cy_println("Block %d page = %d, db_total_item_num = %d",
				HISTORY_START_BLOCK_NUM,
				HISTORY_START_PAGE_NUM + (para_set_value.data.db_total_item_num * ITEM_SIZE / PAGE_BYTE_SIZE),
				para_set_value.data.db_total_item_num
				);
			if(para_set_value.data.db_total_item_num >= (ITEM_SIZE*BLOCK_PAGE_SIZE))    //保存2048条
			{
				SEND_ERROR(READOUTDATA);
				break;
			}


			Nand_ReadPage(HISTORY_START_BLOCK_NUM, HISTORY_START_PAGE_NUM + (para_set_value.data.db_total_item_num * ITEM_SIZE / PAGE_BYTE_SIZE), yqnddata);

			item_index = para_set_value.data.db_total_item_num % PAGE_ITEM_NUM_SIZE;
			db_item_block_temp = (s_db_item_block *)yqnddata;
			db_item_info_temp = (s_db_item_info *)&(db_item_block_temp->item_info_array[item_index]);
			db_item_info_temp->index = para_set_value.data.db_total_item_num;

			while(1)     // read time
			{
				comscreen(dgus_readt,6);	//read time
				while(touch_flag ==0){;}

				if (touchnum[7]>0)
				{
					db_item_info_temp->time[0] = touchnum[6];
					db_item_info_temp->time[1] = touchnum[7];
					db_item_info_temp->time[2] = touchnum[8];
					db_item_info_temp->time[3] = touchnum[10];
					db_item_info_temp->time[4] = touchnum[11];
					db_item_info_temp->time[5] = touchnum[12];
					touch_flag = 0;
					break;
				}else{
					touch_flag = 0;
				}
			}
		sprintf(db_item_info_temp->time,"20%02x-%02x-%02x %02x:%02x:%02x",
						db_item_info_temp->time[0],
						db_item_info_temp->time[1],
						db_item_info_temp->time[2],
						db_item_info_temp->time[3],
						db_item_info_temp->time[4],
						db_item_info_temp->time[5]);   //read time
			
			db_item_info_temp->ID = para_set_value.data.coin_full_rej_pos;
//			db_item_info_temp->m_10yuan		 = disp_buf.m_10yuan;
//			db_item_info_temp->m_1yuan		 = disp_buf.m_1yuan;
//			db_item_info_temp->m_5jiao		 = disp_buf.m_5jiao;
//			db_item_info_temp->m_1jiao		 = disp_buf.m_1jiao;
//			db_item_info_temp->m_1jiao_big = disp_buf.m_1jiao_big;
//			db_item_info_temp->m_5fen			 = disp_buf.m_5fen;
//			db_item_info_temp->m_2fen			 = disp_buf.m_2fen;
//			db_item_info_temp->m_1fen			 = disp_buf.m_1fen;
//			db_item_info_temp->m_10yuan    = disp_buf.m_10yuan;
//			db_item_info_temp->m_5yuan     = disp_buf.m_5yuan;
			for (i = 0; i < NORMAL_COIN_TYPE_NUM; i++){
				db_item_info_temp->coin_ctr[i] = processed_coin_info.coin_ctr[i];
			}
			db_item_info_temp->total_good  = disp_buf.total_good;
			db_item_info_temp->total_ng    = disp_buf.total_ng;
			db_item_info_temp->total_money = disp_buf.total_money;

			Nand_WritePage(HISTORY_START_BLOCK_NUM, (para_set_value.data.db_total_item_num * ITEM_SIZE / PAGE_BYTE_SIZE), yqnddata);   //rNF_WritePage(YQNDHISTORYPAGENUM,yqnddata);	 //公共信息用一页	20块的第0页 写入

			para_set_value.data.db_total_item_num += 1;
			write_para();

			cy_println("save over");
			cmd ();
			break;
		}
		case DBDELETE:
		{
			Nand_EraseBlock(HISTORY_START_BLOCK_NUM);
			para_set_value.data.db_total_item_num = 0;
			write_para();
			cy_println("delete block %d over", HISTORY_START_BLOCK_NUM);
			cmd ();
			break;
		}
		case DBDISPLAY:
		{


			U8 r_code;
			s_db_item_info * db_item_info_temp;
			s_db_item_block * db_item_block_temp;
			U8 yqnddata[PAGE_BYTE_SIZE];


			uint16_t str_addr[6][5] = {   //屏上地址
					{0x0100,0x0110,0x0112,0x0118,0x011E},
					{0x0120,0x0130,0x0132,0x0138,0x013E},
					{0x0140,0x0150,0x0152,0x0158,0x015E},
					{0x0160,0x0170,0x0172,0x0178,0x017E},
					{0x0180,0x0190,0x0192,0x0198,0x019E},
					{0x01A0,0x01B0,0x01B2,0x01B8,0x01BE},
					};
			S8 str_db[20];
			uint32_t temp = 0;
			uint16_t num = 0;     //计时 显示到6次就结束

			int32_t	db_id_temp = 0;

			cy_println ("\ndb_id = %d", db_id);
			memset(str_db,' ',20);
			for(num = 0; num < 6;  num++){
				dgus_chinese(str_addr[num][0],str_db, strlen(str_db));	 // dgus  chinese  time
				dgus_chinese(str_addr[num][0],str_db,8);	 // dgus  chinese  time
				dgus_chinese(str_addr[num][1],str_db,8);	 // dgus  chinese  gh
				dgus_chinese(str_addr[num][2],str_db,8);	 // dgus  chinese  ze
				dgus_chinese(str_addr[num][3],str_db,8);	 // dgus  chinese  zs
				dgus_chinese(str_addr[num][4],str_db,8);	 // dgus  chinese  fg
			}
			if (db_id <= 0){
				cmd ();
				break;
			}


			if ((db_id - 6) >= 0){
				db_id -= 6;
				db_id_temp = db_id;
			}else{
				db_id = 0;
				db_id_temp = db_id;
			}

			cy_print ("*******************************************************************************\n");
			cy_print ("Display pre page data, db_total_item_num = %d\n", para_set_value.data.db_total_item_num);
			cy_print("Block = %d page = %d addr = %d db_id = %d item_index = %d\n",
				YQNDHISTORY_DB_ID_PAGE_ADDR / BLOCK_BYTE_SIZE,
				(YQNDHISTORY_DB_ID_PAGE_ADDR - YQNDHISTORYBLOCK),
				YQNDHISTORY_DB_ID_PAGE_ADDR,
				db_id+1,
				db_id % PAGE_ITEM_NUM_SIZE);

			if(db_id >= 0){
				//int temp_db_id = db_id;
				r_code = Nand_ReadPage(HISTORY_START_BLOCK_NUM, HISTORY_START_PAGE_NUM + (db_id * ITEM_SIZE / PAGE_BYTE_SIZE), yqnddata);

				if (r_code == 0){
					cy_print ("*******************************************************************************\n");
					cy_print ("|-----|-------------------|----|--------------|----------------|-----------|\n");
					cy_print ("|Index|        Time       | ID | Total amount | Total Quantity |   Other   |\n");
					cy_print ("|-----|-------------------|----|--------------|----------------|-----------|\n");

					db_item_block_temp = (s_db_item_block *)yqnddata;

					db_item_info_temp = (s_db_item_info *)&(db_item_block_temp->item_info_array[db_id % PAGE_ITEM_NUM_SIZE]);

					HTTP_START();
					for(num = 0; (num < 6); ){
						fill_http_data (db_item_info_temp);
	 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp = db_item_info_temp->index+1;
						sprintf(str_db,"%05d",temp);
						cy_print("|%s", str_db);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						cy_print("|%s", db_item_info_temp->time);
						dgus_chinese(str_addr[num][0],db_item_info_temp->time,strlen(db_item_info_temp->time));	 // dgus  chinese  time
	 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp = db_item_info_temp->ID;
						sprintf(str_db,"%04d",temp);
						cy_print("|%s", str_db);
						//sprintf(str_db,"%2d",temp);
						sprintf(str_db,"  ");
						dgus_chinese(str_addr[num][1],str_db,strlen(str_db));	 // dgus  chinese  gh
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp =	db_item_info_temp->total_money;
						sprintf(str_db,"%10d.%d%d ",(temp/100),((temp%100)/10),((temp%100)%10));
						cy_print("|%s", str_db);
						sprintf((char*)str_db,"%d.%d",(temp/100),(temp%100));
						dgus_chinese(str_addr[num][2],str_db,strlen(str_db));	 // dgus  chinese  ze
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp =	db_item_info_temp->total_good;
						sprintf(str_db,"%15d ",temp);
						cy_print("|%s", str_db);
						sprintf((char*)str_db,"%d",temp);
						dgus_chinese(str_addr[num][3],str_db,strlen(str_db));	 // dgus  chinese  zs
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp =	db_item_info_temp->total_ng;
						sprintf(str_db,"%10d ",temp);
						cy_print("|%s|", str_db);
						sprintf((char*)str_db,"%d",temp);
						dgus_chinese(str_addr[num][4],str_db,strlen(str_db));	 // dgus  chinese  fg
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				  	cy_print ("\n|-----|-------------------|----|--------------|----------------|-----------|\n");
						num++;
						db_id++;
						if (db_id >= para_set_value.data.db_total_item_num)
							break;
						if (((db_id / PAGE_ITEM_NUM_SIZE) != ((db_id - 1) / PAGE_ITEM_NUM_SIZE)) && (num < 6))
						{
							Nand_ReadPage(HISTORY_START_BLOCK_NUM, HISTORY_START_PAGE_NUM + (db_id * ITEM_SIZE / PAGE_BYTE_SIZE), yqnddata);
							db_item_block_temp = (s_db_item_block *)yqnddata;
						}

						db_item_info_temp = (s_db_item_info *)&(db_item_block_temp->item_info_array[db_id % PAGE_ITEM_NUM_SIZE]);

					}
					HTTP_END ();
				}
				else if (r_code == 1)
				{
					cy_println (" Parameter Error, Buf must be not null!!!");
				}
				else if (r_code == 2)
				{
					cy_println ("ECC Error, Read Op failed!!!");
				}
			}
			db_id = db_id_temp;
			cy_print ("*******************************************************************************\n");
			cmd ();
			break;
		}

		case DBDISPLAYBACK:
		{
			U8 r_code;
			s_db_item_info * db_item_info_temp;
			s_db_item_block * db_item_block_temp;
			U8 yqnddata[PAGE_BYTE_SIZE];

			uint16_t str_addr[6][5] = {   //屏上地址
					{0x0100,0x0110,0x0112,0x0118,0x011E},
					{0x0120,0x0130,0x0132,0x0138,0x013E},
					{0x0140,0x0150,0x0152,0x0158,0x015E},
					{0x0160,0x0170,0x0172,0x0178,0x017E},
					{0x0180,0x0190,0x0192,0x0198,0x019E},
					{0x01A0,0x01B0,0x01B2,0x01B8,0x01BE},
					};
			S8 str_db[20];
			uint32_t temp = 0;
			uint16_t num = 0;     //计时 显示到6次就结束

			int32_t	db_id_temp = 0;


			if ((db_id + 6) < para_set_value.data.db_total_item_num)
			{
				db_id += 6;
				db_id_temp = db_id;
			}
			else
			{
				cmd ();
				break;
			}
			cy_print ("*******************************************************************************\n");
			cy_print ("display next page data, db_total_item_num = %d\n", para_set_value.data.db_total_item_num);
			cy_print("Block = %d page = %d addr = %d db_id = %d item_index = %d\n",
				YQNDHISTORY_DB_ID_PAGE_ADDR / BLOCK_BYTE_SIZE,
				(YQNDHISTORY_DB_ID_PAGE_ADDR - YQNDHISTORYBLOCK),
				YQNDHISTORY_DB_ID_PAGE_ADDR,
				db_id+1,
				db_id % PAGE_ITEM_NUM_SIZE);
			if(db_id < para_set_value.data.db_total_item_num)
			{

				r_code = Nand_ReadPage(HISTORY_START_BLOCK_NUM, HISTORY_START_PAGE_NUM + (db_id * ITEM_SIZE / PAGE_BYTE_SIZE), yqnddata);     // 公共信息页 rLB_ReadPage(YQNDHISTORYPAGENUM,yqnddata);
				if (r_code == 0)
				{
					cy_print ("*******************************************************************************\n");
					cy_print ("|-----|-------------------|----|--------------|----------------|-----------|\n");
					cy_print ("|Index|        Time       | ID | Total amount | Total Quantity |   Other   |\n");
					cy_print ("|-----|-------------------|----|--------------|----------------|-----------|\n");
					memset(str_db,' ',20);

					db_item_block_temp = (s_db_item_block *)yqnddata;

					db_item_info_temp = (s_db_item_info *)&(db_item_block_temp->item_info_array[db_id % PAGE_ITEM_NUM_SIZE]);

					for(num = 0; num <6;  num++)  //if  still have history num , clean dgus screen ,then display
					{
						dgus_chinese(str_addr[num][0],str_db,strlen(str_db));	 // dgus  chinese  time
						dgus_chinese(str_addr[num][1],str_db,strlen(str_db));	 // dgus  chinese  gh
						dgus_chinese(str_addr[num][2],str_db,strlen(str_db));	 // dgus  chinese  ze
						dgus_chinese(str_addr[num][3],str_db,strlen(str_db));	 // dgus  chinese  zs
						dgus_chinese(str_addr[num][4],str_db,strlen(str_db));	 // dgus  chinese  fg
					}
					HTTP_START();
					for(num = 0; ((num <6) && (db_id < para_set_value.data.db_total_item_num));)
					{

						fill_http_data (db_item_info_temp);
	 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp = db_item_info_temp->index+1;
						sprintf(str_db,"%05d",temp);
						cy_print("|%s", str_db);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						cy_print("|%s", db_item_info_temp->time);
						dgus_chinese(str_addr[num][0],db_item_info_temp->time,strlen(db_item_info_temp->time));	 // dgus  chinese  time
	 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp = db_item_info_temp->ID;
						sprintf(str_db,"%04d",temp);
						cy_print("|%s", str_db);
						//sprintf(str_db,"%2d",temp);
						sprintf(str_db,"  ");
						dgus_chinese(str_addr[num][1],str_db,strlen(str_db));	 // dgus  chinese  gh
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp =	db_item_info_temp->total_money;
						sprintf(str_db,"%10d.%d%d ",(temp/100),((temp%100)/10),((temp%100)%10));
						cy_print("|%s", str_db);
						sprintf((char*)str_db,"%d.%d",(temp/100),(temp%100));
						dgus_chinese(str_addr[num][2],str_db,strlen(str_db));	 // dgus  chinese  ze
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp =	db_item_info_temp->total_good;
						sprintf(str_db,"%15d ",temp);
						cy_print("|%s", str_db);
						sprintf((char*)str_db,"%d",temp);
						dgus_chinese(str_addr[num][3],str_db,strlen(str_db));	 // dgus  chinese  zs
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						temp =	db_item_info_temp->total_ng;
						sprintf(str_db,"%10d ",temp);
						cy_print("|%s|", str_db);
						sprintf((char*)str_db,"%d",temp);
						dgus_chinese(str_addr[num][4],str_db,strlen(str_db));	 // dgus  chinese  fg
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				  	cy_print ("\n|-----|-------------------|----|--------------|----------------|-----------|\n");
						num++;
						db_id++;

						if (((db_id / PAGE_ITEM_NUM_SIZE) != ((db_id - 1) / PAGE_ITEM_NUM_SIZE)) && (num < 6))
						{
							Nand_ReadPage(HISTORY_START_BLOCK_NUM, HISTORY_START_PAGE_NUM + (db_id * ITEM_SIZE / PAGE_BYTE_SIZE), yqnddata);
							db_item_block_temp = (s_db_item_block *)yqnddata;
						}

						db_item_info_temp = (s_db_item_info *)&(db_item_block_temp->item_info_array[db_id % PAGE_ITEM_NUM_SIZE]);
					}
					HTTP_END ();
				}
				else if (r_code == 1)
				{
					cy_println (" Parameter Error, Buf must be not null!!!");
				}
				else if (r_code == 2)
				{
					cy_println ("ECC Error, Read Op failed!!!");
				}
			}
			db_id = db_id_temp;
			cy_print ("*******************************************************************************\n");
			cmd ();
			break;
		}
		default:
		{
			break;
		}
	}
}

void fill_txt_data (s_db_item_info * db_item_info_temp)
{
	int i;
	 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cy_print("%5d", db_item_info_temp->index+1);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cy_print("   %s", db_item_info_temp->time);
	 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	cy_print("     %6d", db_item_info_temp->m_1yuan);
//	cy_print("     %6d", db_item_info_temp->m_5jiao);
//	cy_print("     %6d", db_item_info_temp->m_1jiao);
//	cy_print("     %6d", db_item_info_temp->m_1jiao_big);
//	cy_print("     %6d", db_item_info_temp->m_10yuan);
//	cy_print("     %6d", db_item_info_temp->m_5yuan);
	for (i = 0; i < NORMAL_COIN_TYPE_NUM; i++){
		cy_print("     %6d", db_item_info_temp->coin_ctr[i]);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cy_print("     %6d.%d%d", (db_item_info_temp->total_money/100),((db_item_info_temp->total_money%100)/10),((db_item_info_temp->total_money%100)%10));
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cy_print("     %6d", db_item_info_temp->total_good);
	cy_print("     %6d", db_item_info_temp->total_ng);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cy_print ("\r\n");
	//HTTP_INSERT ("\r\n|-----|-------------------|----|--------------|----------------|-----------|\r\n");
}
void export_record (void)
{	
	U8 r_code;
	uint16_t i,j;
	uint16_t record_num = para_set_value.data.db_total_item_num;
	uint16_t record_id = 0;
	s_db_item_info * db_item_info_temp;
	s_db_item_block * db_item_block_temp;
	cy_print ("                                                        广州畅阳电子科技有限公司\r\n\r\n");
	cy_print ("                                                         硬币清分机历史清分记录\r\n\r\n");
	cy_print (" 索引              清分时间      1元(枚)  5角铜(枚)  5角钢(枚)  大1角(枚)  1角钢(枚)  1角铝(枚)    总金额(元) 总枚数(枚) 异币数(枚)\r\n");
	if (record_num > 0){
		i = 0;
		while (record_id < record_num){
			r_code = Nand_ReadPage(HISTORY_START_BLOCK_NUM, HISTORY_START_PAGE_NUM + (record_id * ITEM_SIZE / PAGE_BYTE_SIZE), page_buf);
			if (r_code == 0){
				j = 0;
				db_item_block_temp = (s_db_item_block *)page_buf;
				while ((i < record_num) && (j < 32)){
					db_item_info_temp = (s_db_item_info *)&(db_item_block_temp->item_info_array[j % PAGE_ITEM_NUM_SIZE]);
					fill_txt_data (db_item_info_temp);
					j++;
					i++;
				}
			}else{
				cy_println ("Nand Error");
			}
			record_id += PAGE_ITEM_NUM_SIZE;
		}
	}
}

void export_detect_data (void)
{	
	uint32_t	i;
	cy_println ("                 广州畅阳电子科技有限公司");
	cy_println ("	            硬币清分机检测数据");
	cy_println("---------------------------------------------------------------------");
	cy_println ("              高频        中频          低频");
	for (i = 0; i < good_value_index; i++)
	{
		cy_println ("第%4d枚      %4d        %4d          %4d", i + 1, GOOD_value_buf[i].AD0, GOOD_value_buf[i].AD1, GOOD_value_buf[i].AD2);
	}
	cy_println("---------------------------------------------------------------------");
	for (i = 0; i < ng_value_index; i++)
	{
		cy_println ("第%4d枚      %4d        %4d          %4d", i + 1, NG_value_buf[i].AD0, NG_value_buf[i].AD1, NG_value_buf[i].AD2);
	}
	cy_println("---------------------------------------------------------------------");
	cy_println ("最大值        %4d        %4d          %4d",coin_maxvalue0, coin_maxvalue1, coin_maxvalue2);
	cy_println ("最小值        %4d        %4d          %4d",coin_minvalue0, coin_minvalue1, coin_minvalue2);
}



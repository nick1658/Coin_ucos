#include "s3c2416.h"

volatile uint32_t runtime = 0;   // // 执行件用的计时变量
unsigned short int runstep = 0;   //  部件动作步骤号
volatile uint32_t time = 0;   // //部件测试时踢币电磁铁用的计时变量


void deviceinit(void)	//开机先把通道上的币挡下去 
{
	int i = 0;
	int good_coin;
	
	processed_coin_info.total_coin_old = processed_coin_info.total_coin;
	//cy_println("begin init full coin num %d ...", coin_env.full_stack_num);
	for (i = 0; i < coin_env.full_stack_num; i++){//预置计数模式时，当某种硬币的计数值达到预置值，就可以清零该硬币的计数
		//cy_println("begin init i = %d", i);
		good_coin = coin_env.full_coin_stack[i];
		if (*(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_cur) >= *(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_set)){
			*(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_cur) = 0;
			*pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_full_flag = 0;
		}
	}
	//cy_println("finish init = %d", i);	
	disp_allcount();
	coin_env.full_stack_num = 0;
	ch0_count =0;
	coin_env.ad0_step = 0;		
	coin_env.ad1_step = 0;
	coin_env.ad2_step = 0;
	ccstep = 0;
	
	for (i = 0; i < COIN_Q_LEN; i++){//初始化硬币队列
		coin_env.coin_Q[i] = FREE_Q_FLAG;
	}
	coin_env.coin_Q_index = 0;
	coin_env.coin_Q_remain = 0;
	
	processed_coin_info.coinnumber = 0;
	blockflag = ADBLOCKT;
	sys_env.stop_flag = 0;
	STORAGE_DIR_P();//正转
	runstep =0; //正常工作步骤号	
	sys_env.sys_runing_time = 0;
	//cy_println("finish init coin_env");		
}

void IR_detect_func(void)
{
////////////////////////////////////////////////////////////////////////////////////////////
	static int coin_in_flag = 0;
////////////////////////////////////////////////////////////////////////////////////////////
	if((COIN_DETECT == IR_DETECT_ON)){//对射电眼上升沿检测到硬币
		if ((coin_in_flag == 0)){
			if ((para_set_value.data.coin_full_rej_pos == 2)){//如果开启了第二个剔除真币功能
				if(coin_env.coin_Q[coin_env.coin_Q_index] == COIN_FULL_FLAG){ // 预置数已到，踢真币
					if (coin_env.full_kick_Q[coin_env.full_kick_Q_index] == 0){
						coin_env.full_kick_Q[coin_env.full_kick_Q_index] = para_set_value.data.kick_start_delay_t2;
						coin_env.full_kick_Q_index++;
						coin_env.full_kick_Q_index %= KICK_Q_LEN;
					}else{//剔除工位2队列追尾错误
						SEND_ERROR(KICK2COINERROR);
						dbg ("kick2 error alertflag = %d %s, %d", KICK2COINERROR,  __FILE__, __LINE__);
					}
				}else if(coin_env.coin_Q[coin_env.coin_Q_index] == FREE_Q_FLAG){ // 计数队列超车错误，计数不准
					SEND_ERROR(KICKCOINFULL);
					dbg ("kick2 error alertflag = %d %s, %d", KICKCOINFULL,  __FILE__, __LINE__);
				}else if(coin_env.coin_Q[coin_env.coin_Q_index] == COIN_NG_FLAG){ //异币漏踢错误
					SEND_ERROR(COINNGKICKERROR);
					dbg ("异币漏踢错误 alertflag = %d %s, %d", COINNGKICKERROR,  __FILE__, __LINE__);
				}		
				coin_env.coin_Q[coin_env.coin_Q_index] = FREE_Q_FLAG;
				coin_env.coin_Q_index++;
				coin_env.coin_Q_index %= COIN_Q_LEN;
			}
			coin_in_flag = 1;
		}
	}else{
		coin_in_flag = 0;
	}
}

void runfunction(void)   //部件动作函数
{
	switch (runstep){
		case 0:
			STORAGE_MOTOR_STARTRUN();	//  开转盘电机
			runstep = 1;
			break;
		case 1:
			break;
		case 20://预置数到，自动停机
			STORAGE_MOTOR_STOPRUN();	//  停转盘电机
			runtime = 100; // 2秒
			runstep = 100;
			break;
		case 40://手动停机
			STORAGE_MOTOR_STOPRUN();	//  停转盘电机
			runtime = 100; // 2秒
			runstep = 120;
			break;
		case 100:
			if (runtime == 0){
				prepic_num = JSJM;
				SEND_ERROR(COUNT_FINISHED);
			}
			break;
		case 120:
			if (runtime == 0){
				comscreen(Disp_Indexpic[JSJM],Number_IndexpicB);	 // back to the  picture before alert
				sys_env.workstep = 0;
			}
			break;
		default:break;
	}
}



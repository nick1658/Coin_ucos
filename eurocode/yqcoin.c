//////  本函数包括 计数  自学习

#include "s3c2416.h"

/////////////////////////////////////////////////
 U16 ccstep = 0;

S16 coin_value0 =0;
S16 coin_value1 =0;
S16 coin_value2 =0;

 U32 ch0_counttemp =0;  // 前一次通道0 通过的硬币计数 
 U32 ch1_counttemp =0;  // 前一次通道1 通过的硬币计数 
 U32 ch2_counttemp =0;  // 前一次通道2 通过的硬币计数 

 S16 coin_maxvalue0 = AD0STDSET;
 S16 coin_minvalue0 = AD0STDSET;
 S16 coin_maxvalue1 = AD1STDSET;
 S16 coin_minvalue1 = AD1STDSET;
 S16 coin_maxvalue2 = AD2STDSET;
 S16 coin_minvalue2 = AD2STDSET;
s_processed_coin_info processed_coin_info;

//4*9
U16 coine[COINCNUM][COIN_TYPE_NUM]=     // 由币种决定
{
{100,	50,	50,10,10,5,2,1,10,1000,	500},	//人民币 的面值 倍数  用于计算金额
{200,	100,50,20,10,5,2,1,0, 0, 	0},//欧元的 的面值 倍数
{100, 	50,	25,10, 5,1,0,0,0, 0, 	0},//美元的 的面值 倍数	 实际面额的100倍。没有这种面额就直接置0
{200,	100,50,20,10,5,2,1,0, 0, 	0},//英镑的 的面值 倍数
};


void prepare_coin_cmp_value (void)
{		
	if (ad1_min > 0){
		coin_env.cmp_use_index = 1;
	}else if ( ad0_min > 0){
		coin_env.cmp_use_index = 0;
	}else if (ad2_min > 0){
		coin_env.cmp_use_index = 2;
	}else{
		coin_env.cmp_use_index = 0;
	}	
#ifdef SAMPLE_METHOD_0
	coin_value0 = Detect_AD_Value_buf_p[coin_env.AD_min_index[coin_env.cmp_use_index]].AD0;
	coin_value1 = Detect_AD_Value_buf_p[coin_env.AD_min_index[coin_env.cmp_use_index]].AD1 - coin_value0;
	coin_value2 = Detect_AD_Value_buf_p[coin_env.AD_min_index[coin_env.cmp_use_index]].AD2 - Detect_AD_Value_buf_p[coin_env.AD_min_index[coin_env.cmp_use_index]].AD1;
#endif
#ifdef SAMPLE_METHOD_1
	coin_value0 = Detect_AD_Value_buf_p[coin_env.AD_min_index[coin_env.cmp_use_index]].AD0;
	coin_value1 = Detect_AD_Value_buf_p[coin_env.AD_min_index[coin_env.cmp_use_index]].AD1;
	coin_value2 = Detect_AD_Value_buf_p[coin_env.AD_min_index[coin_env.cmp_use_index]].AD2;
#endif
	if (sys_env.AD_buf_sending == 0){
		sys_env.AD_buf_sending = 1;
		sys_env.Detect_AD_buf_p = Detect_AD_Value_buf_p;
		sys_env.AD_buf_index++;
		sys_env.AD_buf_index %= AD_BUF_GROUP_LEN;
		Detect_AD_Value_buf_p = Detect_AD_Value_buf[sys_env.AD_buf_index];
	}
	memset (Detect_AD_Value_buf_p, 0, sizeof (Detect_AD_Value_buf[0]));
}

S16 is_good_coin (void)
{
	//鉴伪这里时间上还可以优化一下
	S16 i;
	for (i = 0; i < COIN_TYPE_NUM; i++){
		if   ( (( coin_value0 >= coin_cmp_value[i].compare_min0) && ( coin_value0 <= coin_cmp_value[i].compare_max0))
			&& (( coin_value1 >= coin_cmp_value[i].compare_min1) && ( coin_value1 <= coin_cmp_value[i].compare_max1))
			&& (( coin_value2 >= coin_cmp_value[i].compare_min2) && ( coin_value2 <= coin_cmp_value[i].compare_max2)))
		{
			if (sys_env.save_good_data){
				GOOD_value_buf[good_value_index].AD0 = coin_value0;
				GOOD_value_buf[good_value_index].AD1 = coin_value1;
				GOOD_value_buf[good_value_index].AD2 = coin_value2;
				GOOD_value_buf[good_value_index].use_index = coin_env.cmp_use_index;
				good_value_index++;
				if (good_value_index >= GOOD_BUF_LENGTH)
					good_value_index = 0;
			}
			return i;
		}	
	}
	if (sys_env.save_ng_data){
		NG_value_buf[ng_value_index].AD0 = coin_value0;
		NG_value_buf[ng_value_index].AD1 = coin_value1;
		NG_value_buf[ng_value_index].AD2 = coin_value2;
		NG_value_buf[ng_value_index].use_index = coin_env.cmp_use_index;
		ng_value_index++;
		if (ng_value_index >= NG_BUF_LENGTH)
			ng_value_index = 0;
	}
	return -1;
}

U32 coin_num[COIN_TYPE_NUM];    //各币种 计数常量

void cy_precoincount(void)
{
	switch(ccstep)
	{
		case 0:
		{			
			if ( (ch0_counttemp != ch0_count) )	//mean there is a coin come
			{
				ch0_counttemp = ch0_count;
				//ch1_counttemp = ch1_count;
				//ch2_counttemp = ch2_count;
				
				processed_coin_info.coinnumber++;
				
				prepare_coin_cmp_value ();
				ccstep = 8; 
			}
			break;
		}	
		case 8:
		{	 			
			S16 good_coin;
			good_coin = is_good_coin ();
			if (sys_env.stop_flag != 1){//如果不在反转状态
				sys_env.stop_flag = 0;
				sys_env.stop_time = STOP_TIME;//无币停机时间10秒
			}
			if ((good_coin < 0) ||  ((para_set_value.data.coin_full_rej_pos == 1) && 
									 ((*(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_set) == 0) ||
									  (*pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_full_flag == 1)))){ //假币 返回值小于0
				if (coin_env.kick_Q[coin_env.kick_Q_index] == 0){
					coin_env.kick_Q[coin_env.kick_Q_index] = para_set_value.data.kick_start_delay_t1;
					coin_env.kick_Q_index++;
					coin_env.kick_Q_index %= KICK_Q_LEN;
					coin_env.coin_Q[coin_env.coin_Q_remain] = COIN_NG_FLAG;//
				}else{//剔除工位1队列追尾错误
					SEND_ERROR(KICK1COINERROR);
					dbg ("kick1 error alertflag = %d %s, %d", KICK1COINERROR,  __FILE__, __LINE__);
				}
			}else {//真币
				if (*(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_set) == 9999){//只使用清分功能
					coin_num[good_coin]++;
					processed_coin_info.total_money += pre_value.country[coinchoose].coin[good_coin].data.money;
					processed_coin_info.total_good++;
					coin_env.coin_Q[coin_env.coin_Q_remain] = COIN_GOOD_FLAG;//用真币剔除工位剔除
				}else if((*(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_set) == 0) ||
						  (*pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_full_flag == 1)){//不接受此类硬币或者预置数已到
					coin_env.coin_Q[coin_env.coin_Q_remain] = COIN_FULL_FLAG;//用真币剔除工位剔除
				}else{//预置计数
					*(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_current) += 1;
					coin_num[good_coin]++;
					processed_coin_info.total_money += pre_value.country[coinchoose].coin[good_coin].data.money;
					processed_coin_info.total_good++;
					coin_env.coin_Q[coin_env.coin_Q_remain] = COIN_GOOD_FLAG;//
					if( *(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_current) >= *(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_set)){// 当前的币种  数量 达到其预置值
						*pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_full_flag = 1; //此类硬币预置数到，做个标记
						*(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_coinval) += 1;
						coin_env.full_coin_stack[coin_env.full_stack_num] = good_coin;
						coin_env.full_stack_num++;
						if (coin_env.full_stack_num >= para_set_value.data.pre_count_stop_n){
							runstep = 20;   //开始停机
						}
					}
				}
				coin_env.coin_Q_remain++;//鉴伪传感器和红外传感器之间有个硬币循环队列，深度为16，表示之间最多可以夹16个硬币
				coin_env.coin_Q_remain %= COIN_Q_LEN;
				
//				if(coin_env.coin_Q[coin_env.coin_Q_remain] != FREE_Q_FLAG){ // 计数队列追尾错误/*这里先不用，因为第二个传感器可能没装*/
//					SEND_ERROR(DETECTERROR);
//					cy_println ("detect 2 error alertflag = %d %s, %d", DETECTERROR,  __FILE__, __LINE__);
//				}
			}
			processed_coin_info.total_coin++;
			ccstep = 0;
			sys_env.coin_over = 1;
			break;
		}		
	}
}


// 根据AD值 计数并取 AD 最大值最小值
U32 coinlearnnumber = 0;   //count the coin number all proceed
void cy_coinlearn(void)
{
	switch(ccstep)
	{
		case 0:{			
			if ( (ch0_counttemp != ch0_count) && (ch1_counttemp != ch1_count) && (ch2_counttemp != ch2_count)){	//mean there is a coin come
				ch0_counttemp = ch0_count;
				ch1_counttemp = ch1_count;
				ch2_counttemp = ch2_count;
				processed_coin_info.coinnumber++;
				coinlearnnumber++;
				ccstep = 1;				
			}
			break;
		}	
		case 1:{						
			prepare_coin_cmp_value ();	
			coin_env.coin_Q[coin_env.coin_Q_remain] = COIN_GOOD_FLAG;//用真币剔除工位剔除
			coin_env.coin_Q_remain++;
			coin_env.coin_Q_remain %= COIN_Q_LEN;
		  
//			if(coin_env.coin_Q[coin_env.coin_Q_remain] != FREE_Q_FLAG){ // 计数队列追尾错误 /*这里先不用，因为第二个传感器可能没装*/
//				SEND_ERROR(DETECTERROR);
//				cy_println ("detect 2 error alertflag = %d %s, %d", DETECTERROR,  __FILE__, __LINE__);
//			}
			ccstep = 8; 	
			break;
		}
		case 8:{//找出最大值和最小值
			if( ( coin_value0 > coin_maxvalue0)){     //0
				coin_maxvalue0 = coin_value0;
			}
			if( ( coin_value0 < coin_minvalue0)){
				coin_minvalue0 = coin_value0;
			}
			if( ( coin_value1 > coin_maxvalue1)) {  //  1
				coin_maxvalue1 = coin_value1;
			}
			if( ( coin_value1 < coin_minvalue1)){
				coin_minvalue1 = coin_value1;
			}	
			if( ( coin_value2 > coin_maxvalue2)) {  //  2
				coin_maxvalue2 = coin_value2;
			}
			if( ( coin_value2 < coin_minvalue2)){
				coin_minvalue2 = coin_value2;
			}
			ccstep = 0; 
			sys_env.coin_over = 1;	
			break;
		}
		case 11:{
			dgus_tf1word(ADDR_A0MA,coin_maxvalue0);	//	 real time ,pre AD0  max
			dgus_tf1word(ADDR_A0MI,coin_minvalue0);	//	 real time ,pre AD0  min
			ccstep = 12; 
			break;	
		}
 		case 12:{
			dgus_tf1word(ADDR_A1MA,coin_maxvalue1);	//	 real time ,pre AD1  max	
			dgus_tf1word(ADDR_A1MI,coin_minvalue1);	//	 real time ,pre AD1  min
			ccstep = 13; 
			break;	
		}
		case 13:{
			dgus_tf1word(ADDR_A2MA,coin_maxvalue2);	//	 real time ,pre AD2  max
			dgus_tf1word(ADDR_A2MI,coin_minvalue2);	//	 real time ,pre AD2  min
			ccstep = 0; 
			break;			
		}	
											
	}
	return;
}
/*************************
**************************/
void detect_read(void)
{	
	if(COIN_DETECT == IR_DETECT_ON)    // 红外对射传感器
		dgus_tf1word(ADDR_DETCET1,1);	//被遮挡 
	else
		dgus_tf1word(ADDR_DETCET1,0);	//未被遮挡	
}

const char *coin_tips [] = {"一元", "五角", "五角", "一角", "一角", "一角", "五分", "两分", "一分"};

 U16 prepic_prenum =0;      // 用于记录 报错前的界面 
void alertfuc(U16 errorflag) //报错
{
	char str_buf[256];
	dbg ("alert flag is %d", errorflag);
	switch(errorflag)
	{	
		case COINNGKICKERROR:
			ALERT_MSG ("提示", "漏踢错误，请注意检查！");
			break;
		case DETECTERROR:
			ALERT_MSG ("提示", "第二个剔除工位电眼计数没有计数！");
			break;
		case KICKCOINFULL:
			ALERT_MSG ("提示", "第二个剔除工位计数不准确或有漏踢！");
			break;
		case KICK1COINERROR:
			ALERT_MSG ("提示", "第一个剔除工位剔除错误！");
			break;	
		case KICK2COINERROR:
			ALERT_MSG ("提示", "第二个剔除工位剔除错误！");
			break;		
		case PRESSMLOCKED:
			ALERT_MSG ("提示", "轨道堵币！请检查轨道或传感器。再次启动前请先清零！");
			break;	
		case ADSTDEEROR:
			ALERT_MSG ("提示", "传感器异常，请调整基准值，然后重试！");
			break;	
		case COUNT_FINISHED:
			switch (coin_env.full_stack_num){
				case 1:
					if (coin_env.full_coin_stack[0] < 8){
						sprintf (str_buf, "请更换%s的纸筒。", coin_tips[coin_env.full_coin_stack[0]]);
					}else{
						sprintf (str_buf, "数组越界: %d", coin_env.full_coin_stack[0]);
					}
					break;
				case 2:
					if ((coin_env.full_coin_stack[0] < 8) && 
						(coin_env.full_coin_stack[1] < 8)){
						sprintf (str_buf, "请更换%s和%s的纸筒。", 	coin_tips[coin_env.full_coin_stack[0]], 
																	coin_tips[coin_env.full_coin_stack[1]]);
					}else{
						sprintf (str_buf, "数组越界: %d, %d", coin_env.full_coin_stack[0], coin_env.full_coin_stack[1]);
					}
					break;
				case 3:
					if ((coin_env.full_coin_stack[0] < 8) && 
						(coin_env.full_coin_stack[1] < 8) &&
						(coin_env.full_coin_stack[2] < 8)){
						sprintf (str_buf, "请更换%s、%s和%s的纸筒。", 	coin_tips[coin_env.full_coin_stack[0]], 
																		coin_tips[coin_env.full_coin_stack[1]], 
																		coin_tips[coin_env.full_coin_stack[2]]);
					}else{
						sprintf (str_buf, "数组越界: %d, %d, %d", coin_env.full_coin_stack[0], coin_env.full_coin_stack[1], coin_env.full_coin_stack[2]);
					}
					break;
				default:sprintf (str_buf, "ERROR: 1001");break;
			}
			ALERT_MSG ("提示", str_buf);
			break;	
		case COMPLETE_UPDATE:
			ALERT_MSG ("提示", "程序更新完成，请重新启动");
			break;
		default:
			ALERT_MSG ("提示", "异常，请重新启动");
			break;
	}
	return;
}
	



//;----------------------------------------------------------

//;----------------------------------------------------------
#include "s3c2416.h"
#include "Nand.h"

S8 alertflag = 0; 		 //报错标志位

//uint32_t time_20ms;
uint32_t time_20ms_old;


//#define CODE_FLAG		(*(volatile unsigned *)0x33F80000)  		//MPLL lock time conut
__align(4) uint32_t code_flag __attribute__((at(0x32104000), zero_init));


int check_ad1_ad2_value (void);



extern void TftpTrm250ms(void); //250MS调用一次
extern u32_t sys_now(void);
void LwIP_Periodic_Handle(uint32_t localtime)
{
	static uint32_t pre_time = 0;
  /* TCP periodic process every 250 ms */
	if (localtime - pre_time >= 250/*TCP_TMR_INTERVAL*/){
		pre_time =  localtime;
		//tcp_tmr();
		TftpTrm250ms();
	}
}
//主函数

void coin_init (void)
{
	uint16_t i=0;
	rWTCON = 0;	// 关闭开门狗
	system_env_init ();
	coin_env_init ();
	//////////////
	alertflag = 0; 		 //报错标志位

	coinchoose = CN0;   // 币种选择 国家级别的
	touch_flag =0;   // 串口2接收 标志位
	uartcount = 0;  // 串口2接收 字节 计数
	sys_env.coin_index = 0;   //当前  学习 币种 名称
	tscount = 0;
	blockflag = ADBLOCKT; //此变量在yqadc.c文件中定义
	adtime = 0;    //定时中断里 计时


//	db_id = 0;   //历史数据 表格已经显示 数

	ch0_count = 0;

	ch0_coin_come = 0;;

	coinlearnnumber =0;  //自学习 数量
	prepic_prenum = 0;     // 用于记录 报错前的界面
	db_id = 0;   //历史数据 表格已经显示 数

	coin_maxvalue0 = 0;
	coin_minvalue0 = 1023;
	coin_maxvalue1 = 0;
	coin_minvalue1 = 1023;
	coin_maxvalue2 = 0;
	coin_minvalue2 = 1023;

	std_ad0 = AD0STDSET;
	std_ad1 = AD1STDSET;
	std_ad2 = AD2STDSET;

	for(i = 0;i<TSGET_NUM;i++)
	{
		touchnum[i] = 0;
	}
	////////////////

	uart1_init();//串口打印机
	uart2_init();//屏幕
	cy_println ("\n#####    Program For YQ ##### ");
	i = 1;
	watchdog_reset();/*初始化看门狗,T = WTCNT * t_watchdog*/
	adc_init();    //初始化ADC
	Timer_Init ();

	//init RTC************************************************************
	RTC_Time Time = {
		2017, 6, 28, 3, 12, 0, 0
	};


//	init_Iic();
//	yqi2c_init();
    rNF_Init();
	initial_nandflash();    //nandflash

	Hsmmc_Init ();//SD卡
	cy_println ("Hsmmc_init_flag is %d", Hsmmc_exist ());

	ini_picaddr();	 //初始化触摸屏变量
	/*下面把触摸屏上的一些变量初始化*/
	ini_screen ();
	prepic_num = JSJM;
	prepic_prenum = JSJM;
	cy_println ("Coin Detector System start");
	if (code_flag == 0x55555555){//如果是JTAG下载启动，就更新程序到Flash
		U8 r_code;
		extern unsigned int __CodeAddr__;
		extern unsigned int __CodeSize__;
	#if 1
		cy_println ("Code_flag = 0x%x, Begin Write code to flash...", code_flag);
		r_code = WriteAppToAppSpace ((uint32_t)__CodeAddr__, __CodeSize__);
		if (r_code == 0)
			cy_println ("write code to nand flash block 10 page 0 nand addr 0 completed");
		else
			cy_println ("write code to nand flash block 10 page 0 nand addr 0 failed");
	#endif
		code_flag = 0;
	}else{
		cy_println ("Code_flag = 0x%x, No Need Update code!", code_flag);
	}
	//begin init RTC************************************************************
	while(1) {    // read time
		comscreen(dgus_readt,6);	//read time
		tscount = 50;//1000ms 延时
		while(touch_flag == 0 ){if (tscount == 0) break;}//1秒后还没收到触摸屏的信息，有可能没有接屏，直接跳过
		Time.Year 	= HEX_TO_DEC (touchnum[6]) + 2000;
		Time.Month 	= HEX_TO_DEC (touchnum[7]);
		Time.Day 	= HEX_TO_DEC (touchnum[8]);
		Time.Week 	= HEX_TO_DEC (touchnum[9]);
		Time.Hour 	= HEX_TO_DEC (touchnum[10]);
		Time.Min 	= HEX_TO_DEC (touchnum[11]);
		Time.Sec 	= HEX_TO_DEC (touchnum[12]);
		touch_flag = 0;
		break;
	}
	RTC_Init(&Time);
	RTC_GetTime(&Time);
	cy_println("Time: %4d/%02d/%02d %02d:%02d:%02d", Time.Year,
				Time.Month, Time.Day, Time.Hour, Time.Min, Time.Sec);
	//end init RTC************************************************************

	LED1_ON;

 	sys_env.workstep = 0; //停机状态
	print_system_env_info ();//串口打印编译信息和系统环境变量，便于调试。
//	setStdValue	();//设置鉴伪基准值，后面每次启动之前都会设置一次，因为鉴伪基准值会随温度在一定范围内变化
//	adstd_offset ();//设置补偿值，后面每次启动之前都会补偿一次，因为鉴伪基准值会随温度在一定范围内变化

	comscreen(Disp_Indexpic[JSJM],Number_IndexpicB);	  // 跳转到主界面
	sys_env.system_delay = para_set_value.data.system_boot_delay;
	/*开机预热，如果时间不够，可适当延长*/
	//delay_ms(ELECTRICTIME);    //开机 延时 这些时间再给 单片机发
}
//int print_ad0, print_ad1, print_ad2;
void main_task(void)
{
	static unsigned int running_state = 0;
	running_state++;
	if (running_state >= 1000){
		running_state = 0;
		LED1_NOT;
	}
	if (sys_env.sys_runing_time){
		sys_env.sys_runing_time++;
	}
	AD_Sample_All ();
	switch (sys_env.workstep)
	{
		case 10:{        //main  proceed
			cy_ad0_valueget();    //check coin wave and get ADSAMPNUM of ad  values
			cy_ad1_valueget();    //check coin wave and get ADSAMPNUM of ad  values
			cy_ad2_valueget();    //check coin wave and get ADSAMPNUM of ad  values
			//////////////////////////////////////////////////////////////////////
			cy_precoincount();   //鉴伪、计数
			IR_detect_func();   //第二个踢币程序
			break;
		}
		case 22:{
			cy_ad0_valueget();    //check coin wave and get ADSAMPNUM of ad  values
			cy_ad1_valueget();    //check coin wave and get ADSAMPNUM of ad  values
			cy_ad2_valueget();    //check coin wave and get ADSAMPNUM of ad  values
			//////////////////////////////////////////////////////////////////////
			cy_coinlearn();   //特征学习
			break;
		}
		case 103:{
			cy_ad0_valueget();    //check coin wave and get ADSAMPNUM of ad  values
			cy_ad1_valueget();    //check coin wave and get ADSAMPNUM of ad  values
			cy_ad2_valueget();    //check coin wave and get ADSAMPNUM of ad  values
			break;
		}
		default:{
			break;
		}
	}
}




//NET处理 任务
//#define NET_MSG_SIZE 32
//OS_EVENT * net_msg;			//串口消息队列
//void * netMsgGrp[NET_MSG_SIZE];			//消息队列存储地址,最大支持32个消息
void Task3(void *pdata)
{
//	u8 err;
	(void)pdata;
	while (1) {
		if (sys_env.update_flag != NET_UPDATEING){
			OSTimeDly(20);
		}
		if (sys_env.workstep != 1){//待机状态下才能联网更新
			continue;
		}

		if (DM9000_GetReceiveStatus()){
			ethernetif_input(&DM9000_netif);
		}
		LwIP_Periodic_Handle(sys_now ());
		// Handle timeouts
		sys_check_timeouts();
	}
}
void Task2(void *pdata)
{
	(void)pdata;
	while (1) {
		//LED1_NOT;
		OSTimeDly(20); //
		if( touch_flag ==1){
			touchresult();//判断触摸 状态的函数
			touch_flag =0;
		}
		if (sys_env.uart0_cmd_flag == 1){
			vTaskCmdAnalyze ();//串口命令处理函数
			sys_env.uart0_cmd_flag = 0;
		}
		if (sys_env.tty_online_ms == 1){
			sys_env.tty_online_ms = 0;
			update_finish (sys_env.update_flag);
		}
		if (sys_env.net_task == 1){
			sys_env.net_task = 0;
			OSTimeDly(500);
			run_command ("reset");
		}
	}
}
void Task1(void *pdata)
{
	(void)pdata;
	while (1) {
		LED2_NOT;
		OSTimeDly(500); // LED3 1000ms闪烁void
//		cy_print ("IN0:%d ", A0IN0);
//		cy_print ("IN1:%d ", A0IN1);
//		cy_print ("IN2:%d ", A0IN2);
//		cy_print ("IN3:%d ", A0IN3);
//		cy_print ("IN4:%d ", A0IN4);
//		cy_print ("IN5:%d ", A0IN5);
//		cy_print ("IN6:%d ", A0IN6);
//		cy_print ("IN7:%d ", A0IN7);
//		cy_println ();
		//AD_Sample_All ();//		//Read AD 0 ////////////////////////////////////////////////////////////////////////////////////////
//	rADCMUX = 0;		//setup channel
//	rADCCON|=0x1;									//start ADC
//	while(rADCCON & 0x1);							//check if Enable_start is low
//	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
//	print_ad0 = ((int)rADCDAT0 & 0x3ff);
//
//	//Read AD 1 ////////////////////////////////////////////////////////////////////////////////////////
//	rADCMUX = 0x01;		//setup channel
//	rADCCON|=0x1;									//start ADC
//	while(rADCCON & 0x1);							//check if Enable_start is low
//	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
//	print_ad1 = ((int)rADCDAT0 & 0x3ff);
//
//	//Read AD 2 ////////////////////////////////////////////////////////////////////////////////////////
//	rADCMUX = 0x02;		//setup channel
//	rADCCON|=0x1;									//start ADC
//	while(rADCCON & 0x1);							//check if Enable_start is low
//	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
//	print_ad2 = ((int)rADCDAT0 & 0x3ff);

//		cy_println ("AD0:	%d,	AD1:	%d,	AD2:	%d", print_ad0,
//																						 print_ad1,
//																						 print_ad2);
		//cy_print(" OSIdleCtrRun: %ld  OSIdleCtrMax: %ld  \n", OSIdleCtrRun, OSIdleCtrMax);
		//cy_print(" CPU Usage: %02d%%\n",OSCPUUsage);
		//dgus_tf1word(ADDR_CPU_USAGE, OSCPUUsage);	//清分等级，暂时没有设置
		//cy_println ("***********************task 1***********************");
//		if((sys_env.workstep == 10) && (sys_env.print_wave_to_pc == 0)){
//			disp_allcount_to_pc ();
//		}
	}
}

OS_STK  TaskStartStk[TASK_START_STK_SIZE];
OS_STK  Task1Stk[TASK1_STK_SIZE];
OS_STK  Task2Stk[TASK2_STK_SIZE];
OS_STK  Task3Stk[TASK3_STK_SIZE];


void TaskStart(void *pdata)
{
	uint16_t test_v = 0;
	int i = 0;
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif
	(void)pdata;
	OSStatInit(); //开启统计任务
	coin_init ();
	// Initilaize the LwIP stack
	lwip_init();
	// ip address 192, 168, 1, 20
	ethernetif_config();
	httpd_init();
	//IAP_httpd_init ();
	tftp_init ();
	cy_println ("[Please press ENTER to activate this console]");

	test_v = calc_fb_func (900, 900, 200);
	cy_println ("%d", test_v);
	test_v = calc_fb_func (900, 900, 150);
	cy_println ("%d", test_v);
	test_v = calc_fb_func (900, 1000, 200);
	cy_println ("%d", test_v);
	test_v = calc_fb_func (900, 1000, 150);
	cy_println ("%d", test_v);
	test_v = calc_fb_func (900, 850, 200);
	cy_println ("%d", test_v);
	test_v = calc_fb_func (900, 850, 150);
	cy_println ("%d", test_v);


	//创建NET消息队列
	//net_msg=OSQCreate(&netMsgGrp[0], NET_MSG_SIZE);	//创建消息队列
	OSTaskCreate(Task1, (void *)0, &Task1Stk[TASK1_STK_SIZE - 1], Task1Prio);
	OSTaskCreate(Task2, (void *)0, &Task2Stk[TASK2_STK_SIZE - 1], Task2Prio);
	OSTaskCreate(Task3, (void *)0, &Task3Stk[TASK3_STK_SIZE - 1], Task3Prio);

	while (1) {
		OSTimeDly(10); // LED2 500ms闪烁

		switch (sys_env.workstep)
		{
			case 0:{
				ALL_STOP();//停掉所有的输出
				if (sys_env.sys_runing_time_total > 0){
					sys_env.coin_speed = ((processed_coin_info.total_coin - processed_coin_info.total_coin_old) * 60) / (sys_env.sys_runing_time_total / 10000);
					refresh_data ();
				}
				disp_allcount ();
				sys_env.workstep = 1;
				break;
			}
			case 1://待机状态
				break;
			case 3:{  //清分启动
				sys_env.workstep = 6;  //主函数 步骤号
				deviceinit();//初始化变量
				break;
			}
			case 6: {
				if( adstd_offset() == 1){//  检测基准值，并进行补偿
				setStdValue	();//设置鉴伪基准值
				//if (1) {//  检测基准值，并进行补偿
					sys_env.stop_time = STOP_TIME;//无币停机时间
					sys_env.workstep =10;
					if ((sys_env.auto_clear == 1) || para_set_value.data.coin_full_rej_pos == 3){//如果设置自动清零，则每次启动都清零计数
						for (i = 0; i < COIN_TYPE_NUM; i++){
							*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_full_flag = 0; //
							*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_cur = 0; //
							coin_env.full_stack_num = 0;
						}
						processed_coin_info.total_money =0;
						processed_coin_info.total_coin = 0;
						processed_coin_info.total_good = 0;
						processed_coin_info.total_ng = 0;
						processed_coin_info.coinnumber = 0;
						good_value_index = 0;
						ng_value_index = 0;
						disp_allcount();
						disp_data(ADDR_CPZE,ADDR_CPZS,ADDR_CPFG);			//when counting pre ze zs foege data variable
					}
				}else{
					SEND_ERROR(ADSTDEEROR);   //传感器下有币
					dbg("the voltage is wrong \r\n");
				}
				break;
			}
			case 10:{        //main  proceed
				runfunction();	 //转盘动作函数
				if(blockflag == 0){//堵币
					SEND_ERROR(PRESSMLOCKED);
				}
				if ((sys_env.coin_under_sensor == 1) || (sys_env.rej_over == 1)){
					sys_env.coin_under_sensor = 0;
					sys_env.rej_over = 0;
					disp_allcount ();
				}
				if (sys_env.stop_time == 0){
					switch (sys_env.stop_flag){
						case 0:
							STORAGE_DIR_N();//反转
							sys_env.stop_time = 25;
							sys_env.stop_flag = 1;
							break;
						case 1:
							STORAGE_DIR_P();//正转
							sys_env.stop_time = STOP_TIME;//无币停机时间10秒
							sys_env.stop_flag = 2;
							break;
						case 2:
							STORAGE_MOTOR_STOPRUN();	//  转盘电机
							sys_env.stop_time = 100;//STOP_TIME;//无币停机时间2秒
							sys_env.stop_flag = 3;
							break;
						case 3:
							comscreen(Disp_Indexpic[JSJM],Number_IndexpicB);	 // back to the  picture before alert
							sys_env.workstep =0;
							sys_env.stop_flag = 6;
							break;
						case 4://等待反转完成
							STORAGE_DIR_P();//正转
							sys_env.stop_time = STOP_TIME;//无币停机时间10秒
							sys_env.stop_flag = 0;
							break;
						case 5:
							break;
						default:break;
					}
				}
				if (sys_env.print_wave_to_pc == 1){
					if (sys_env.AD_buf_sending == 1){
						send_sample_data (sys_env.Detect_AD_buf_p, sys_env.AD_data_len);
						sys_env.AD_buf_sending = 0;
					}
				}
				break;
			}
			/////////////////
			case 13: {//特征学习
				sys_env.workstep = 20;  //主函数 步骤号
				deviceinit();
				break;
			}
			case 20:{
				if( adstd_offset() == 1){//检测 基准值    不调试到正常值  不能进行 自学习
					sys_env.workstep =22;
				}else{
					SEND_ERROR(ADSTDEEROR);   //  请调整基准值
					//cy_print("the voltage is wrong\n");
				}
				break;
			}
			case 22:{
				runfunction();	 //转盘动作函数
				if (sys_env.coin_under_sensor == 1){
					sys_env.coin_under_sensor = 0;
					dgus_tf1word(ADDR_A0MA,coin_maxvalue0);	//	 real time ,pre AD0  max
					dgus_tf1word(ADDR_A0MI,coin_minvalue0);	//	 real time ,pre AD0  min
					dgus_tf1word(ADDR_A1MA,coin_maxvalue1);	//	 real time ,pre AD1  max
					dgus_tf1word(ADDR_A1MI,coin_minvalue1);	//	 real time ,pre AD1  min
					dgus_tf1word(ADDR_A2MA,coin_maxvalue2);	//	 real time ,pre AD2  max
					dgus_tf1word(ADDR_A2MI,coin_minvalue2);	//	 real time ,pre AD2  min
				}

				if(blockflag == 0){//堵币
					SEND_ERROR(PRESSMLOCKED);
				}
				break;
			}
			case 88:{//报错
				ALL_STOP();
				alertfuc(alertflag);
				sys_env.workstep = 0;
				break;
			}
			case 100:{////基准调试
				cy_adstd_adj ();
				break;
			}
			case 101:{////红外传感器读取
				detect_read ();
				break;
			}
			case 103:{// 进行特征值波形采样，上传电脑
				if (sys_env.coin_leave == 1){
					OS_ENTER_CRITICAL();
					sys_env.coin_leave = 0;
					OS_EXIT_CRITICAL();
					send_sample_data (sys_env.Detect_AD_buf_p, sys_env.AD_data_len);
					sys_env.AD_buf_sending = 0;
				}
				break;
			}
			default:{
				break;
			}
		}
	}
}
extern int System_call(uint32_t call_id);
int main (void)
{
//	int re;
	//main_task (0);
 	port_Init();
	uart_init();//115200bps
//	re = System_call (9);
	Init_OS_ticks ();
	OSInit(); // 初始化uCOS
	OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_START_STK_SIZE-1], TaskStartPrio);
    OSStart(); // 开始uCOS调度
	return 0;
}


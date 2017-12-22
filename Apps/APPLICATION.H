#define CH00  1
#define CH01  1<<1
#define CH02  1<<2
#define CH03  1<<3
#define CH04  1<<4
#define CH05  1<<5
#define CH06  1<<6
#define CH07  1<<7
#define CH08  1<<8
#define CH09  1<<9
#define CH10  1<<10
#define CH11  1<<11
#define CH12  1<<12
#define CH13  1<<13
#define CH14  1<<14
#define CH15  1<<15
#define CH16  1<<16
#define CH17  1<<17
#define CH18  1<<18
#define CH19  1<<19
#define CH20  1<<20
#define CH21  1<<21
#define CH22  1<<22
#define CH23  1<<23
#define CH24  1<<24
#define CH25  1<<25
#define CH26  1<<26
#define CH27  1<<27
#define CH28  1<<28
#define CH29  1<<29
#define CH30  1<<30
#define CH31  0x80000000
/*---------------------------------------------------------------------------*/
/*时间定义*/
#define PerCent		250				/*10毫秒*/
#define TenPerCent	PerCent * 10 	/*0.1秒*/
#define Secend		TenPerCent * 10 /*1秒*/
#define TenSecend	TenPerCent * 100 /*10秒*/
#define Motor220Wait TenPerCent*8 /*0.8秒交流电机防立刻反转损坏的时间间隔*/
#define Motor220_1_Max  Secend*8 /*1号220V交流电机最多转动时间，此值应比实际行程略长0.5秒，这是为了安全*/
#define Motor220_2_Max  Secend*8 /*2号220V交流电机最多转动时间，此值应比实际行程略长0.5秒，这是为了安全*/
#define Motor220_3_Max  Secend*8 /*3号220V交流电机最多转动时间，此值应比实际行程略长0.5秒，这是为了安全*/
#define Motor220_4_Max  Secend*8 /*4号220V交流电机最多转动时间，此值应比实际行程略长0.5秒，这是为了安全*/
#define Motor220_5_Max  Secend*8 /*5号220V交流电机最多转动时间，此值应比实际行程略长0.5秒，这是为了安全*/
#define Motor220_6_Max  Secend*8 /*6号220V交流电机最多转动时间，此值应比实际行程略长0.5秒，这是为了安全*/
#define Motor220_7_Max  Secend*8 /*7号220V交流电机最多转动时间，此值应比实际行程略长0.5秒，这是为了安全*/
#define Motor220_8_Max  Secend*8 /*8号220V交流电机最多转动时间，此值应比实际行程略长0.5秒，这是为了安全*/
#define Motor220_9_Max  Secend*8 /*9号220V交流电机最多转动时间，此值应比实际行程略长0.5秒，这是为了安全*/
/*---------------------------------------------------------------------------*/
/*关于传感器的定义2011-06-15*/
#define SeperaterOnStart  CH00 /*传感器：分开片零位      线号08*/
#define TapeOnStart       CH01 /*传感器：纸夹零位        线号07*/
#define HostOnTapeStage   CH03 /*传感器：捆扎部绕纸位    线号05*/
#define HostOnHeatStage   CH02 /*传感器：捆扎部烫合位    线号06*/
#define Tape2Ready        CH04 /*传感器：纸道对射        线号04*/
#define Tape1Ready        CH05 /*传感器：纸道对射        线号03*/
#define TapeRight         CH06 /*传感器：纸量检测        线号02*/
#define TapeLeft          CH07 /*传感器：纸量检测        线号01*/ 

#define BoardOverLow      CH08 /*传感器：托钞板下限位    线号16*/
#define BoardOverTop      CH09 /*传感器：托钞板上限位    线号15*/
#define HostOnLow         CH10 /*传感器：捆扎部低位      线号14*/
#define HostOnStart       CH11 /*传感器：捆扎部零位      线号13*/
#define HostOnTop         CH12 /*传感器：捆扎部高位      线号12*/
#define TurnerOnStart     CH13 /*传感器：翻转部0位       线号11*/
#define TapeWrapedRight   CH14 /*传感器：纸带已拉紧      线号10*/
#define TapeWrapedLeft    CH15 /*传感器：纸带已拉紧      线号09*/
#define ValueNumber	128
//#define HostOverLow       CH11 /*传感器：捆扎部最低位(微动开关)   线号13*/
//#define HostOverTop       CH12 /*传感器：捆扎部最高位(微动开关)   线号12*/

#define TapeHozontal      CH16 /*传感器：纸夹水平位      线号24*/
#define StartBotton       CH17 /*启动按钮，              线号23*/
#define TapeOutBotton     CH18 /*退纸带按钮，            线号22*/
#define TapeInBotton      CH19 /*进纸带按钮，            线号21*/
#define MoneyOnBoard      CH20 /*从航空端子来的传感器信号，托钞板上有钱把  线号20*/
#define WrapperWaitSensor CH21 /*从航空端子来的传感器信号，捆扎部延时翻出  线号19*/
#define MoneyOnTape       CH22 /*从航空端子来的传感器信号：传送带有钱把    线号18*/
#define BoxOnStart        CH23 /*传感器：整理斗零位     线号17*/

#define PullStop          CH29 /*推币汽缸回到初始位        线号27*/
#define PushStop          CH30 /*推币汽缸到停止位        线号26*/
#define TapeOnSeperator   CH31 /*纸夹在分开片位置        线号25*/

/*---------------------------------------------------------------------------*/
/*时间常量设定*/
#define WrapperWait  15000*TimerSpeed /*捆扎部延时翻出的时间*/
#define CombineTime  89900 /*分开爪合并的时间*/
#define HostGoDownTime   40000*TimerSpeed /*捆扎部下压，压紧10把钱的时间*/
#define BoardDownStep 65 /*200---340--280托钞板降一格 对应步进电机的步数*/
#define BoardSpeed    6  /*托钞板速度*/
#define SpeedOfTapeIn 10  /*进纸速度*/
#define SpeedTapeWrap 4*TimerSpeed  /*绕纸速度*/
#define AmountOfTapeBack 80 /*=80绕纸程序中的退纸量*/

#define AmountOfTapeIn 1050 /*预进纸量1000--1400--1230-1310-1120*/

#define TapeInMax  12000*TimerSpeed /*进纸程序最多执行时间，毫秒数*/
#define TapeRunMax 63000*TimerSpeed /*进纸程序最多执行时间，毫秒数   5000=1S*/
#define AfterHeat  15000 /*烫头停止加热后等待时间        33000时 检测到烫头下压到抬起总共  4.78S左右   20000   变2.89  */
#define BoxGoBackMax 25000 /*整理斗退回过程要限定在此时间内，超时会报错。应该>=20000*/
/*---------------------------------------------------------------------------*/
/*关于LCD输出的定义*/
#define LCD_RESET P031 /*P0.31口*/
#define LCD_SCLK  P028 /*P0.28口*/
#define LCD_CS    P029 /*P0.29口*/
#define LCD_SID   P030 /*P0.30口*/

/*关于220V输出的定义*/
#define P220_0 CH17 /*主电机降*/
#define P220_1 CH19 /*主电机升*/
#define P220_2 CH31 /*空*/
#define P220_3 CH28 /*烫头*/

/* --------------------------------------------------   */

#define P24_00 CH30 /*推把币汽缸电磁阀1 传送带推一把钱的汽缸        从上到下数第1（CH30），线号CY1*/
#define P24_08 CH27 /*推把币汽缸电磁阀2 传送带推一把钱的汽缸        从上到下数第2（CH27），线号CY2*/
#define P24_01 CH29 /*分爪汽缸，                从上到下数第3，线号CY3*/
#define P24_03 CH24 /*气缸C1动作烫合纸带并切带，从上到下数第4，线号CY4*/
#define P24_02 CH22 /*拍齐汽缸，                从上到下数第5，线号CY5*/
#define P24_04 CH23    /*推整理斗气缸           从上到下数第6，线号CY6,推10把币汽缸*/
#define P24_09 CH11 /*主气路电磁阀              从上到下数第7，线号CY7*/
#define P24_05 CH10  /*空，                      从上到下数第8，*/
#define P24_06 CH08 /*去控制柜1                 从上到下数第9，        线号50*/
#define P24_07 CH07 /*去控制柜2                 从上到下数第10，       线号51*/
#define P24_10 CH04
#define P24_11 CH01

/* --------------------------------------------------   */
/*关于步进电机的定义2011-06-14更新*/
/*分开片电机*/
#define PU00 CH09 /*步进电机00的脉冲*/
#define DR00 CH06 /*步进电机00的方向*/
#define StepMotor00_ms 10; /*步进电机03一步，半周期毫秒数*/
/*进纸带步进电机*/
#define PU01 CH13 /*步进电机01的脉冲*/
#define DR01 CH15 /*步进电机01的方向*/
#define StepMotor01_ms 10; /*步进电机02一步，半周期毫秒数*/
/*托钞板电机*/
#define PU02 CH26 /*步进电机02的脉冲*/
#define DR02 CH25 /*步进电机02的方向*/
#define StepMotor02_ms 10; /*步进电机05一步，半周期毫秒数*/
/*绕纸夹电机*/
#define PU03 CH20 /*步进电机03的脉冲*/
#define DR03 CH18 /*步进电机03的方向*/
#define StepMotor03_ms 10; /*步进电机04一步，半周期毫秒数*/

#define PU04 CH03 /*步进电机04的脉冲*/
#define DR04 CH02 /*步进电机04的方向*/
#define StepMotor04_ms 10; /*步进电机01一步，半周期毫秒数*/

#define PU05 CH05 /*步进电机05的脉冲*/
#define DR05 CH00 /*步进电机05的方向*/
#define StepMotor05_ms 10; /*步进电机00一步，半周期毫秒数*/

/*---------------------------------------------------------------------------*/
/*函数定义*/
/*低32位开关量输入函数*/
//unsigned int InputLow32(void);
//void ErrorReport(unsigned int ErrorNumber);
/*通用步进电机转动函数。参数：步进电机号。返回：剩余脉冲数*/
//unsigned int StepMotorRun(unsigned int Motor);
/*通用步进电机设置函数。参数：步进电机号，方向，脉冲数，半脉冲毫秒数。返回：1正确，0步进电机号错误。*/
//unsigned int StepMotorSet(unsigned int Motor,unsigned int drection,long Pulse,unsigned int ms);
/*220V电机状态。参数：无。返回：0不转，P220_0降，P220_1升*/
//unsigned int Motor220Status(void);
/*220V电机升。参数：转动时间。返回：1开始转动，0正在降或等待*/
//unsigned int Motor220RunUp(unsigned int ms);
/*220V电机降。参数：转动时间。返回：1开始转动，0正在升或等待*/
//unsigned int Motor220RunDown(unsigned int ms);
/*220V电机停止*/
//void Motor220Stop(void);

/*整理机托钞板(使用步进电机02)的初始化*/
//void Process02Format(void);
/*整理机进纸初始化*/
//void ProcessTapeIn(void);
/*---------------------------------------------------------------------------*/
/*错误代码定义*/
#define Err220OnTop       1001    /*报错信息：捆扎部超上限*//*报错:主电机到低位*/  
#define Err220OnLow       1002    /*报错信息：捆扎部超下限*/
#define ErrPowerOut         1007    /*提示:已拍急停按钮*/
#define ErrStepNumer        1003    /*报错信息：程序出现非标准步骤号*/
#define Err220TimeRunOut    1004    /*报错信息：主电机转动超时*/
#define ErrTurnerOnStart    1005    /*报错:翻转机构没有回归零位*/
#define ErrBoxOnStart		1008 	/*报错:整理斗没有回归零位(可能气压不足)*/
#define	ErrBoardOverTop     1010  	/*报错:托钞板超时未到上限位*/
#define	ErrTapeOnStart		1012 	/*报错:绕纸夹超时未到零位*/ /*绕纸夹脉冲数过多也没到0位*/
#define ErrSeperaterOnStart 1013  	/*报错:分开片超时未到零位*/ /*脉冲数过多也没到0位*/
#define ErrTapeReady 		1016 	/*报错:纸带未进到位(可能是长度相差过大)*/ /*脉冲数过多也没有进纸到位*//*可能卡纸*/
#define ErrTapeEquip 		1018 	/*报错:纸带不等*/


#define	ErrTapeWraped		1022	/*报错:纸带未拉紧*/
#define	ErrTapeRun  		1023	/*报错:绕纸超时未完成*/
#define	ErrBoardOverLow		1024	/*报错:托钞板未到下限位 脉冲数过多也没到下限位*/
#define ErrBoardOverLow2    1026	/*报错:托钞板提前到下限位,计数错误*/
#define ErrTapeOut          1027    /*报错:请换纸带*/
#define ErrMoneyUnwrap      1028    /*报错:钱把散了*/
#define ErrMoneyLess        1030    /*报错:钱把不足*/
#define ErrPush             1035    /*报错30:请检查翻转机构零位与托钞板下限位*/
#define ErrPush2            1037    /*报错：整理斗未动作*/
#define ErrTapeGet          1041    /*报错:绕纸夹脉冲丢失*/

#define ErrMoneyOnBoard     1080    /*报错:落入的钱把过厚*/
                                        /*报错:请检查落入的钱把数量是否为：*/
#define ErrBoard            1081    /*报错:请取走托钞板内的钱把*/

/*---------------------------------------------------------------------------*/


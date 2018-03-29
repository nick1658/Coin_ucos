#ifndef __YQDRIVE_H__
#define __YQDRIVE_H__


//#define _MY_DEBUG_NO_DOOR_CHECK_
#define _MY_DEBUG_NO_PAN_BLOCK_ERROR_
#define _MY_DEBUG_NO_CHANEL_BLOCK_ERROR_
#define _MY_DEBUG_NO_KICK_ERROR_
//输出 端口
#define STARTRUN 0
#define STOPRUN 1

////////////////////////////////////new pcb open this
//////////////////////////

	
#define OUT7(x)		(rGPGDAT = (rGPGDAT & (~0x8)) |(x<<3))	//EINT11/GPG3
#define OUT6(x) 	(rGPBDAT = (rGPBDAT & (~0x2)) |(x<<1))	 	//TOUT1/GPB1
#define OUT5(x) 	(rGPBDAT = (rGPBDAT & (~0x1)) |(x))	 	//TOUT0/GPB0
#define OUT4(x) 	(rGPHDAT = (rGPHDAT & (~0x2000)) |(x<<13))	//CLKOUT0/GPH13	
#define OUT3(x) 	(rGPHDAT = (rGPHDAT & (~0x4000)) |(x<<14))	//CLKOUT1/GPH14	
#define OUT2(x) 	(rGPGDAT = (rGPGDAT & (~0x80)) |(x<<7))	//EINT15/GPG7 
#define OUT1(x) 	(rGPFDAT = (rGPFDAT & (~0x2)) |(x<<1))	//EINT1/GPF1/sd0_CDN
#define OUT0(x) 	(rGPEDAT = (rGPEDAT & (~0x20)) |(x<<5))	//SD0_CLK/GPE5
				  
#define  PAN_MOTOR_LEFT(x) 	 OUT7(x)			//P2^0; 	//EINT11/GPG3 //  转盘反转
#define  PAN_MOTOR_RIGHT(x)  OUT6(x)			//P2^1; TOUT1/GPB1  //  转盘正转 PAN_MOTOR_RIGHT(x)
#define  PRESS_MOTOR(x) 	 OUT5(x)			//P2^2; TOUT0/GPB0  //压币电机

#define  STORAGE_MOTOR(x) 	 OUT0(x)			//P2^3; CLKOUT0/GPH13  //斗送入电机
#define  STORAGE_DIR(x) 	 OUT1(x)			//P2^3; CLKOUT0/GPH13  //斗送入电机转向
#define  EMKICK1(x) 		 OUT3(x)			//P2^4; CLKOUT1/GPH14      // kick out 
#define  EMKICK2(x) 		 OUT2(x)			//P2^5; //EINT15/GPG7   //kick back



#define STORAGE_MOTOR_STARTRUN() STORAGE_MOTOR(1)
#define STORAGE_MOTOR_STOPRUN() STORAGE_MOTOR(0)


#define STORAGE_DIR_P() STORAGE_DIR(0) //正转
#define STORAGE_DIR_N() STORAGE_DIR(1) //反转



#define ALL_STOP()	STORAGE_MOTOR_STOPRUN(); \
					STORAGE_DIR_P(); \
					OUT2(STOPRUN); \
					OUT3(STOPRUN); \
					OUT4(STOPRUN); \
					OUT5(STOPRUN); \
					OUT6(STOPRUN); \
					OUT7(STOPRUN)

#define A0IN7 	((rGPEDAT & 0x40)>>6)	//SD0_CMD/GPE6
#define A0IN6 	((rGPEDAT & 0x80)>>7)	//SD0_DAT0/GPE7
#define A0IN5 	((rGPEDAT & 0x100)>>8)	//SD0_DAT1/GPE8
#define A0IN4 	((rGPEDAT & 0x200)>>9)	//SD0_DAT2/GPE9
#define A0IN3 	((rGPEDAT & 0x400)>>10)	//SD0_DAT3/GPE10

////////////////////////////////////old pcb open this
//#define A0IN0 	((rGPGDAT & 0x10)>>4)	//EINT12/GPG4
//#define A0IN1 	((rGPGDAT & 0x20)>>5)	//EINT13/GPG5
//#define A0IN0 	((rGPGDAT & 0x40)>>6)	//EINT14/GPG6
////////////////////////////////////new pcb open this
#define A0IN2 	((rGPGDAT & 0x40)>>6)	//EINT14/GPG6
#define A0IN1 	((rGPGDAT & 0x20)>>5)	//EINT13/GPG5
#define A0IN0 	((rGPGDAT & 0x10)>>4)	//EINT12/GPG4


#ifdef _MY_DEBUG_NO_DOOR_CHECK_
	#define COIN_DETECT      1	//P0^0;  //转盘槽型传感器       EINT12/GPG4  	
	#define PANCOIN_OVER  0	//P0^1;  //转盘满币对射传感器   EINT13/GPG5 
	#define DC_COINOVER   1	//P0^2;  // 清分 满币 传感器    EINT14/GPG6 
	#define DOOR_MAG      0	//P0^3;  //压币带门 传感器      SD0_DAT3/GPE10
	#define DOOR_PAN      0	//P0^4;  //转盘门 传感器  	    SD0_DAT2/GPE9
	#define PRESS_MICROSW 0	//P0^5; //压币带压合传感器   	SD0_DAT1/GPE8
	#define DT_METAL      1	//P0^6; //真假币金属传感器   	SD0_DAT0/GPE7
	#define DT_NOMETAL    1	//P0^7 ;  // 间隙金属传感器  	SD0_CMD/GPE6
#else
	#define PANCOIN_OVER      A0IN0	//P0^0;  //转盘槽型传感器       EINT12/GPG4  	
	#define COIN_DETECT   A0IN1	//P0^1;  //转盘满币对射传感器   EINT13/GPG5 
	#define DC_COINOVER   A0IN2	//P0^2;  // 清分 满币 传感器    EINT14/GPG6 
	#define DOOR_MAG      A0IN3	//P0^3;  //压币带门 传感器      SD0_DAT3/GPE10
	#define DOOR_PAN      A0IN4	//P0^4;  //转盘门 传感器  	    SD0_DAT2/GPE9
	#define PRESS_MICROSW A0IN5	//P0^5; //压币带压合传感器   	SD0_DAT1/GPE8
	#define DT_METAL      A0IN6	//P0^6; //真假币金属传感器   	SD0_DAT0/GPE7
	#define MOTOR_NG      A0IN7	//P0^7 ;  // 间隙金属传感器  	SD0_CMD/GPE6
#endif

#define  IR_DETECT_ON   0	//  //红外对射传感器 0 表示有遮挡 

////////////////////////////
void deviceinit(void);	//开机前初始化

#define STOP_TIME para_set_value.data.motor_idle_t//无币停机时间10秒

extern volatile uint32_t runtime;   // // 执行件 用的计时
extern unsigned short int runstep;  //  部件动作步骤号
extern volatile uint32_t time;   // //踢币电磁铁 用的计时


void runfunction(void);   //部件动作函数
void IR_detect_func(void);	 //红外对射电眼踢币程序










#endif /*__YQDRIVE_H__*/

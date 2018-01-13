
#ifndef __TQ2416_YQSCREEN_H__
#define __TQ2416_YQSCREEN_H__


#include "def.h"


void comscreen(U8* str,short int length);  //发送 指令至液晶屏
extern volatile U16 touch_flag;
extern volatile U16 uartcount;  // 串口2接收 字节 计数

void touchget_from_uart0(void);     //receive the touch from dgus at real time 
void touchget_from_uart2(void);     //receive the touch from dgus at real time 
//int dgus_rtc(char str[]);  // read back the RTC
#define TIMEBP1 50     //beep time  for  kai ji
#define TIMEBP2 100     //beep time  for  alert
void dgus_tfbeep(int bztime); 			 	 //uart1,dgus deep control
void dgus_chinese(U16 addr,S8 str[],U16 strnum);    // dgus  chinese
void dgus_tf2word(int addr,long name);  	  //transfer 2word data variables to specify address 
void dgus_tf1word(int addr,long data);  	  //transfer 1word data variables to specify address 
void disp_KJAmount(void); // initial addr on zhu jiemian ze zs forge
void disp_data(int addr1,int addr2,int addr3); 	 //pre picture ,specify address display data variable
void disp_preselflearn(int max0,int min0,int max1,int min1,int max2,int min2);    //pre coin admax admin when self learning 
void counter_clear (void); //


/////////////////变量/////////////////////////

#define	ADDR_CPZE 	0x0000 	 // 数据变量显示 计数界面 金额
#define	ADDR_CPZS 	0x0002 	 // 数据变量显示   计数界面 数量
#define	ADDR_CPFG 	0x0004 	 // 数据变量显示  计数界面 异币

#define ADDR_CRUN    0x06  	 // 按键值返回 计数界面  启动按钮 
#define ADDR_PGH1	0x07  	 // 按键值返回   工号
#define	ADDR_TZBC 	0x08  	//  按键值返回
#define	ADDR_DBDATA 0x09  	//  按键值返回 
#define	ADDR_BJZX 	0x0A  	//  按键值返回  bujian zhixing jiemian 

#define	ADDR_XD10 	0x000B  	// 数据变量显示  详单  jiemian 1
#define	ADDR_XD5 	0x000D  	// 数据变量显示  详单   jiemian 0.5
#define	ADDR_XD1 	0x000F  	// 数据变量显示   详单   jiemian 0.1
#define	ADDR_XD05 	0x0011  	// 数据变量显示  详单  jiemian 0.05
#define	ADDR_XD02 	0x0013  	// 数据变量显示  详单  jiemian 0.02
#define	ADDR_XD01 	0x0015  	// 数据变量显示   详单 jiemian 0.01
#define	ADDR_XDB1 	0x0017  	// 数据变量显示  详单   jiemian 大1角
#define	ADDR_XDB2 	0x0019  	// 数据变量显示   详单  jiemian 补2 现用于欧元第8种
//纪念币
#define	ADDR_XD1000 	0x001B  	// 数据变量显示   详单  jiemian 纪念币10元
#define	ADDR_XD500 	0x001D  	// 数据变量显示   详单  jiemian 纪念币5元

#define	ADDR_XDZS 	ADDR_CPZS 	// 数据变量显示  详单  jiemian ZS 
#define	ADDR_XDFG 	ADDR_CPFG 	// 数据变量显示  详单  jiemian FORGE
//详单   加币种  可以使用的地址  001B -001C  001D- 001E
#define ADDR_YZ_RESET  0x1F   		// 按键值返回   预置,	mianzhi  coin name



#define	ADDR_YZS0 	0x0029 	 // 数据变量显示 ,  预置,	data variable
#define	ADDR_YZS1	0x002B  //  数据变量显示 ,  预置,,	data variable
#define	ADDR_YZS2	0x002D  //  数据变量显示 ,  预置,  , data variable
#define	ADDR_YZS3 	0x002F  //  数据变量显示 ,  预置,,	data variable
#define	ADDR_YZS4	0x0031  //  数据变量显示 ,  预置,	data variable
#define	ADDR_YZS5	0x0033  //  数据变量显示 ,  预置, data variable
#define	ADDR_YZS6	0x0035  //  数据变量显示 ,  预置, data variable
#define	ADDR_YZS7	0x0037  //  数据变量显示 ,  预置, data variable
#define	ADDR_YZS8	0x0039  //  数据变量显示 ,  预置, data variable

//纪念币
#define	ADDR_YZS9	0x0539  //  数据变量显示 ,  预置10, data variable
#define	ADDR_YZS10	0x053b  //  数据变量显示 ,  预置5, data variable

//-003A
#define ADDR_CNTB   0x003B  //  图标变量显示  zi xuexi  jiemian learning ,coin name tubiao dispaly
#define ADDR_CNTB1   0x3B   //  按键值返回  ,  zi xuexi  jiemian learning ,coin name tubiao dispaly
#define ADDR_LRUN    0x3C	//   按键值返回  ,zi xuexi  jiemian run
#define	ADDR_A0MA 	0x003D  //   数据变量显示 ,  zi xuexi  jiemian learning ,pre AD0  max
#define	ADDR_A0MI 	0x003E  //   数据变量显示 , zi xuexi  jiemian learning ,pre AD0  min
#define	ADDR_A1MA 	0x003F  //   数据变量显示 , zi xuexi  jiemian learning ,pre AD1  max
#define	ADDR_A1MI 	0x0040  //   数据变量显示 , zi xuexi  jiemian learning ,pre AD1  min
#define	ADDR_A2MA 	0x0041  //   数据变量显示 , zi xuexi  jiemian learning ,pre AD2  max
#define	ADDR_A2MI 	0x0042  //   数据变量显示 , zi xuexi  jiemian learning ,pre AD2  min
#define ADDR_GSTB   0x0043  //   图标变量显示   zi xuexi  jiemian learning ,ganshe tubiao dispaly addr
#define	ADDR_PRIT 	0x0044  	//  按键值返回 print

#define ADDR_KICK1_M	0x0045   //  图标变量显示  bujian zhixing jiemian, cyline 1
#define ADDR_KICK2_M	0x0046   //  图标变量显示  bujian zhixing jiemian, cyline 2
#define ADDR_STORAGE_MOTOR	0x0047   //  图标变量显示  bujian zhixing jiemian, pan motor right zheng
#define ADDR_DETCET1	0x0048   //  图标变量显示  bujian zhixing jiemian, pan motor left fan
#define ADDR_DEBUG	0x0049   //  图标变量显示  bujian zhixing jiemian, chuangsong motor
#define ADDR_MODE	0x004A   //  图标变量显示  bujian zhixing jiemian, yabi motor
#define ADDR_ZXLD	0x004B   //  图标变量显示  bujian zhixing jiemian, three  motor run together


#define ADDR_ZXD1	0x004C   //  数据变量显示 不需要初始化  bujian zhixing jiemian, detector 
#define ADDR_ZXD2	0x004D   //   数据变量显示  不需要初始化  bujian zhixing jiemian, detector 
#define ADDR_ZXD3	0x004E   //   数据变量显示  不需要初始化  bujian zhixing jiemian, detector 
#define ADDR_ZXD4	0x004F   //   数据变量显示   不需要初始化  bujian zhixing jiemian, detector 
#define ADDR_ZXD5	0x0050   //   数据变量显示  不需要初始化  bujian zhixing jiemian, detector 
#define ADDR_ZXD6	0x0051   //   数据变量显示  不需要初始化 bujian zhixing jiemian, detector 
#define ADDR_ZXD7	0x0052   //   数据变量显示  不需要初始化 bujian zhixing jiemian, detector 
#define ADDR_ZXD8	0x005C   //   数据变量显示  不需要初始化 bujian zhixing jiemian, detector 
//0052
#define	ADDR_STDH 	0x0053  //  数据变量显示   不需要初始化  jizhun tiaoshi jiemian h frequence std display,	data variable
#define	ADDR_STDM 	0x0054  //  数据变量显示    不需要初始化 jizhun tiaoshi jiemian m frequence std display,	data variable
#define	ADDR_STDL 	0x0055  //  数据变量显示    不需要初始化 jizhun tiaoshi jiemian l frequence std display,	data variable
#define	ADDR_STDT 	0x005A  //  数据变量显示    不需要初始化 jizhun tiaoshi jiemian 温度值 frequence std display,	data variable

#define	ADDR_KICK_DELAY_T1 	0x0056  //  数据变量显示  修改币种 不需要初始化   canshu shezhi jiemian , kick out delay time
#define	ADDR_KICK_KEEP_T1 	0x0057  //  数据变量显示 修改币种 不需要初始化      canshu shezhi jiemian , kick back delay time
#define	ADDR_KICK_DELAY_T2 	0x0058  //  数据变量显示 修改币种 不需要初始化      canshu shezhi jiemian , pan motor blocked detector delay time
#define	ADDR_KICK_KEEP_T2 	0x0059  //  数据变量显示  修改币种 不需要初始化     canshu shezhi jiemian , STORE motor blocked detector delay time
#define	ADDR_MOTOR_IDLE_T	0x005A 	//        canshu shezhi jiemian , STORE motor blocked detector delay time
#define	ADDR_PRE_COUNT_STOP_N 0x005B 	//        canshu shezhi jiemian 


//5E --5F
#define	ADDR_ZLE 	0x0060  // 数据变量显示  zhu jiemian leiji money display,	data variable       NINTIAL  REALTIME  KEEPIN
#define	ADDR_ZLS	0x0062  // 数据变量显示   zhu jiemian leiji number display,data variable       NINTIAL  REALTIME  KEEPIN
#define	ADDR_ZLF 	0x0064  // 数据变量显示   zhu jiemian leiji forge display,	data variable       NINTIAL  REALTIME  KEEPIN
#define	ADDR_ZLH 	0x0066  // 数据变量显示   zhu jiemian leiji history number display,	data variable       NINTIAL  REALTIME  KEEPIN
//68-6d
#define	ADDR_CNCH 	0x006F  //  图标变量显示 币种 图标
#define	ADDR_CNCH1 	0x6F 	//  按键值返回  , 币种 图标     


#define ADDR_LEVEL100 	0x200   // 1元硬币的清分等级设置
#define ADDR_LEVEL50 	0x201   // 0.5元硬币的清分等级设置
#define ADDR_LEVEL10 	0x202   // 0.1元硬币的清分等级设置
#define ADDR_LEVEL5 	0x203   // 0.05元硬币的清分等级设置
#define ADDR_LEVEL2 	0x204   // 0.02元硬币的清分等级设置
#define ADDR_LEVEL1 	0x205   // 0.01元硬币的清分等级设置
#define ADDR_LEVEL1000 	0x206   // 10元硬币的清分等级设置
#define ADDR_LEVEL500 	0x207   // 5元硬币的清分等级设置

#define ADDR_CPU_USAGE 0x300
#define ADDR_PASSWORD 0x302 //password
#define ADDR_RESET 0x304 //reset

//报错 函数  
#define ALERT_MSG(TIPS,MSG) { \
	dgus_tfbeep(TIMEBP2); \
	prepic_prenum = prepic_num; \
	dgus_chinese(0x400, TIPS, strlen(TIPS)); \
	dgus_chinese(0x416, MSG, strlen(MSG)); \
	comscreen(Disp_Indexpic[BJJM],Number_IndexpicB); \
}

#define SEND_ERROR(CODE) { \
	alertflag = CODE; \
	sys_env.workstep =88; \
}
///////////////////////////////////////////////////

extern volatile U8 scrdis[5][50];  //根据币种 确定图片
#define KJWC	scrdis[coinchoose][0]    //开机完成  10
#define KJYE1	scrdis[coinchoose][1]    //开机预热2  6 
#define KJYE2	scrdis[coinchoose][2]    //开机预热2  7
#define KJYE3	scrdis[coinchoose][3]    //开机预热3  8
#define KJYE4	scrdis[coinchoose][4]    //开机预热4  9

#define	JSJM	scrdis[coinchoose][5]    //计数界面11 13 15 133
#define JSYX	scrdis[coinchoose][6]    //计数运行17 19 21 135 
#define YCCS	scrdis[coinchoose][7]   //隐藏测试29 31 33 139 
#define XDJM	scrdis[coinchoose][8]   //详单界面35 37 39  141
#define YZJS	scrdis[coinchoose][9]   //预置计数41 43 45  143
#define YZYX    scrdis[coinchoose][10]   //预置运行47 49 51   145
#define TZJM	scrdis[coinchoose][11]   //特征界面59 61 63  149
#define TZBC	scrdis[coinchoose][12]   //特征保存65 67 69 151
#define TZYX	scrdis[coinchoose][13]   //特征运行71 73 75 153
#define LSSJ	scrdis[coinchoose][14]   //历史数据77 79 81 155
#define BJJM	scrdis[coinchoose][15]   //报警界面 83 85 87 157
#define BJCS	scrdis[coinchoose][16]   //部件测试89 91 93  159
#define JZTS	scrdis[coinchoose][17]   //基准调试95 97 99 161
#define LJXS	scrdis[coinchoose][18]    //累计显示107 109 111 165 
#define XTSZ	scrdis[coinchoose][19]   //系统设置113 115 117  167  

#define Number_IndexpicA   255 
#define Number_IndexpicB  7 
extern U8 Disp1_Indexpic[Number_IndexpicB];         //MCU控制图片寄存器数据  切换 屏图片 指令数组
extern U8 Disp_Indexpic[Number_IndexpicA][Number_IndexpicB];         //MCU控制图片寄存器数据  切换 屏图片 指令数组

extern volatile U16 prepic_num; 	//记录当前的 图片号 所显示的
S16 test_repete(void);   

#define TSGET_NUM 32 //触摸屏数据帧最大长度
extern volatile U8 touchnum[TSGET_NUM];
extern volatile S32 db_id;   //历史数据 表格已经显示 数
void touchresult(void); 	 //manage the instruct from dgus
void  disp_allcount(void);	  // 取出界面 显示	 当前计数  详单
void disp_allcount_to_pc (void);


#define PRECOIN0  0   //用于   预置计数	 
#define PRECOIN1  1
#define PRECOIN2  2
#define PRECOIN3  3
#define PRECOIN4  4
#define PRECOIN5  5 
#define PRECOIN6  6 
#define PRECOIN7  7 
#define PRECOIN8  8 
#define PRECOIN9  9 
#define PRECOIN10  10 

struct precoin_num 	 //用于   预置计数	 
{
	U32 pre_count_cur;
	U32 pre_count_set;
	U32 coinval; //卷数
	U32 full_flag;
};
extern struct precoin_num count_coin_temp[11];   //用于   预置计数	 

extern struct disp_count_buf disp_buf;

struct disp_count_buf{
	U32 m_1yuan;
	U32 m_5jiao;
	U32 m_1jiao;
	U32 m_1jiao_big;
	U32 m_5fen;
	U32 m_2fen;
	U32 m_1fen;
	U32 m_10yuan;
	U32 m_5yuan;
	U32 total_good;
	U32 total_ng;
	U32 total_money;
};


void disp_precount(long ze,long zs,long fg);      //   预置计数 界面   实时 显示  币种 金额  数量   预置值  币数  异币


#define PRESETMAX 999999   // 6  表示  只能设到7位   9999999   7位表示能设到8位
#define PRESLA  8
#define PRESLB  21
#define PREKICKMAX 99 

#endif

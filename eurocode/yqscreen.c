#include "s3c2416.h"

void comscreen(U8* str,int16_t length)  //���� ָ����Һ����57600bps
{
	uint32_t i;
	U8 temp;
	for(i = 0;i<length;i++)
	{
		temp = 	*str;
		Uart2_sendchar(temp);
		str++;
	}

}

/*********************************
DGUS(UART 1)  read touch back values
**********************************/
char touch_serialnum[TSGET_NUM] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
volatile U8 touchnum[TSGET_NUM] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
volatile uint16_t uartcount = 0;  // ����2���� �ֽ� ����
volatile uint16_t touch_flag =0;  // ����2���� ��־λ
void touchget_from_uart2(void)    //receive the touch from dgus at real time
{

	U8 temp;
	while(!(rUTRSTAT2&0x1));
	temp = rURXH2;//��ȡ�Ĵ���ֵ

	if(touch_flag ==0)
	{
		if( (uartcount == 0)&& (temp == 0xA5))//0xa5 ����
		{
			touchnum[uartcount] = temp;
//			cy_print("%d:%x ",uartcount,touchnum[uartcount]);
			uartcount++;
		}
		else if( (uartcount == 1)&& (temp == 0x5A))//0x5a ����
		{
			touchnum[uartcount] = temp;
//			cy_print("%d:%x ",uartcount,touchnum[uartcount]);
			uartcount++;
		}
		else if( (uartcount == 2))//�ֽ���
		{
			touchnum[uartcount] = temp;
//			cy_print("%d:%x ",uartcount,touchnum[uartcount]);
			uartcount++;
		}
		//�ܹ�Ҫ���ܵ��ֽ��� + 3����Ϊ������֡ͷA5 5A �ͳ��ȹ������ֽ�
		else if( (uartcount > 2)&& (uartcount < (touchnum[2]+3)))
		{
			touchnum[uartcount] = temp;
//			cy_print("%d:%x ",uartcount,touchnum[uartcount]);
			if((uartcount ==  (touchnum[2]+2)))
			{
				touch_flag =1; //���������ݽ������
				////////////////////////////////////////////////////////////////////////////////////////
//				temp = touchnum[2]+3;
//				cy_println ("\n---------------------------------------------------------");
//				for( uartcount=0;uartcount<temp;uartcount++)//TSGET_NUM = 12
//				{
//					cy_print("%02d ",uartcount);
//				}
//				cy_print("\n");
//				for( uartcount=0;uartcount<temp;uartcount++)//TSGET_NUM = 12
//				{
//					touchnum[uartcount]  = touchnum[uartcount];
//					cy_print("%02x ",touchnum[uartcount]);
//				}
//				cy_print("\n");
				/////////////////////////////////////////////////////////////////////////////////////
				uartcount = 0;
				return;
			}
			uartcount++;
		}
		else
		{
			cy_print("U %d ",uartcount);
			uartcount = 0;
		}
	}
	else
	{
		uartcount = 0;
	}
	return;
}

/*********************************
DGUS(UART 1) beep
**********************************/
U8 dgus_beep[6]={0xA5,0x5A,0x03,0x80,0x02,0x64};   // 10ms * 100
void dgus_tfbeep(int bztime)    //uart1,dgus deep control
{
 	dgus_beep[5]=(bztime)&0xff;
	comscreen(dgus_beep,6);
}

/*********************************
DGUS(UART 1) read real time from dgus
//read rtc from dgus :A5 5A 03 81 20 10��index=0x20, backword= 0x10=16
//back :A5 5A 13 81 20 10 13 06 05 03 14 35 00 00 00 00 00 00 00 00 00 00
//only need y-m-d h:m,so send index: A5 5A 03 81 20 06,backword = 0x06= 6
//back :A5 5A 09 81 20 06 13 06 05 03 14 40
**********************************/

/*********************************
chinese
**********************************/
void dgus_chinese(uint16_t addr,S8 str[],uint16_t strnum)    // dgus  chinese
{

	U8 str1[6] = {0xA5,0x5A,0x00,0x82,0x0F,0x00};
	U8 str2[2] = {0xFF,0xFF};
	str1[5]=(addr)&0xff;
	str1[4]=(addr>>8)&0xff;
	str1[2]=(strnum+5)&0xff;
	//cy_print("str1[2] = %d \r\n",str1[2]);
	comscreen(str1,6);	//type corporation
	comscreen((U8*)str,strnum);	//type corporation
	comscreen(str2,2);	//type corporation
}

/*********************************
DGUS(UART 1)
values for specified address
1-word 2-word
**********************************/
U8 dgus_2word[10]={0xA5,0x5A,0x07,0x82,0x00,0x00,0x00,0x00,0x00,0x00};  // write 2-word to  data register
void dgus_tf2word(int addr,long data)    //transfer 2word data variables to specify address
{
 	dgus_2word[9]=(data)&0xff;
	dgus_2word[8]=(data>>8)&0xff;
	dgus_2word[7]=(data>>16)&0xff;
	dgus_2word[6]=(data>>24)&0xff;
 	dgus_2word[5]=(addr)&0xff;
	dgus_2word[4]=(addr>>8)&0xff;
	comscreen(dgus_2word,10);
}

U8 dgus_1word[8]= {0xA5,0x5A,0x05,0x82,0x00,0x00,0x00,0x00};   //write 1-word to  data register
void dgus_tf1word(int addr,long data)    //transfer values to specifed address
{

	dgus_1word[7]=(data)&0xff;
	dgus_1word[6]=(data>>8)&0xff;
 	dgus_1word[5]=(addr)&0xff;
	dgus_1word[4]=(addr>>8)&0xff;
	comscreen(dgus_1word,8);
}


///////////////////////////////////////
void disp_KJAmount(void) // initial addr on zhu jiemian ze zs forge
{
	dgus_tf2word(ADDR_ZLE,para_set_value.data.total_money);	//pre worker leiji money
	dgus_tf2word(ADDR_ZLS,para_set_value.data.total_good);	//pre worker leiji number
	dgus_tf2word(ADDR_ZLF,para_set_value.data.total_ng);	//pre worker leiji forge
	dgus_tf2word(ADDR_ZLH, para_set_value.data.db_total_item_num);	//pre worker leiji history number
}
/*********************************
uart 1  pre picture ,specify address
real time display data variable
**********************************/
void disp_data(int addr1,int addr2,int addr3)     //pre picture ,specify address display data variable
{
	dgus_tf2word(addr1,processed_coin_info.total_money);	//pre worker money
	dgus_tf2word(addr2,processed_coin_info.total_good);	//pre worker number
	dgus_tf2word(addr3,processed_coin_info.total_ng);	//pre worker forge
}

/////////////////////////////////////////////////////////////


struct disp_count_buf disp_buf;

void disp_allcount(void)     //pre counting ,detail list
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
//    OS_CPU_SR  cpu_sr = 0u;
#endif
	if( coinchoose == CN0){
    //OS_ENTER_CRITICAL();
		disp_buf.m_1yuan = *(pre_value.country[COUNTRY_ID].coin[0].data.p_pre_count_cur);
		disp_buf.m_5jiao = (*(pre_value.country[COUNTRY_ID].coin[1].data.p_pre_count_cur));
		disp_buf.m_1jiao_big = (*(pre_value.country[COUNTRY_ID].coin[3].data.p_pre_count_cur));
		disp_buf.m_1jiao = (*(pre_value.country[COUNTRY_ID].coin[4].data.p_pre_count_cur));
		disp_buf.m_5fen =  *(pre_value.country[COUNTRY_ID].coin[6].data.p_pre_count_cur);
		disp_buf.m_2fen =  *(pre_value.country[COUNTRY_ID].coin[7].data.p_pre_count_cur);
		disp_buf.m_1fen =  *(pre_value.country[COUNTRY_ID].coin[8].data.p_pre_count_cur);
		disp_buf.m_10yuan =  *(pre_value.country[COUNTRY_ID].coin[9].data.p_pre_count_cur);
		disp_buf.m_5yuan =  *(pre_value.country[COUNTRY_ID].coin[10].data.p_pre_count_cur);
		disp_buf.total_good = processed_coin_info.total_good;
		disp_buf.total_ng = processed_coin_info.total_ng;
		disp_buf.total_money = processed_coin_info.total_money;
		disp_buf.coin_speed = sys_env.coin_speed;
    //OS_EXIT_CRITICAL();

		dgus_tf2word(ADDR_XD10, disp_buf.m_1yuan);		//list 1
		dgus_tf2word(ADDR_XD5, disp_buf.m_5jiao);					//	 list 0.5
		dgus_tf2word(ADDR_XD1, disp_buf.m_1jiao);	//	 list 0.1
		dgus_tf2word(ADDR_XDB1, disp_buf.m_1jiao_big);	//	 list 0.1
		dgus_tf2word(ADDR_XD05, disp_buf.m_5fen);			//	  list 0.05
		dgus_tf2word(ADDR_XD02, disp_buf.m_2fen);			//	  list 0.02
		dgus_tf2word(ADDR_XD01, disp_buf.m_1fen);			//	  list 0.01
		dgus_tf2word(ADDR_XD1000, disp_buf.m_10yuan);			//	  list 10
		dgus_tf2word(ADDR_XD500, disp_buf.m_5yuan);			//	  list 5
		dgus_tf2word(ADDR_XDZS,disp_buf.total_good);				//	  list number
		dgus_tf2word(ADDR_XDFG,disp_buf.total_ng);			//	  list forge
		dgus_tf2word(ADDR_CPZE, disp_buf.total_money);	//pre worker money
		//disp_allcount_to_pc ();
	}
}

void disp_allcount_to_pc (void)
{
	pc_print("%d,%d;", 13, disp_buf.m_1yuan);
	pc_print("%d,%d;", 14, disp_buf.m_5jiao);
	pc_print("%d,%d;", 15, disp_buf.m_1jiao);
	pc_print("%d,%d;", 16, disp_buf.m_5fen);
	pc_print("%d,%d;",17, disp_buf.m_2fen);
	pc_print("%d,%d;",18, disp_buf.m_1fen);
	pc_print("%d,%d;",19, disp_buf.total_good);
	pc_print("%d,%d.%d%d;",20, (disp_buf.total_money/100),((disp_buf.total_money%100)/10),((disp_buf.total_money%100)%10));
	pc_print("%d,%d;",21, disp_buf.total_ng);
	pc_print("%d,%d;",28, OSCPUUsage);
	pc_print("%d,%d;",30, disp_buf.m_1jiao_big);
	if (sys_env.workstep == 1){//ͣ��
		pc_print("%d,%d;",50, 0);
	}else if(sys_env.workstep == 10){//����
		pc_print("%d,%d;",50, 1);
	}
	pc_print("%d,%d;",58, sys_env.coin_speed);
	pc_println();
}

///////////////////////////////////////////////////////////////
/*********************************
self learning fuction
**********************************/


void disp_preselflearn(int max0,int min0,int max1,int min1,int max2,int min2)    //real time ,pre coin admax admin when self learning
{
	dgus_tf1word(ADDR_A0MA,max0);	//	 real time ,pre AD0  max
	dgus_tf1word(ADDR_A0MI,min0);	//	 real time ,pre AD0  min
	dgus_tf1word(ADDR_A1MA,max1);	//	 real time ,pre AD1  max
	dgus_tf1word(ADDR_A1MI,min1);	//	 real time ,pre AD1  min
	dgus_tf1word(ADDR_A2MA,max2);	//	 real time ,pre AD2  max
	dgus_tf1word(ADDR_A2MI,min2);	//	 real time ,pre AD2  min
}
/////////////////////////////////
struct precoin_num count_coin_temp[COIN_TYPE_NUM];   //����   Ԥ�ü���



/////////////////ָ����ַд�Ĵ���/////////////////////////////////////////

U8 Disp_Indexpic[Number_IndexpicA][Number_IndexpicB]=         //MCU����ͼƬ�Ĵ�������  �л� ��ͼƬ ָ������
{
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x00},   //��ʾ ���� 0
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x01},	//��ʾ ���� 1
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x02},	//��ʾ ���� 2
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x03},	//��ʾ ���� 3
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x04},	//��ʾ ���� 4
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x05},	//��ʾ ���� 5
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x06},   //��ʾ ���� 6
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x07},	//��ʾ ���� 7
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x08},	//��ʾ ���� 8
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x09},	//��ʾ ���� 9
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0A},	//��ʾ ���� 10
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0B},	//��ʾ ���� 11
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0C},	//��ʾ ���� 12
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0D},	//��ʾ ���� 13
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0E},	//��ʾ ���� 14
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0F},	//��ʾ ���� 15
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x10},	//��ʾ ���� 16
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x11},	//��ʾ ���� 17
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x12},	//��ʾ ���� 18
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x13},	//��ʾ ���� 19
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x14},	//��ʾ ���� 20
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x15},	//��ʾ ���� 21
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x16},	//��ʾ ���� 22
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x17},	//��ʾ ���� 23
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x18},	//��ʾ ���� 24
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x19},	//��ʾ ���� 25
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1A},	//��ʾ ���� 26
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1B},	//��ʾ ���� 27
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1C},	//��ʾ ���� 28
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1D},	//��ʾ ���� 29
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1E},	//��ʾ ���� 30
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1F},	//��ʾ ���� 31
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x20},	//��ʾ ���� 32
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x21},	//��ʾ ���� 33
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x22},	//��ʾ ���� 34
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x23},	//��ʾ ���� 35
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x24},	//��ʾ ���� 36
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x25},	//��ʾ ���� 37
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x26},	//��ʾ ���� 38
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x27},	//��ʾ ���� 39
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x28},	//��ʾ ���� 40
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x29},	//��ʾ ���� 41
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2A},	//��ʾ ���� 42
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2B},	//��ʾ ���� 43
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2C},	//��ʾ ���� 44
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2D},	//��ʾ ���� 45
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2E},	//��ʾ ���� 46
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2F},	//��ʾ ���� 47
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x30},	//��ʾ ���� 48
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x31},	//��ʾ ���� 49
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x32},	//��ʾ ���� 50
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x33},	//��ʾ ���� 51
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x34},	//��ʾ ���� 52
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x35},	//��ʾ ���� 53
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x36},	//��ʾ ���� 54
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x37},	//��ʾ ���� 55
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x38},	//��ʾ ���� 56
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x39},	//��ʾ ���� 57
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3a},	//��ʾ ���� 58
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3b},	//��ʾ ���� 59
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3c},	//��ʾ ���� 60
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3d},	//��ʾ ���� 61
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3e},	//��ʾ ���� 62
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3f},	//��ʾ ���� 63
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x40},	//��ʾ ���� 64
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x41},	//��ʾ ���� 65
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x42},	//��ʾ ���� 66
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x43},	//��ʾ ���� 67
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x44},	//��ʾ ���� 68
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x45},	//��ʾ ���� 69
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x46},	//��ʾ ���� 70
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x47},	//��ʾ ���� 71
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x48},	//��ʾ ���� 72
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x49},	//��ʾ ���� 73
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4a},	//��ʾ ���� 74
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4b},	//��ʾ ���� 75
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4c},	//��ʾ ���� 76
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4d},	//��ʾ ���� 77
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4e},	//��ʾ ���� 78
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4f},	//��ʾ ���� 79
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x50},	//��ʾ ���� 80
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x51},	//��ʾ ���� 81
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x52},	//��ʾ ���� 82
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x53},	//��ʾ ���� 83
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x54},	//��ʾ ���� 84
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x55},	//��ʾ ���� 85
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x56},	//��ʾ ���� 86
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x57},	//��ʾ ���� 87
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x58},	//��ʾ ���� 88
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x59},	//��ʾ ���� 89
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5a},	//��ʾ ���� 90
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5b},	//��ʾ ���� 91
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5c},	//��ʾ ���� 92
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5d},	//��ʾ ���� 93
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5e},	//��ʾ ���� 94
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5f},	//��ʾ ���� 95
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x60},	//��ʾ ���� 96
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x61},	//��ʾ ���� 97
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x62},	//��ʾ ���� 98
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x63},	//��ʾ ���� 99
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x64},	//��ʾ ���� 100
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x65},	//��ʾ ���� 101
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x66},	//��ʾ ���� 102
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x67},	//��ʾ ���� 103
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x68},	//��ʾ ���� 104
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x69},	//��ʾ ���� 105
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6a},	//��ʾ ���� 106
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6b},	//��ʾ ���� 107
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6c},	//��ʾ ���� 108
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6d},	//��ʾ ���� 109
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6e},	//��ʾ ���� 110
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6f},	//��ʾ ���� 111
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x70},	//��ʾ ���� 112
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x71},	//��ʾ ���� 113
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x72},	//��ʾ ���� 114
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x73},	//��ʾ ���� 115
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x74},	//��ʾ ���� 116
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x75},	//��ʾ ���� 117
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x76},	//��ʾ ���� 118
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x77},	//��ʾ ���� 119
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x78},	//��ʾ ���� 120
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x79},	//��ʾ ���� 121
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7A},	//��ʾ ���� 122
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7B},	//��ʾ ���� 123
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7C},	//��ʾ ���� 124
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7D},	//��ʾ ���� 125
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7E},	//��ʾ ���� 126
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7F},	//��ʾ ���� 127
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x80},	//��ʾ ���� 128
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x81},	//��ʾ ���� 129
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x82},	//��ʾ ���� 130
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x83},	//��ʾ ���� 131
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x84},	//��ʾ ���� 132
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x85},	//��ʾ ���� 133
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x86},	//��ʾ ���� 134
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x87},	//��ʾ ���� 135
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x88},	//��ʾ ���� 136
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x89},	//��ʾ ���� 137
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8A},	//��ʾ ���� 138
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8B},	//��ʾ ���� 139
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8C},	//��ʾ ���� 140
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8D},	//��ʾ ���� 141
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8E},	//��ʾ ���� 142
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8F},	//��ʾ ���� 143
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x90},	//��ʾ ���� 144
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x91},	//��ʾ ���� 145
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x92},	//��ʾ ���� 146
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x93},	//��ʾ ���� 147
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x94},	//��ʾ ���� 148
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x95},	//��ʾ ���� 149
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x96},	//��ʾ ���� 150
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x97},	//��ʾ ���� 151
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x98},	//��ʾ ���� 152
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x99},	//��ʾ ���� 153
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9A},	//��ʾ ���� 154
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9B},	//��ʾ ���� 155
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9C},	//��ʾ ���� 156
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9D},	//��ʾ ���� 157
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9E},	//��ʾ ���� 158
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9F},	//��ʾ ���� 159
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA0},	//��ʾ ���� 160
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA1},	//��ʾ ���� 161
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA2},	//��ʾ ���� 162
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA3},	//��ʾ ���� 163
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA4},	//��ʾ ���� 164
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA5},	//��ʾ ���� 165
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA6},	//��ʾ ���� 166
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA7},	//��ʾ ���� 167
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA8},	//��ʾ ���� 168
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA9},	//��ʾ ���� 169
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAA},	//��ʾ ���� 170
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAB},	//��ʾ ���� 171
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAC},	//��ʾ ���� 172
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAD},	//��ʾ ���� 173
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAE},	//��ʾ ���� 174
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAF},	//��ʾ ���� 175
};


volatile U8 scrdis[5][50]=
{
//0 1 2 3 4  5  6  7  8  9 10 11 12 13 14 15 16 17  18  19 20 21 22
{10,6,7,8,9,13, 19, 31,37,43,49,61,67,73,79,85,91,97,109,115},	  //�����
{10,6,7,8,9,11, 17, 29,35,41,47,59,65,71,77,83,89,95,107,113},   //ŷԪ
{10,6,7,8,9,15, 21, 33,39,45,51,63,69,75,81,87,93,99,111,117},	  //��Ԫ
{10,6,7,8,9,133,135,139,141,143,145,149,151,153,155,157,159,161,165,167},	  //Ӣ��
};


int16_t test_repete(void)    //  �б� �ǲ����رҵĺ���
{

	int16_t ei = 0;

	ei = is_repeate (sys_env.coin_index);

	if (ei == 0)
	{
		pre_value.country[coinchoose].coin[sys_env.coin_index].data.max0 = coin_maxvalue0;
		pre_value.country[coinchoose].coin[sys_env.coin_index].data.min0= coin_minvalue0;
		pre_value.country[coinchoose].coin[sys_env.coin_index].data.max1 = coin_maxvalue1;
		pre_value.country[coinchoose].coin[sys_env.coin_index].data.min1= coin_minvalue1;
		pre_value.country[coinchoose].coin[sys_env.coin_index].data.max2 = coin_maxvalue2;
		pre_value.country[coinchoose].coin[sys_env.coin_index].data.min2= coin_minvalue2;
		pre_value.country[coinchoose].coin[sys_env.coin_index].data.std0 = std_ad0;
		pre_value.country[coinchoose].coin[sys_env.coin_index].data.std1 = std_ad1;
		pre_value.country[coinchoose].coin[sys_env.coin_index].data.std2 = std_ad2;
		write_para ();
		cy_println ("Total learned coin number is %d", coinlearnnumber);
		cy_println ("Save Complete");

		prepic_num = TZJM;
		comscreen(Disp_Indexpic[prepic_num],Number_IndexpicB); //��ʾ
	}
	else
	{
		cy_println ("DA min0 = %d DA max0 = %d", coin_minvalue0, coin_maxvalue0);
		cy_println ("DA min1 = %d DA max1 = %d", coin_minvalue1, coin_maxvalue1);
		cy_println ("DA min2 = %d DA max2 = %d", coin_minvalue2, coin_maxvalue2);
		cy_println ("ei = %d", ei);
	}
	return ei;

}


///////////////////////////////////////////////////
void counter_clear (void) //
{
	int i;
	for (i = 0; i < COIN_TYPE_NUM; i++){
		*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_full_flag = 0; //
		*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_cur = 0; //
		*pre_value.country[COUNTRY_ID].coin[i].data.p_coinval = 0;
		coin_env.full_stack_num = 0;
		processed_coin_info.coin_ctr[i] =0;
	}
	
	coin_env.coin_ir_ctr1 = 0;
	coin_env.coin_ir_ctr2 = 0;
	coin_env.coin_detect_ctr = 0;
	coin_env.kick1_ctr = 0;
	coin_env.kick2_ctr = 0;
	coin_env.ir_in_time = 0;
	coin_env.max_ir_in_time = 0;
	coin_env.min_ir_in_time = 0xffff;
	coin_env.ir_interval_time = 0;
	coin_env.max_ir_interval_time = 0;
	coin_env.min_ir_interval_time = 0xffff;
	coin_env.ir_filter_time = 0;
	coin_env.process_coin_index = 0;
	
	processed_coin_info.total_money =0;
	processed_coin_info.total_coin = 0;
	processed_coin_info.total_good = 0;
	processed_coin_info.total_ng = 0;
	processed_coin_info.coinnumber = 0;
	good_value_index = 0;
	ng_value_index = 0;
	coin_maxvalue0 = 0;
	coin_minvalue0 = 1023;
	coin_maxvalue1 = 0;
	coin_minvalue1 = 1023;
	coin_maxvalue2 = 0;
	coin_minvalue2 = 1023;
	coinlearnnumber = 0;
	disp_allcount();
	disp_data(ADDR_CPZE,ADDR_CPZS,ADDR_CPFG);			//when counting pre ze zs foege data variable
}

volatile uint16_t prepic_num =0 ;   // ����֮ǰ��ͼƬ
volatile int32_t db_id = 0;   //��ʷ���� ����Ѿ���ʾ ��
/*A5 5A 06 83 ��ַL,H + ���������� + ����*/


void set_offset_value (uint16_t addr, uint16_t coin_index, int16 offset)
{
	pre_value.country[coinchoose].coin[coin_index].data.offsetmax0 = (offset);       //
	pre_value.country[coinchoose].coin[coin_index].data.offsetmin0 = (offset)*(-1);       //
	pre_value.country[coinchoose].coin[coin_index].data.offsetmax1 = (offset);       //
	pre_value.country[coinchoose].coin[coin_index].data.offsetmin1 = (offset)*(-1);       //
	pre_value.country[coinchoose].coin[coin_index].data.offsetmax2 = (offset);       //
	pre_value.country[coinchoose].coin[coin_index].data.offsetmin2 = (offset)*(-1);       //
	dgus_tf1word(addr, pre_value.country[coinchoose].coin[coin_index].data.offsetmax0);
	write_para ();
}
	
void touchresult(void)      //���ݽ��յ���  �� ������ ִ�е�����
{
	uint16_t addr, value, i;
	char str_buf[256];
	addr = (touchnum[4] << 8) | (touchnum[5]);
	value = (touchnum[7] << 8) | (touchnum[8]);
	///////////////A5 5A 06 83 00 06 01 00 0x:1 2/////////////////////////
	switch (addr){
	case ADDR_CRUN:  //��ַADDR_CRUN 0X06  ��������ֵ�ж�
		if( (value == 0x01)){
			// A5 5A 06 83 00 06 01 00 01	��������� start
			coin_start ();
		}else if( (value == 0x02)){
		  //A5 5A 06 83 00 06 01 00 02	  ��������� stop
			coin_stop ();
		}
		break;
	////////////////A5 5A 06 83 00 3C 01 00 0x:1.2//////////////////////////
	case ADDR_LRUN:  //��ַADDR_LRUN 0X3C  ��������ֵ�ж� ����ѧϰ start
		if( (value == 0x01)){//A5 5A 06 83 00 3C 01 00 01	����ѧϰ start
			if (sys_env.system_delay == 0){
				good_value_index = 0;
				ng_value_index = 0;
				prepic_num = TZJM;
				coin_maxvalue0 = 0;
				coin_minvalue0 = 1023;
				coin_maxvalue1 = 0;
				coin_minvalue1 = 1023;
				coin_maxvalue2 = 0;
				coin_minvalue2 = 1023;
				coinlearnnumber = 0;
				disp_preselflearn(coin_maxvalue0,coin_minvalue0,coin_maxvalue1,coin_minvalue1,coin_maxvalue2,coin_minvalue2) ;				   //��ʾ��ǰ  ͨ��   ����ֵ
				comscreen(Disp_Indexpic[TZYX],Number_IndexpicB);  // back to the  picture before alert

				sys_env.workstep =13;
				cy_print("start learning %s %d\n", __FILE__, __LINE__);
			}
		}else if( (value == 0x02)){
			 //A5 5A 06 83 00 3C 01 00 02	����ѧϰ stop
			disp_preselflearn(coin_maxvalue0,coin_minvalue0,coin_maxvalue1,coin_minvalue1,coin_maxvalue2,coin_minvalue2); //pre coin admax admin when self learning
			STORAGE_MOTOR_STOPRUN();   //��������
			comscreen(Disp_Indexpic[TZBC],Number_IndexpicB);  // back to the  picture before alert
			sys_env.workstep = 0;
			cy_println("end working %s %d", __FILE__, __LINE__);
		}
		break;
	////////////////////////////////////////////////
	case ADDR_PGH1:  //��ַADDR_PGH1 0X07
		para_set_value.data.coin_full_rej_pos = (int)touchnum[8];
		dgus_tf1word(ADDR_PGH1,para_set_value.data.coin_full_rej_pos);	//make sure  the return one
		Writekick_value();
		sys_env.workstep = 0;	// �ȴ� ����
		break;
	case ADDR_DBDATA:  //��ַADDR_DBDATA 0X09  ��������ֵ�ж�
		if( (value == 0x01)){	//back value  delete jiemian
		   // A5 5A 06 83 00 09 01 00 01  ����ֵ����
			disp_KJAmount(); // initial addr on zhu jiemian ze zs forge
			comscreen(Disp_Indexpic[LJXS],Number_IndexpicB);	 //  to the  picture
			sys_env.workstep = 0;	// �ȴ� ����
		}else if( (value == 0x02)){	//back value 04  delete jiemian
		   // A5 5A 06 83 00 09 01 00 02  ����ֵ����		�����ǰ�ܶ�����
			para_set_value.data.total_money = 0;
			write_para();	//Ϊ���� ��ǰ  �ۼ�����  ���  ���
			disp_KJAmount(); // initial addr on zhu jiemian ze zs forge
			sys_env.workstep = 0;	// �ȴ� ����
		}else if( (value == 0x03)){	//back value 04  delete jiemian
		   // A5 5A 06 83 00 09 01 00 03  ����ֵ����	ɾ����ǰ���������
			para_set_value.data.total_good = 0;
			para_set_value.data.total_ng = 0;
			write_para();	// 0 Ϊ���� ��ǰ ���� �ۼ� ����  ���  ���
			disp_KJAmount(); // initial addr on zhu jiemian ze zs forge
			sys_env.workstep = 0;	// �ȴ� ����
		}else if( (value == 0x04)){	//back value 04  delete jiemian
		   // A5 5A 06 83 00 09 01 00 04  ����ֵ���� ɾ����ǰ�������� ���� ��� ���
			cy_print ("Delete History Data\n");
			para_set_value.data.total_money = 0;
			para_set_value.data.total_good = 0;
			para_set_value.data.total_ng = 0;
			yqsql_exec(DBDELETE);	  // delete db table data
			write_para(); // 3 Ϊ���� ��ǰ����� ��������
			disp_KJAmount(); // initial addr on zhu jiemian ze zs forge
			sys_env.workstep = 0;	// �ȴ� ����
		}else if( (value == 0x06)){	//back value    history data jiemian
		 // A5 5A 06 83 00 09 01 00 06  ������ʷ���ݱ��
			cy_println ("Enter History Data menu\n");
			db_id = para_set_value.data.db_total_item_num;
			yqsql_exec(DBDISPLAY);	  //
			comscreen(Disp_Indexpic[LSSJ],Number_IndexpicB);	 //  to the  picture 	///////////////////////////////////////////////////////////
		}else if( (value == 0x07)){	//back value   history data jiemian
		 // A5 5A 06 83 00 09 01 00 07  ����ֵ���� �鿴��ҳ
			cy_print ("Pre page\n");
			yqsql_exec(DBDISPLAY);	  /////////////////////////////////////////////////////////////////
		}else if( (value == 0x08)){	//back value    history data jiemian
		// A5 5A 06 83 00 09 01 00 08  ����ֵ���� �鿴��ҳ
			cy_print ("Next page\n");
			yqsql_exec(DBDISPLAYBACK);	  /////////////////////////////////////////////////////////
		}
		break;
   ///////////////////////////////////////////////////
	case ADDR_TZBC:  //��ַADDR_B1  0X08 ��������ֵ�ж� //back value  / ����ѧϰ save or not
		if( (value == 0x03)){	//back value 04 ��������� xiandan
			// A5 5A 06 83 00 08 01 00 03  ����ֵ����	�굥
			cy_print("display xiandan\r\n");
			disp_allcount();
			comscreen(Disp_Indexpic[XDJM],Number_IndexpicB);	 //  to the  picture
			sys_env.workstep = 0;	// �ȴ� ����
		}else if( (value == 0x04)){	//back value 04 ��������� xiandan save
			 // A5 5A 06 83 00 08 01 00 04  ����ֵ����	���� ��ǰ����
		    if( (processed_coin_info.total_money >0) || (processed_coin_info.total_good >0)  || (processed_coin_info.total_ng >0)){
				uint16_t ci = 0;
				prepic_num = JSJM;

				para_set_value.data.total_money += processed_coin_info.total_money;
				para_set_value.data.total_good += processed_coin_info.total_good;
				para_set_value.data.total_ng += processed_coin_info.total_ng;
				yqsql_exec(DBINSERT);
				disp_KJAmount(); // initial addr on zhu jiemian ze zs forge
				for(ci = 0;ci<9;ci++)
					*(pre_value.country[COUNTRY_ID].coin[ci].data.p_pre_count_cur) = 0;
 				processed_coin_info.total_money =0;
				processed_coin_info.total_good = 0;
				processed_coin_info.total_ng = 0;
				processed_coin_info.total_coin = 0;
				processed_coin_info.coinnumber = 0;
				disp_allcount();
				disp_data(ADDR_CPZE,ADDR_CPZS,ADDR_CPFG);			//when counting pre ze zs foege data variable
				comscreen(Disp_Indexpic[XDJM],Number_IndexpicB);	 // back to the  picture before alert
		    }
			sys_env.workstep = 0;	// �ȴ� ����
		}else if( (value == 0x05)){	//back value 04 ��������� xiangdan delete
			// A5 5A 06 83 00 08 01 00 05  ��������� ���� ����
			para_set_value.data.total_money += processed_coin_info.total_money;
			para_set_value.data.total_good += processed_coin_info.total_good;
			para_set_value.data.total_ng += processed_coin_info.total_ng;
			if (processed_coin_info.total_coin > 0)
			{
				yqsql_exec(DBINSERT);
			}
			counter_clear ();
		}else if( (value == 0x0B)){	//back value  ����ѧϰ save or not
			if (coinlearnnumber == 0){
				cy_println ("coin learnnumber = 0, data will be clear");
				run_command ("set save-f");
				run_command ("version");
			}else{
				prepic_num = TZJM;
				adstd_offset ();
				i = is_repeate (sys_env.coin_index);
				if (i == 0){
					sprintf (str_buf, "����ֵ������ϣ�����ѧϰӲ������:%dö", coinlearnnumber);
					run_command ("set save");
					run_command ("version");
				}else{
					switch (i){
						case 1:
							sprintf (str_buf, "����ֵ��1ԪӲ���г�ͻ����ȷ���Ƿ����1Ԫ��Ӳ�ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 2:
							sprintf (str_buf, "����ֵ�����ͭӲ���г�ͻ����ȷ���Ƿ�������ͭ��Ӳ�ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 3:
							sprintf (str_buf, "����ֵ����Ǹ�Ӳ���г�ͻ����ȷ���Ƿ������Ǹֵ�Ӳ�ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 4:
							sprintf (str_buf, "����ֵ��һ�Ǵ���Ӳ���г�ͻ����ȷ���Ƿ����һ�Ǵ�����Ӳ�ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 5:
							sprintf (str_buf, "����ֵ��һ��С��Ӳ���г�ͻ����ȷ���Ƿ����һ��С�ֵ�Ӳ�ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 6:
							sprintf (str_buf, "����ֵ��һ��С��Ӳ���г�ͻ����ȷ���Ƿ����һ��С����Ӳ�ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 7:
							sprintf (str_buf, "����ֵ�����Ӳ���г�ͻ����ȷ���Ƿ������ֵ�Ӳ�ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 8:
							sprintf (str_buf, "����ֵ������Ӳ���г�ͻ����ȷ���Ƿ�������ֵ�Ӳ�ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 9:
							sprintf (str_buf, "����ֵ��һ��Ӳ���г�ͻ����ȷ���Ƿ����һ�ֵ�Ӳ�ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 10:
							sprintf (str_buf, "����ֵ��10Ԫ������г�ͻ����ȷ���Ƿ����10Ԫ����ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 11:
							sprintf (str_buf, "����ֵ��5Ԫ������г�ͻ����ȷ���Ƿ����5Ԫ����ң������������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						case 1002:
							sprintf (str_buf, "����ֵ��Χ̫����ȷ���Ƿ�������������Ӳ�ң����������ѧϰ���˴����ݲ��ᱣ�档����ѧϰӲ������:%dö", coinlearnnumber);
							break;
						default:
							sprintf (str_buf, "����ѧϰ�쳣���������ݲ��ᱣ�棬������");break;
					}
				}
				ALERT_MSG ("��ʾ", str_buf);
				coinlearnnumber = 0;
			}
		}else if( (value == 0x0C)){	//back value  ����ѧϰnot save
			disp_preselflearn(pre_value.country[coinchoose].coin[sys_env.coin_index].data.max0, pre_value.country[coinchoose].coin[sys_env.coin_index].data.min0,
							  pre_value.country[coinchoose].coin[sys_env.coin_index].data.max1, pre_value.country[coinchoose].coin[sys_env.coin_index].data.min1,
							  pre_value.country[coinchoose].coin[sys_env.coin_index].data.max2, pre_value.country[coinchoose].coin[sys_env.coin_index].data.min2);
			prepic_num = TZJM;
			comscreen(Disp_Indexpic[prepic_num],Number_IndexpicB);	 // back to the  picture before alert
		}else if( (value == 0x0F)){	// back value OF, ��������   back to picture
			coin_clear_alarm ();
		}
		break;
	case ADDR_PRIT:  //��ӡ
		if( (value == 0x06)){	//back value 04 ��������� xiandan print
			coin_print ();
		}
		break;
//////////////////////////////
	case ADDR_CNTB:  //��ַADDR_CNTB1 0X3B  ͼ�����
		sys_env.coin_index = (int)touchnum[8];
		//dgus_tf1word(ADDR_CNTB,sys_env.coin_index);	//make sure the tubiao is the return one
		switch (sys_env.coin_index)
		{
			case 0:
				DISPLAY_COIN_INDEX ("һԪ");
				break;
			case 1:
				DISPLAY_COIN_INDEX ("���ͭ");
				break;
			case 2:
				DISPLAY_COIN_INDEX ("��Ǹ�");
				break;
			case 3:
				DISPLAY_COIN_INDEX ("һ�Ǵ���");
				break;
			case 4:
				DISPLAY_COIN_INDEX ("һ�Ǹ�");
				break;
			case 5:
				DISPLAY_COIN_INDEX ("һ��С��");
				break;
			case 6:
				DISPLAY_COIN_INDEX ("���");
				break;
			case 7:
				DISPLAY_COIN_INDEX ("����");
				break;
			case 8:
				DISPLAY_COIN_INDEX ("һ��");
				break;
			case 9:
				DISPLAY_COIN_INDEX ("�����ʮԪ");
				break;
			case 10:
				DISPLAY_COIN_INDEX ("�������Ԫ");
				break;
			default:break;
		}
		disp_preselflearn(pre_value.country[coinchoose].coin[sys_env.coin_index].data.max0, pre_value.country[coinchoose].coin[sys_env.coin_index].data.min0,
						  pre_value.country[coinchoose].coin[sys_env.coin_index].data.max1, pre_value.country[coinchoose].coin[sys_env.coin_index].data.min1,
						  pre_value.country[coinchoose].coin[sys_env.coin_index].data.max2, pre_value.country[coinchoose].coin[sys_env.coin_index].data.min2);
		sys_env.workstep = 0;	//ֹͣ	���ж���  // �ȴ� ����
		break;
	case ADDR_KICK_DELAY_T1:  //��ַADDR_KICK_DELAY_T1 0X56 �߱���ʱ
		para_set_value.data.kick_start_delay_t1 = (int)(touchnum[7]*256 )+(int)touchnum[8];       //delay time
		dgus_tf1word(ADDR_KICK_DELAY_T1, para_set_value.data.kick_start_delay_t1);	//make sure the return one
		Writekick_value();
		sys_env.workstep = 0;	//ֹͣ	���ж���  // �ȴ� ����
		break;
	case ADDR_KICK_KEEP_T1:  //��ַADDR_KBDT1 0X57 �߱�time
		para_set_value.data.kick_keep_t1 = (int)(touchnum[7]*256 )+(int)touchnum[8];       //kick time
		dgus_tf1word(ADDR_KICK_KEEP_T1, para_set_value.data.kick_keep_t1);	//make sure  the return one
		Writekick_value();
		sys_env.workstep = 0;	//ֹͣ	���ж���  // �ȴ� ����
		break;
	case ADDR_KICK_DELAY_T2:
		para_set_value.data.kick_start_delay_t2 = (int)(touchnum[7]*256 )+(int)touchnum[8];       //delay time
		dgus_tf1word(ADDR_KICK_DELAY_T2, para_set_value.data.kick_start_delay_t2);	//make sure the return one
		Writekick_value();
		sys_env.workstep = 0;	//ֹͣ	���ж���  // �ȴ� ����
		break;
	case ADDR_KICK_KEEP_T2:  //��ַADDR_KBDT1 0X57 �߱�time
		para_set_value.data.kick_keep_t2 = (int)(touchnum[7]*256 )+(int)touchnum[8];       //kick time
		dgus_tf1word(ADDR_KICK_KEEP_T2, para_set_value.data.kick_keep_t2);	//make sure  the return one
		Writekick_value();
		sys_env.workstep = 0;	//ֹͣ	���ж���  // �ȴ� ����
		break;
	case ADDR_MOTOR_IDLE_T:  //
		para_set_value.data.motor_idle_t = (int)(touchnum[7]*256 )+(int)touchnum[8];       //kick time
		dgus_tf1word(ADDR_MOTOR_IDLE_T, para_set_value.data.motor_idle_t);	//make sure  the return one
		Writekick_value();
		sys_env.workstep = 0;	//ֹͣ	���ж���  // �ȴ� ����
		break;
	case ADDR_SYS_DELAY:
		para_set_value.data.system_boot_delay = (int)(touchnum[7]*256 )+(int)touchnum[8];       //
		dgus_tf1word(ADDR_SYS_DELAY, para_set_value.data.system_boot_delay);	//
		Writekick_value();
		break;
	case ADDR_PRE_COUNT_STOP_N:
		para_set_value.data.pre_count_stop_n = (int)(touchnum[7]*256 )+(int)touchnum[8];       //
		dgus_tf1word(ADDR_PRE_COUNT_STOP_N, para_set_value.data.pre_count_stop_n);	//
		Writekick_value();
		break;
	case ADDR_LEVEL100:  //��ַ1ԪӲ�ҵ���ֵȼ�����
		set_offset_value (ADDR_LEVEL100, 0, (int)(touchnum[7]*256 )+(int)touchnum[8]);
		break;
	case ADDR_LEVEL50:  //��ַ���ͭӲ�ҵ���ֵȼ�����
		set_offset_value (ADDR_LEVEL50, 1, (int)(touchnum[7]*256 )+(int)touchnum[8]);
		break;
	case ADDR_LEVEL51:  //��ַ5�Ǹ�Ӳ�ҵ���ֵȼ�����
		set_offset_value (ADDR_LEVEL51, 2, (int)(touchnum[7]*256 )+(int)touchnum[8]);
		break;
	case ADDR_LEVEL10:  //��ַһ�Ǵ���Ӳ�ҵ���ֵȼ�����
		set_offset_value (ADDR_LEVEL10, 3, (int)(touchnum[7]*256 )+(int)touchnum[8]);
		break;
	case ADDR_LEVEL11:  //��ַһ�Ǹ�Ӳ�ҵ���ֵȼ�����
		set_offset_value (ADDR_LEVEL11, 4, (int)(touchnum[7]*256 )+(int)touchnum[8]);
		break;
	case ADDR_LEVEL12:  //��ַһ��С��Ӳ�ҵ���ֵȼ�����
		set_offset_value (ADDR_LEVEL12, 5, (int)(touchnum[7]*256 )+(int)touchnum[8]);
		break;
	case ADDR_LEVEL1000:  //��ַ�����ʮԪӲ�ҵ���ֵȼ�����
		set_offset_value (ADDR_LEVEL1000, 9, (int)(touchnum[7]*256 )+(int)touchnum[8]);
		break;
	case ADDR_LEVEL500:  //��ַ�������ԪӲ�ҵ���ֵȼ�����
		set_offset_value (ADDR_LEVEL500, 10, (int)(touchnum[7]*256 )+(int)touchnum[8]);
		break;
	case ADDR_YZS0:
	case ADDR_YZS1:
	case ADDR_YZS3:
	case ADDR_YZS4:
	case ADDR_YZS6:
	case ADDR_YZS7:
	case ADDR_YZS8:
	case ADDR_YZS9:
	case ADDR_YZS10:
///////////////////////////////////////////////////////
		for (i = 0; i < COIN_TYPE_NUM; i++){
			if (addr == pre_value.country[COUNTRY_ID].coin[i].data.hmi_pre_count_set_addr){
				*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set =  (int)(touchnum[7]*256 )+(int)touchnum[8];
				*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_cur = 0;//��ǰ����ֵ ����
				*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_full_flag = 0;
				if (para_set_value.data.system_mode == 0){
					para_set_value.data.precoin_set_num[pre_value.country[COUNTRY_ID].coin[i].data.coin_type] = *pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set;
					write_para (); //д��Ԥ��ֵ
				}

				dgus_tf1word(pre_value.country[COUNTRY_ID].coin[i].data.hmi_pre_count_set_addr, *pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set);
				break;
			}
		}
		break;
	case ADDR_YZ_RESET:  //����
		if( (value == 0x05)){
			int i;
			for (i = 0; i < COIN_TYPE_NUM; i++){
				*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_full_flag = 0; //
				*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_cur = 0; //
				coin_env.full_stack_num = 0;
			}
		}
		break;
//////////////////////////////////////////////////////////////////
	case ADDR_BJZX:  //��ַADDR_BJZX ����ִ��
		switch (value)
		{
			case 0x01://������俪��״̬��ȡ
				detect_read();
				comscreen(Disp_Indexpic[BJCS],Number_IndexpicB);	 // ������Խ���
				sys_env.workstep = 101;	//ֹͣ	���ж���  // �ȴ� ����
				break;
			case 0x02:// �������Է���
				dgus_tf1word(ADDR_KICK1_M,0);
				dgus_tf1word(ADDR_KICK2_M,0);
				dgus_tf1word(ADDR_STORAGE_MOTOR,0);
				dgus_tf1word(ADDR_ZXLD,0);

				STORAGE_MOTOR_STOPRUN();   //��������
				comscreen(Disp_Indexpic[YCCS],Number_IndexpicB);	 // back to the  picture before alert
				sys_env.workstep = 0;	//ֹͣ	���ж���  // �ȴ� ����
				break;
			case 0x03://back value ��׼���� BEGIN
				sys_env.workstep = 100;
				dgus_tf1word(ADDR_STDH,980); //	high frequence
				dgus_tf1word(ADDR_STDM,980); //	middle frequence
				dgus_tf1word(ADDR_STDL,980); //	low frequence
				comscreen(Disp_Indexpic[JZTS],Number_IndexpicB);	 // back to the  picture before alert
				break;
			case 0x04://back value ��׼���� over
				sys_env.workstep = 0;	//ֹͣ	���ж���  // �ȴ� ����
				prepic_num = BJCS;
				comscreen(Disp_Indexpic[prepic_num],Number_IndexpicB);	 // back to the  picture before alert
				break;
			case 0x05:// bujian zhixing
				detect_read();    //���ش�����״̬
				break;
			case 0x06://���β���
				comscreen(Disp_Indexpic[JZTS],Number_IndexpicB);
				sys_env.workstep =103;
				if (sys_env.print_wave_to_pc == 1){
					sys_env.print_wave_to_pc = 0;
				}else{
					sys_env.print_wave_to_pc = 1;
				}
				break;
			case 0x07:  //��ַADDR_KICK1_M bujian zhixing
				time = para_set_value.data.kick_start_delay_t1; //kick_start_delay_time*0.1ms
				dgus_tf1word(ADDR_KICK1_M,1);
				while(time != 0){;}
				EMKICK1(STARTRUN);	  // kick out
				cy_println ("kick1 start");
				time = para_set_value.data.kick_keep_t1;	  //kick_keep_time*0.1ms
				while(time != 0){;}
				EMKICK1(STOPRUN);	  // kick in
				cy_println ("kick1 stop");
				dgus_tf1word(ADDR_KICK1_M,0);
				break;
			case 0x08:  //��ַADDR_KICK2_M bujian zhixing
				time = para_set_value.data.kick_start_delay_t2; //kick_start_delay_time*1ms
				dgus_tf1word(ADDR_KICK2_M,1);
				while(time != 0){;}
				EMKICK2(STARTRUN);	  // kick out
				cy_println ("kick2 start");
				time = para_set_value.data.kick_keep_t2;	  //kick_keep_time*1ms
				while(time != 0){;}
				EMKICK2(STOPRUN);	  // kick in
				cy_println ("kick2 stop");
				dgus_tf1word(ADDR_KICK2_M,0);
				break;
			default:break;
		}
		break;
	case ADDR_STORAGE_MOTOR:  //��ַADDR_ZXCY21 bujian zhixing
		if( (value == 0x00)){	 // 0�� 1��   ת����ת
			STORAGE_MOTOR_STOPRUN();   //��������
			STORAGE_DIR_P();//��ת
			dgus_tf1word(ADDR_STORAGE_MOTOR,0);
		}else if( (value == 0x01)){	// ת����ת
			STORAGE_MOTOR_STARTRUN();   //��������
			dgus_tf1word(ADDR_STORAGE_MOTOR,1);
		}
		break;
	case ADDR_PASSWORD:
		if (value == sys_env.password){
			value = 0;
			comscreen(Disp_Indexpic[25],Number_IndexpicB);	 // back to the  picture before alert
		}
		break;
	case ADDR_RESET:  //
		if (value == 0x0123){
			comscreen(Disp_Indexpic[0],Number_IndexpicB);
			cy_println ("Software Reset...");
			run_command ("reset");
		}
		break;
	case ADDR_MODE:
		change_coin_mode (value);
		write_para (); //д��Ԥ��ֵ
		break;
	/////////////////////////////////////////////
	//�����1 ����������0 ��ѧϰֵ ���¸�ֵ  ����Χ ���¸�ֵ
	case ADDR_CNCH1:  //��ַADDR_ZXCY21 bujian zhixing
		dgus_tf1word(ADDR_CNCH,coinchoose);  //ȷ�� ͼ�����
		break;
	default:break;
	}
}

void change_coin_mode (uint16_t value)
{
	int i, set_data;
	para_set_value.data.system_mode = value;
	for (i = 0; i < COIN_TYPE_NUM; i++){
		if (value == 0x00){
			set_data = para_set_value.data.precoin_set_num[pre_value.country[COUNTRY_ID].coin[i].data.coin_type];//Ԥ�ü�������ֵ��ʼ��
		}else{
			set_data = 9999;
		}
		*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set = set_data;
		*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_cur = 0;//��ǰ����ֵ ����
		*pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_full_flag = 0;

		dgus_tf1word(pre_value.country[COUNTRY_ID].coin[i].data.hmi_pre_count_set_addr, *pre_value.country[COUNTRY_ID].coin[i].data.p_pre_count_set);
	}
	counter_clear ();
}

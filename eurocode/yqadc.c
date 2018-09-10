////
#include "s3c2416.h"

s_coin_env coin_env;// (std_ad0-40)  //�����½� ��ֵ  990   950  ��ǰ��׼�½�40


void adcsininget(uint16_t ad0,uint16_t ad1,uint16_t ad2)
{

	coin_env.std_down_value0= (ad0 - WAVE_DOWN_VALUE);  //�����½� ��ֵ 40
	coin_env.std_up_value0= (ad0 - WAVE_UP_VALUE);  //���λָ� 20
	//cy_println( "up: %d", coin_env.std_down_value0);
	//cy_println( "dn: %d", coin_env.std_up_value0);
}


volatile uint32_t adtime = 0;    //��ʱ�ж��� ��ʱ

//////////////////////////////////////////////////////
 uint16_t ad0_mintemp = 0;    // = AD0STDVALUE;		//��Ϊ ��ad1_ad_value ֵ���бȽϵ�ֵ��Ϊ��ȡ�ò������ֵ
 uint16_t ad0_maxtemp = 0;   //���λָ�  �ο��Ƚ�ֵ
 uint16_t wave0down_flagone = 0;
 uint16_t wave0up_flagone = 0;    //�������� �ο��Ƚ�ֵ
 uint16_t wave0up_flag = 0;
 uint16_t wave0down_flagtwo =0;   //˫��ʱ�������жϵڶ�����
 uint32_t ch0_count =0;  //ͨ��0 ͨ����Ӳ�Ҽ���
 uint32_t ch0_pre_count =0;  //ͨ��0 ��Ӳ�� ���
 uint32_t ch0_coin_come =0;  //ͨ�� ��Ӳ�� ���
 volatile uint16_t blockflag = 0;      //�±ұ�־����
 uint16_t ad0_min = 0;     //����ÿöӲ�ҹ�ȥ��� ���ֵ


//int16_t coin_env.ad0_averaged_value = 0;    //�����й����� ��ADSAMPNUM�� value�����ֵ
 int16_t ad0_ad_value = 0;  //�����й����� ��ADSAMPNUM�� value�ĺ�ֵ��ƽ��ֵ


 int16_t ad0_temp_value = 0;  //����ƽ��ֵʱ�õ�����ʱ ����
 int16_t ad0_samp_number = 0;   // ad value sample times
 int16_t ad1_temp_value = 0;  //����ƽ��ֵʱ�õ�����ʱ ����
 int16_t ad1_samp_number = 0;   // ad value sample times
 int16_t ad2_temp_value = 0;  //����ƽ��ֵʱ�õ�����ʱ ����
 int16_t ad2_samp_number = 0;   // ad value sample times

 int16_t ad0_ad_value_buf[ADSAMPNUM0];	//ad value sample  ADSAMPNUM times
 int16_t ad1_ad_value_buf[ADSAMPNUM0];	//ad value sample  ADSAMPNUM times
 int16_t ad2_ad_value_buf[ADSAMPNUM0];	//ad value sample  ADSAMPNUM times



 uint16_t ad1_ad_value= 0;	//ad value sample  ADSAMPNUM times
 uint16_t ad1_mintemp = 0;// = AD1STDVALUE;		//��Ϊ ��ad1_ad_value ֵ���бȽϵ�ֵ��Ϊ��ȡ�ò������ֵ
 uint16_t ad1_maxtemp = 0;   //���λָ�  �ο��Ƚ�ֵ
 uint16_t ad1_min = 0;     //����ÿöӲ�ҹ�ȥ��� ���ֵ
 uint32_t ch1_coin_come =0;  //ͨ��0 ��Ӳ�� ���


 uint16_t ad2_ad_value = 0;	//ad value sample  ADSAMPNUM times
 uint16_t ad2_mintemp = 0;		//��Ϊ ��ad2_ad_value ֵ���бȽϵ�ֵ��Ϊ��ȡ�ò������ֵ
 uint16_t wave2up_flagone =0;    //�������� �ο��Ƚ�ֵ
 uint32_t ch2_coin_come =0;  //ͨ��0 ��Ӳ�� ���
 uint16_t ad2_min = 0;     //����ÿöӲ�ҹ�ȥ��� ���ֵ
 uint32_t ch1_count1temp =0;  //ͨ��1 ��Ӳ�� ���


//ģ��ת��  0  ����
//Ŀǰ��������ƽ���˲��㷨���д��� N= 10 ���һ�����������˲������д����жϲ��ε�����

void coin_env_init (void)
{
	int i;
	S8 *p = (S8 *) &coin_env;
	for (i = 0; i < sizeof(s_coin_env); i++)
	{
		*(p++) = 0;
	}

	Detect_AD_Value_buf_p = Detect_AD_Value_buf[0];
}
extern int print_ad0, print_ad1, print_ad2;
uint16_t ad_updated = 0;
//ģ��������
void AD_Sample_All (void)
{
	//Read AD 0 ////////////////////////////////////////////////////////////////////////////////////////
	rADCMUX = 0;		//setup channel
	rADCCON|=0x1;									//start ADC
	while(rADCCON & 0x1);							//check if Enable_start is low

	//PreProcess AD 0 Start*********************************************************************
	if( (ad0_samp_number > (ADSAMPNUM0-1)) ){   //����ɼ�����ADSAMPNUM������10
		ad0_samp_number = 0;
	}
	ad0_temp_value = ad0_ad_value_buf[ad0_samp_number];//old ad0_samp_number = 0����һ�β���ֵ,����2
	coin_env.ad0_averaged_value -= ad0_temp_value;//��ͼ�ȥ��һ��ADֵ
	//PreProcess AD 0 End**********************************************************************

	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
	ad0_ad_value_buf[ad0_samp_number] = ((int)rADCDAT0 & 0x3ff);
//	print_ad0 = ad0_ad_value_buf[ad0_samp_number];

	//Read AD 1 ////////////////////////////////////////////////////////////////////////////////////////
	rADCMUX = 0x01;		//setup channel
	rADCCON|=0x1;									//start ADC
	while(rADCCON & 0x1);							//check if Enable_start is low

	//Process AD 0 Start ***********************************************************************
	/*coin_env.ad0_averaged_value ���*/
	coin_env.ad0_averaged_value += ad0_ad_value_buf[ad0_samp_number];//coin_env.ad0_averaged_value ��ֵ�����
	ad0_ad_value = (coin_env.ad0_averaged_value)/ADSAMPNUM0;   //��ǰ�����ܺͺ���ƽ��ֵ
	Detect_AD_Value_buf_p[detect_sample_data_buf_index].AD0 = ad0_ad_value; // save ad to buffer
	ad0_samp_number++;
	if( (ad0_samp_number > (ADSAMPNUM0-1)) ){   //����ɼ�����ADSAMPNUM������10
		ad0_samp_number = 0;
	}
	//Process AD 0 End ***********************************************************************

	//PreProcess AD 1 Start*********************************************************************
	if( (ad1_samp_number > (ADSAMPNUM0-1)) ){   //����ɼ�����ADSAMPNUM������10
		ad1_samp_number = 0;
	}
	ad1_temp_value = ad1_ad_value_buf[ad1_samp_number];//old ad1_samp_number = 0����һ�β���ֵ,����2
	coin_env.ad1_averaged_value -= ad1_temp_value;//��ͼ�ȥ��һ��ADֵ
	//PreProcess AD 1 End**********************************************************************

	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high

	ad1_ad_value_buf[ad1_samp_number] = ((int)rADCDAT0 & 0x3ff);
//	print_ad1 = ad1_ad_value_buf[ad1_samp_number];

	//Read AD 2 ////////////////////////////////////////////////////////////////////////////////////////
	rADCMUX = 0x02;		//setup channel
	rADCCON|=0x1;									//start ADC
	while(rADCCON & 0x1);							//check if Enable_start is low

	//Process AD 1 Start ***********************************************************************
	/*coin_env.ad1_averaged_value �����*/
	coin_env.ad1_averaged_value += ad1_ad_value_buf[ad1_samp_number];//coin_env.ad1_averaged_value ��ֵ�����
	ad1_ad_value = (coin_env.ad1_averaged_value)/ADSAMPNUM0;   //��ǰ�����ܺͺ���ƽ��ֵ
	Detect_AD_Value_buf_p[detect_sample_data_buf_index].AD1 = ad1_ad_value; // save ad to buffer
	ad1_samp_number++;
	if( (ad1_samp_number > (ADSAMPNUM0-1)) ){   //����ɼ�����ADSAMPNUM������10
		ad1_samp_number = 0;
	}
	//Process AD 1 End ***********************************************************************

	//PreProcess AD 2 Start*********************************************************************
	if( (ad2_samp_number > (ADSAMPNUM0-1)) ){   //����ɼ�����ADSAMPNUM������10
		ad2_samp_number = 0;
	}
	ad2_temp_value = ad2_ad_value_buf[ad2_samp_number];//old ad2_samp_number = 0����һ�β���ֵ,����2
	coin_env.ad2_averaged_value -= ad2_temp_value;//��ͼ�ȥ��һ��ADֵ
	//PreProcess AD 2 End**********************************************************************

	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
	ad2_ad_value_buf[ad2_samp_number] = ((int)rADCDAT0 & 0x3ff);
//	print_ad2 = ad2_ad_value_buf[ad2_samp_number];

	//Process AD 2 Start ***********************************************************************
	/*coin_env.ad2_averaged_value �����*/
	coin_env.ad2_averaged_value += ad2_ad_value_buf[ad2_samp_number];//coin_env.ad2_averaged_value ��ֵ�����
	ad2_ad_value = (coin_env.ad2_averaged_value)/ADSAMPNUM0;   //��ǰ�����ܺͺ���ƽ��ֵ
	Detect_AD_Value_buf_p[detect_sample_data_buf_index].AD2 = ad2_ad_value; // save ad to buffer
	ad2_samp_number++;
	if( (ad2_samp_number > (ADSAMPNUM0-1)) ){   //����ɼ�����ADSAMPNUM������10
		ad2_samp_number = 0;
	}
	//Process AD 2 End ***********************************************************************
//		cy_println ("AD0:	%d,	AD1:	%d,	AD2:	%d", ad0_ad_value_buf[ad0_samp_number-1],
//																						 ad1_ad_value_buf[ad1_samp_number-1],
//																						 ad2_ad_value_buf[ad2_samp_number-1]);


	detect_sample_data_buf_index++;
	if (detect_sample_data_buf_index >= DATA_BUF_LENGTH){
		detect_sample_data_buf_index = 0;
	}
	if (ad_updated == 0){
		ad_updated = 1;
	}
}

uint32_t coin_cross_time = 0;

void cy_ad0_valueget(void)
{

	switch(coin_env.ad0_step)
	{
		case 0:
		{
			ch0_count = 0;
			ch0_pre_count = ch0_count;

			ch0_coin_come = 0;

			wave0down_flagone = 0;
			wave0up_flagone = 0;
			wave0up_flag = 0;
			wave0down_flagtwo = 0;

			detect_sample_data_buf_index = 0;

			ad0_samp_number = 0;
			//coin_env.ad0_averaged_value = 0;

			ad0_ad_value = AD0STDSET;
			ad0_mintemp = AD0STDSET;//900
			ad0_min = AD0STDSET;//900

			blockflag = ADBLOCKT;	   //ʹ�ü�α������ ���±� �±�ʱ��5
			coin_env.ad0_step = 3;
			sys_env.AD_buf_index = 0;
			Detect_AD_Value_buf_p = Detect_AD_Value_buf[sys_env.AD_buf_index];
			break;
		}
		/*��ȡͨ��0,ad0_ad_value����10����Ϊ��ֵ*/
		case 2:
		{
			ad0_ad_value_buf[ad0_samp_number] = ReadAdc0();
			coin_env.ad0_averaged_value += ad0_ad_value_buf[ad0_samp_number];   //��ǰ�����ܺ�
			ad0_samp_number++;
			blockflag = ADBLOCKT;//�±�ʱ�� 400ms
			if( (ad0_samp_number > (ADSAMPNUM0-1)) ) {  //����ɼ�����ADSAMPNUM������ 10
				ad0_samp_number = 0;
				ad0_ad_value = coin_env.ad0_averaged_value / ADSAMPNUM0;   //��ǰ�����ƽ��ֵ
				coin_env.ad0_step = 3; //10 �βŻ���������3
				dbg ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
			}
			break;
		}
/*�����Ա�,�ɲ���10��ֵ-�ɲ���ĳ��ֵ+��ǰ����ֵ/����*/
		case 3:
		{
			blockflag = ADBLOCKT;	   //ʹ�ü�α������ ���±�
			if( (ad0_ad_value < coin_env.std_down_value0)){  //std_value	standard value for detect  ��ƽ��ֵ�ļ���Ϊ��ǰֵ�ļ��
				wave0down_flagone++;    //С�ڲο�ֵ  ���ʾ��Ӳ�ҹ���
			}else{// if((ad0_ad_value >= coin_env.std_down_value0)){/*��ֵ���ڵ��ڲο�ֵʱ*/
				wave0down_flagone = 0;
				detect_sample_data_buf_index = 0;
			}
			/*����ֵС�ڲο�ֵ,������WAVE_DOWN_TO_STD_N�����ϣ���������*/
			if(  wave0down_flagone > WAVE_DOWN_TO_STD_N){     //WAVE_DOWN_TO_STD_N 4
				//coin_env.ad0_step  = 9;
				coin_cross_time = 0;//��ʼ��ʱ,����Ӳ��ͨ����������ʱ��
				wave0down_flagone = 0;

				ad0_mintemp = ad0_ad_value;
				ch0_coin_come++;  //ͨ�� 0 ��⵽��Ӳ��  ֪ͨ����ͨ����ʼ����
				coin_env.ad0_step  = 10;
			}
			break;
		}
		case 10:						//ad0 value get
		{
			//AD_Sample_All ();

			if( ad0_ad_value < ad0_mintemp ){  //���γ����½��д˴�ʹ�õ� �Ƚϲ����ǳ���Ҫ��ԭ����ad0_ad_value_buf[ad0_samp_number] ��һ��ad0_ad_value
				ad0_mintemp = ad0_ad_value;	//������Сֵ
				coin_env.AD_min_index[0] = detect_sample_data_buf_index;
				wave0up_flagone = 0;
			}else if((ad0_ad_value > ad0_mintemp )&&((ad0_ad_value - ad0_mintemp ) > WAVE_COIN_TWO_UP_VALUE)){    //����    ����
				wave0up_flagone++;
			}else{
				wave0up_flagone = 0;
			}
			/*������WAVE_GO_UP_N��,���λ���*/
			if( wave0up_flagone  > WAVE_GO_UP_N){     //ȷ�ϲ��η�ֵ
				//coin_env.ad0_step = 19;
				wave0up_flagone = 0;

				ad0_min = ad0_mintemp;
				ad0_mintemp = AD0STDSET;//900
				ch0_count++;    //if ch0_count != ch_counttemp ,���ʾͨ��0�ɼ���һö
				ad0_maxtemp = ad0_ad_value ;//ad0_ad_value��;�ֵ
				coin_env.ad0_step = 25; ///25
			}
			break;
		}
		case 25:						//read in ad value ADSAMPNUM times
		{
			//AD_Sample_All ();
			/*��ֵ���ڲο�ֵ,��ʾһö�Ѿ���ȥ*/
			if( (ad0_ad_value >= coin_env.std_up_value0) ){    //��� ���λָ����ο�ֵ����ʾһö����
				wave0up_flag++;
				wave0down_flagtwo = 0;
				ad0_maxtemp = ad0_ad_value ;
			}else if( 	(ad0_ad_value < coin_env.std_down_value0) &&
						(ad0_ad_value < ad0_maxtemp) &&
						((ad0_maxtemp - ad0_ad_value) > WAVE_COIN_TWO_DOWN_VALUE) ){
				wave0up_flag = 0;
				wave0down_flagtwo++;//�����������,����ʱ�����ȹ�ȥ��û�ָ����ο�ֵ��������һ������,
			}else if(ad0_ad_value >= ad0_maxtemp){/*������λ�������������ȷ�����ֵ*/
				wave0up_flag = 0;
				wave0down_flagtwo = 0;
				ad0_maxtemp = ad0_ad_value ;
			}

			/*�Ѿ��ָ��ο�ֵ,���س�ʼ����*/
			if( wave0up_flag > WAVE_UP_TO_STD_N){// WAVE_UP_TO_STD_N 2
				//COIN_KICK_OP_0 ();//Ӳ�ҳ�������ʼ�޳�
				coin_env.ad0_step = 3; //
				sys_env.coin_cross_time = coin_cross_time;//Ӳ�ҳ����ˣ�����ͳ��Ӳ�Ҿ�����ʱ��
				sys_env.AD_data_len = detect_sample_data_buf_index;
				blockflag = ADBLOCKT;      //�±�ʱ�临λ
				wave0up_flag =0;
				wave0down_flagtwo = 0;
				if (sys_env.AD_buf_sending == 0){
					sys_env.AD_buf_sending = 1;
					sys_env.Detect_AD_buf_p = Detect_AD_Value_buf_p;
					sys_env.AD_buf_index++;
					sys_env.AD_buf_index %= AD_BUF_GROUP_LEN;
					Detect_AD_Value_buf_p = Detect_AD_Value_buf[sys_env.AD_buf_index];
				}
				memset (Detect_AD_Value_buf_p, 0, sizeof (AD_Value) * DATA_BUF_LENGTH);
				detect_sample_data_buf_index = 0;
				break;
			}
			/*����˫����,����״̬10*/
			if( (wave0down_flagtwo > WAVE_DOWN_TWO_N)) {    //��ʾ˫������ WAVE_DOWN_TWO_N 8
				//COIN_KICK_OP_0 ();//����״̬�ң�ǰһ���ٱ������￪ʼ�޳�
				coin_env.ad0_step = 10;
				blockflag = ADBLOCKT;      //ʹ�ü�α������ ���±� 5
				wave0up_flag =0;
				wave0down_flagtwo = 0;
				sys_env.coin_cross_time = coin_cross_time;//Ӳ�ҳ����ˣ�����ͳ��Ӳ�Ҿ�����ʱ��
				coin_cross_time = 0;
				ad0_mintemp = ad0_ad_value;
				ch0_coin_come++;  //ͨ�� 0 ��⵽����  ֪ͨ����ͨ����ʼ����
				break;
			}
			break;
		}
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////


//ģ��ת��  1  ����
void cy_ad1_valueget(void)
{
	switch(coin_env.ad1_step)
	{
		case 0:
		{
			ch1_coin_come = ch0_coin_come;
			ad1_ad_value = AD1STDSET;
			ad1_mintemp = AD1STDSET;
			ad1_min = AD1STDSET;
			ad1_samp_number = 0;
//			coin_env.ad1_averaged_value = 0;

			coin_env.ad1_step = 3;
			break;
		}
		case 2:
		{
			//**********************************************************************
			//cy_println ("Start Sample AD1");
			//*************************************************************************
			ad1_ad_value_buf[ad1_samp_number] = ReadAdc1();
			coin_env.ad1_averaged_value = coin_env.ad1_averaged_value + ad1_ad_value_buf[ad1_samp_number];   //��ǰ�����ܺ�
			ad1_samp_number++;
			if( (ad1_samp_number > (ADSAMPNUM0-1)) ){//����ɼ�����ADSAMPNUM������ 10

				ad1_samp_number = 0;
				ad1_ad_value = coin_env.ad1_averaged_value/ADSAMPNUM0;   //��ǰ�����ƽ��ֵ
				coin_env.ad1_step = 3; //10 �βŻ���������3
				dbg ("go to  coin_env.ad1_step %d", coin_env.ad0_step);
			}
			break;
		}
		case 3://�ȴ�����Ӳ�Ҽ��
		{
			if(( ch1_coin_come != ch0_coin_come)){//Ӳ������,+1ֵ��һ��
				coin_env.ad1_step = 5;
				ad1_mintemp =  ad1_ad_value;
				ch1_coin_come  =  ch0_coin_come;
				break;
			}
			break;
		}
		case 5://����Ӳ�Ҽ��
		{
			if(ad1_mintemp > ad1_ad_value){
				ad1_mintemp =  ad1_ad_value;
				coin_env.AD_min_index[1] = detect_sample_data_buf_index;
			}
			/**/
 			if((ch0_pre_count != ch0_count)){//���0ͨ�������ˡ�
				coin_env.ad1_step = 3;
  				ad1_min = ad1_mintemp;
				ad1_mintemp = AD1STDSET;
			}
 			break;
		}
	}
}

//ģ��ת��  2  ����
void cy_ad2_valueget(void)
{
	switch(coin_env.ad2_step)
	{
		/*��ʼ��ֵ*/
		case 0:
		{
			ch2_coin_come = ch0_coin_come;

			ad2_ad_value = AD2STDSET;//900
			ad2_mintemp = AD2STDSET;//900
			ad2_min = AD2STDSET;//900

			ad2_samp_number = 0;
			//coin_env.ad2_averaged_value = 0;

			coin_env.ad2_step = 3;
			break;
		}
		case 2:
		{
			//**********************************************************************
			//cy_println ("Start Sample AD2");
			ad2_ad_value_buf[ad2_samp_number] = ReadAdc2();
			coin_env.ad2_averaged_value = coin_env.ad2_averaged_value + ad2_ad_value_buf[ad2_samp_number];   //��ǰ�����ܺ�
			ad2_samp_number++;
			if( (ad2_samp_number > (ADSAMPNUM0-1)) ){   //����ɼ�����ADSAMPNUM������ 10
				ad2_samp_number = 0;
				ad2_ad_value = coin_env.ad2_averaged_value/ADSAMPNUM0;   //��ǰ�����ƽ��ֵ
				coin_env.ad2_step = 3; //10 �βŻ���������3

				dbg ("go to  coin_env.ad2_step %d", coin_env.ad0_step);
			}
			//*************************************************************************
			break;
		}
		case 3://�ȴ�����Ӳ�Ҽ��
		{
			if(( ch2_coin_come != ch0_coin_come)){
				coin_env.ad2_step	 = 5;
				ad2_mintemp = ad2_ad_value;
				ch2_coin_come = ch0_coin_come;
				break;
			}
			break;
		}
		case 5://����Ӳ�� ���
		{

			if( ad2_mintemp > ad2_ad_value ){
				ad2_mintemp = ad2_ad_value;
				coin_env.AD_min_index[2] = detect_sample_data_buf_index;
			}

			if((ch0_pre_count != ch0_count)){
				coin_env.ad2_step	 = 3;
				ad2_min =  ad2_mintemp;
				ad2_mintemp = AD2STDSET;//900
			}
			break;
		}
	}
	return;
}


////////////////////////////////////////////////////////////////////////////////
uint16_t std_ad0 = 0;
uint16_t std_ad1 = 0;
uint16_t std_ad2 = 0;
uint16_t std_ad3= 0;

 int temperstd = 281;   //20��  20*10 +600  = 800MV;  800/3300 *1024 = 248.24
 int temperpre = 250;
#define GETTEMPERCP 0 //std_ad3 - temperstd) // 0  //(((std_ad3 - temperstd)*10)/22)
#define GETTEMPER2CP 0 //(std_ad3 - temperstd)  //0  //(((std_ad3 - temperstd)*10)/40) //((( std_ad3 - temperpre)*15)/10)



///////////////////////////////////////
// ��ֵȼ�
// U8 cn0copmaxc0[COINCNUM] = {15,33,40,10}; //20 //
// U8 cn0copminc0[COINCNUM] = {30,65,40,20};  //20
// U8 cn0copmaxc1[COINCNUM] = {15,40,40,10};  //60
// U8 cn0copminc1[COINCNUM] = {30,50,40,20}; //30
// U8 cn0copmaxc2[COINCNUM] = {15,40,40,10}; //40
// U8 cn0copminc2[COINCNUM] = {50,40,40,20}; //40


 s_coin_compare_value coin_cmp_value[COIN_TYPE_NUM];

void print_std_value(void)    //  ��� ��׼ֵ   �в���ƫ����в���
{
//	uint16_t is = 0;
//	uint16_t ad2_std[AD2STDNUM];
//	uint16_t ad1_std[AD1STDNUM];
//	uint16_t ad0_std[AD0STDNUM];
//	uint16_t ad2countis = 10;
//	std_ad3 = ReadAdc3();

//	for(is =0;is<ad2countis;is++)
//	{
//		ad2_std[is] = ReadAdc2();
//	}
//	std_ad2 = 0;
//	for(is =0;is<ad2countis;is++)
//	{
//		std_ad2 = std_ad2+ad2_std[is];
//	}
//
//	std_ad2 =std_ad2/ad2countis;
//	/////////////////////////////////////////
//	for(is=0;is<AD1STDNUM;is++)
//	{
//		ad1_std[is] = ReadAdc1();
//	}
//	std_ad1 = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;
//	////////////////////////////////////
//	for(is=0;is<AD0STDNUM;is++)
//	{
//		ad0_std[is] = ReadAdc0();
//	}
//	std_ad0 = (ad0_std[2] +ad0_std[3] +ad0_std[4]+ad0_std[5]+ad0_std[6]+ad0_std[7]+ad0_std[8]+ad0_std[9])/8;
	std_ad0 = ad0_ad_value;
	std_ad1 = ad1_ad_value;
	std_ad2 = ad2_ad_value;
}

void get_ad_value (void)
{
	uint16_t is;
	std_ad0 = 0;
	std_ad1 = 0;
	std_ad2 = 0;
	for (is = 0; is < ADSAMPNUM+2; is++){
		while (ad_updated == 0);
		ad_updated = 0;
		if (is > 1){
			std_ad0 += ad0_ad_value;
			std_ad1 += ad1_ad_value;
			std_ad2 += ad2_ad_value;
		}
		//cy_print("--%02d:A0 :%d   A1 :%d  A2 :%d  \r\n",is, ad0_ad_value,ad1_ad_value,ad2_ad_value);
	}
	std_ad0 /= ADSAMPNUM;
	std_ad1 /= ADSAMPNUM;
	std_ad2 /= ADSAMPNUM;
	cy_print("A0 :%d   A1 :%d  A2 :%d  \r\n",std_ad0,std_ad1,std_ad2);
}

uint16_t adstd_offset()    //  ��� ��׼ֵ   �в���ƫ����в���
{
	int16_t std0_offset, std1_offset, std2_offset;
	uint16_t is;
//		////////////////////////////////////
//	uint16_t ad0_std[AD0STDNUM];
//	uint16_t ad2_std[AD2STDNUM];
//	uint16_t ad1_std[AD1STDNUM];
//


//	for(is=0;is<AD0STDNUM;is++){
//		ad0_std[is] = ReadAdc0();
//	}
//	std_ad0 = (ad0_std[2] +ad0_std[3] +ad0_std[4]+ad0_std[5]+ad0_std[6]+ad0_std[7]+ad0_std[8]+ad0_std[9])/8;
////	std_ad0 = (ad0_std[2]  +ad0_std[4]+ad0_std[5]+ad0_std[7]+ad0_std[9])/5;

//
///////////////////////////
//	for(is =0;is<AD2STDNUM;is++){
//		ad2_std[is] = ReadAdc2();
//	}
//	std_ad2 = (ad2_std[2]+ad2_std[3]+ad2_std[4] +ad2_std[5] +ad2_std[6] +ad2_std[7]+ad2_std[8] +ad2_std[9])/8;
//	/////////////////////////////////////////
//	for(is=0;is<AD1STDNUM;is++){
//		ad1_std[is] = ReadAdc1();
//	}
//	std_ad1  = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;

	//offset_ad0 = std_ad0 - AD0STDSET;
	/////////////////////////ÿ��Ӳ�� �������в���
//	std_ad3 = ReadAdc3();
	get_ad_value ();
	//���ƫ����200 ���Խ��в�����������ܴ������������ˣ�����������ʾ
	if( ( ((std_ad0-GETTEMPERCP) < (pre_value.country[coinchoose].coin[0].data.std0 + OFFSETMAX))) &&
		  ((std_ad0-GETTEMPERCP) > (pre_value.country[coinchoose].coin[0].data.std0 - OFFSETMIN)) ){
	}else {
		return 0;
	}


#define UP_POW (0.3)
#define DOWN_POW (0.8)
	/////////////
	dbg ("real     std0 = %4d          std1 = %4d     std2 = %4d", std_ad0, std_ad1, std_ad2);
	for (is = 0; is < COIN_TYPE_NUM; is++){ //����ֵ
		std0_offset = std_ad0 - pre_value.country[coinchoose].coin[is].data.std0;
		std1_offset = std_ad1 - pre_value.country[coinchoose].coin[is].data.std1;
		std2_offset = std_ad2 - pre_value.country[coinchoose].coin[is].data.std2;
/////////////////////////////////////////////////////////////////////////////////
		if (std0_offset > 0){
			std0_offset *= UP_POW;
		}else{
			std0_offset *= DOWN_POW;
		}
		if (std1_offset > 0){
			std1_offset *= UP_POW;
		}else{
			std1_offset *= DOWN_POW;
		}
		if (std2_offset > 0){
			std2_offset *= UP_POW;
		}else{
			std2_offset *= DOWN_POW;
		}
/////////////////////////////////////////////////////////////////////////////////
	#ifdef SAMPLE_METHOD_0
		coin_cmp_value[is].compare_max0 = (pre_value.country[coinchoose].coin[is].data.max0 + pre_value.country[coinchoose].coin[is].data.offsetmax0 +
										  std0_offset);
		if (pre_value.country[coinchoose].coin[is].data.min0 > 5){
			coin_cmp_value[is].compare_min0 = (pre_value.country[coinchoose].coin[is].data.min0 + pre_value.country[coinchoose].coin[is].data.offsetmin0 +
										  std0_offset);
		}else{
			coin_cmp_value[is].compare_min0 = pre_value.country[coinchoose].coin[is].data.min0;
		}

		coin_cmp_value[is].compare_max1 = (pre_value.country[coinchoose].coin[is].data.max1 + pre_value.country[coinchoose].coin[is].data.offsetmax1 +
										  std1_offset - std0_offset);
		coin_cmp_value[is].compare_min1 = (pre_value.country[coinchoose].coin[is].data.min1 + pre_value.country[coinchoose].coin[is].data.offsetmin1 +
										  std1_offset - std0_offset);

		coin_cmp_value[is].compare_max2 = (pre_value.country[coinchoose].coin[is].data.max2 + pre_value.country[coinchoose].coin[is].data.offsetmax2 +
										  std2_offset - std1_offset);
		coin_cmp_value[is].compare_min2 = (pre_value.country[coinchoose].coin[is].data.min2 + pre_value.country[coinchoose].coin[is].data.offsetmin2 +
										  std2_offset - std1_offset);
	#endif
	#ifdef SAMPLE_METHOD_1
		coin_cmp_value[is].compare_max0 = (pre_value.country[coinchoose].coin[is].data.max0 + pre_value.country[coinchoose].coin[is].data.offsetmax0 +
										  std0_offset);
		if (pre_value.country[coinchoose].coin[is].data.min0 > 5){
			coin_cmp_value[is].compare_min0 = (pre_value.country[coinchoose].coin[is].data.min0 + pre_value.country[coinchoose].coin[is].data.offsetmin0 +
										  std0_offset);
		}else{
			coin_cmp_value[is].compare_min0 = pre_value.country[coinchoose].coin[is].data.min0;
		}

		coin_cmp_value[is].compare_max1 = (pre_value.country[coinchoose].coin[is].data.max1 + pre_value.country[coinchoose].coin[is].data.offsetmax1 +
										  std1_offset);
		coin_cmp_value[is].compare_min1 = (pre_value.country[coinchoose].coin[is].data.min1 + pre_value.country[coinchoose].coin[is].data.offsetmin1 +
										  std1_offset);

		coin_cmp_value[is].compare_max2 = (pre_value.country[coinchoose].coin[is].data.max2 + pre_value.country[coinchoose].coin[is].data.offsetmax2 +
										  std2_offset);
		coin_cmp_value[is].compare_min2 = (pre_value.country[coinchoose].coin[is].data.min2 + pre_value.country[coinchoose].coin[is].data.offsetmin2 +
										  std2_offset);

		if (coin_cmp_value[is].compare_max0 < 10){
			coin_cmp_value[is].compare_max0 = 10;
		}
		if (coin_cmp_value[is].compare_max1 < 10){
			coin_cmp_value[is].compare_max1 = 10;
		}
		if (coin_cmp_value[is].compare_max2 < 10){
			coin_cmp_value[is].compare_max2 = 10;
		}

	#endif
	}
	adcsininget(std_ad0,std_ad1,std_ad2);//AD ���ν��� �ķ�ֵ

	return 1;
	////////////////////////////////////
}


volatile uint32_t start_sample = 0;



AD_Value AD_Value_buf[DATA_BUF_LENGTH];
AD_Value Detect_AD_Value_buf[AD_BUF_GROUP_LEN][DATA_BUF_LENGTH];
AD_Value *Detect_AD_Value_buf_p;
//volatile AD_Value Adj_AD_Value_buf[ADJ_BUF_LENGTH];
volatile AD_Value NG_value_buf[NG_BUF_LENGTH];
volatile AD_Value GOOD_value_buf[NG_BUF_LENGTH];
volatile uint32_t ng_value_index = 0;
volatile uint32_t good_value_index = 0;
volatile uint32_t  sample_data_buf_index = 0;
volatile uint32_t detect_sample_data_buf_index = 0;

/////////////////////////////////

extern void Uart_SendByte(int data);

void send_sample_data (AD_Value ad_value_buf[DATA_BUF_LENGTH], int counter)
{
	uint16_t  minTempAD0 = 1000;
	uint16_t  minTempAD1 = 1000;
	uint16_t  minTempAD2 = 1000;
	int i = 0;
	int H_min_index = 0;
	int M_min_index = 0;
	int L_min_index = 0;

	cy_print ("start\n");
	for (i = 0; i < counter; i++){
		if (minTempAD0 > ad_value_buf[i].AD0){
			minTempAD0 = ad_value_buf[i].AD0;
			H_min_index = i;
		}
		if (minTempAD1 > ad_value_buf[i].AD1){
			minTempAD1 = ad_value_buf[i].AD1;
			M_min_index = i;
		}
		if (minTempAD2 > ad_value_buf[i].AD2){
			minTempAD2 = ad_value_buf[i].AD2;
			L_min_index = i;
		}
		cy_print("%d	%d	%d	\n",ad_value_buf[i].AD0, ad_value_buf[i].AD1, ad_value_buf[i].AD2);
	}
	cy_print ("end\n");

	if ( minTempAD0 > 0){
		minTempAD0 = ad_value_buf[H_min_index].AD0;
		minTempAD1 = ad_value_buf[H_min_index].AD1;
		minTempAD2 = ad_value_buf[H_min_index].AD2;
	}else if (minTempAD1 > 0){
		minTempAD0 = ad_value_buf[M_min_index].AD0;
		minTempAD1 = ad_value_buf[M_min_index].AD1;
		minTempAD2 = ad_value_buf[M_min_index].AD2;
	}else{
		minTempAD0 = ad_value_buf[L_min_index].AD0;
		minTempAD1 = ad_value_buf[L_min_index].AD1;
		minTempAD2 = ad_value_buf[L_min_index].AD2;
	}
	dgus_tf1word(ADDR_STDH, (minTempAD0)); //	high frequence
	dgus_tf1word(ADDR_STDM,  minTempAD1); //	middle frequence
	dgus_tf1word(ADDR_STDL, (minTempAD2)); //	low frequence
}
uint16_t adstd_sample(void)    //��׼ֵ����
{
	uint16_t is = 0;
	uint16_t ad2_std[AD2STDNUM];
	uint16_t ad1_std[AD1STDNUM];
	uint16_t ad0_std[AD0STDNUM];
//	uint16_t ad2countis = 10;

	for(is=0;is<AD0STDNUM;is++)
	{
		ad0_std[is] = ReadAdc0();
		ad1_std[is] = ReadAdc1();
		ad2_std[is] = ReadAdc2();
	}
	std_ad0 = (ad0_std[2] +ad0_std[3] +ad0_std[4]+ad0_std[5]+ad0_std[6]+ad0_std[7]+ad0_std[8]+ad0_std[9])/8;
	std_ad1 = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;
	std_ad2 = (ad2_std[2] +ad2_std[3] +ad2_std[4]+ad2_std[5]+ad1_std[6]+ad2_std[7]+ad2_std[8]+ad2_std[9])/8;
	//if(adtime ==0)
	if ((std_ad0 < 850) && (start_sample == 0))
	{
		//cy_print ("here %d sample_data_buf_index %d\n", start_sample, sample_data_buf_index);
		start_sample = 1;
		sample_data_buf_index = 0;
	}
	else if (start_sample)
	{
		if ((sample_data_buf_index < 1200))
		{
			//cy_print ("here1 %d sample_data_buf_index %d\n", start_sample, sample_data_buf_index);
			AD_Value_buf[sample_data_buf_index].AD0 = std_ad0;
			AD_Value_buf[sample_data_buf_index].AD1 = std_ad1;
			AD_Value_buf[sample_data_buf_index].AD2 = std_ad2;
			//adtime = ADADJACTERTIME;
			adtime = 0;
			sample_data_buf_index++;
			if (sample_data_buf_index >= DATA_BUF_LENGTH){
				sample_data_buf_index = 0;
			}
		}
		else
		{
			send_sample_data (AD_Value_buf, sample_data_buf_index);
			start_sample = 0;
			sample_data_buf_index = 0;

		}
	}
	return 1;
}

uint16_t cy_adstd_adj(void)    //��׼����
{
//	uint16_t is = 0;
//	uint16_t ad2_std[AD2STDNUM];
//	uint16_t ad1_std[AD1STDNUM];
//	uint16_t ad0_std[AD0STDNUM];
//	uint16_t ad2countis = 10;
//	std_ad3 = ReadAdc3();

//	for(is =0;is<ad2countis;is++){
//		ad2_std[is] = ReadAdc2();
//	}
//	std_ad2 = 0;
//	for(is =0;is<ad2countis;is++){
//		std_ad2 = std_ad2+ad2_std[is];
//	}
//
//	std_ad2 =std_ad2/ad2countis;
//	/////////////////////////////////////////
//	for(is=0;is<AD1STDNUM;is++){
//		ad1_std[is] = ReadAdc1();
//	}
//	std_ad1 = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;
//	////////////////////////////////////
//	for(is=0;is<AD0STDNUM;is++){
//		ad0_std[is] = ReadAdc0();
//	}
//	std_ad0 = (ad0_std[2] +ad0_std[3] +ad0_std[4]+ad0_std[5]+ad0_std[6]+ad0_std[7]+ad0_std[8]+ad0_std[9])/8;
	std_ad0 = ad0_ad_value;
	std_ad1 = ad1_ad_value;
	std_ad2 = ad2_ad_value;
	/////////////
	if(adtime ==0){
		dgus_tf1word(ADDR_STDH,(std_ad0 - GETTEMPERCP)); //	high frequence
		dgus_tf1word(ADDR_STDM,std_ad1); //	middle frequence
		dgus_tf1word(ADDR_STDL,(std_ad2+GETTEMPER2CP)); //	low frequence
		cy_println ("\nNick-Cmd:%4d %4d %4d %4d",std_ad0,std_ad1,std_ad2,std_ad3);
		adtime = ADADJACTERTIME;
	}
//	cy_print("adstd_adjtime = %d\r\n",adstd_adjtime);
	return 1;
}


void setStdValue (void)
{
//	uint16_t is;
//		////////////////////////////////////
//	uint16_t ad0_std[AD0STDNUM];
//	uint16_t ad2_std[AD2STDNUM];
//	uint16_t ad1_std[AD1STDNUM];

//	for(is=0;is<AD0STDNUM;is++)
//	{
//		ad0_std[is] = ReadAdc0();
//	}
//	std_ad0 = (ad0_std[2]  +ad0_std[4]+ad0_std[5]+ad0_std[7]+ad0_std[9])/5;

//
///////////////////////////
//	for(is =0;is<AD2STDNUM;is++)
//	{
//		ad2_std[is] = ReadAdc2();
//	}
//	std_ad2 = (ad2_std[2]+ad2_std[3]+ad2_std[4] +ad2_std[5] +ad2_std[6] +ad2_std[7]+ad2_std[8] +ad2_std[9])/8;
//	/////////////////////////////////////////
//	for(is=0;is<AD1STDNUM;is++)
//	{
//		ad1_std[is] = ReadAdc1();
//	}
//	std_ad1  = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;

//	get_ad_value ();


	adcsininget(std_ad0,std_ad1,std_ad2);//AD ���ν��� �ķ�ֵ
}

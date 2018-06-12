
#include "S3C2416.h"

cmd_analyze_struct cmd_analyze;

void printf_hello(int32_t argc, void *cmd_arg);
void do_go(int32_t argc, void *cmd_arg);
void do_set(int32_t argc, void *cmd_arg);
void do_print(int32_t argc, void *cmd_arg);
void do_help(int32_t argc, void *cmd_arg);
void do_read(int32_t argc, void *cmd_arg);
void do_write(int32_t argc, void *cmd_arg);
void do_erase(int32_t argc, void *cmd_arg);
void do_reset_cpu(int32_t argc, void *cmd_arg);
void do_db(int32_t argc, void *cmd_arg);


/*命令表*/
const cmd_list_struct cmd_list[]={
/*   命令    参数数目    处理函数        帮助信息                         */
{"help",		0,	do_help,		"help"},
{"go",			8,	do_go,			"go <yccs>"},
{"set",			8,	do_set,			"set <print> <1>"},
{"print",		8,	do_print,		"print <env> "},
{"read",		8,	do_read,		"read <from> <nand> <block> <page>"},
{"write",		8,	do_write,		"write <to> <nand> <block> <page>"},
{"erase",		8,	do_erase,		"erase <to> <nand> <block> <page>"},
{"reset",		8,	do_reset_cpu,		"reset system"},
{"r",		8,	do_reset_cpu,		"reset system"},
{"db",		8,	do_db,					"db delete all"},
};

s_system_env sys_env;

static int16_t ng_info_echo_off = 0;

char iap_code_buf[CODE_BUF_SIZE]; // 程序缓冲区256K
//U8 hex_file_buf[8192*2][64];//hex文件

void system_env_init (void)
{
	int i;
	S8 *p = (S8 *) &sys_env;
	for (i = 0; i < sizeof(s_system_env); i++){
		*(p++) = 0;
	}
	sys_env.country_index = coinchoose;
	sys_env.save_ng_data = 1;
	sys_env.save_good_data = 1;
	sys_env.uart0_cmd_flag = 0xA5;//console 未激活
	sys_env.password = 1573;
}

/**
* 使用SecureCRT串口收发工具,在发送的字符流中可能带有不需要的字符以及控制字符,
* 比如退格键,左右移动键等等,在使用命令行工具解析字符流之前,需要将这些无用字符以
* 及控制字符去除掉.
* 支持的控制字符有:
*   上移:1B 5B 41
*   下移:1B 5B 42
*   右移:1B 5B 43
*   左移:1B 5B 44
*   回车换行:0D 0A
*  Backspace:08
*  Delete:7F
*/
static uint32_t get_true_char_stream(char *dest, const char *src)
{
   uint32_t dest_count=0;
   uint32_t src_count=0;

    while(src[src_count] != '\0'){//0x0D && src[src_count+1]!=0x0A)
       if(isprint(src[src_count])){
           dest[dest_count++]=src[src_count++];
       }else{
           switch(src[src_count]){
                case 0x08:{
                    if(dest_count>0){
                        dest_count --;
                    }
                    src_count ++;
                }break;
                case 0x1B:{
                    if(src[src_count+1]==0x5B){
                        if(src[src_count+2]==0x41 || src[src_count+2]==0x42){
                            src_count +=3;              //上移和下移键键值
                        }else if(src[src_count+2]==0x43){
                            dest_count++;               //右移键键值
                            src_count+=3;
                        }else if(src[src_count+2]==0x44){
                            if(dest_count >0){//左移键键值
                                dest_count --;
                            }
                           src_count +=3;
                        }else{
                            src_count +=3;
                        }
                    }else{
                        src_count ++;
                    }
                }break;
                default:{
                    src_count++;
                }break;
           }
       }
    }
   dest[dest_count++]=src[src_count++];
    return dest_count;
}




unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

long simple_strtol(const char *cp,char **endp,unsigned int base)
{
	if(*cp=='-')
		return -simple_strtoul(cp+1,endp,base);
	return simple_strtoul(cp,endp,base);
}




/*字符串转10/16进制数*/
static int32_t string_to_dec(uint8_t *buf)
{
   uint32_t i=0;
   uint32_t base=10;       //基数
   int32_t  neg=1;         //表示正负,1=正数
   int32_t  result=0;

    if((buf[0]=='0')&&(buf[1]=='x'))
    {
       base=16;
       neg=1;
       i=2;
    }
    else if(buf[0]=='-')
    {
       base=10;
       neg=-1;
       i=1;
    }
    for(; buf[i] != 0; i++)
    {
       if(buf[i]==0x20 || buf[i]==0x0D || buf[i] == '\0')    //为空格
       {
           break;
       }

       result *= base;
       if(isdigit(buf[i]))                 //是否为0~9
       {
           result += buf[i]-'0';
       }
       else if(isxdigit(buf[i]))           //是否为a~f或者A~F
       {
            result+=tolower(buf[i])-87;
       }
       else
       {
           result += buf[i]-'0';
       }
    }
   result *= neg;

    return result ;
}



/**
* 命令参数分析函数,以空格作为一个参数结束,支持输入十六进制数(如:0x15),支持输入负数(如-15)
* @param rec_buf   命令参数缓存区
* @param len       命令的最大可能长度
* @return -1:       参数个数过多,其它:参数个数
*/
static int32_t cmd_arg_analyze(const char *rec_buf, unsigned int len)
{
   uint32_t i;
   uint32_t blank_space_flag=0;    //空格标志
   uint32_t arg_num=0;             //参数数目
   uint32_t index[ARG_NUM];        //有效参数首个数字的数组索引

    /*先做一遍分析,找出参数的数目,以及参数段的首个数字所在rec_buf数组中的下标*/
    for(i=0;i<len;i++)
    {
       if(rec_buf[i]==0x20)        //为空格
       {
           blank_space_flag=1;
           continue;
       }
       else if(rec_buf[i]==0x0D || rec_buf[i] == '\0')   //换行
       {
           break;
       }
       else
       {
           if(blank_space_flag==1)
           {
                blank_space_flag=0;
                if(arg_num < ARG_NUM)
                {
                   index[arg_num]=i;
                    arg_num++;
                }
                else
                {
                    return -1;      //参数个数太多
                }
           }
       }
    }

    for(i=0;i<arg_num;i++)
    {
        cmd_analyze.cmd_arg[i]=string_to_dec((unsigned char *)(rec_buf+index[i]));
    }
    return arg_num;
}





void run_command (char * _cmd_str)
{
	int i = 0;

	SetWatchDog(); //看门狗喂狗

	while (*(_cmd_str + i) )
	{
		if ( (*(_cmd_str + i) ) == 0x0a)
			break;
		cmd_analyze.rec_buf[i] = *(_cmd_str + i);
		//cy_print ("%02x ", cmd_analyze.rec_buf[i]);
		i++;
	}
	if (i == 0)
		return;
	cmd_analyze.rec_buf[i] = '\0';
	//cmd_analyze.rec_buf[i + 1] = 0x0A;
	vTaskCmdAnalyze ();
}

	/*命令行分析任务*/
void vTaskCmdAnalyze( void )
{
   uint32_t i;
   int32_t rec_arg_num;
    char cmd_buf[CMD_LEN];
    uint32_t rec_num;


	rec_num=get_true_char_stream(cmd_analyze.processed_buf, cmd_analyze.rec_buf);

	//cy_println ("rec_buf = %s", cmd_analyze.rec_buf);
	//cy_println ("rec_num = %d", rec_num);
	//cy_println ("processed_buf = %s", cmd_analyze.processed_buf);

      /*从接收数据中提取命令*/
      for(i=0;i<CMD_LEN;i++){
          if((i > 0) && ((cmd_analyze.processed_buf[i]==' ') || (cmd_analyze.processed_buf[i]==0x0D)  || (cmd_analyze.processed_buf[i] == 0) )){
               cmd_buf[i]='\0';        //字符串结束符
               break;
          }else{
               cmd_buf[i]=cmd_analyze.processed_buf[i];
          }
      }

      rec_arg_num=cmd_arg_analyze(&cmd_analyze.processed_buf[i], rec_num);

      for(i=0;i<sizeof(cmd_list)/sizeof(cmd_list[0]);i++){
          if(!strcmp(cmd_buf,cmd_list[i].cmd_name)){       //字符串相等

               if(rec_arg_num<0 || rec_arg_num>cmd_list[i].max_args) {
                   cy_println("Too Much arg\n");
               }else {
					clear_ctrlc	();
					cmd_list[i].handle(rec_arg_num,(void *)cmd_analyze.cmd_arg);
					//cy_println ("<End cmd_list[%d].%s>", i, cmd_list[i].cmd_name);
					cy_println ();
               }
               break;
          }

      }
	if(i>=sizeof(cmd_list)/sizeof(cmd_list[0])){
		//cy_println("Unsurport Cmd: %s", cmd_buf);
		//cmd();
		strcpy (cmd_buf, cmd_analyze.rec_buf);
		if (my_run_command (cmd_buf, 0) >= 0){
		}
	}
	cmd ();
	sys_env.uart0_cmd_flag = 0;
}


//接收数据
static uint32_t rec_count=0;
int len;
char str[8];

typedef struct{
	U8 start;
	U8 len;
	uint16_t addr;
	U8 type;
	char data[16];
	U8 sum_check;
}s_hex_file;

int AnalyseHEX(char hex[],int len)
{
	char i=1;
	char data;
	int cc=0;
	char temp[3];
	do{
		temp[0] = hex[i++];
		temp[1] = hex[i++];
		temp[2] = '\0';
		data = simple_strtoul(temp, NULL, 16);
		cc += data;
	} while (i<(len-2));
	cc%=256;
	cc=0x100-cc;
	cc &= 0xFF;
	temp[0] = hex[i++];
	temp[1] = hex[i++];
	data = simple_strtoul(temp, NULL, 16);
	return (cc==data)?0:1;
}

int get_hex_struct (s_hex_file *p_hex, char *_data_buf)
{
	char temp[16];
	int i = 0, j;
	p_hex->start = _data_buf[i++];
	temp[0] = _data_buf[i++];
	temp[1] = _data_buf[i++];
	temp[2] = '\0';
	p_hex->len = simple_strtoul(temp, NULL, 16);
	temp[0] = _data_buf[i++];
	temp[1] = _data_buf[i++];
	temp[2] = _data_buf[i++];
	temp[3] = _data_buf[i++];
	temp[4] = '\0';
	p_hex->addr = simple_strtoul(temp, NULL, 16);
	temp[0] = _data_buf[i++];
	temp[1] = _data_buf[i++];
	temp[2] = '\0';
	p_hex->type = simple_strtoul(temp, NULL, 16);
	if (p_hex->len > 0){
		for (j = 0; j < p_hex->len; j++){
			temp[0] = _data_buf[i++];
			temp[1] = _data_buf[i++];
			temp[2] = '\0';
			p_hex->data[j] = simple_strtoul(temp, NULL, 16);
		}
	}
	temp[0] = _data_buf[i++];
	temp[1] = _data_buf[i++];
	temp[2] = '\0';
	p_hex->sum_check = simple_strtoul(temp, NULL, 16);
	return AnalyseHEX(_data_buf, p_hex->len * 2 + 11);
}
//static uint32_t section_addr = 0, app_size = 0;
int get_hex_data (char * buf)
{
//	int i = 0;
	s_hex_file p_hex;
	uint16_t func_code = 0;
	uint16_t para_value = 0;

		if (get_hex_struct (&p_hex, buf)){//文件错误
			return -1;
		}
		switch (p_hex.type){
			case 0x00://data
//				flash_addr = section_addr + p_hex.addr;
//				for (i = 0; i < p_hex.len; i++){
//					iap_code_buf[flash_addr] = p_hex.data[i];
//					flash_addr++;
//					app_size++;
//				}
				break;
			case 0x01://end
				break;
			case 0x04://ex addr
//				section_addr = (p_hex.data[0]<< 8 | p_hex.data[1]) << 16;
//				section_addr &= 0xFFFFFFF;
//				comscreen(Disp_Indexpic[22],Number_IndexpicB);	 // back to the  picture before alert
				break;
			case 0x08://ex func
				func_code = (p_hex.data[0] << 8 | p_hex.data[1]);
				switch (func_code)
				{
					case 0x000E://复位指令
						LOG ("Reset System...");
						delay_ms (20);
						rSWIRST = 0x533C2416;
						break;
					case 0x0001://刷新数据
						refresh_data ();
						break;
					case 0x0002://启动
						coin_start ();
						break;
					case 0x0003://停止
						coin_stop ();
						break;
					case 0x0004://打印
						coin_print ();
						break;
					case 0x0005://清零
						counter_clear ();
						break;
					case 0x0006://poll
						poll_data ();
						break;
					case 0x0007://导出数据
						export_detect_data ();
						break;
					case 0x0008://导出数据
						export_record();
						break;
					case 0xE001://清除报警
						coin_clear_alarm ();
						break;
					default :break;
				}
				break;
			case 0x09://ex write
				para_value = (p_hex.data[0] << 8 | p_hex.data[1]);
				switch (p_hex.addr)
				{
					case 1:
						para_set_value.data.kick_start_delay_t1 = para_value;
						break;
					case 2:
						para_set_value.data.kick_keep_t1 = para_value;
						break;
					case 3:
						para_set_value.data.kick_start_delay_t2 = para_value;
						break;
					case 4:
						para_set_value.data.kick_keep_t2 = para_value;
						break;
					case 5:
						para_set_value.data.precoin_set_num[0] = para_value;
						break;
					case 6:
						para_set_value.data.precoin_set_num[1] = para_value;
						break;
					case 7:
						para_set_value.data.precoin_set_num[4] = para_value;
						break;
					case 8:
						para_set_value.data.precoin_set_num[6] = para_value;
						break;
					case 9:
						para_set_value.data.precoin_set_num[7] = para_value;
						break;
					case 10:
						para_set_value.data.precoin_set_num[8] = para_value;
						break;
					case 11:
						para_set_value.data.precoin_set_num[9] = para_value;
						break;
					case 12:
						para_set_value.data.precoin_set_num[10] = para_value;
						break;
					case 22:
						para_set_value.data.motor_idle_t = para_value;
						break;
					case 23:
						para_set_value.data.pre_count_stop_n = para_value;
						break;
					case 24:
						para_set_value.data.coin_full_rej_pos = para_value;
						break;
					case 25:
						para_set_value.data.system_boot_delay = para_value;
						break;
					case 26:
						para_set_value.data.system_mode = para_value;
						break;
					case 27:
						para_set_value.data.precoin_set_num[3] = para_value;
						break;
					case 51:
						sys_env.coin_index = para_value;
						break;
					case 52:
						pre_value.country[coinchoose].coin[sys_env.coin_index].data.max0 = para_value;
						break;
					case 53:
						pre_value.country[coinchoose].coin[sys_env.coin_index].data.min0 = para_value;
						break;
					case 54:
						pre_value.country[coinchoose].coin[sys_env.coin_index].data.max1 = para_value;
						break;
					case 55:
						pre_value.country[coinchoose].coin[sys_env.coin_index].data.min1 = para_value;
						break;
					case 56:
						pre_value.country[coinchoose].coin[sys_env.coin_index].data.max2 = para_value;
						break;
					case 57:
						pre_value.country[coinchoose].coin[sys_env.coin_index].data.min2 = para_value;
						break;
					default:
						break;
				}
				write_para (); //保存参数
				ini_screen ();
				refresh_data ();
				break;
			default:break;
		}
	return 0;
}

u8 auchCRCHi[]=
{
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40

};

u8 auchCRCLo[] =
{
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
};

u16 CRC16(char * _data_buf,int len)
{
	u8 uchCRCHi=0xff;
	u8 uchCRCLo=0xff;
	u16 uindex;
	while(len--){
		uindex = uchCRCHi^ *_data_buf++;
		uchCRCHi = uchCRCLo^auchCRCHi[uindex];
		uchCRCLo = auchCRCLo[uindex];
	}
	return (uchCRCHi<<8|uchCRCLo);
}

void update_finish (e_update_flag flag)
{
	u16 crc = 0;
	switch (flag)
	{
		case UART_UPDATE:
			if (rec_count > 1){
				crc = CRC16 (iap_code_buf, rec_count - 2);//CRC 校验
				if (crc == ((iap_code_buf[rec_count - 2] << 8) | iap_code_buf[rec_count - 1])){//最后两个字节是校验码
					cy_println("OK");//校验通过
					cmd();
					run_command ("write flash");
				}else{
					cy_println("ERROR");//校验失败请求重发
					cmd();
					comscreen(Disp_Indexpic[27],Number_IndexpicB);	//触摸屏跳转到提示固件丢失界面
				}
				rec_count = 0;
				sys_env.tty_mode = 0;
				memset (cmd_analyze.rec_buf, 0, sizeof(cmd_analyze.rec_buf));
			}
		break;
		case NET_UPDATE:
			run_command ("write flash");
			break;
		case UART_COMMAND:
			if (rec_count > 1){
				get_hex_data (cmd_analyze.rec_buf);
				rec_count = 0;
				sys_env.tty_mode = 0;
				memset (cmd_analyze.rec_buf, 0, sizeof(cmd_analyze.rec_buf));
			}
			break;
		default:break;
	}
	sys_env.update_flag = NULL_UPDATE;
}
/*提供给串口中断服务程序，保存串口接收到的单个字符*/
void fill_rec_buf(char data)
{
	if ((data == CTRL_C ) && (sys_env.tty_mode == 0))
	{
		cy_println ("Op Cancel!");
		sys_env.sys_break = 1;
		return;
	}
	if (sys_env.uart0_cmd_flag == 0xA5){
		if (0x0D != data){
			return;
		}else{
			sys_env.uart0_cmd_flag = 0;
		}
	}
	if (sys_env.tty_mode == 0x55){ //程序下载
		sys_env.tty_online_ms = TTY_ONLINE_TIME;
		iap_code_buf[rec_count++] = data;
		if (rec_count == 1){
			cy_print ("\n");
		}
		if (rec_count % 1024 == 0){
			cy_print (".");
		}
		if (rec_count % (1024*64) == 0){
			cy_print ("\n");
		}
		if (rec_count >= CODE_BUF_SIZE){
			rec_count=0;
			sys_env.tty_mode = 0;
			cy_println("ERROR");//数组越界，表示接受失败
		}
	}else if (sys_env.tty_mode == 0xaa){
		sys_env.tty_online_ms = TTY_ONLINE_TIME;
		cmd_analyze.rec_buf[rec_count++] = data;
	}else if (sys_env.uart0_cmd_flag == 0){
		if (data == '\b'){
			if (rec_count > 0){
				Uart0_sendchar('\b');
				Uart0_sendchar(' ');
				Uart0_sendchar('\b');
				Uart0_sendchar('\b');
				if (rec_count > 1){
					Uart0_sendchar(cmd_analyze.rec_buf[rec_count - 2]);
				}else{
					Uart0_sendchar(':');
				}
				rec_count--;
				cmd_analyze.rec_buf[rec_count] = ' ';
			}
			return;
		}else if (data == ':'){
			sys_env.tty_mode = 0xaa;
			sys_env.update_flag = UART_COMMAND;
			cmd_analyze.rec_buf[rec_count++] = data;
			sys_env.tty_online_ms = TTY_ONLINE_TIME;
		}else{
			Uart0_sendchar(data);
			if(0x0D == data){// && 0x0D==cmd_analyze.rec_buf[rec_count-1])
				if (rec_count > 0){
					cmd_analyze.rec_buf[rec_count] = '\0';
					//cy_println ("rec_count = %d", rec_count);
					rec_count=0;
				}
				Uart0_sendchar('\n');
				sys_env.uart0_cmd_flag = 1;
				//cy_println ("rec_count = %d", rec_count);
				//vTaskCmdAnalyze ();
			}else{
				cmd_analyze.rec_buf[rec_count] = data;
				rec_count++;
			   /*防御性代码，防止数组越界*/
			   if(rec_count>=CMD_BUF_LEN) {
				   rec_count=0;
			   }
			}
		}
	}else{
		rec_count = 0;
	}
}
void print_ng_data (int16_t index);
/*打印字符串:Hello world!*/
 void do_help(int32_t argc, void *cmd_arg)
 {
	int i;
	cy_println("\n----------------------------------------------------");
	for (i = 0; i < (sizeof (cmd_list) / sizeof (cmd_list_struct)); i++)
	{
		cy_println("--%s			%s", cmd_list[i].cmd_name, cmd_list[i].help);
	}
	cy_println("----------------------------------------------------\n");
 }

void do_go(int32_t argc, void *cmd_arg)
{
   int32_t  *arg=(int32_t *)cmd_arg;

	switch (argc)
	{
		case 0:
			cy_println("There is no arg");
			break;
		case 1:
			if (arg[argc - 1] == string_to_dec((uint8 *)("yccs")))
			{
				cy_println("goto YCCS menu");
				comscreen(Disp_Indexpic[YCCS],Number_IndexpicB);	 // 进入调试界面
			}
			else if (arg[argc - 1] == string_to_dec((uint8 *)("tzcy"))) // 进行特征值采样
			{
				cy_println("goto JZTS menu");
				comscreen(Disp_Indexpic[JZTS],Number_IndexpicB);
				sys_env.workstep =103;
			}
			else if (arg[argc - 1] == string_to_dec((uint8 *)("jzts"))) // 进行基准值调试
			{
				cy_println("goto JZTS menu");
				comscreen(Disp_Indexpic[JZTS],Number_IndexpicB);
				sys_env.workstep =100;
			}
			else if (arg[argc - 1] == string_to_dec((uint8 *)("jsjm"))) // 进入计数界面
			{
				cy_println("goto JSJM menu");
				comscreen(Disp_Indexpic[JSJM],Number_IndexpicB);
				sys_env.workstep =60;
				sys_env.sim_count_flag = 1;
			}
			else if (arg[argc - 1] == string_to_dec((uint8 *)("tzxx"))) // 进入特征学习
			{
				if( adstd_offset() == 1)
				{
					cy_println("goto TZBC menu");
					comscreen(Disp_Indexpic[TZJM],Number_IndexpicB);
					coin_maxvalue0 = 0;
					coin_maxvalue1 = 0;
					coin_maxvalue2 = 0;
					coin_minvalue0 = 1024;
					coin_minvalue1 = 1024;
					coin_minvalue2 = 1024;

					dgus_tf1word(ADDR_A0MA,coin_maxvalue0);	//	 real time ,pre AD0  max
					dgus_tf1word(ADDR_A0MI,coin_minvalue0);	//	 real time ,pre AD0  min
					dgus_tf1word(ADDR_A1MA,coin_maxvalue1);	//	 real time ,pre AD1  max
					dgus_tf1word(ADDR_A1MI,coin_minvalue1);	//	 real time ,pre AD1  min
					dgus_tf1word(ADDR_A2MA,coin_maxvalue2);	//	 real time ,pre AD2  max
					dgus_tf1word(ADDR_A2MI,coin_minvalue2);	//	 real time ,pre AD2  min
					//sys_env.workstep =101;
				}
				else
				{
					cy_println("the voltage is wrong please adjust it!");
					return;
				}
			}
			else if (arg[argc - 1] == string_to_dec((uint8 *)("mnjs"))) // 进行模拟计数
			{
				cy_println("goto JSJM menu");
				comscreen(Disp_Indexpic[JSJM],Number_IndexpicB);
				sys_env.workstep =102;
				sys_env.sim_count_flag = 1;
			}
			else
			{
				cy_println ("Error arg");
			}
			break;
		case 2:
			cy_println("2 arg");
			break;
		case 3:
			cy_println("3 arg");
			break;
		default: break;
	}
}




int32_t get_coin_index (int32_t _coin_name)
{
	if (_coin_name == string_to_dec((uint8 *)("cnnm0"))) //
	{
		return 0;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm1"))) //
	{
		return 1;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm2"))) //
	{
		return 2;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm3"))) //
	{
		return 3;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm4"))) //
	{
		return 4;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm5"))) //
	{
		return 5;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm6"))) //
	{
		return 6;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm7"))) //
	{
		return 7;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm8"))) //
	{
		return 8;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm9"))) //
	{
		return 9;
	}
	else if (_coin_name == string_to_dec((uint8 *)("cnnm10"))) //
	{
		return 10;
	}
	else
	{
		cy_println ("coin_name must be cnnm 0 ... %d", COIN_TYPE_NUM - 1);
		return -1;
	}
}

void coin_start (void)
{
	if (sys_env.system_delay == 0){
		prepic_num =JSJM;
		comscreen(Disp_Indexpic[JSYX],Number_IndexpicB);	 // back to the  picture before alert
		sys_env.workstep = 3;
		dbg("begin working\r\n");
	}
}

void coin_stop (void)
{
	runstep = 40;   //开始停机
	dbg("end working %s %d\n", __FILE__, __LINE__);
}

void coin_clear (void)
{
	counter_clear();
}

void coin_clear_alarm (void)
{
	prepic_num = prepic_prenum;
	comscreen(Disp_Indexpic[prepic_num],Number_IndexpicB);	 // back to the  picture before alert
}

void coin_print (void)
{
	print_func();	 //打印函数
	sys_env.workstep = 0;	//停止	所有动作  // 等待 触摸
}

void refresh_data (void)
{
	pc_print("%d,%d;",1, para_set_value.data.kick_start_delay_t1);
	pc_print("%d,%d;",2, para_set_value.data.kick_keep_t1);
	pc_print("%d,%d;",3, para_set_value.data.kick_start_delay_t2);
	pc_print("%d,%d;",4, para_set_value.data.kick_keep_t1);
	pc_print("%d,%d;",5, *pre_value.country[COUNTRY_ID].coin[0].data.p_pre_count_set);
	pc_print("%d,%d;",6, *pre_value.country[COUNTRY_ID].coin[1].data.p_pre_count_set);
	pc_print("%d,%d;",7, *pre_value.country[COUNTRY_ID].coin[4].data.p_pre_count_set);
	pc_print("%d,%d;",8, *pre_value.country[COUNTRY_ID].coin[6].data.p_pre_count_set);
	pc_print("%d,%d;",9, *pre_value.country[COUNTRY_ID].coin[7].data.p_pre_count_set);
	pc_print("%d,%d;",10, *pre_value.country[COUNTRY_ID].coin[8].data.p_pre_count_set);
	pc_print("%d,%d;",11, *pre_value.country[COUNTRY_ID].coin[9].data.p_pre_count_set);
	pc_print("%d,%d;",12, *pre_value.country[COUNTRY_ID].coin[10].data.p_pre_count_set);
	pc_print("%d,%d;",22, para_set_value.data.motor_idle_t);
	pc_print("%d,%d;",23, para_set_value.data.pre_count_stop_n);
	pc_print("%d,%d;",24, para_set_value.data.coin_full_rej_pos);
	pc_print("%d,%d;",25, para_set_value.data.system_boot_delay);
	pc_print("%d,%d;",26, para_set_value.data.system_mode);
	pc_print("%d,%d;",27, *pre_value.country[COUNTRY_ID].coin[3].data.p_pre_count_set);
	pc_print("%d,%d;",51, sys_env.coin_index);
	pc_print("%d,%d;",52, pre_value.country[coinchoose].coin[sys_env.coin_index].data.max0);
	pc_print("%d,%d;",53, pre_value.country[coinchoose].coin[sys_env.coin_index].data.min0);
	pc_print("%d,%d;",54, pre_value.country[coinchoose].coin[sys_env.coin_index].data.max1);
	pc_print("%d,%d;",55, pre_value.country[coinchoose].coin[sys_env.coin_index].data.min1);
	pc_print("%d,%d;",56, pre_value.country[coinchoose].coin[sys_env.coin_index].data.max2);
	pc_print("%d,%d;",57, pre_value.country[coinchoose].coin[sys_env.coin_index].data.min2);
	disp_allcount_to_pc ();
}

void poll_data (void)
{
	disp_allcount_to_pc ();
}

void set_para_1  (int32_t arg[])
{
	if (arg[0] == string_to_dec((uint8 *)("save")))
	{
		if (is_repeate (sys_env.coin_index))
		{
			cy_println ("**********************************************************");
			cy_println ("**The Value May Be not Correct so Data Will not be Saved**");
			cy_println ("**********************************************************");
			return ;
		}
		if (coinlearnnumber == 0)
		{
			dbg ("coinlearnnumber = %d, No data to save arg", coinlearnnumber);
			return;
		}
	}else if (arg[0] == string_to_dec((uint8 *)("clear"))){ //force to save
		coin_clear ();
		return;
	}else if (arg[0] == string_to_dec((uint8 *)("clear-alram"))){ //force to save
		coin_clear_alarm ();
		return;
	}else if (arg[0] == string_to_dec((uint8 *)("start"))){ //force to save
		coin_start ();
		return;
	}else if (arg[0] == string_to_dec((uint8 *)("stop"))){ //force to save
		coin_stop ();
		return;
	}else if (arg[0] == string_to_dec((uint8 *)("print"))){ //force to save
		coin_print ();
		return;
	}else if (arg[0] == string_to_dec((uint8 *)("refresh"))){ //force to save
		refresh_data ();
		return;
	}else if (arg[0] == string_to_dec((uint8 *)("save-f"))){ //force to save
	}else{
		cy_println ("set_para_1 arg[0] Error arg");
		return;
	}
	pre_value.country[coinchoose].coin[sys_env.coin_index].data.max0 = coin_maxvalue0;
	pre_value.country[coinchoose].coin[sys_env.coin_index].data.min0 = coin_minvalue0;
	pre_value.country[coinchoose].coin[sys_env.coin_index].data.max1 = coin_maxvalue1;
	pre_value.country[coinchoose].coin[sys_env.coin_index].data.min1 = coin_minvalue1;
	pre_value.country[coinchoose].coin[sys_env.coin_index].data.max2 = coin_maxvalue2;
	pre_value.country[coinchoose].coin[sys_env.coin_index].data.min2 = coin_minvalue2;
	pre_value.country[coinchoose].coin[sys_env.coin_index].data.std0 = std_ad0;
	pre_value.country[coinchoose].coin[sys_env.coin_index].data.std1 = std_ad1;
	pre_value.country[coinchoose].coin[sys_env.coin_index].data.std2 = std_ad2;
	write_para ();
	dbg ("Total learned coin number is %d", coinlearnnumber);
	coinlearnnumber = 0;
	prepic_num = TZJM;
	comscreen(Disp_Indexpic[prepic_num],Number_IndexpicB); //显示
	dbg ("Save Complete");
}


void set_para_2  (int32_t arg[])
{
	if (arg[0] == string_to_dec((uint8 *)("print"))){ //打印开关
		if (arg[1] > 0){
			cy_println("turn on print wave to pc");
			sys_env.print_wave_to_pc = 1;
		}else{
			cy_println("turn off print wave to pc");
			sys_env.print_wave_to_pc = 0;
		}
	}else if (arg[0] == string_to_dec((uint8 *)("autostop"))){ // 异常自动停止
		if (arg[1] > 0) {
			cy_println("turn on auto_stop");
			sys_env.auto_stop = 1;
		}else{
			cy_println("turn off auto_stop");
			sys_env.auto_stop = 0;
		}
	}else if (arg[0] == string_to_dec((uint8 *)("autoclear"))){ // 异常自动停止
		if (arg[1] > 0){
			cy_println("turn on auto_clear");
			sys_env.auto_clear = 1;
		}else{
			cy_println("turn off auto_clear");
			sys_env.auto_clear = 0;
		}
	}
	else if (arg[0] == string_to_dec((uint8 *)("save_ng"))){
		if (arg[1] > 0){
			cy_println("turn on save_ng_data");
			sys_env.save_ng_data = 1;
		}else{
			cy_println("turn off save_ng_data");
			sys_env.save_ng_data = 0;
		}
	}
	else if (arg[0] == string_to_dec((uint8 *)("save_gd"))){
		if (arg[1] > 0){
			cy_println("turn on save_good_data");
			sys_env.save_good_data = 1;
		}else{
			cy_println("turn off save_good_data");
			sys_env.save_good_data = 0;
		}
	}
	else if (arg[0] == string_to_dec((uint8 *)("coin_index"))){
		if (arg[1] >= 0 && arg[1]  < COIN_TYPE_NUM){
			cy_println("set coin_index = %d", arg[1]);
			sys_env.coin_index = arg[1] ;
			dgus_tf1word(ADDR_CNTB,sys_env.coin_index);	//initial addr on zixuexi jiemian coin name tubiao
		}else{
			cy_println("coin_index must > 0 and < %d", COIN_TYPE_NUM);
			return;
		}
	}
	else if (arg[0] == string_to_dec((uint8 *)("country_index"))){
		if (arg[1] >= 0 && arg[1]  < COINCNUM){
			cy_println("set country_index = %d", arg[1]);
			sys_env.country_index = arg[1] ;
			coinchoose = sys_env.country_index;
		}else{
			cy_println("coin_index must > 0 and < %d", COINCNUM);
			return;
		}
	}else if (arg[0] == string_to_dec((uint8 *)("stop"))){
		cy_println("set sys_env.workstep = %d", arg[1]);
		sys_env.workstep = arg[1];
	}else if (arg[0] == string_to_dec((uint8 *)("run"))){
		cy_println("set sys_env.workstep = %d", arg[1]);
		sys_env.workstep = arg[1];
	}else if (arg[0] == string_to_dec((uint8 *)("kick-start1"))){
		cy_println("set kick_start_delay_time1 = %d", arg[1]);
		para_set_value.data.kick_start_delay_t1 = arg[1];
		write_para ();
	}else if (arg[0] == string_to_dec((uint8 *)("kick-keep1"))){
		cy_println("set kick_keep_time1 = %d", arg[1]);
		para_set_value.data.kick_keep_t1 = arg[1];
		write_para ();
	}else if (arg[0] == string_to_dec((uint8 *)("boot-delay"))){
		cy_println("set system_boot_delay = %d", arg[1]);
		para_set_value.data.system_boot_delay = arg[1];
		write_para ();
	}else if (arg[0] == string_to_dec((uint8 *)("kick-start2"))){
		cy_println("set kick_start_delay_time2 = %d", arg[1]);
		para_set_value.data.kick_start_delay_t2 = arg[1];
		write_para ();
	}else if (arg[0] == string_to_dec((uint8 *)("kick-keep2"))){
		cy_println("set kick_keep_time2 = %d", arg[1]);
		para_set_value.data.kick_keep_t2 = arg[1];
		write_para ();
	}else if (arg[0] == string_to_dec((uint8 *)("coin-size"))){
		cy_println("set coin_size = %d", arg[1]);
		para_set_value.data.coin_size = arg[1];
		write_para ();
	}else if (arg[0] == string_to_dec((uint8 *)("coin-push-size"))){
		cy_println("set coin-push-size = %d", arg[1]);
		para_set_value.data.coin_push_size = arg[1];
		write_para ();
	}else if (arg[0] == string_to_dec((uint8 *)("coin-h"))){
		cy_println("set coin_h = %d", arg[1]);
		para_set_value.data.coin_h = arg[1];
		write_para ();
	}else if (arg[0] == string_to_dec((uint8 *)("motor-dir"))){
		cy_println("set motor-dir = %d", arg[1]);
		if (arg[1] == 1){
			STORAGE_DIR_N();
		}else if (arg[1] == 0){
			STORAGE_DIR_P();
		}
	}else if (arg[0] == string_to_dec((uint8 *)("adj-offset"))){
		cy_println("set adj_offset_position = %d", arg[1]);
		if (arg[1] < 6000){
			arg[1] = 6000;
		}
		para_set_value.data.adj_offset_position = arg[1];
		write_para ();
	}else{
		cy_println ("set_para_2 arg[0] Error arg");
		return;
	}
	print_system_env_info ();
}



void write_to_flash (int32_t _country_index, int32_t _coin_index)
{
	write_para ();
	cy_println ("write_to_flash _country_index = %d _coin_index = %d", _country_index, _coin_index);
	disp_preselflearn(pre_value.country[coinchoose].coin[_coin_index].data.max0,pre_value.country[coinchoose].coin[_coin_index].data.min0,
					  pre_value.country[coinchoose].coin[_coin_index].data.max1,pre_value.country[coinchoose].coin[_coin_index].data.min1,
					  pre_value.country[coinchoose].coin[_coin_index].data.max2,pre_value.country[coinchoose].coin[_coin_index].data.min2);
	if (_country_index == coinchoose)
	{
		dgus_tf1word(ADDR_CNTB, _coin_index);	//initial addr on zixuexi jiemian coin name tubiao
		comscreen(Disp_Indexpic[TZJM],Number_IndexpicB);	 // back to the  picture before alert
	}
	else
	{
		cy_println ("Note !!! Current coinchoose is %d, and set coinchoose is %d", coinchoose, _country_index);
	}
}

void save_prevalue_coin (int32_t _country_index, int32_t _coin_index, int32_t _value_index, int32_t _value)
{
	//nick add begin********************************************************************
	//read_coin_value (_coin_index);
	switch (_value_index)
	{
		case 0 :pre_value.country[coinchoose].coin[_coin_index].data.max0 = _value; break;
		case 1 :pre_value.country[coinchoose].coin[_coin_index].data.min0 = _value; break;
		case 2 :pre_value.country[coinchoose].coin[_coin_index].data.max1 = _value; break;
		case 3 :pre_value.country[coinchoose].coin[_coin_index].data.min1 = _value; break;
		case 4 :pre_value.country[coinchoose].coin[_coin_index].data.max2 = _value; break;
		case 5 :pre_value.country[coinchoose].coin[_coin_index].data.min2 = _value; break;
		case 6 :pre_value.country[coinchoose].coin[_coin_index].data.std0 = _value; break;
		case 8 :pre_value.country[coinchoose].coin[_coin_index].data.std1 = _value; break;
		case 10:pre_value.country[coinchoose].coin[_coin_index].data.std2 = _value; break;
		case 12:pre_value.country[coinchoose].coin[_coin_index].data.offsetmax0 = _value; break;
		case 13:pre_value.country[coinchoose].coin[_coin_index].data.offsetmin0 = _value; break;
		case 14:pre_value.country[coinchoose].coin[_coin_index].data.offsetmax1 = _value; break;
		case 15:pre_value.country[coinchoose].coin[_coin_index].data.offsetmin1 = _value; break;
		case 16:pre_value.country[coinchoose].coin[_coin_index].data.offsetmax2 = _value; break;
		case 17:pre_value.country[coinchoose].coin[_coin_index].data.offsetmin2 = _value; break;
		default:return;
	}
	write_para ();
	//nick add end**************************************************************---------

	disp_preselflearn(pre_value.country[coinchoose].coin[_coin_index].data.max0,pre_value.country[coinchoose].coin[_coin_index].data.min0,
					  pre_value.country[coinchoose].coin[_coin_index].data.max1,pre_value.country[coinchoose].coin[_coin_index].data.min1,
					  pre_value.country[coinchoose].coin[_coin_index].data.max2,pre_value.country[coinchoose].coin[_coin_index].data.min2);
	if (_country_index == coinchoose)
		comscreen(Disp_Indexpic[TZJM],Number_IndexpicB);	 // back to the  picture before alert
	else
		cy_println ("Note !!! Current coinchoose is %d, and set coinchoose is %d", coinchoose, _country_index);

}


void save_all_prevalue_coin (int32_t _country_index, int32_t _coin_index, int32_t _value_buf[])
{

	//cy_println ("_country_index = %d _coin_index = %d _value_buf[0] = %d", _country_index, _coin_index,  _value_buf[0]);
	pre_value.country[coinchoose].coin[_coin_index].data.max0 = _value_buf[0];
	pre_value.country[coinchoose].coin[_coin_index].data.min0 = _value_buf[1];
	pre_value.country[coinchoose].coin[_coin_index].data.max1 = _value_buf[2];
	pre_value.country[coinchoose].coin[_coin_index].data.min1 = _value_buf[3];
	pre_value.country[coinchoose].coin[_coin_index].data.max2 = _value_buf[4];
	pre_value.country[coinchoose].coin[_coin_index].data.min2 = _value_buf[5];

	write_to_flash (_country_index, _coin_index);
}
void save_all_offsetvalue_coin (int32_t _country_index, int32_t _coin_index, int32_t _value_buf[])
{

	//cy_println ("_country_index = %d _coin_index = %d _value_buf[0] = %d", _country_index, _coin_index,  _value_buf[0]);
	pre_value.country[coinchoose].coin[_coin_index].data.offsetmax0 = _value_buf[0];
	pre_value.country[coinchoose].coin[_coin_index].data.offsetmin0 = _value_buf[1];
	pre_value.country[coinchoose].coin[_coin_index].data.offsetmax1 = _value_buf[2];
	pre_value.country[coinchoose].coin[_coin_index].data.offsetmin1 = _value_buf[3];
	pre_value.country[coinchoose].coin[_coin_index].data.offsetmax2 = _value_buf[4];
	pre_value.country[coinchoose].coin[_coin_index].data.offsetmin2 = _value_buf[5];

	write_to_flash (_country_index, _coin_index);
}
void save_all_stdvalue_coin (int32_t _country_index, int32_t _coin_index, int32_t _value_buf[])
{
	//cy_println ("_country_index = %d _coin_index = %d _value_buf[0] = %d", _country_index, _coin_index,  _value_buf[0]);
	pre_value.country[coinchoose].coin[_coin_index].data.std0 = _value_buf[0];
	pre_value.country[coinchoose].coin[_coin_index].data.std1 = _value_buf[1];
	pre_value.country[coinchoose].coin[_coin_index].data.std2 = _value_buf[2];

	write_to_flash (_country_index, _coin_index);
}

int32_t run_step_motor (int32_t arg[])
{
	static int  motor_dir = 0, motor_step = 0;
	int repeate;
	int32_t motor_id, step, motor_speed;
	motor_id = arg[0];
	step = arg[1];
	motor_speed = arg[2];
	repeate = arg[3];
	do{
		if (step < 0){
			motor_dir = 0;
			motor_step = step*(-1);
		}else if(step > 0){
			motor_dir = 1;
			motor_step = step;
		}else{
			motor_dir ^= 1;
		}
		cy_println("run motor:[id = %d, step = %d, pwm_width = %d, dir = %d]", motor_id, motor_step, motor_speed, motor_dir);
		step = 0;
		set_motor_dir (motor_id, motor_dir);
		if (start_motor (motor_id, motor_step, motor_speed) < 0){
			return -1;
		}
		if (had_ctrlc ()){
			return -1;
		}
	}while (repeate == 1);

	return 0;
}

void set_para_3  (int32_t arg[])
{
	#define VALUE_SIZE 6 + 3 + 6
	int32_t coin_index_temp = 0;
	int32_t value_temp_buf[VALUE_SIZE];
	int i;

	coin_index_temp = get_coin_index (arg[0]);
	if (coin_index_temp < 0)
		return;

	if (arg[1] == string_to_dec((uint8 *)("all"))) //
	{
		for (i = 0; i < VALUE_SIZE; i++)
		{
			value_temp_buf[i] = arg[2];
		}
		save_all_prevalue_coin (coinchoose, coin_index_temp, value_temp_buf);
		cy_println ("save all prevalue_coin op successfull");
		save_all_stdvalue_coin (coinchoose, coin_index_temp, &value_temp_buf[6]);
		cy_println ("save all stdvalue_coin op successfull");
		save_all_offsetvalue_coin (coinchoose, coin_index_temp, &value_temp_buf[9]);
		cy_println ("save all offsetvalue_coin op successfull");
	}
	else if (arg[1] == string_to_dec((uint8 *)("pre"))) //
	{
		for (i = 0; i < 6; i++)
		{
			value_temp_buf[i] = arg[2];
		}
		save_all_prevalue_coin (coinchoose, coin_index_temp, value_temp_buf);
		cy_println ("save all prevalue_coin op successfull");
	}
	else if (arg[1] == string_to_dec((uint8 *)("std"))) //
	{
		for (i = 0; i < 3; i++)
		{
			value_temp_buf[i] = arg[2];
		}
		save_all_stdvalue_coin (coinchoose, coin_index_temp, value_temp_buf);
		cy_println ("save all stdvalue_coin op successfull");
	}
	else if (arg[1] == string_to_dec((uint8 *)("offset"))) //
	{
		for (i = 0; i < 6; i++)
		{
			if (i % 2 == 0)
				value_temp_buf[i] = arg[2];
			else
				value_temp_buf[i] = -1 * arg[2];
		}
		save_all_offsetvalue_coin (coinchoose, coin_index_temp, value_temp_buf);
		cy_println ("save all offsetvalue_coin op successfull");
	}
	else
	{
		cy_println ("set_para_3 Error arg");
		return;
	}
	print_ng_data (coin_index_temp);
}
void set_para_4 (int32_t arg[])
{
	int32_t coin_index_temp = 0;
	int32_t value_index_temp = 0;
	int32_t value_temp = arg[3];

	coin_index_temp = get_coin_index (arg[0]);
	if (coin_index_temp < 0)
		return;

	if (arg[1]  == string_to_dec((uint8 *)("max")))
	{
		value_index_temp = 0;
	}
	else if (arg[1]  == string_to_dec((uint8 *)("min")))
	{
		value_index_temp = 1;
	}
	else if (arg[1]  == string_to_dec((uint8 *)("std")))
	{
		value_index_temp = 6;
	}
	else
	{
		cy_println ("value index must be max or min or std");
		return;
	}

	if (arg[2] >= 0 && arg[2] < 3)
	{
		value_index_temp += arg[2] * 2;
	}
	else
	{
		cy_println ("chanel value must be 1 or 2 or 3");
		return;
	}

	save_prevalue_coin (coinchoose, coin_index_temp, value_index_temp, value_temp);
	cy_println ("save op successfull");
	print_ng_data (coin_index_temp);
}

void set_para_5 (int32_t arg[])
{
	int32_t coin_index_temp = 0;
	int32_t value_index_temp = 0;
	int32_t value_temp;

	coin_index_temp = get_coin_index (arg[0]);
	if (coin_index_temp < 0)
		return;

	if (arg[1]  == string_to_dec((uint8 *)("std")))
	{
		cy_println ("save stdvalue_coin %4d %4d %4d", arg[2], arg[3], arg[4]);
		save_all_stdvalue_coin (coinchoose, coin_index_temp, &arg[2]);
		cy_println ("save stdvalue_coin op successfull");
	}
	else if (arg[1]  == string_to_dec((uint8 *)("offset")))
	{

		if (arg[2]  == string_to_dec((uint8 *)("max")))
		{
			value_index_temp = 12;
		}
		else if (arg[2]  == string_to_dec((uint8 *)("min")))
		{
			value_index_temp = 13;
		}
		else
		{
			cy_println ("arg[2] value index must be max or min");
			return;
		}

		if (arg[3] >= 0 && arg[3] < 3)
		{
			value_index_temp += arg[3] * 2;
		}
		else
		{
			cy_println ("arg[3] chanel value must be 1 or 2 or 3");
			return;
		}
		value_temp = arg[4];
		save_prevalue_coin (coinchoose, coin_index_temp, value_index_temp, value_temp);
		cy_println ("save offset value op successfull");
	}
	else
	{
		cy_println ("arg[1] value index must be std or offset");
		return;
	}

	print_ng_data (coin_index_temp);
}


void set_para_7 (int32_t arg[])
{
	int32_t coin_index_temp = 0;
	coin_index_temp = get_coin_index (arg[0]);
	if (coin_index_temp < 0)
		return;

	coin_maxvalue0 = arg[1];
	coin_minvalue0 = arg[2];
	coin_maxvalue1 = arg[3];
	coin_minvalue1 = arg[4];
	coin_maxvalue2 = arg[5];
	coin_minvalue2 = arg[6];
	if (is_repeate (coin_index_temp))
	{
		return;
	}
	save_all_prevalue_coin (coinchoose, coin_index_temp, &arg[1]);
	cy_println ("save all pre op successfull");
	print_ng_data (coin_index_temp);
}
void set_para_8 (int32_t arg[])
{
	int32_t coin_index_temp = 0;
	coin_index_temp = get_coin_index (arg[0]);
	if (coin_index_temp < 0)
		return;
	if (arg[1]  == string_to_dec((uint8 *)("offset")))
	{
	}
	else
	{
		cy_println ("arg[1] value index must be offset");
		return;
	}
	save_all_offsetvalue_coin (coinchoose, coin_index_temp, &arg[2]);
	cy_println ("save all offset op successfull");
	print_ng_data (coin_index_temp);
}

void do_set(int32_t argc, void *cmd_arg)
{
	switch (argc)
	{
		case 1:
			set_para_1 ((int32_t *)cmd_arg);
			break;
		case 2:
			set_para_2 ((int32_t *)cmd_arg);
			break;
		case 3:
			set_para_3 ((int32_t *)cmd_arg);
			break;
		case 4:
			set_para_4 ((int32_t *)cmd_arg);
			break;
		case 5:
			set_para_5 ((int32_t *)cmd_arg);
			break;
		case 7:
			set_para_7 ((int32_t *)cmd_arg);
			break;
		case 8:
			set_para_8 ((int32_t *)cmd_arg);
			break;
		default:
			cy_println("%d arg", argc); break;
	}
}

void print_system_env_info (void)
{
	cy_println("\n----------------------------------------------------");
	PRINT_VERSION();
	cy_println("----------------print system env info---------------");
	cy_println ("print_wave_to_pc       = %d", sys_env.print_wave_to_pc);
	//cy_println ("auto_stop            = %d", sys_env.auto_stop);
	cy_println ("auto_clear             = %d", sys_env.auto_clear);
	cy_println ("save_ng_data           = %d", sys_env.save_ng_data);
	cy_println ("save_good_data         = %d", sys_env.save_good_data);
	cy_println ("coin_index             = %d", sys_env.coin_index);
	cy_println ("country_index          = %d", sys_env.country_index);
	cy_println ("workstep               = %d", sys_env.workstep);
	cy_println ("system_boot_delay      = %d", para_set_value.data.system_boot_delay);
	cy_println ("system_mode            = %d", para_set_value.data.system_mode);
	cy_println ("coin_cross_time        = %d", sys_env.coin_cross_time);
	cy_println("----------------------------------------------------");
	cy_println ("kick_start_delay_time1 = %d", para_set_value.data.kick_start_delay_t1);
	cy_println ("kick_keep_time1        = %d", para_set_value.data.kick_keep_t1);
	cy_println("----------------------------------------------------");
	cy_println ("kick_start_delay_time2 = %d", para_set_value.data.kick_start_delay_t2);
	cy_println ("kick_keep_time2        = %d", para_set_value.data.kick_keep_t2);
	cy_println("----------------------------------------------------");
	cy_println ("pre_count_stop_n       = %d", para_set_value.data.pre_count_stop_n);
	cy_println ("coin_full_rej_pos      = %d", para_set_value.data.coin_full_rej_pos);
	cy_println("----------------------------------------------------");
	cy_println ("motor_idle_t           = %d", para_set_value.data.motor_idle_t);
	cy_println ("coin_size              = %d", para_set_value.data.coin_size);
	cy_println ("coin_push_size         = %d", para_set_value.data.coin_push_size);
	cy_println ("coin_h                 = %d", para_set_value.data.coin_h);
	cy_println("----------------------------------------------------");
}

void print_coin_env_info (void)
{
	cy_println("-----------------print coin env info----------------");
	cy_println ("ad0_averaged_value    = %d", coin_env.ad0_averaged_value / ADSAMPNUM0);
	cy_println ("ad1_averaged_value    = %d", coin_env.ad1_averaged_value / ADSAMPNUM0);
	cy_println ("ad2_averaged_value    = %d", coin_env.ad2_averaged_value / ADSAMPNUM0);
	cy_println ("std_down_value0       = %d", coin_env.std_down_value0);
	cy_println ("std_up_value0         = %d", coin_env.std_up_value0);
	cy_println ("ad0_step              = %d", coin_env.ad0_step);
	cy_println ("ad1_step              = %d", coin_env.ad1_step);
	cy_println ("ad2_step              = %d", coin_env.ad2_step);
	cy_println("----------------------------------------------------");
	cy_println ("runstep               = %d", runstep);
	cy_println ("coin_env.kick_Q_index = %d", coin_env.kick_Q_index);
	cy_println ("ccstep                = %d", ccstep);
	cy_println("----------------------------------------------------");
}



void print_cmp_data (int16_t _coin_index)
{
	cy_println ("ad std offset complete");
	cy_println ("----------------------------------------------------------------------");
	cy_println ("----------print country %d coin %2d cmp data----------------------------", sys_env.country_index, _coin_index);
	cy_println ("----------------------------------------------------------------------");
//	if (adstd_offset () != 1){
//		cy_println("ad std offset error");
//		return;
//	}
	cy_println ("real     std0 = %4d          std1 = %4d     std2 = %4d", std_ad0, std_ad1, std_ad2);
	cy_println ("----------------------------------------------------------------------");
	cy_println ("save     std0 = %4d          std1 = %4d     std2 = %4d",
			pre_value.country[coinchoose].coin[_coin_index].data.std0,
			pre_value.country[coinchoose].coin[_coin_index].data.std1,
			pre_value.country[coinchoose].coin[_coin_index].data.std2);

	cy_println ("offset   max0 = %4d          max1 = %4d     max2 = %4d",
			pre_value.country[coinchoose].coin[_coin_index].data.offsetmax0,
			pre_value.country[coinchoose].coin[_coin_index].data.offsetmax1,
			pre_value.country[coinchoose].coin[_coin_index].data.offsetmax2);
	cy_println ("original max0 = %4d          max1 = %4d     max2 = %4d",
			pre_value.country[coinchoose].coin[_coin_index].data.max0,
			pre_value.country[coinchoose].coin[_coin_index].data.max1,
			pre_value.country[coinchoose].coin[_coin_index].data.max2);
	cy_println ("offset   min0 = %4d          min1 = %4d     min2 = %4d",
			pre_value.country[coinchoose].coin[_coin_index].data.offsetmin0,
			pre_value.country[coinchoose].coin[_coin_index].data.offsetmin1,
			pre_value.country[coinchoose].coin[_coin_index].data.offsetmin2);
	cy_println ("original min0 = %4d          min1 = %4d     min2 = %4d",
			pre_value.country[coinchoose].coin[_coin_index].data.min0,
			pre_value.country[coinchoose].coin[_coin_index].data.min1,
			pre_value.country[coinchoose].coin[_coin_index].data.min2);
	cy_println("----------------------------------------------------------------------");
#ifdef SAMPLE_METHOD_0
	cy_println ("                  H                     M-H                    L-M");
#endif
#ifdef SAMPLE_METHOD_1
	cy_println ("                  H                      M                      L ");
#endif
	cy_println ("compare_max0  = %4d    compare_max1 = %4d    compare_max2 = %4d",
				coin_cmp_value[_coin_index].compare_max0,
				coin_cmp_value[_coin_index].compare_max1,
				coin_cmp_value[_coin_index].compare_max2);
	cy_println ("compare_min0  = %4d    compare_min1 = %4d    compare_min2 = %4d",
				coin_cmp_value[_coin_index].compare_min0,
				coin_cmp_value[_coin_index].compare_min1,
				coin_cmp_value[_coin_index].compare_min2);
	cy_println("----------------------------------------------------------------------");
}

void print_ng_data (int16_t index)
{
	int	i;
	if (ng_info_echo_off)
		return;
	print_cmp_data (index);
	cy_println("-------------------print %6d ng data-------------------------------", ng_value_index);
	//cy_println ("   index   ad_index    coin_value0   coin_value1   coin_value2");
	cy_println ("   index   ad_index         H             M             L");
	for (i = 0; i < ng_value_index; i++)
	{
		cy_println ("%d   %4d      %4d        %4d          %4d          %4d", NG_value_buf[i].use_index, i + 1, NG_value_buf[i].ad_index, NG_value_buf[i].AD0, NG_value_buf[i].AD1, NG_value_buf[i].AD2);
	}
	cy_println("----------------------------------------------------------------------");
}

void print_good_data (int16_t index)
{
	int	i;

	print_cmp_data (index);
	cy_println("-------------------print %6d good data-----------------------------", good_value_index);
	//cy_println ("   index   ad_index    coin_value0   coin_value1   coin_value2");
	cy_println ("   index   ad_index         H             M             L");
	for (i = 0; i < good_value_index; i++)
	{
		cy_println ("%d   %4d      %4d        %4d          %4d          %4d", GOOD_value_buf[i].use_index, i + 1, GOOD_value_buf[i].ad_index, GOOD_value_buf[i].AD0, GOOD_value_buf[i].AD1, GOOD_value_buf[i].AD2);
	}
	cy_println("---------------------------------------------------------------------");
}


void print_coin_pre_value (int16_t index)
{
	int i = index;
	cy_println("--------------------print country %2d coin value---------------------", coinchoose);
	cy_println (" coin  min0        max0        min1        max1        min2        max2 ");
	cy_println ("   %2d (%4d %3d)  (%4d %3d)  (%4d %3d)  (%4d %3d)  (%4d %3d)  (%4d %3d)",
								i,
								pre_value.country[coinchoose].coin[i].data.min0,
								pre_value.country[coinchoose].coin[i].data.offsetmin0,
								pre_value.country[coinchoose].coin[i].data.max0,
								pre_value.country[coinchoose].coin[i].data.offsetmax0,
								pre_value.country[coinchoose].coin[i].data.min1,
								pre_value.country[coinchoose].coin[i].data.offsetmin1,
								pre_value.country[coinchoose].coin[i].data.max1,
								pre_value.country[coinchoose].coin[i].data.offsetmax1,
								pre_value.country[coinchoose].coin[i].data.min2,
								pre_value.country[coinchoose].coin[i].data.offsetmin2,
								pre_value.country[coinchoose].coin[i].data.max2,
								pre_value.country[coinchoose].coin[i].data.offsetmax2
								);
	cy_println("---------------------------------------------------------------------");
}

void print_all_coin_pre_value (void)
{
	int16_t i;
	cy_println("--------------------print country %2d coin value---------------------", coinchoose);
	cy_println (" coin  min0        max0        min1        max1        min2        max2 ");
	for (i = 0; i < COIN_TYPE_NUM; i++)
	{
		cy_println ("   %2d (%4d %3d)  (%4d %3d)  (%4d %3d)  (%4d %3d)  (%4d %3d)  (%4d %3d)",
								i,
								pre_value.country[coinchoose].coin[i].data.min0,
								pre_value.country[coinchoose].coin[i].data.offsetmin0,
								pre_value.country[coinchoose].coin[i].data.max0,
								pre_value.country[coinchoose].coin[i].data.offsetmax0,
								pre_value.country[coinchoose].coin[i].data.min1,
								pre_value.country[coinchoose].coin[i].data.offsetmin1,
								pre_value.country[coinchoose].coin[i].data.max1,
								pre_value.country[coinchoose].coin[i].data.offsetmax1,
								pre_value.country[coinchoose].coin[i].data.min2,
								pre_value.country[coinchoose].coin[i].data.offsetmin2,
								pre_value.country[coinchoose].coin[i].data.max2,
								pre_value.country[coinchoose].coin[i].data.offsetmax2
								);
	}
	cy_println("---------------------------------------------------------------------");
}

void print_all_country_pre_value (void)
{
}

void print_coin_pre_value_info (int32_t arg[])
{
	if (arg[1]  == string_to_dec((uint8 *)("all")))
	{
		print_all_coin_pre_value ();
	}
	else if (arg[1] >= 0 && arg[1] < COIN_TYPE_NUM)
	{
		print_coin_pre_value (arg[1]);
	}
	else
	{
		cy_println ("arg Error!");
		return;
	}
}

void print_pre_count_set_value (void)
{
	int16_t i;
	cy_println("----------------------print pre_count_set value----------------------", coinchoose);
	for (i = 0; i < 9; i++)
	{
		cy_println ("   %d      %6d", i, *(pre_value.country[coinchoose].coin[i].data.p_pre_count_set));
	}
	cy_println("---------------------------------------------------------------------");
}
void print_pre_count_current (void)
{
	int16_t i;
	cy_println("--------------------print pre_count_current value--------------------", coinchoose);
	for (i = 0; i < 9; i++)
	{
		cy_println ("   %d      %6d", i, *(pre_value.country[coinchoose].coin[i].data.p_pre_count_cur));
	}
	cy_println("---------------------------------------------------------------------");
}
void print_speed (void)
{
	if (sys_env.sys_runing_time_total > 0){
		cy_print ("\n----------------------------------------------------------------------\n");
		cy_print("   详细信息:  \n");
		cy_print("   总数:     %d + %d = %d 枚\n",processed_coin_info.total_good, processed_coin_info.total_ng, processed_coin_info.total_coin);
		cy_print("   异币:     %d 枚\n",processed_coin_info.total_ng);
		cy_print("   金额:     %d.%d%d 元\n",(processed_coin_info.total_money/100),((processed_coin_info.total_money%100)/10),((processed_coin_info.total_money%100)%10));
		cy_print("   本次清分耗时: %d Sec 速度: %d 枚/ Min\n", (sys_env.sys_runing_time_total / 10000), sys_env.coin_speed);
	}
}

void print_input_status (void)
{
	while (1){
		cy_print ("IN0:%d ", A0IN0);
		cy_print ("IN1:%d ", A0IN1);
		cy_print ("IN2:%d ", A0IN2);
		cy_print ("IN3:%d ", A0IN3);
		cy_print ("IN4:%d ", A0IN4);
		cy_print ("IN5:%d ", A0IN5);
		cy_print ("IN6:%d ", A0IN6);
		cy_print ("IN7:%d ", A0IN7);
		cy_println ();
		OSTimeDly(500); // LED3 1000ms闪烁void
		if (had_ctrlc ()){
			return;
		}
	}
}

void do_print(int32_t argc, void *cmd_arg)
{
	int32_t  *arg=(int32_t *)cmd_arg;

	switch (argc)
	{
		case 1:
			if (arg[argc - 1] == string_to_dec((uint8 *)("ng"))){
				print_ng_data (sys_env.coin_index);
			}else if (arg[argc - 1] == string_to_dec((uint8 *)("input"))){ // 进行特征值采样
				print_input_status ();
			}else if (arg[argc - 1] == string_to_dec((uint8 *)("gd"))){ // 进行特征值采样
				print_good_data (sys_env.coin_index);
			}else if (arg[argc - 1] == string_to_dec((uint8 *)("env-s"))){ // 进行特征值采样
				print_system_env_info();
			}else if (arg[argc - 1] == string_to_dec((uint8 *)("speed"))){
				print_speed ();
			}else if (arg[argc - 1] == string_to_dec((uint8 *)("env-c"))){ // 进行特征值采样
				print_coin_env_info();
			}else if (arg[argc - 1] == string_to_dec((uint8 *)("pre-set"))){ // 进行特征值采样
				print_pre_count_set_value ();
			}else if (arg[argc - 1] == string_to_dec((uint8 *)("pre-current"))){ // 进行特征值采样
				print_pre_count_current ();
			}else if (arg[argc - 1] == string_to_dec((uint8 *)("password"))){ // 进行特征值采样
				cy_println ("%d", sys_env.password);
			}else{
				cy_println ("Error %d arg", argc);
			}
			break;
		case 2:
			if (arg[argc - 2] == string_to_dec((uint8 *)("ng")))
			{
				print_ng_data (arg[argc - 1]);
			}
			else if (arg[argc - 2] == string_to_dec((uint8 *)("gd")))
			{
				print_good_data (arg[argc - 1]);
			}
			else if (arg[argc - 2] == string_to_dec((uint8 *)("coin"))) // 进行特征值采样
			{
				print_coin_pre_value_info ((int32_t *)cmd_arg);
			}
			else
			{
				cy_println ("Error %d arg", argc);
			}
			break;
		default:
				cy_println ("Error %d arg", argc);break;
	}
}

#define BUFF_SIZE 512
void read_para_1(int32_t arg[])
{
	int i;
	FATFS fs;
	FIL file;
	FRESULT Res;
	char buf[BUFF_SIZE];

	if (arg[0]  == string_to_dec((uint8 *)("coin-config")))
	{
	}
	else
	{
		cy_println ("arg1 must be coin-config");
		return;
	}

	Res = f_mount(&fs, "" , 0);

	cy_print("f_mount Res = %d, ", Res);

	Res =  f_open(&file, "coin-config.txt", FA_READ | FA_OPEN_EXISTING);

	if (Res != FR_OK) {
		cy_print("Open file failed ");
		cy_println("Res = %d", Res);
		return;
	}
	//Res = f_read(&file, buf, BUFF_SIZE, &ByteRead);
	cy_println("Read coin-config.txt:");
	cmd ();
	ng_info_echo_off = 1; //turn off display ng info for a moment
	for (i = 0; i <1024; i++)
	{
		if (f_eof(&file))
			break;
		f_gets (buf, 64, &file);
		if (buf[0] == '\n' || buf[1] == '\r' || (buf[0] == '\r' || buf[1] == '\n'))
		{
			//cy_println ("NULL");
			continue;
		}
		cy_print("%s", buf);
		run_command (buf);
	}
	cy_println("exec coin-config.txt done");
	ng_info_echo_off = 0; //turn on display ng info for a moment
	f_close(&file);
}

void read_para_2(int32_t arg[])
{
	U8 read_page_buf[2048];
	int i, j;
	U8 r_code;
	if (arg[0]  == string_to_dec((uint8 *)("nand")))
	{
	}
	else
	{
		cy_println ("device must be nand or sd");
		return;
	}
	r_code = Nand_ReadPage (arg[1], 0, read_page_buf);
	if (r_code == 0)
	{
		cy_println ("Read nand block %d page 0", arg[1]);
		for(i = 0; i < 16; i++)
		{
			for(j = 0; j < 16; j++)
			{
				cy_print("%02x ", read_page_buf[i*16+j]);
			}
			cy_println();
		}
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

void read_para_3 (int32_t arg[])
{
	uint32_t nand_addr;
	U8 read_page_buf[2048];
	U8 r_code;
	int i, j;
	if (arg[0]  == string_to_dec((uint8 *)("nand")))
	{     					  //block * 2048 * 64 +   page * 2048
		nand_addr = (uint32_t)(arg[1] * 2048 * 64 + arg[2] * 2048);
	}
	else
	{
		cy_println ("device must be nand or ddr");
		return;
	}
	r_code = Nand_ReadSkipBad (nand_addr, read_page_buf, 2048);
	if (r_code == 0)
	{
		cy_println ("Read nand block %d page %d", arg[1], arg[2]);
		for(i = 0; i < 16; i++)
		{
			for(j = 0; j < 16; j++)
			{
				cy_print("%02x ", read_page_buf[i*16+j]);
			}
			cy_println();
		}
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

void do_read(int32_t argc, void *cmd_arg)
{
	switch (argc)
	{
		case 1:
			read_para_1 ((int32_t *)cmd_arg);
			break;
		case 2:
			read_para_2 ((int32_t *)cmd_arg);
			break;
		case 3:
			read_para_3 ((int32_t *)cmd_arg);
			break;
		default:
			cy_println("%d arg", argc); break;
	}
}

void write_sd_coin_coinfig (void)
{
	int i;
	FATFS fs;
	FIL file;
	FRESULT Res;

	Res = f_mount(&fs, "" , 0);

	cy_print("f_mount Res = %d, ", Res);

	Res =  f_open(&file, "coin-config.txt", FA_WRITE | FA_CREATE_ALWAYS);

	if (Res != FR_OK) {
		cy_println("Open file failed");
		cy_println("Res = %d", Res);
		return;
	}
	//Res = f_read(&file, buf, BUFF_SIZE, &ByteRead);
	cy_println("Write coin-config.txt:");
	for (i = 0; i < COIN_TYPE_NUM; i++)
	{
		f_printf (&file, "set cnnm%d %d %d %d %d %d %d\n", i, pre_value.country[coinchoose].coin[i].data.max0,
															  pre_value.country[coinchoose].coin[i].data.min0,
															  pre_value.country[coinchoose].coin[i].data.max1,
															  pre_value.country[coinchoose].coin[i].data.min1,
															  pre_value.country[coinchoose].coin[i].data.max2,
															  pre_value.country[coinchoose].coin[i].data.min2
		);
		f_printf (&file, "set cnnm%d std %d %d %d\n", i, pre_value.country[coinchoose].coin[i].data.std0,
														 pre_value.country[coinchoose].coin[i].data.std1,
														 pre_value.country[coinchoose].coin[i].data.std2
		);
		f_printf (&file, "set cnnm%d offset %d %d %d %d %d %d\n", i, pre_value.country[coinchoose].coin[i].data.offsetmax0,
																	 pre_value.country[coinchoose].coin[i].data.offsetmin0,
																	 pre_value.country[coinchoose].coin[i].data.offsetmax1,
																	 pre_value.country[coinchoose].coin[i].data.offsetmin1,
																	 pre_value.country[coinchoose].coin[i].data.offsetmax2,
																	 pre_value.country[coinchoose].coin[i].data.offsetmin2
		);
	}
	f_close(&file);
}

void write_para_1 (int32_t arg[])
{
	U8 r_code;
	if (arg[0] == string_to_dec((uint8 *)("code"))){
		r_code = WriteCodeToNand ();
		if (r_code == 0)
			cy_println ("write code to nand flash block 0 page 0 nand addr 0 completed");
		else
			cy_println ("write code to nand flash block 0 page 0 nand addr 0 failed");
	}else if (arg[0] == string_to_dec((uint8 *)("reset"))){
		sys_env.tty_mode = 0;
		rec_count = 0;
		memset (iap_code_buf, 0, sizeof(iap_code_buf));
	}else if (arg[0] == string_to_dec((uint8 *)("start"))){
		comscreen(Disp_Indexpic[22],Number_IndexpicB);
		sys_env.tty_mode = 0x55;
		sys_env.update_flag = UART_UPDATE;
		rec_count = 0;
	}else if (arg[0] == string_to_dec((uint8 *)("flash"))){
		int *magic_num = (int*)(&iap_code_buf[0x3c]);
		if (*magic_num == PROGRAM_MAGIC_NUM){
			r_code = WriteAppToAppSpace ((uint32_t)&iap_code_buf, sizeof(iap_code_buf));
			if (r_code == 0)
				cy_println ("write iap_code_buf to nand flash block 10 page 0 nand addr 0 completed");
			else
				cy_println ("write iap_code_buf to nand flash block 10 page 0 nand addr 0 failed");
			comscreen(Disp_Indexpic[23],Number_IndexpicB);	//触摸屏跳转到更新完成界面
		}else{
			cy_println ("The app was not valid!!!");
		}
	}else if (arg[0] == string_to_dec((uint8 *)("coin-config"))){
		write_sd_coin_coinfig ();
	}else{
		cy_println ("write_para_1 arg Error!");
		return;
	}
}


void write_para_3 (int32_t arg[])
{
	U8 r_code;
	r_code = Nand_WritePage (arg[1], 0, (U8 *)&arg[2]);
	if (r_code == 0)
	{
		cy_println ("write data %d to nand block %d page 0 completed", arg[2], arg[1]);
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

void write_para_4 (int32_t arg[])
{
	uint32_t nand_addr;
	U8 write_page_buf[2048], r_code;
	int i;

	for (i = 0; i < 2048; i++)
	{
		write_page_buf[i] = arg[3];
	}

	if (arg[0] == string_to_dec((uint8 *)("nand")))
	{
							  //block * 2048 * 64 +   page * 2048
		nand_addr = (uint32_t)(arg[1] * 2048 * 64 + arg[2] * 2048);
	}
	else
	{
		cy_println ("device must be nand or ddr");
		return;
	}
	//nand flash 必须以块为单位擦除，所以写特定页时要先保存不需要更改的页数据
	r_code = Nand_WritePage (arg[1], arg[2], write_page_buf);
	if (r_code == 0)
	{
		cy_println ("write block %d page %d nand addr %d completed", arg[1], arg[2], nand_addr);
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
void do_write(int32_t argc, void *cmd_arg)
{
	switch (argc)
	{
		case 1:
			write_para_1 ((int32_t *)cmd_arg);
			break;
		case 3:
			write_para_3 ((int32_t *)cmd_arg);
			break;
		case 4:
			write_para_4 ((int32_t *)cmd_arg);
			break;
		default:
			cy_println("%d arg", argc); break;
	}
}

void erase_para_2 (int32_t arg[])
{
	if (arg[0]  == string_to_dec((uint8 *)("nand")))
	{
	}
	else
	{
		cy_println ("device must be nand or ddr");
		return;
	}
	if (arg[1] < 20){
		cy_println ("can not erase program block %d", arg[1]);
	}else{
		Nand_EraseBlock (arg[1]);
		cy_println ("erase block %d completed", arg[1]);
	}
}

void do_erase(int32_t argc, void *cmd_arg)
{
	switch (argc)
	{
		case 2:
			erase_para_2 ((int32_t *)cmd_arg);
			break;
		default:
			cy_println("%d arg", argc); break;
	}
}

/*
 * reset the cpu by setting up the watchdog timer and let him time out
 */
void do_reset_cpu(int32_t argc, void *cmd_arg)
{
	cy_print("reset... \n\n\n");
	//LOG ("Reset System...");
	rSWIRST = 0x533C2416;
//	Timer4_Stop ();
//	rWTCON = 0x00000000;
//	rWTCNT = 0x00000001;
//	rWTDAT = 0x0;
//	rWTCON = 0x00000021;

	/* loop forever and wait for reset to happen */
	while (1);
 	/*NOTREACHED*/
}

void delete_db_all (void)
{
	cy_println ("Delete History Data");
//	para_set_value.data.db_total_item_num = 0;
//	para_set_value.data.total_money = 0;
//	para_set_value.data.total_good = 0;
//	para_set_value.data.total_ng = 0;
	yqsql_exec(DBDELETE);	  // delete db table data
	write_para(); // 3 为保存 当前保存的 数据条数
}

void db_para_2 (int32_t arg[])
{

	if (arg[0]  == string_to_dec((uint8 *)("delete")))
	{
	}
	else
	{
		cy_println ("db arg 1 must be delete");
		return;
	}
	if (arg[1] == string_to_dec((uint8 *)("all")))
	{
		delete_db_all ();
	}
	else
	{
		cy_println ("db arg 2 must be all");
		return;
	}
}


void do_db(int32_t argc, void *cmd_arg)
{
	switch (argc)
	{
		case 2:
			db_para_2 ((int32_t *)cmd_arg);
			break;
		default:
			cy_println("%d arg", argc); break;
	}
}



int16_t is_repeate (int16_t _coin_index)
{
	int16_t ei, i;
	ei = 0;

	int16_t temp_coin_maxvalue0;
	int16_t temp_coin_minvalue0;
	int16_t temp_coin_maxvalue1;
	int16_t temp_coin_minvalue1;
	int16_t temp_coin_maxvalue2;
	int16_t temp_coin_minvalue2;

	temp_coin_maxvalue0 = coin_maxvalue0 + pre_value.country[coinchoose].coin[_coin_index].data.offsetmax0;
	temp_coin_minvalue0 = coin_minvalue0 + pre_value.country[coinchoose].coin[_coin_index].data.offsetmin0;
	temp_coin_maxvalue1 = coin_maxvalue1 + pre_value.country[coinchoose].coin[_coin_index].data.offsetmax1;
	temp_coin_minvalue1 = coin_minvalue1 + pre_value.country[coinchoose].coin[_coin_index].data.offsetmin1;
	temp_coin_maxvalue2 = coin_maxvalue2 + pre_value.country[coinchoose].coin[_coin_index].data.offsetmax2;
	temp_coin_minvalue2 = coin_minvalue2 + pre_value.country[coinchoose].coin[_coin_index].data.offsetmin2;

	for (i = 0; i < COIN_TYPE_NUM; i++){
		if ( _coin_index != i){
			if (ng_info_echo_off != 1)
				cy_println ("Compare with Coin %d", i);
			if(
				(
					((temp_coin_maxvalue0 >= coin_cmp_value[i].compare_max0) && (temp_coin_minvalue0 <= coin_cmp_value[i].compare_min0)) || //与1元   通道0比
					((temp_coin_maxvalue0 <= coin_cmp_value[i].compare_max0) && (temp_coin_maxvalue0 >= coin_cmp_value[i].compare_min0)) ||
					((temp_coin_minvalue0 <= coin_cmp_value[i].compare_max0) && (temp_coin_minvalue0 >= coin_cmp_value[i].compare_min0))
				) && (pre_value.country[coinchoose].coin[i].data.max0 > pre_value.country[coinchoose].coin[i].data.min0) &&
				(
					((temp_coin_maxvalue1 >= coin_cmp_value[i].compare_max1) && (temp_coin_minvalue1 <= coin_cmp_value[i].compare_min1)) || //与1元   通道0比
					((temp_coin_maxvalue1 <= coin_cmp_value[i].compare_max1) && (temp_coin_maxvalue1 >= coin_cmp_value[i].compare_min1)) ||
					((temp_coin_minvalue1 <= coin_cmp_value[i].compare_max1) && (temp_coin_minvalue1 >= coin_cmp_value[i].compare_min1))
				) && (pre_value.country[coinchoose].coin[i].data.max1 > pre_value.country[coinchoose].coin[i].data.min1) &&
				(
					((temp_coin_maxvalue2 >= coin_cmp_value[i].compare_max2) && (temp_coin_minvalue2 <= coin_cmp_value[i].compare_min2)) || //与1元   通道0比
					((temp_coin_maxvalue2 <= coin_cmp_value[i].compare_max2) && (temp_coin_maxvalue2 >= coin_cmp_value[i].compare_min2)) ||
					((temp_coin_minvalue2 <= coin_cmp_value[i].compare_max2) && (temp_coin_minvalue2 >= coin_cmp_value[i].compare_min2))
				) && (pre_value.country[coinchoose].coin[i].data.max2 > pre_value.country[coinchoose].coin[i].data.min2)
			   ){
				cy_println ("Note!!! value Repeat with Coin %d please comfirm !!!", i);
				cy_println ("coin %d value is:", _coin_index);
				cy_println ("min0 = %d max0 = %d", temp_coin_minvalue0, temp_coin_maxvalue0);
				cy_println ("min1 = %d max1 = %d", temp_coin_minvalue1, temp_coin_maxvalue1);
				cy_println ("min2 = %d max2 = %d", temp_coin_minvalue2, temp_coin_maxvalue2);
				cy_println ("coin %d value is:", i);
				cy_println ("min0 = %d max0 = %d", coin_cmp_value[i].compare_min0, coin_cmp_value[i].compare_max0);
				cy_println ("min1 = %d max1 = %d", coin_cmp_value[i].compare_min1, coin_cmp_value[i].compare_max1);
				cy_println ("min2 = %d max2 = %d", coin_cmp_value[i].compare_min2, coin_cmp_value[i].compare_max2);
				ei = i+1;	//提示 交叉
				return ei;
			}
		}
	}
	if(((coin_maxvalue0 - coin_minvalue0) > 100)){
		ei = 1002;//在图标里，这个是提示 学习范围太大
	}
	return ei;
}

int do_task_manager (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int test1, test2, test3;
	switch (argc) {
	case 1 :
		cy_println ("no arg");
		break;
	case 2 :
		test1 = simple_strtol (argv[1], NULL, 16);
		cy_println ("test1 = 0x%x", test1);
		break;
	case 3 :
		test1 = simple_strtol (argv[1], NULL, 16);
		test2 = simple_strtol (argv[2], NULL, 10);
		cy_println ("test1 = 0x%x, test2 = %d", test1, test2);
		break;
	case 4 :
		test1 = simple_strtol (argv[1], NULL, 16);
		test2 = simple_strtol (argv[2], NULL, 10);
		test3 = simple_strtol (argv[3], NULL, 10);
		cy_println ("test1 = 0x%x, test2 = %d, test3 = %d", test1, test2, test3);
		break;
	default:
		cy_print ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	return 0;
}

MY_CMD(
	task_manager,	4,	0,	do_task_manager,
	"task_manager - task_manager from USB device\n",
	"task_manager dev:part\n"
);


int do_print_version  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	cy_println("\n----------------------------------------------------");
	PRINT_VERSION()
	cy_println("----------------------------------------------------");
	return 0;
}
MY_CMD(
	version,	4,	1,	do_print_version,
	"version - print version information\n",
	"version\n"
);


int do_motor  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int32_t arg[4];
	switch (argc) {
	case 2:
		arg[0] = simple_strtol (argv[1], NULL, 10);
		set_motor (0, MOTOR_DIR_UP, arg[0], 6, MOTOR_WATCH_POS1);
		return 0;
	case 4 :
		arg[0] = simple_strtol (argv[1], NULL, 10);
		arg[1] = simple_strtol (argv[2], NULL, 10);
		arg[2] = simple_strtol (argv[3], NULL, 10);
		arg[3] = 0;
		break;
	case 5 :
		arg[0] = simple_strtol (argv[1], NULL, 10);
		arg[1] = simple_strtol (argv[2], NULL, 10);
		arg[2] = simple_strtol (argv[3], NULL, 10);
		arg[3] = simple_strtol (argv[4], NULL, 10);
		break;
	default:
		cy_print ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	run_step_motor (arg);
	return 0;
}
MY_CMD(
	motor,	6,	1,	do_motor,
	"motor id step speed [repeat]\n",
	"motor - run motor\n"
);

int do_0  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	sys_env.coin_count = 0;
	SET_MOTOR_CPLT_CALLBACK (1);
	set_motor_cplt_callback (0, motor_0_cplt_callback);
	set_motor (0, MOTOR_DIR_UP, 600000, MOTOR_0_SPEED, MOTOR_WATCH_POS1);
	set_motor (1, MOTOR_DIR_OPEN, 600000, MOTOR_1_SPEED, MOTOR_WATCH_POS1);
	return 0;
}
MY_CMD(
	0,	6,	1,	do_0,
	"0 run step 0 program\n",
	"0 - run step 0\n"
);

int do_1  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	set_motor (1, MOTOR_DIR_CLOSE, para_set_value.data.coin_size, 2, MOTOR_WATCH_NONE);
	return 0;
}
MY_CMD(
	1,	6,	1,	do_1,
	"1 run step 1 program\n",
	"1 - run step 1\n"
);
int do_2  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	set_motor (0, MOTOR_DIR_DOWN, 600000, MOTOR_0_SPEED, MOTOR_WATCH_POS2);
	return 0;
}
MY_CMD(
	2,	6,	1,	do_2,
	"2 run step 2 program\n",
	"2 - run step 2\n"
);
int do_3  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	set_motor (1, MOTOR_DIR_CLOSE, para_set_value.data.coin_push_size, 2, MOTOR_WATCH_NONE);
	return 0;
}
MY_CMD(
	3,	6,	1,	do_3,
	"3 run step 1 program\n",
	"3 - run step 1\n"
);
int do_4  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	set_motor (0, MOTOR_DIR_DOWN, 600000, MOTOR_0_SPEED, MOTOR_WATCH_POS3);
	return 0;
}
MY_CMD(
	4,	6,	1,	do_4,
	"4 run step 4 program\n",
	"4 - run step 4\n"
);
int do_5  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	set_motor (2, MOTOR_DIR_NONE, 400, MOTOR_2_SPEED, MOTOR_WATCH_NONE);
	return 0;
}
MY_CMD(
	5,	6,	1,	do_5,
	"5 run step 5 program\n",
	"5 - run step 5\n"
);
int do_6  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	set_motor (1, MOTOR_DIR_OPEN, 600000, MOTOR_1_SPEED, MOTOR_WATCH_POS1);
	return 0;
}
MY_CMD(
	6,	6,	1,	do_6,
	"6 run step 6 program\n",
	"6 - run step 6\n"
);
int do_7  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	set_motor (2, MOTOR_DIR_NONE, 400, MOTOR_2_SPEED, MOTOR_WATCH_NONE);
	return 0;
}
MY_CMD(
	7,	6,	1,	do_7,
	"7 run step 7 program\n",
	"7 - run step 7\n"
);
int do_8  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	//set_motor (2, MOTOR_DIR_DOWN, 400, 4, MOTOR_WATCH_NONE);
	return 0;
}
MY_CMD(
	8,	6,	1,	do_8,
	"8 run step 8 program\n",
	"8 - run step 8\n"
);

int do_receive (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	switch (argc) {
	case 1:
		cy_println ("rec_count %d", rec_count);
		break;
	case 2 :
		cy_println ("Enter Receive Code Mode");
		sys_env.tty_mode = 0x55;
		rec_count = 0;
		break;
	default:
		cy_print ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	return 0;
}

MY_CMD(
	receive,	4,	0,	do_receive,
	"receive - receive code from PC\n",
	"receive code\n"
);



/* test if ctrl-c was pressed */
static int ctrlc_disabled = 0;	/* see disable_ctrl() */
//static int ctrlc_was_pressed = 0;
int ctrlc (void)
{
//	if (!ctrlc_disabled && gd->have_console) {
//		if (tstc ()) {
//			switch (getc ()) {
//			case 0x03:		/* ^C - Control C */
//				ctrlc_was_pressed = 1;
//				return 1;
//			default:
//				break;
//			}
//		}
//	}
	return 0;
}

int do_my_help (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

MY_CMD(
	help,	CFG_MAXARGS,	0,	do_my_help,
 	"help    - print online help\n",
 	"[command ...]\n"
 	"    - show help information (for 'command')\n"
 	"'help' prints online help for the monitor commands.\n\n"
 	"Without arguments, it prints a short usage message for all commands.\n\n"
 	"To get detailed help information for specific commands you can type\n"
  "'help' with one or more command names as arguments.\n"
);

/* This do not ust the U_BOOT_CMD macro as ? can't be used in symbol names */
#ifdef  CFG_LONGHELP
cmd_tbl_t __u_my__cmd_question_mark Struct_Section = {
	"?",	CFG_MAXARGS,	0,	do_my_help,
 	"?       - alias for 'help'\n",
	NULL
};
#else
cmd_tbl_t __u_my__cmd_question_mark Struct_Section = {
	"?",	CFG_MAXARGS,	0,	do_my_help,
 	"?       - alias for 'help'\n"
};
#endif /* CFG_LONGHELP */
int do_my_help (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	int rcode = 0;
	cmdtp = &__my_cmd_help; //加上这一句防止my_cmd_sec这个Section 被编译器优化掉
	if (argc == 1)
	{	/*show list of commands */

		int cmd_items = (unsigned int)MY_CMD_RO_SEC_LENGTH / sizeof (cmd_tbl_t);	/* pointer arith! */
		cmd_tbl_t *cmd_array[cmd_items];
		int i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = (cmd_tbl_t *)MY_CMD_RO_SEC_START;
		for (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		for (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			for (j = 0; j < i; ++j) {
				if (strcmp (cmd_array[j]->name,
					    cmd_array[j + 1]->name) > 0) {
					cmd_tbl_t *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			if (!swaps)
				break;
		}

		/* print short help (usage) */
		for (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			if (had_ctrlc ())
				return 1;
			if (usage == NULL)
				continue;
			cy_puts (usage);
		}
		return 0;
	}
	/*
	 * command help (long version)
	 */
	for (i = 1; i < argc; ++i) {
		if ((cmdtp = find_cmd (argv[i])) != NULL) {
#ifdef	CFG_LONGHELP
			/* found - print (long) help info */
			cy_puts (cmdtp->name);
			cy_putc (' ');
			if (cmdtp->help) {
				cy_puts (cmdtp->help);
			} else {
				cy_puts ("- No help available.\n");
				rcode = 1;
			}
			cy_putc ('\n');
#else	/* no long help available */
			if (cmdtp->usage)
				cy_puts (cmdtp->usage);
#endif	/* CFG_LONGHELP */
		} else {
			cy_print ("Unknown command '%s' - try 'help'"
				" without arguments for list of all"
				" known commands\n\n", argv[i]
					);
			rcode = 1;
		}
	}
	return rcode;
}


/***************************************************************************
 * find command table entry for a command
 */
cmd_tbl_t *find_cmd (const char *cmd)
{
	cmd_tbl_t *cmdtp;
	cmd_tbl_t *cmdtp_temp = (cmd_tbl_t *)MY_CMD_RO_SEC_START;	/*Init value */
	const char *p;
	int len;
	int n_found = 0;

	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	for (cmdtp = (cmd_tbl_t *)MY_CMD_RO_SEC_START;
	     cmdtp != (cmd_tbl_t *)MY_CMD_RO_SEC_END;
	     cmdtp++) {
		if (strncmp (cmd, cmdtp->name, len) == 0) {
			if (len == strlen (cmdtp->name))
				return cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	if (n_found == 1) {			/* exactly one match */
		return cmdtp_temp;
	}

	return NULL;	/* not found or ambiguous command */
}

/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
int disable_ctrlc (int disable)
{
	int prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	return prev;
}

int had_ctrlc (void)
{
	return sys_env.sys_break;
}

void clear_ctrlc (void)
{
	sys_env.sys_break = 0;
}
/****************************************************************************/

int parse_line (char *line, char *argv[])
{
	int nargs = 0;

#ifdef DEBUG_PARSER
	cy_print ("parse_line: \"%s\"\n", line);
#endif
	while (nargs < CFG_MAXARGS) {

		/* skip any white space */
		while ((*line == ' ') || (*line == '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		cy_print ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		while (*line && (*line != ' ') && (*line != '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		cy_print ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	cy_print ("** Too many args (max. %d) **\n", CFG_MAXARGS);

#ifdef DEBUG_PARSER
	cy_print ("parse_line: nargs=%d\n", nargs);
#endif
	return (nargs);
}

/****************************************************************************/

static void process_macros (const char *input, char *output)
{
	char c, prev;
	const char *varname_start = NULL;
	int inputcnt = strlen (input);
	int outputcnt = CFG_CBSIZE;
	int state = 0;		/* 0 = waiting for '$'  */

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
#ifdef DEBUG_PARSER
	char *output_start = output;

	cy_print ("[PROCESS_MACROS] INPUT len %d: \"%s\"\n", strlen (input),
		input);
#endif

	prev = '\0';		/* previous character   */

	while (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		if (state != 3) {
			/* remove one level of escape characters */
			if ((c == '\\') && (prev != '\\')) {
				if (inputcnt-- == 0)
					break;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		case 0:	/* Waiting for (unescaped) $    */
			if ((c == '\'') && (prev != '\\')) {
				state = 3;
				break;
			}
			if ((c == '$') && (prev != '\\')) {
				state++;
			} else {
				*(output++) = c;
				outputcnt--;
			}
			break;
		case 1:	/* Waiting for (        */
			if (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} else {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				if (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			break;
		case 2:	/* Waiting for )        */
			if (c == ')' || c == '}') {
				int i;
				char envname[CFG_CBSIZE], *envval;
				int envcnt = input - varname_start - 1;	/* Varname # of chars */

				/* Get the varname */
				for (i = 0; i < envcnt; i++) {
					envname[i] = varname_start[i];
				}
				envname[i] = 0;

				/* Get its value */
				envval = getenv (envname);

				/* Copy into the line if it exists */
				if (envval != NULL)
					while ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			break;
		case 3:	/* Waiting for '        */
			if ((c == '\'') && (prev != '\\')) {
				state = 0;
			} else {
				*(output++) = c;
				outputcnt--;
			}
			break;
		}
		prev = c;
	}

	if (outputcnt)
		*output = 0;

#ifdef DEBUG_PARSER
	cy_print ("[PROCESS_MACROS] OUTPUT len %d: \"%s\"\n",
		strlen (output_start), output_start);
#endif
}

/****************************************************************************
 * returns:
 *	1  - command executed, repeatable
 *	0  - command executed but not repeatable, interrupted commands are
 *	     always considered not repeatable
 *	-1 - not executed (unrecognized, bootd recursion or too many args)
 *           (If cmd is NULL or "" or longer than CFG_CBSIZE-1 it is
 *           considered unrecognized)
 *
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from getenv(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */

int my_run_command (const char *cmd, int flag)
{
	cmd_tbl_t *cmdtp;
	char cmdbuf[CFG_CBSIZE];	/* working copy of cmd		*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	char finaltoken[CFG_CBSIZE];
	char *str = cmdbuf;
	char *argv[CFG_MAXARGS + 1];	/* NULL terminated	*/
	int argc, inquotes;
	int repeatable = 1;
	int rc = 0;

#ifdef DEBUG_PARSER
	cy_print ("[RUN_COMMAND] cmd[%p]=\"", cmd);
	cy_puts (cmd ? cmd : "NULL");	/* use cy_puts - string may be loooong */
	cy_puts ("\"\n");
#endif

	clear_ctrlc();		/* forget any previous Control C */

	if (!cmd || !*cmd) {
		return -1;	/* empty command */
	}

	if (strlen(cmd) >= CFG_CBSIZE) {
		cy_puts ("## Command too long!\n");
		return -1;
	}

	strcpy (cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

#ifdef DEBUG_PARSER
	cy_print ("[PROCESS_SEPARATORS] %s\n", cmd);
#endif
	while (*str) {

		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		for (inquotes = 0, sep = str; *sep; sep++) {
			if ((*sep=='\'') &&
			    (*(sep-1) != '\\'))
				inquotes=!inquotes;

			if (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    ( sep != str) &&	/* past string start	*/
			    (*(sep-1) != '\\'))	/* and NOT escaped	*/
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		if (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		}
		else
			str = sep;	/* no more commands for next pass */
#ifdef DEBUG_PARSER
		cy_print ("token: \"%s\"\n", token);
#endif

		/* find macros in this token and replace them */
		process_macros (token, finaltoken);

		/* Extract arguments */
		if ((argc = parse_line (finaltoken, argv)) == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		/* Look up command in command table */
		if ((cmdtp = find_cmd(argv[0])) == NULL) {
			cy_print ("Unknown my_cmd command '%s' - now try 'vTaskCmdAnalyze'\n", argv[0]);
			rc = -1;	/* give up after bad command */
			continue;
		}

		/* found - check max args */
		if (argc > cmdtp->maxargs) {
			cy_print ("Usage:\n%s\n", cmdtp->usage);
			rc = -1;
			continue;
		}

#if (CONFIG_COMMANDS & CFG_CMD_BOOTD)
		/* avoid "bootd" recursion */
		if (cmdtp->cmdhandle == do_bootd) {
#ifdef DEBUG_PARSER
			cy_print ("[%s]\n", finaltoken);
#endif
			if (flag & CMD_FLAG_BOOTD) {
				cy_puts ("'bootd' recursion detected\n");
				rc = -1;
				continue;
			} else {
				flag |= CMD_FLAG_BOOTD;
			}
		}
#endif	/* CFG_CMD_BOOTD */

		/* OK - call function to do the command */
		if ((cmdtp->cmdhandle) (cmdtp, flag, argc, argv) != 0) {
			rc = -1;
		}

		repeatable &= cmdtp->repeatable;

		/* Did the user stop this? */
		if (had_ctrlc ())
			return 0;	/* if stopped then not repeatable */
	}

	return rc ? rc : repeatable;
}
















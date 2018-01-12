#ifndef _MY_CMD_
#define _MY_CMD_

#include "def.h"


#define PROGRAM_VERSION "V1.0\n(Compile Time:", __DATE__, __TIME__, ")"
#define PRINT_VERSION() cy_println ("Software Version : %s %s %s%s", PROGRAM_VERSION);  


#define PROGRAM_MAGIC_NUM 0x55aa1a25

#define HEX_TO_DEC(X) (X >> 4) * 10 + (X & 0xf)


/*
 * Monitor Command Table
 */
 
#define CFG_CBSIZE	64		/* Console I/O Buffer Size	*/
 #define CFG_MAXARGS		16	/* max number of command args   */
#define CFG_LONGHELP				/* undef to save memory		*/
//#define CONFIG_AUTO_COMPLETE	1       /* add autocompletion support   */

struct cmd_tbl_s {
	char		*name;		/* Command Name			*/
	int		maxargs;	/* maximum number of arguments	*/
	int		repeatable;	/* autorepeat allowed?		*/
					/* Implementation function	*/
	int		(*cmdhandle)(struct cmd_tbl_s *, int, int, char *[]);
	char		*usage;		/* Usage message	(short)	*/
#ifdef	CFG_LONGHELP
	char		*help;		/* Help  message	(long)	*/
#endif
#ifdef CONFIG_AUTO_COMPLETE
	/* do auto completion on the arguments */
	int		(*complete)(int argc, char *argv[], char last_char, int maxv, char *cmdv[]);
#endif
};

typedef struct cmd_tbl_s	cmd_tbl_t;


extern cmd_tbl_t  __u_boot_cmd_start;
extern cmd_tbl_t  __u_boot_cmd_end;

#define Struct_Section __attribute__ ((unused, section("my_cmd_sec")))


#define MY_CMD(name,maxargs,rep,cmd,usage,help) \
 __align(4) cmd_tbl_t __my_cmd_##name Struct_Section = {#name, maxargs, rep, cmd, usage, help}
 
extern cmd_tbl_t __my_cmd_task_manager;


typedef struct 
{  
	char const *cmd_name;                        //命令字符串  
	int32_t max_args;                            //最大参数数目  
	void (*handle)(int argc, void * cmd_arg);     //命令回调函数  
	char  *help;                                 //帮助信息  
}cmd_list_struct; 


#define ARG_NUM     8          //命令中允许的参数个数  
#define CMD_LEN     20         //命令名占用的最大字符长度  
#define CMD_BUF_LEN 512+3         //命令缓存的最大长度  
       
typedef struct 
{  
	char rec_buf[CMD_BUF_LEN];            //接收命令缓冲区  
	char processed_buf[CMD_BUF_LEN];      //存储加工后的命令(去除控制字符)  
	int32_t cmd_arg[ARG_NUM];             //保存命令的参数  
}cmd_analyze_struct;  


typedef struct
{
	U32 print_wave_to_pc;
	U32 auto_stop;
	U32 auto_clear;
	U32 sim_count_flag;
	U32 save_ng_data;
	U32 save_good_data;
	
	U32 country_index;
	U32 coin_index;
	U32 workstep;
	U32 uart0_cmd_flag;
	U32 tty_mode;
	U32 hmi_debug_flag;
	U32 stop_time;
	U32 stop_flag;
	U32 coin_cross_time;
	U32 tty_online_ms;
	U32 coin_over;
	U32 coin_leave;
	U32 AD_data_len;
	U32 AD_buf_index;
	U32 AD_buf_sending;
	AD_Value *Detect_AD_buf_p;
	U32 system_delay;
	U32 sys_runing_time;
	U32 sys_runing_time_total;
	U32 password;
}s_system_env;

extern s_system_env sys_env;
#define CODE_BUF_SIZE 256*1024
extern char iap_code_buf[CODE_BUF_SIZE]; // 程序缓冲区256K

void system_env_init (void);
void print_ng_data (S16 index);
void print_system_env_info (void);
void fill_rec_buf(char data);
void update_finish (void);
u16 CRC16(char * _data_buf, int len);
void vTaskCmdAnalyze( void );
void run_command (char * _cmd_str);
int my_run_command (const char *cmd, int flag);

void coin_start (void);
void coin_stop (void);
void coin_print (void);
void coin_clear (void);
void coin_clear_alarm (void);
void refresh_data (void);

S16 is_repeate (S16 _coin_index);//  判别 是不是重币的函数

#define TTY_ONLINE_TIME 25



extern unsigned int Image$$MY_CMD_RO_SEC$$Base;
extern unsigned int Image$$MY_CMD_RO_SEC$$Length;

#define MY_CMD_RO_SEC_LENGTH 		(&Image$$MY_CMD_RO_SEC$$Length)
#define MY_CMD_RO_SEC_START 		(&Image$$MY_CMD_RO_SEC$$Base)
#define MY_CMD_RO_SEC_END 			((unsigned int)(&Image$$MY_CMD_RO_SEC$$Base) + (unsigned int)MY_CMD_RO_SEC_LENGTH)
	
cmd_tbl_t *find_cmd (const char *cmd);




#endif

#ifndef _MY_CMD_
#define _MY_CMD_

#include "def.h"


#define PROGRAM_VERSION "V1.0\n(Compile Time:", __DATE__, __TIME__, ")"
#define PRINT_VERSION() cy_println ("Software Version : %s %s %s%s", PROGRAM_VERSION);  


#define PROGRAM_MAGIC_NUM 0x55aa1a25

#define HEX_TO_DEC(X) (X >> 4) * 10 + (X & 0xf)


#define CTRL_C 0X03

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
	char const *cmd_name;                        //�����ַ���  
	int32_t max_args;                            //��������Ŀ  
	void (*handle)(int argc, void * cmd_arg);     //����ص�����  
	char  *help;                                 //������Ϣ  
}cmd_list_struct; 


#define ARG_NUM     8          //����������Ĳ�������  
#define CMD_LEN     20         //������ռ�õ�����ַ�����  
#define CMD_BUF_LEN 512+3         //��������󳤶�  
       
typedef struct 
{  
	char rec_buf[CMD_BUF_LEN];            //�����������  
	char processed_buf[CMD_BUF_LEN];      //�洢�ӹ��������(ȥ�������ַ�)  
	int32_t cmd_arg[ARG_NUM];             //��������Ĳ���  
}cmd_analyze_struct;  


typedef enum {
  NULL_UPDATE = 0,
  UART_UPDATE = 1,
  NET_UPDATE = 2,
  NET_UPDATEING = 3,
  UART_COMMAND = 4
} e_update_flag;

typedef struct
{
	uint32_t print_wave_to_pc;
	uint32_t auto_stop;
	uint32_t auto_clear;
	uint32_t sim_count_flag;
	uint32_t save_ng_data;
	uint32_t save_good_data;
	
	uint32_t country_index;
	uint32_t coin_index;
	uint32_t workstep;
	uint32_t uart0_cmd_flag;
	uint32_t tty_mode;
	uint32_t hmi_debug_flag;
	uint32_t stop_time;
	uint32_t stop_flag;
	uint32_t coin_cross_time;
	uint32_t tty_online_ms;
	uint32_t coin_under_sensor;
	uint32_t rej_over;
	uint32_t coin_ng_flag;
	uint32_t coin_leave;
	uint32_t AD_data_len;
	uint32_t AD_buf_index;
	uint32_t AD_buf_sending;
	AD_Value *Detect_AD_buf_p;
	uint32_t system_delay;
	uint32_t sys_runing_time;
	uint32_t sys_runing_time_total;
	uint32_t password;
	e_update_flag update_flag;
	uint32_t net_task;
	uint32_t coin_speed;
	uint32_t coin_count;
	int16_t sys_break;
}s_system_env;

extern s_system_env sys_env;
extern uint32_t time_out;
#define CODE_BUF_SIZE 256*1024
extern char iap_code_buf[CODE_BUF_SIZE]; // ���򻺳���256K



extern void Delay_ms(uint32_t Count);	
extern void Delay_us(uint32_t Count);

void system_env_init (void);
void print_ng_data (int16_t index);
void print_system_env_info (void);
void fill_rec_buf(char data);
void update_finish (e_update_flag flag);
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
void poll_data (void);

int had_ctrlc (void);
void clear_ctrlc (void);

int16_t is_repeate (int16_t _coin_index);//  �б� �ǲ����رҵĺ���

#define TTY_ONLINE_TIME 25



extern unsigned int Image$$MY_CMD_RO_SEC$$Base;
extern unsigned int Image$$MY_CMD_RO_SEC$$Length;

#define MY_CMD_RO_SEC_LENGTH 		(&Image$$MY_CMD_RO_SEC$$Length)
#define MY_CMD_RO_SEC_START 		(&Image$$MY_CMD_RO_SEC$$Base)
#define MY_CMD_RO_SEC_END 			((unsigned int)(&Image$$MY_CMD_RO_SEC$$Base) + (unsigned int)MY_CMD_RO_SEC_LENGTH)
	
cmd_tbl_t *find_cmd (const char *cmd);




#endif

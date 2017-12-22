#ifndef __OS_CPU_H__
#define __OS_CPU_H__

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

/*
***********************************************************
*  uCOS-II使用到的数据类型定义
***********************************************************
*/
typedef unsigned char  BOOLEAN; /* 布尔变量               */
typedef unsigned char  INT8U;   /* 无符号8位整型变量      */
typedef signed   char  INT8S;   /* 有符号8位整型变量      */
typedef unsigned short INT16U;  /* 无符号16位整型变量     */
typedef signed   short INT16S;  /* 有符号16位整型变量     */
typedef unsigned int   INT32U;  /* 无符号32位整型变量     */
typedef signed   int   INT32S;  /* 有符号32位整型变量     */
typedef float          FP32;    /* 单精度浮点数(32位长度) */
typedef double         FP64;    /* 双精度浮点数(64位长度) */

/*
************************************************************
*  栈配置
************************************************************
*/
typedef INT32U         OS_STK;   /* 堆栈是32位宽度        */
#define OS_STK_GROWTH  1         /*  堆栈是从上往下长的   */

/*
************************************************************
*  临界区访问,开关中断配置
************************************************************
*/
#define OS_CRITICAL_METHOD      3 /* 中断状态保存在变量中 */
typedef INT32U         OS_CPU_SR;
#define OS_ENTER_CRITICAL()    {cpu_sr = CPU_SR_Save();}	/* 关中断 */
#define OS_EXIT_CRITICAL()     {CPU_SR_Restore(cpu_sr);}    /* 恢复中断状态 */

/*
************************************************************
*  汇编实现的函数声明
************************************************************
*/
#define OS_TASK_SW()           OSCtxSw() /* 任务级任务切换函数*/
OS_CPU_SR CPU_SR_Save(void);
void CPU_SR_Restore(OS_CPU_SR cpu_sr);
void OSCtxSw(void);
void OSIntCtxSw(void);
void OSStartHighRdy(void);

#endif 

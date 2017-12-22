;/********************************************************************/
;/* s3c2416.s: start code for samsung s3c2416/50/51(arm9)            */
;/********************************************************************/

; Clock setting(External Crystal 12M):
; MPLLCLK = 800M, EPLLCLK = 96M
; ARMCLK = 400M, HCLK = 133M
; DDRCLK = 266M, SSMCCLK = 66M, PCLK = 66M
; HSMMC1 = 24M

; Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs
Mode_USR        EQU     0x10
Mode_FIQ        EQU     0x11
Mode_IRQ        EQU     0x12
Mode_SVC        EQU     0x13
Mode_ABT        EQU     0x17
Mode_UND        EQU     0x1B
Mode_SYS        EQU     0x1F
; when I bit is set, IRQ is disabled
I_Bit           EQU     0x80 
; when F bit is set, FIQ is disabled
F_Bit           EQU     0x40            

; Stack Configuration
UND_Stack_Size  EQU     0x00000020
SVC_Stack_Size  EQU     0x00000020
ABT_Stack_Size  EQU     0x00000020
FIQ_Stack_Size  EQU     0x00000100
IRQ_Stack_Size  EQU     0x00000400
USR_Stack_Size  EQU     0x00001000
ISR_Stack_Size  EQU     (UND_Stack_Size + SVC_Stack_Size + \
						ABT_Stack_Size + FIQ_Stack_Size + \
						IRQ_Stack_Size)
                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   USR_Stack_Size
__initial_sp    SPACE   ISR_Stack_Size
Stack_Top

; Heap Configuration
Heap_Size       EQU     0x00000200
                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

; Internal Memory Base Addresses
IRAM_BASE       EQU     0x40000000

; Watchdog Timer Base Address
WT_BASE         EQU     0x53000000  

; Interrupt Register Base Address
INT_BASE		EQU		0x4A000000
INTMSK1_OFS		EQU		0x08
INTSUBMSK_OFS	EQU		0x1C
INTMSK2_OFS		EQU		0x48

; Clock Base Address
CLOCK_BASE		EQU		0x4C000000
LOCKCON0_OFS    EQU		0x00
LOCKCON1_OFS    EQU		0x04
MPLLCON_OFS		EQU		0x10
EPLLCON_OFS		EQU		0x18
CLKSRC_OFS		EQU		0x20
CLKDIV0_OFS		EQU		0x24
CLKDIV1_OFS		EQU		0x28
CLKDIV2_OFS		EQU		0x2C
	
; Nand controller base address
NFCONF			EQU		0x4E000000

;----------------------- CODE -------------------------------------------
                PRESERVE8
                
;  Area Definition and Entry Point
;  Startup Code must be linked first at Address at which it expects to run.

            AREA    RESET, CODE, READONLY
;		ENTRY
            ARM
Vectors     B       Reset_Handler        
            LDR     PC, Undef_Addr
            LDR     PC, SWI_Addr
            LDR     PC, PAbt_Addr
            LDR     PC, DAbt_Addr
			LDR		PC, Notuse_Addr
			LDR		PC, IRQ_Addr
			LDR		PC, FIQ_Addr

			IMPORT	Undef_Handler
			IMPORT	SWI_Handler
			IMPORT	PAbt_Handler
			IMPORT	DAbt_Handler
			IMPORT	IRQ_SaveContext_os
			IMPORT	FIQ_Handler	
			IMPORT  IRQ_Handler	
			IMPORT  SWI_IRQ			
Reset_Addr  DCD   Reset_Handler
Undef_Addr  DCD   Undef_Handler
SWI_Addr    DCD   SWI_IRQ
PAbt_Addr   DCD   PAbt_Handler
DAbt_Addr   DCD   DAbt_Handler
Notuse_Addr DCD   0           ; Reserved Address 
IRQ_Addr	DCD   IRQ_SaveContext_os
Magic_num	DCD	  0x55aa1a25
FIQ_Addr	DCD	  FIQ_Handler
IRQ_SaveContext
;保存中断上下文，支持中断嵌套				
			SUB	LR, LR, #4 ; 计算返回地址
			STMFD SP!, {R0-R12, LR} ;所有寄存器压栈保存
			MRS 	R0, SPSR ; 保存中断前的CPSR(即现在的SPSR)
			STMFD	SP!, {R0} ;
			MSR	CPSR_cxsf, #Mode_SYS+I_Bit ; 切换到系统模式	
			STMFD 	SP!, {LR} ; 压栈系统模式LR
				
			LDR 	R0, =IRQ_Handler ;系统模式下进行IRQ代码处理
			MOV		LR, PC ; 准备函数的返回地址
 			BX		R0 ; 调用中断处理函数
			
			LDMFD	SP!, {LR} ; 出栈系统模式LR
			MSR	CPSR_cxsf, #Mode_IRQ+I_Bit ; 切换到IRQ模式					
			LDMFD 	SP!, {R0} ; 返回中断前的CPSR				
			MSR   	SPSR_cxsf, R0
			LDMFD  SP!, {R0-R12, PC}^ ; ^表示同时把spsr恢复给cpsr

        EXPORT  Reset_Handler
Reset_Handler
;/***********************************************************************/
; 看门狗关闭
		LDR     R0, =WT_BASE 
		LDR     R1, =0
		STR     R1, [R0]	

;/***********************************************************************/
; 关闭所有外设中断
		LDR		R0, =INT_BASE
		LDR		R1, =0xFFFFFFFF
		STR		R1, [R0, #INTMSK1_OFS]
		STR		R1, [R0, #INTMSK2_OFS]
		STR		R1, [R0, #INTSUBMSK_OFS]

		LDR		SP, =Stack_Top 
;/************************************************************************/				
; MMU初始化				
		IMPORT	MMU_Init 
		BL		MMU_Init

;/*************************************************************************/
; 堆栈初始化
        LDR     R0, =Stack_Top

;  Enter Undefined Instruction Mode and set its Stack Pointer
        MSR     CPSR_c, #Mode_UND:OR:I_Bit:OR:F_Bit
        MOV     SP, R0
        SUB     R0, R0, #UND_Stack_Size

;  Enter Abort Mode and set its Stack Pointer
        MSR     CPSR_c, #Mode_ABT:OR:I_Bit:OR:F_Bit
        MOV     SP, R0
        SUB     R0, R0, #ABT_Stack_Size

;  Enter FIQ Mode and set its Stack Pointer
        MSR     CPSR_c, #Mode_FIQ:OR:I_Bit:OR:F_Bit
        MOV     SP, R0
        SUB     R0, R0, #FIQ_Stack_Size

;  Enter IRQ Mode and set its Stack Pointer
        MSR     CPSR_c, #Mode_IRQ:OR:I_Bit:OR:F_Bit
        MOV     SP, R0
        SUB     R0, R0, #IRQ_Stack_Size

;  Enter Supervisor Mode and set its Stack Pointer
        MSR     CPSR_c, #Mode_SVC:OR:I_Bit:OR:F_Bit
        MOV     SP, R0
        SUB     R0, R0, #SVC_Stack_Size

;  Enter System Mode and set its Stack Pointer
        MSR     CPSR_c, #Mode_SYS
        MOV     SP, R0
        SUB     SL, SP, #USR_Stack_Size

; 是否使用了KEIL的微库
        IF      :DEF:__MICROLIB
        EXPORT  __initial_sp
        ELSE
        MOV     SP, R0
        SUB     SL, SP, #USR_Stack_Size
        ENDIF
				
;/***********************************************************************/
; 绝对地址跳转到c入口
       IMPORT  __main
       LDR     R0, =__main
       BX      R0

       IF      :DEF:__MICROLIB

       EXPORT  __heap_base
       EXPORT  __heap_limit

       ELSE
; User Initial Stack & Heap
       AREA    |.text|, CODE, READONLY

       IMPORT  __use_two_region_memory
       EXPORT  __user_initial_stackheap
__user_initial_stackheap

       LDR     R0, =  Heap_Mem
       LDR     R1, =(Stack_Mem + USR_Stack_Size)
       LDR     R2, = (Heap_Mem +      Heap_Size)
       LDR     R3, = Stack_Mem
       BX      LR
       ENDIF

	   END






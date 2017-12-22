
		PRESERVE8               
        AREA  OS_CPU_A, CODE, READONLY
;***************************************************************************
;        START MULTITASKING
;    void OSStartHighRdy(void)
;(1) 禁止中断切换到管理模式,所有任务均工作在管理模式
;(2) 调用任务切换钩子函数，即先调用OSTaskSwHook()函数
;(3) 标记uCOS-II内核已启动运行，OSRunning = 1
;(4) 获得最高优先级任务TCB，得到任务栈指针，SP切换到任务栈
;(5) 出栈SP中的任务栈，包括任务状态寄存器CPSR，R0-R12，LR，继续执行任务。
;***************************************************************************
I_Bit           EQU     0x80 ; IRQ中断禁止位
F_Bit           EQU     0x40 ; FIQ中断禁止位
Mode_SVC		EQU		0x13 ; 管理模式
Mode_SYS		EQU		0x1f ; 系统模式
	IMPORT	OSTaskSwHook
    IMPORT  OSRunning
    IMPORT  OSTCBHighRdy
	EXPORT  OSStartHighRdy
OSStartHighRdy
	MSR     CPSR_c, #(I_Bit+F_Bit+Mode_SVC)	; 禁止中断切换到管理模式

	LDR     R0, =OSTaskSwHook ; 调用任务切换钩子函数
	MOV     LR, PC            ; 准备函数返回地址
	BX      R0	          ; 支持Thumb、ARM混编

	LDR		R0, =OSRunning    ;设置OSRunning为1
	MOV		R1, #1
	STRB	R1, [R0]

	LDR     R0, =OSTCBHighRdy     ; 获得最高优先级任务TCB
	LDR     R0, [R0]              ; 获得任务栈指针
	LDR     SP, [R0]              ; 切换到新任务栈

	LDMFD   SP!, {R0}             ; 出栈新任务的CPSR
	MSR     SPSR_cxsf, R0
	LDMFD   SP!, {R0-R12, LR, PC}^ ; 出栈新任务的上下文

;*************************************************************************************
;    PERFORM A CONTEXT SWITCH (From task level) - OSCtxSw()
;     PERFORM A CONTEXT SWITCH (From interrupt level) - OSIntCtxSw()
;             void OSCtxSw(void)   void OSIntCtxSw(void)
;(1) 保存当前任务的上下文(R0-R12，LR，任务打断的PC地址，状态寄存器CPSR)到当前任务栈中
;(2) 根据当前任务TCB(任务控制块)，获得当前任务栈指针，并把当前任务SP栈保存进栈指针
;(3) 调用任务切换钩子函数，即先调用OSTaskSwHook()函数
;(4) 把即将运行的最高优先级任务优先级更新到当前优先级变量中
;(5) 把即将运行的最高优先级任务TCB(任务控制块)地址更新到当前TCB(任务控制块)地址变量中
;(6) 获得最高优先级任务栈指针，SP切换到最高优先级任务栈，并出栈新任务的上下文，执行新任务	 
;****************************************************************************************
Mode_THUMB	EQU		0x20 ; THUMB模式
	IMPORT  OSTCBCur
    IMPORT  OSTCBHighRdy
    IMPORT  OSPrioCur
    IMPORT  OSPrioHighRdy

    EXPORT  OSCtxSw
	EXPORT  OSIntCtxSw
OSCtxSw
	STMFD   SP!, {LR} ; 压栈当前任务PC
	STMFD   SP!, {LR} ; 压栈当前任务LR
	STMFD   SP!, {R0-R12}  ; 压栈当前任务R0-R12
	MRS     R0, CPSR       ; 获得当前任务CPSR
	TST     LR, #1         ; 测试任务是否工作在Thumb模式
	ORRNE   R0, R0, #Mode_THUMB  ; 是Thumb则状态改成Thumb模式
	STMFD   SP!, {R0}            ; 压栈CPSR

	LDR     R0, =OSTCBCur     ; 获得当前任务TCB
	LDR     R1, [R0]          ; 由TCB获得当前任务栈指针
	STR     SP, [R1]          ; SP栈保存进当前任务栈指针

OSIntCtxSw
	LDR     R0, =OSTaskSwHook ; 调用任务切换钩子函数
	MOV     LR, PC            ; 准备函数返回地址
	BX      R0

	LDR     R0, =OSPrioCur    ; 获得当前任务优先级保存指针
	LDR     R1, =OSPrioHighRdy ; 获得最高优先级任务优先级保存指针
	LDRB    R2, [R1]           ; 获得最高优先级任务优先级
	STRB    R2, [R0]           ; 保存进当前任务优先级指针变量中

	LDR     R0, =OSTCBCur      ; 获得当前任务TCB保存指针
	LDR     R1, =OSTCBHighRdy  ; 获得最高优先级任务TCB保存指针
	LDR     R2, [R1]           ; 最高优先级TCB地址保存进当前任务TCB指针
	STR     R2, [R0]

	LDR     SP, [R2]             ; SP切换到最高优先级任务栈

	LDMFD   SP!, {R0}            ; 出栈新任务的CPSR
	MSR     SPSR_cxsf, R0
	LDMFD   SP!, {R0-R12, LR, PC}^  ; 出栈新任务的上下文

;**************************************************************************************
;    CRITICAL SECTION FUNCTIONS
; 临界区访问从局部变量保存/恢复中断状态再开关中断
;**************************************************************************************
	EXPORT  CPU_SR_Save
	EXPORT  CPU_SR_Restore
CPU_SR_Save
	MRS 	R0, CPSR
	ORR 	R1, R0, #0xC0 ; 设置IRQ,FIQ均禁止中断
	MSR 	CPSR_c, R1
	BX  	LR	      ; 禁止中断,返回中断状态到R0中

CPU_SR_Restore		
	MSR     CPSR_c, R0
	BX  	LR


	EXPORT  System_call
System_call
	SWI		12
	BX  	LR
	
	IMPORT SWI_Handler
	EXPORT  SWI_IRQ
SWI_IRQ
	STMFD	SP!, {R1-R12,  LR}
	LDR 	R4, [LR, #-4]                                   ; LR - 4 为指令" swi xxx" 的地址，低
	BIC   	R4, R4, #0xFF000000                    ; 取得ARM指令24位立即数
    LDR     R2, =SWI_Handler
	MOV		LR, PC 
	BX		R2
	ADD		R0, R0, R4
	LDMIA     SP!, {R1-R12, PC}^  
	
;****************************************************************************************
;   IRQ处理
; 其它异常均应类型时行上下文保存处理
;(1) 临时性使用到一些寄存器，对用到的寄存器压栈到IRQ栈上
;(2) 切换到管理模式，禁止中断，任务运行在管理模式，这步将切换SP到被中断打断的任务栈上
;(3) 把被打断任务的上下文压入任务的栈
;(4) 跟踪中断嵌套计数,判断是任务被中断还是中断嵌套,中断嵌套不用更新任务栈
;(5) 非中断嵌套,根据当前任务TCB(任务控制块)获得栈指针，并把打断任务SP栈保存进栈指针
;(6) 调用OSIntEnter()函数进行中断嵌套加计数
;(7) 切换到系统模式，并压栈LR，这步是为了使用系统模式栈来处理中断函数，减轻任务栈的使用。
;(8) 调用IRQ_Handler()函数实质处理IRQ中断服务，在中断服务中可再打开IRQ中断，支持中断嵌套 
;(9) 中断服务执行完后，出栈LR，并切换到管理模式，禁止中断，此时SP将切换到被打断任务的任务栈上
;(10) 调用OSIntExit()函数进行中断嵌套减计数，如果中断嵌套计数OSIntNesting为0，则说明所有中断
;    退出，将调用OSIntCtxSw()进行中断级任务切换，继续执行任务
;(11) 如果中断嵌套计数OSIntNesting不为0，中断未全部退出，则出栈上一个中断的上下文，执行被
;     嵌套的上一级中断。
;*****************************************************************************************
    IMPORT  OSIntEnter
    IMPORT  OSIntExit
	IMPORT	OSIntNesting
    IMPORT  IRQ_Handler
	
	
	IMPORT	is_system_ticks
		
	
    EXPORT  IRQ_SaveContext_os
IRQ_SaveContext_os
	SUB     LR, LR, #4                  ; IRQ异常返回地址LR-4
	STMFD   SP!, {R0-R2}                ; 临时使用的工作寄存器压入IRQ栈
	MRS     R0, SPSR                    ; 保存异常出现前的CPSR
	MOV     R1, LR                      ; 保存LR
	MOV     R2, SP                      ; 保存IRQ栈指针,用来出栈工作寄存器
	ADD     SP, SP, #(3 * 4)            ; 恢复IRQ栈原来的位置
                                                                
	MSR     CPSR_c, #(I_Bit+F_Bit+Mode_SVC)	; 禁止中断、切换到管理模式保存现场

	STMFD   SP!, {R1}                   ; 压栈打断任务的PC
	STMFD   SP!, {LR}                   ; 压栈打断任务的LR
	STMFD   SP!, {R3-R12}               ; 压栈打断任务的R12-R3
	LDMFD   R2!, {R5-R7}                ; 从IRQ栈恢复R2-R0
	STMFD   SP!, {R5-R7}                ; 压栈打断任务的R2-R0
	STMFD   SP!, {R0}                   ; 压栈打断任务的CPSR
	
	;LDR		R0, =OSIntNesting           ; 获得中断嵌套计数
	;LDRB	R1, [R0]
	;CMP		R1, #0                      ; 判断任务被中断还是中断嵌套
	;BNE		IntteruptNesting            ; 中断嵌套不用更新任务栈指针
	
	LDR     R0, =OSTCBCur               ; 任务被中断打断,获得打断任务TCB
	LDR     R1, [R0]                    ; 获得被打断任务栈指针
	STR     SP, [R1]                    ; SP栈保存进打断任务栈指针
	
;IntteruptNesting	
	;LDR		R0, =OSIntEnter             ; 调用OSIntEnter()进行中断嵌套计数
	;MOV		LR, PC
	;BX		R0

;开始处理中断
 	MSR     CPSR_c, #(I_Bit+F_Bit+Mode_SYS)	; 切换到系统模式,使用系统模式栈处理中断
	STMFD 	SP!, {LR} ; 压栈系统模式LR
	
	LDR		R0, =IRQ_Handler             ; 调用IRQ处理函数
	MOV		LR, PC 
	BX		R0

	LDMFD	SP!, {LR} ; 出栈系统模式LR  
	
	
	MSR     CPSR_c, #(I_Bit+F_Bit+Mode_SVC)	; 切换到管理模式,使用任务栈进行出栈	
	
	LDR		R0, =is_system_ticks           ; 判断是不是系统心跳中断
	MOV		LR, PC 
	BX		R0

IntReturn
	LDMFD   SP!, {R0}                     ; 中断发生嵌套,出栈上一个中断的上下文
	MSR     SPSR_cxsf, R0
	LDMFD   SP!, {R0-R12, LR, PC}^

    END


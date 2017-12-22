
; DRAM controller base address
DRAM_BASE		EQU 	0x48000000
BANKCFG_OFS		EQU		0x00
BANKCON1_OFS		EQU		0x04
BANKCON2_OFS		EQU		0x08
BANKCON3_OFS		EQU		0x0C
REFRESH_OFS		EQU		0x10
TIMEOUT_OFS		EQU		0x14

        PRESERVE8
        AREA    LOWLEVELINIT, CODE, READONLY
        ARM
		EXPORT  ERAM_Init
; K4T51163QJ-BCE79(DDR2@400M 5-5-5),64MB,Raw Addr A0~A12,Column Addr A0~A9
; 设置DDR0 13位行地址，10位列地址，DDR2接口，16位总线
; DDR命令根据nRAS,nCAS,nWE,nCS控制总线分辨
; Active命令,打开行及bank地址
; Read命令,在Active后,打开列地址读
; Write命令,在Active后,打开列地址写
; Precharge命令,关闭bank,根据A[10]确定关闭指定或所有bank
; AUTO REFRESH or SELF REFRESH命令,刷新命令
; LOAD MODE REGISTER命令,写模式寄存器
ERAM_Init
		LDR		R0, =DRAM_BASE
		LDR		R1, =(2<<17)+(2<<11)+(0<<6)+(1<<1)+(1<<0)
		STR		R1, [R0, #BANKCFG_OFS]
; DQS delay 3,Write buffer,Auto pre-charge,bank address 在高位 				
		LDR		R1, =(3<<28)+(1<<26)+(1<<8)+(0<<7)+ \
							 (1<<6)+(0<<5)+(1<<4)
		STR		R1, [R0, #BANKCON1_OFS]		
				
; s3c2416 ddr2寄存器的clk设置值是相对HCLK的
; RAS [23:20] Row active time 45ns HCLK=133M DDR2=266M 6clock
; Active命令到Precharge命令的最小时间45ns
; ARFC [19:16] Row cycle time tRFC=105ns 14clock
; 指令刷新时间105ns
; CAS Latency [5:4] CAS latency control 12.5ns 2clock
; Read/Write命令发出后经过5tCK=12.5ns数据才有效
; tRCD [3:2] RAS to CAS delay 12.5ns 2clock
; Active命令需经5tCK=12.5ns后才发出Read/Write命令
; tRP [1:0] Row pre-charge time 12.5ns 2clock
; Precharge命令到发送Active命令5tCK=12.5ns
; 故两个Active命令所需的最小时间 tRC=tRAS+tRP=57.5ns
		LDR		R1, =(6<<20)+(13<<16)+(3<<4)+(2<<2)+(2<<0)
		STR		R1, [R0, #BANKCON2_OFS]	

; issue a PALL(pre-charge all) command,即Precharge命令
		LDR		R1, [R0, #BANKCON1_OFS]
		BIC		R1, R1, #0x03
		ORR		R1, R1, #0x01
		STR		R1, [R0, #BANKCON1_OFS]

; issue an EMRS(extern mode register) command to EMR(2)
		LDR		R1, =(0x2<<30)+(0<<23)+(0<<19)+(0<<16)
		STR		R1, [R0, #BANKCON3_OFS]
		LDR		R1, [R0, #BANKCON1_OFS]
		ORR		R1, R1, #0x03				
		STR		R1, [R0, #BANKCON1_OFS]

; issue an EMRS(extern mode register) command to EMR(3)
		LDR		R1, =(0x3<<30)
		STR		R1, [R0, #BANKCON3_OFS]
		LDR		R1, [R0, #BANKCON1_OFS]
		ORR		R1, R1, #0x03				
		STR		R1, [R0, #BANKCON1_OFS]
				
; issue an EMRS to enable DLL and RDQS, nDQS, ODT disable
		LDR		R1, =0xFFFF0000
		LDR		R2, [R0, #BANKCON3_OFS]
		BIC		R2, R2, R1
		LDR		R1, =(0x1<<30)+(0<<28)+(0<<27)+(1<<26)+ \
					 (7<<23)+(0<<19)+(0<<22)+(0<<18)+ \
					 (0x0<<17)+(0<<16)				
		ORR		R1, R1, R2
		STR		R1, [R0, #BANKCON3_OFS]
		LDR		R1, [R0, #BANKCON1_OFS]
		ORR		R1, R1, #0x03				
		STR		R1, [R0, #BANKCON1_OFS]

; issue a mode register set command for DLL reset 
		LDR		R1, =0x0000FFFF
		LDR		R2, [R0, #BANKCON3_OFS]
		BIC		R2, R2, R1
		LDR		R1, =(0x1<<9)+(1<<8)+(0<<7)+(3<<4)						
		ORR		R1, R1, R2
		STR		R1, [R0, #BANKCON3_OFS]
		LDR		R1, [R0, #BANKCON1_OFS]
		BIC		R1, R1, #0x03
		ORR		R1, R1, #0x02				
		STR		R1, [R0, #BANKCON1_OFS]
				
; Issue a PALL(pre-charge all) command	
		LDR		R1, [R0, #BANKCON1_OFS]
		BIC		R1, R1, #0x03
		ORR		R1, R1, #0x01
		STR		R1, [R0, #BANKCON1_OFS]
				
; Issue 2 or more auto-refresh commands
		LDR		R1, =0x20
		STR		R1, [R0, #REFRESH_OFS]
				

; Issue a MRS command with LOW to A8 to initialize device operation
		LDR		R1, =0x0000FFFF
		LDR		R2, [R0, #BANKCON3_OFS]
		BIC		R2, R2, R1
		LDR		R1, =(0x1<<9)+(0<<8)+(0<<7)+(3<<4)					
		ORR		R1, R1, R2
		STR		R1, [R0, #BANKCON3_OFS]
		LDR		R1, [R0, #BANKCON1_OFS]
		BIC		R1, R1, #0x03
		ORR		R1, R1, #0x02				
		STR		R1, [R0, #BANKCON1_OFS]
				
; Wait 200 clock, execute OCD Calibration
		LDR		R1, =200
0		SUBS		R1, R1, #1
		BNE		%B0
				
; Issue a EMRS1 command to over OCD Mode Calibration
		LDR		R1, =0xFFFF0000
		LDR		R2, [R0, #BANKCON3_OFS]
		BIC		R2, R2, R1
		LDR		R1, =(0x1<<30)+(0<<28)+(0<<27)+(1<<26)+ \
					 (0<<23)+(0<<19)+(0<<22)+(0<<18)+ \
					 (0x0<<17)+(0<<16)				
		ORR		R1, R1, R2
		STR		R1, [R0, #BANKCON3_OFS]
		LDR		R1, [R0, #BANKCON1_OFS]
		ORR		R1, R1, #0x03				
		STR		R1, [R0, #BANKCON1_OFS]

; Refresh period is 7.8us, HCLK=100M, REFCYC=780
		LDR		R1, =780
		STR		R1, [R0, #REFRESH_OFS]

; issue a Normal mode
		LDR		R1, [R0, #BANKCON1_OFS]
		BIC		R1, R1, #0x03
		STR		R1, [R0, #BANKCON1_OFS]
				
		BX		LR
				

; SD/MMC Device Boot Block Assignment
eFuseBlockSize  		EQU		1
SdReservedBlockSize		EQU		1
BL1BlockSize			EQU		16
SdBL1BlockStart		EQU		SdReservedBlockSize + \
						eFuseBlockSize + BL1BlockSize
globalBlockSizeHide		EQU		0x40003FFC
CopyMovitoMem			EQU		0x40004008

; Nand controller base address
NFCONF			EQU		0x4E000000

				EXPORT	__CodeSize__
				EXPORT	__CodeAddr__
; 引出代码搬移函数，以供启动代码调用
				EXPORT  CopyCodeToRAM
; 引入Nand启动的初始化函数及Nand读函数
				IMPORT	Nand_Init
				IMPORT	Nand_ReadSkipBad
; 引入链接器产生符号，以确定代码运行位置，编译生成的大小
				IMPORT  ||Image$$ER_ROM0$$Base||
				IMPORT  ||Load$$ER_ROM0$$Length||	
				IMPORT  ||Load$$ER_ROM1$$Length||	
				IMPORT  ||Load$$RW_RAM$$RW$$Length||

; 链接器产生代码链接运行位置
__CodeAddr__	DCD		||Image$$ER_ROM0$$Base||
; 链接器各个段需保存的代码以及需初始代的变量大小
__CodeSize__	DCD		||Load$$ER_ROM0$$Length|| + \
				||Load$$ER_ROM1$$Length|| + \
				||Load$$RW_RAM$$RW$$Length||	
									
CopyCodeToRAM
		STMFD	SP!, {LR} ; 保存返回地址
; 判断NFCONF的最高位为1，说明启动设备为Nand		
		LDR		R0, =NFCONF
		LDR		R1, [R0]
		AND		R1, R1, #0x80000000
		CMP		R1, #0x80000000
		BNE		MMC_SD_Boot	

Nand_Boot		
		BL		Nand_Init ; Nand初始化
		MOV		R0, #0
		LDR		R1, __CodeAddr__	
		LDR		R2, __CodeSize__
		BL		Nand_ReadSkipBad; 调用Nand读函数
		MOVS	R0, R0 ; 返回值确定函数成功还是失败
Nand_Boot_Loop	
		BNE		Nand_Boot_Loop; 返回非0说明拷贝失败
		B		AfterCopy
		
MMC_SD_Boot
; 不需要卡初始化
		LDR		R3, =0 
; 拷贝sd卡代码到链接执行域内存代码处
		LDR		R2, __CodeAddr__
				
; 计算代码的大小，以block计，不足512字节的算1个block
; 代码的大小包括Code RO-data RW-data(代码需保存需初始化的RW的初始值)
; 代码保存在ROM中,应从加载域得到ROM的大小,而不是执行域,编译器可能压缩
; 代码段保存在加载域的ROM中
		LDR		R0, __CodeSize__
		LDR		R1, =0x1ff
		TST		R0, R1 ; 是否不足一个block(512Bytes)
		BEQ		%F0 ; 代码恰好block对齐，不用加多一个block
		ADD		R0, R0, #512				
0		LSR		R1, R0, #9 ; 得到代码的block大小

; 计算代码在SD/MMC卡中的block起启地址
		LDR		R4, =SdBL1BlockStart	
		LDR		R0, =globalBlockSizeHide
		LDR		R0, [R0] ; SD/MMC的总block块
		SUB		R0, R4 ; 减去保留块及BL1大小
		CMP		R1, #16 ; 代码不足8k,直接BL1处拷贝
		BLS		ParameterOK ; 代码少于16个block跳转	
		SUB		R0, R1 ; 再减去代码的大小，代码的block位置

; 调用IROM Movi拷贝函数，仅适用于IROM启动，卡访问时钟25M	
ParameterOK
		LDR		R4, =CopyMovitoMem
		LDR		R4, [R4]
		MOV		LR, PC ; 准备函数的返回地址
		BX		R4
		MOVS	R0, R0 ; 返回值确定函数成功还是失败
MMC_SD_Boot_Loop			
		BEQ		MMC_SD_Boot_Loop ; 返回0说明拷贝失败
				
AfterCopy	
		LDMFD	SP!, {PC} ; 函数返回			

		END
				
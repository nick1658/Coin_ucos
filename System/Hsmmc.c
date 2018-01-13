#include "s3c2416.h"
//#define DEBUG_HSMMC
#ifdef  DEBUG_HSMMC
#define Debug cy_println
#else
#define Debug(x...) 
#endif

static unsigned char CardType; // 卡类型
static unsigned int RCA; // 卡相对地址
// Hsmmc缓存,内存空间关cache,以作DMA传输,声明从段section("No_Cache")分配内存,16k
__align(4) static unsigned char Hsmmc_Buffer[16*1024] __attribute__((section("No_Cache"), zero_init));

__asm void Delay_us(unsigned int nCount) 
{
//延时1us,共延时nCount(R0) us
Delay1	
		LDR  R1, =100  // Arm clock为400M		
Delay2
		SUBS R1, R1, #1  // 一个Arm clock
		BNE  Delay2      // 跳转会清流水线，3个Arm clock
		SUBS R0, R0, #1	 // 调用者确保nCount不为0
		BNE	 Delay1
		BX   LR
}


static void Hsmmc_ClockOn(unsigned char On)
{
	if (On) {
		rHM1_CLKCON |= (1<<2); // sd时钟使能
		while (!(rHM1_CLKCON & (1<<3))) {
			// 等待SD输出时钟稳定
		}
	} else {
		rHM1_CLKCON &= ~(1<<2); // sd时钟禁止
	}
}

static void Hsmmc_SetClock(unsigned int Div)
{
	Hsmmc_ClockOn(0); // 关闭时钟	
	// 选择SCLK_HSMMC:EPLLout
	rCLKSRC &= ~(1<<17); // HSMMC1 EPLL(96M)
	rHM1_CONTROL2 = 0xc0000120; // SCLK_HSMMC
	rHM1_CONTROL3 = (0<<31) | (0<<23) | (0<<15) | (0<<7);
	// SDCLK频率值并使能内部时钟
	rHM1_CLKCON &= ~(0xff<<8);
	rHM1_CLKCON |= (Div<<8) | (1<<0);
	while (!(rHM1_CLKCON & (1<<1))) {
		// 等待内部时钟振荡稳定
	}
	Hsmmc_ClockOn(1); // 全能时钟

}

static int Hsmmc_WaitForCommandDone()
{
	unsigned int i;	
	int ErrorState;
	// 等待命令发送完成
	for (i=0; i<20000000; i++) {
		if (rHM1_NORINTSTS & (1<<15)) { // 出现错误
			break;
		}
		if (rHM1_NORINTSTS & (1<<0)) {
			do {
				rHM1_NORINTSTS = (1<<0); // 清除命令完成位
			} while (rHM1_NORINTSTS & (1<<0));			
			return 0; // 命令发送成功
		}
	}
	ErrorState = rHM1_ERRINTSTS & 0x1ff; // 可能通信错误,CRC检验错误,超时等
	rHM1_NORINTSTS = rHM1_NORINTSTS; // 清除中断标志
	rHM1_ERRINTSTS = rHM1_ERRINTSTS; // 清除错误中断标志	
	do {
		rHM1_NORINTSTS = (1<<0); // 清除命令完成位
	} while (rHM1_NORINTSTS & (1<<0));
	
	Debug("Command error, rHM1_ERRINTSTS = 0x%x ", ErrorState);	
	return ErrorState; // 命令发送出错	
}

static int Hsmmc_WaitForTransferDone()
{
	int ErrorState;
	unsigned int i;
	// 等待数据传输完成
	for (i=0; i<20000000; i++) {
		if (rHM1_NORINTSTS & (1<<15)) { // 出现错误
				break;
		}											
		if (rHM1_NORINTSTS & (1<<1)) { // 数据传输完								
			do {
				rHM1_NORINTSTS |= (1<<1); // 清除传输完成位
			} while (rHM1_NORINTSTS & (1<<1));	
			rHM1_NORINTSTS = (1<<3); // 清除DMA中断标志								
			return 0;
		}
		Delay_us(1);
	}

	ErrorState = rHM1_ERRINTSTS & 0x1ff; // 可能通信错误,CRC检验错误,超时等
	rHM1_NORINTSTS = rHM1_NORINTSTS; // 清除中断标志
	rHM1_ERRINTSTS = rHM1_ERRINTSTS; // 清除错误中断标志
	Debug("Transfer error, rHM1_ERRINTSTS = 0x%04x\n\r", ErrorState);	
	do {
		rHM1_NORINTSTS = (1<<1); // 出错后清除数据完成位
	} while (rHM1_NORINTSTS & (1<<1));
	
	return ErrorState; // 数据传输出错		
}

static int Hsmmc_IssueCommand(unsigned char Cmd, unsigned int Arg, unsigned char Data, unsigned char Response)
{
	unsigned int i;
	unsigned int Value;
	unsigned int ErrorState;
	// 检查CMD线是否准备好发送命令
	for (i=0; i<1000000; i++) {
		if (!(rHM1_PRNSTS & (1<<0))) {
			break;
		}
	}
	if (i == 1000000) {
		Debug("CMD line time out, rHM1_PRNSTS: %04x\n\r", rHM1_PRNSTS);
		return -1; // 命令超时
	}
	// 检查DAT线是否准备好
	if (Response == Response_R1b) { // R1b回复通过DAT0反馈忙信号
		for (i=0; i<1000000; i++) {
			if (!(rHM1_PRNSTS & (1<<1))) {
				break;
			}		
		}
		if (i == 1000000) {
			Debug("Data line time out, rHM1_PRNSTS: %04x\n\r", rHM1_PRNSTS);			
			return -2;
		}		
	}

	rHM1_ARGUMENT = Arg; // 写入命令参数
	Value = (Cmd << 8); // command index
	// CMD12可终止传输
	if (Cmd == 0x12) {
		Value |= (0x3 << 6); // command type
	}
	if (Data) {
		Value |= (1 << 5); // 需使用DAT线作为传输等
	}	
	
	switch (Response) {
	case Response_NONE:
		Value |= (0<<4) | (0<<3) | 0x0; // 没有回复,不检查命令及CRC
		break;
	case Response_R1:
	case Response_R5:
	case Response_R6:
	case Response_R7:		
		Value |= (1<<4) | (1<<3) | 0x2; // 检查回复中的命令,CRC
		break;
	case Response_R2:
		Value |= (0<<4) | (1<<3) | 0x1; // 回复长度为136位,包含CRC
		break;
	case Response_R3:
	case Response_R4:
		Value |= (0<<4) | (0<<3) | 0x2; // 回复长度48位,不包含命令及CRC
		break;
	case Response_R1b:
		Value |= (1<<4) | (1<<3) | 0x3; // 回复带忙信号,会占用Data[0]线
		break;
	default:
		break;	
	}
	rHM1_CMDREG = Value;
	
	ErrorState = Hsmmc_WaitForCommandDone();
	if (ErrorState) {
		Debug("Command = %d\r\n", Cmd);
	}	
	return ErrorState; // 命令发送出错
}

// 512位的sd卡扩展状态位
int Hsmmc_GetSdState(unsigned char *pState)
{
	int ErrorState;
	unsigned int i;
	if (CardType == SD_HC || CardType == SD_V2 || CardType == SD_V1) {
		if (Hsmmc_GetCardState() != 4) { // 必需在transfer status
			return -1; // 卡状态错误
		}		
		Hsmmc_IssueCommand(CMD55, RCA<<16, 0, Response_R1);
		
		rHM1_SYSAD = (unsigned int)Hsmmc_Buffer; // 缓存地址	
		rHM1_BLKSIZE = (7<<12) | (64<<0); // 最大DMA缓存大小,block为512位64字节			
		rHM1_BLKCNT = 1; // 写入这次读1 block的sd状态数据
		rHM1_ARGUMENT = 0; // 写入命令参数	

		// DMA传输使能,读单块		
		rHM1_TRNMOD = (0<<5) | (1<<4) | (0<<2) | (1<<1) | (1<<0);	
		// 设置命令寄存器,读状态命令CMD13,R1回复
		rHM1_CMDREG = (CMD13<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;
		ErrorState = Hsmmc_WaitForCommandDone();
		if (ErrorState) {
			Debug("CMD13 error\r\n");
			return ErrorState;
		}
		ErrorState = Hsmmc_WaitForTransferDone();
		if (ErrorState) {
			Debug("Get sd status error\r\n");
			return ErrorState;
		}
		for (i=0; i<64; i++) {
			*pState++ = Hsmmc_Buffer[i];
		}
		return 0;
	}
	return -1; // 非sd卡
}

int Hsmmc_Get_CSD(unsigned char *pCSD)
{
	unsigned int i;
	unsigned int Response[4];
	int State = 1;

	if (CardType != SD_HC && CardType != SD_V1 && CardType != SD_V2) {
		return State; // 未识别的卡
	}
	// 取消卡选择,任何卡均不回复,已选择的卡通过RCA=0取消选择,
	// 卡回到stand-by状态
	Hsmmc_IssueCommand(CMD7, 0, 0, Response_NONE);
	for (i=0; i<1000; i++) {
		if (Hsmmc_GetCardState() == 3) { // CMD9命令需在standy-by status
			Debug("Get CSD: Enter to the Stand-by State\n\r");					
				break; // 状态正确
		}
			Delay_us(100);
	}
	if (i == 1000) {
		return State; // 状态错误
	}	
	// 请求已标记卡发送卡特定数据(CSD),获得卡信息
	if (!Hsmmc_IssueCommand(CMD9, RCA<<16, 0, Response_R2)) {
		pCSD++; // 路过第一字节,CSD中[127:8]位对位寄存器中的[119:0]
		Response[0] = rHM1_RSPREG0;
		Response[1] = rHM1_RSPREG1;
		Response[2] = rHM1_RSPREG2;
		Response[3] = rHM1_RSPREG3;	
		Debug("CSD: ");
		for (i=0; i<15; i++) { // 拷贝回复寄存器中的[119:0]到pCSD中
			*pCSD++ = ((unsigned char *)Response)[i];
			Debug("%02x", *(pCSD-1));
		}
		State = 0; // CSD获取成功
	}

	Hsmmc_IssueCommand(CMD7, RCA<<16, 0, Response_R1); // 选择卡,卡回到transfer状态
	return State;
}

// R1回复中包含了32位的card state,卡识别后,可在任一状态通过CMD13获得卡状态
int Hsmmc_GetCardState(void)
{
	if (Hsmmc_IssueCommand(CMD13, RCA<<16, 0, Response_R1)) {
		return -1; // 卡出错
	} else {
		return ((rHM1_RSPREG0>>9) & 0xf); // 返回R1回复中的[12:9]卡状态
	}
}


static int Hsmmc_SetBusWidth(unsigned char Width)
{
	int State;
	if ((Width != 1) || (Width != 4)) {
		return -1;
	}
	State = -1; // 设置初始为未成功
	rHM1_NORINTSTSEN &= ~(1<<8); // 关闭卡中断
	Hsmmc_IssueCommand(CMD55, RCA<<16, 0, Response_R1);
	if (Width == 1) {
		if (!Hsmmc_IssueCommand(CMD6, 0, 0, Response_R1)) { // 1位宽
			rHM_HOSTCTL &= ~(1<<1);
			State = 0; // 命令成功
		}
	} else {
		if (!Hsmmc_IssueCommand(CMD6, 2, 0, Response_R1)) { // 4位宽
			rHM_HOSTCTL |= (1<<1);
			State = 0; // 命令成功
		}
	}
	rHM1_NORINTSTSEN |= (1<<8); // 打开卡中断	
	return State; // 返回0为成功
}

int Hsmmc_EraseBlock(unsigned int StartBlock, unsigned int EndBlock)
{
	unsigned int i;

	if (CardType == SD_V1 || CardType == SD_V2) {
		StartBlock <<= 9; // 标准卡为字节地址
		EndBlock <<= 9;
	} else if (CardType != SD_HC) {
		return -1; // 未识别的卡
	}	
	Hsmmc_IssueCommand(CMD32, StartBlock, 0, Response_R1);
	Hsmmc_IssueCommand(CMD33, EndBlock, 0, Response_R1);
	if (!Hsmmc_IssueCommand(CMD38, 0, 0, Response_R1b)) {
		for (i=0; i<10000; i++) {
			if (Hsmmc_GetCardState() == 4) { // 擦除完成后返回到transfer状态
				Debug("erasing complete!\n\r");
				return 0; // 擦除成功				
			}
			Delay_us(1000);			
		}		
	}		

	Debug("Erase block failed\n\r");
	return 1; // 擦除失败
}

int Hsmmc_ReadBlock(unsigned char *pBuffer, unsigned int BlockAddr, unsigned int BlockNumber)
{
	unsigned int Address = 0;
	unsigned int ReadBlock;
	unsigned int i;
	int ErrorState;
	
	if (pBuffer == 0 || BlockNumber == 0) {
		return -1;
	}

	// 均不中断使能,产生相应的中断信号
	rHM1_NORINTSIGEN &= ~0xffff; // 清除所有中断使能
	rHM1_NORINTSIGEN |= (1<<1); // 命令完成中断使能
	
	while (BlockNumber > 0) {
		for (i=0; i<1000; i++) {
			if (Hsmmc_GetCardState() == 4) { // 读写数据需在transfer status
				break; // 状态正确
			}
			Delay_us(100);
		}
		if (i == 1000) {
			return -2; // 状态错误
		}		
		
		if (BlockNumber <= sizeof(Hsmmc_Buffer)/512) {
			ReadBlock = BlockNumber; // 读取的块数小于缓存32 Block(16k)
			BlockNumber = 0; // 剩余读取块数为0
		} else {
			ReadBlock = sizeof(Hsmmc_Buffer)/512; // 读取的块数大于32 Block,分多次读
			BlockNumber -= ReadBlock;
		}
		// 根据sd主机控制器标准,按顺序写入主机控制器相应的寄存器
		rHM1_SYSAD = (unsigned int)Hsmmc_Buffer; // 缓存地址,内存区域为关闭cache,作DMA传输			
		rHM1_BLKSIZE = (7<<12) | (512<<0); // 最大DMA缓存大小,block为512字节			
		rHM1_BLKCNT = ReadBlock; // 写入这次读block数目
		

		if (CardType == SD_HC) {
			Address = BlockAddr; // SDHC卡写入地址为block地址
		} else if (CardType == SD_V1 || CardType == SD_V2) {
			Address = BlockAddr << 9; // 标准卡写入地址为字节地址
		}	
		BlockAddr += ReadBlock; // 下一次读块的地址
		rHM1_ARGUMENT = Address; // 写入命令参数		
		
		if (ReadBlock == 1) {
			// 设置传输模式,DMA传输使能,读单块
			rHM1_TRNMOD = (0<<5) | (1<<4) | (0<<2) | (1<<1) | (1<<0);
			// 设置命令寄存器,单块读CMD17,R1回复
			rHM1_CMDREG = (CMD17<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;
		} else {
			// 设置传输模式,DMA传输使能,读多块			
			rHM1_TRNMOD = (1<<5) | (1<<4) | (1<<2) | (1<<1) | (1<<0);
			// 设置命令寄存器,多块读CMD18,R1回复	
			rHM1_CMDREG = (CMD18<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;			
		}	
		ErrorState = Hsmmc_WaitForCommandDone();
		if (ErrorState) {
			Debug("Read Command error\r\n");
			return ErrorState;
		}		
		ErrorState = Hsmmc_WaitForTransferDone();
		if (ErrorState) {
			Debug("Read block error\r\n");
			return ErrorState;
		}
		// 数据传输成功,拷贝DMA缓存的数据到指定内存
		for (i=0; i<ReadBlock*512; i++) {
			*pBuffer++ = Hsmmc_Buffer[i];
		}		
	}
	return 0; // 所有块读完
}

int Hsmmc_WriteBlock(unsigned char *pBuffer, unsigned int BlockAddr, unsigned int BlockNumber)
{
	unsigned int Address = 0;
	unsigned int WriteBlock;
	unsigned int i;
	int ErrorState;
	
	if (pBuffer == 0 || BlockNumber == 0) {
		return -1; // 参数错误
	}
	
	rHM1_NORINTSIGEN &= ~0xffff; // 清除所有中断使能
	// 数据传输完成中断使能
	rHM1_NORINTSIGEN |= (1<<0);
	
	while (BlockNumber > 0) {
		for (i=0; i<1000; i++) {
			if (Hsmmc_GetCardState() == 4) { // 读写数据需在transfer status
				break; // 状态正确
			}
			Delay_us(100);
		}
		if (i == 1000) {
			return -2; // 状态错误或Programming超时
		}
		
		if (BlockNumber <= sizeof(Hsmmc_Buffer)/512) {
			WriteBlock = BlockNumber;// 写入的块数小于缓存32 Block(16k)
			BlockNumber = 0; // 剩余写入块数为0
		} else {
			WriteBlock = sizeof(Hsmmc_Buffer)/512; // 写入的块数大于32 Block,分多次写
			BlockNumber -= WriteBlock;
		}
		if (WriteBlock > 1) { // 多块写,发送ACMD23先设置预擦除块数
			Hsmmc_IssueCommand(CMD55, RCA<<16, 0, Response_R1);
			Hsmmc_IssueCommand(CMD23, WriteBlock, 0, Response_R1);
		}
		
		for (i=0; i<WriteBlock*512; i++) {
			Hsmmc_Buffer[i] = *pBuffer++; // 待写数据从指定内存区拷贝到缓存区
		}
		// 根据sd主机控制器标准,按顺序写入主机控制器相应的寄存器		
		rHM1_SYSAD = (unsigned int)Hsmmc_Buffer; // 缓存地址,内存区域为关闭cache,作DMA传输		
		rHM1_BLKSIZE = (7<<12) | (512<<0); // 最大DMA缓存大小,block为512字节		
		rHM1_BLKCNT = WriteBlock; // 写入block数目	
		
		if (CardType == SD_HC) {
			Address = BlockAddr; // SDHC卡写入地址为block地址
		} else if (CardType == SD_V1 || CardType == SD_V2) {
			Address = BlockAddr << 9; // 标准卡写入地址为字节地址
		}
		BlockAddr += WriteBlock; // 下一次写地址
		rHM1_ARGUMENT = Address; // 写入命令参数			
		
		if (WriteBlock == 1) {
			// 设置传输模式,DMA传输写单块
			rHM1_TRNMOD = (0<<5) | (0<<4) | (0<<2) | (1<<1) | (1<<0);
			// 设置命令寄存器,单块写CMD24,R1回复
			rHM1_CMDREG = (CMD24<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;			
		} else {
			// 设置传输模式,DMA传输写多块
			rHM1_TRNMOD = (1<<5) | (0<<4) | (1<<2) | (1<<1) | (1<<0);
			// 设置命令寄存器,多块写CMD25,R1回复
			rHM1_CMDREG = (CMD25<<8)|(1<<5)|(1<<4)|(1<<3)|0x2;					
		}
		ErrorState = Hsmmc_WaitForCommandDone();
		if (ErrorState) {
			Debug("Write Command error\r\n");
			return ErrorState;
		}		
		ErrorState = Hsmmc_WaitForTransferDone();
		if (ErrorState) {
			Debug("Write block error\r\n");
			return ErrorState;
		}
	}
	return 0; // 写完所有数据
}

static S16 Hsmmc_init_flag = 0;
int Hsmmc_Init()
{
	unsigned int i;
	unsigned int OCR;

	if (Hsmmc_init_flag)
		return 0;
	cy_print ("Init SD Card\n");
	// 设置HSMMC1的接口引脚配置
	rGPLCON &= ~((0xffff<<0) | (0xf<<16));
	rGPLCON |= (0xaaaa<<0) | (0xa<<16);
	rGPLUDP &= ~((0xffff<<0) | (0xf<<16)); // 上下拉禁止
		
	rHM1_SWRST = 0x7; // 复位HSMMC
	Hsmmc_SetClock(0x80); // SDCLK=96M/256=375K
	rHM1_TIMEOUTCON = (0xe << 0); // 最大超时时间
	rHM1_HOSTCTL &= ~(1<<2); // 正常速度模式
	rHM1_NORINTSTS = rHM1_NORINTSTS; // 清除中断状态标志
	rHM1_ERRINTSTS = rHM1_ERRINTSTS; // 清除错误中断状态标志
	rHM1_NORINTSTSEN = 0x7fff; // [14:0]中断使能
	rHM1_ERRINTSTSEN = 0x3ff; // [9:0]错误中断使能
	Hsmmc_IssueCommand(CMD0, 0, 0, Response_NONE); // 复位所有卡到空闲状态
	
	CardType = UnusableCard; // 卡类型初始化不可用
	if (Hsmmc_IssueCommand(CMD8, 0x1aa, 0, Response_R7)) { // 没有回复,MMC/sd v1.x/not card
			for (i=0; i<1000; i++) {
				Hsmmc_IssueCommand(CMD55, 0, 0, Response_R1);
				if (!Hsmmc_IssueCommand(CMD41, 0, 0, Response_R3)) { // CMD41有回复说明为sd卡
					OCR = rHM1_RSPREG0; // 获得回复的OCR(操作条件寄存器)值
					if (OCR & 0x80000000) { // 卡上电是否完成上电流程,是否busy
						CardType = SD_V1; // 正确识别出sd v1.x卡
						Debug("SD card version 1.x is detected\n\r");
						break;
					}
				} else {
					// MMC卡识别
				}
				Delay_us(100);				
			}
	} 
	else 
	{ // sd v2.0
		if (((rHM1_RSPREG0&0xff) == 0xaa) && (((rHM1_RSPREG0>>8)&0xf) == 0x1)) { // 判断卡是否支持2.7~3.3v电压
			OCR = 0;
			for (i=0; i<1000; i++) {
				Hsmmc_IssueCommand(CMD55, 0, 0, Response_R1);
				Hsmmc_IssueCommand(CMD41, OCR | 1 << 30, 0, Response_R3); // reday态
				OCR = rHM1_RSPREG0;
				if (OCR & 0x80000000) { // 卡上电是否完成上电流程,是否busy
					if (OCR & (1<<30)) { // 判断卡为标准卡还是高容量卡
						CardType = SD_HC; // 高容量卡
						Debug("SDHC card is detected\n\r");
					} else {
						CardType = SD_V2; // 标准卡
						Debug("SD version 2.0 standard card is detected\n\r");
					}
					break;
				}
				Delay_us(100);
			}
		}
	}
	switch (CardType)
	{
		case SD_HC:
			Debug ("CardType = SD_HC");
			break;
		case SD_V1:
			Debug ("CardType = SD_V1");
			break;
		case SD_V2:
			Debug ("CardType = SD_V2");
			break;
		default:Debug ("CardType = Unknown");break;
	}
	if (CardType == SD_HC || CardType == SD_V1 || CardType == SD_V2) 
	{
		Hsmmc_IssueCommand(CMD2, 0, 0, Response_R2); // 请求卡发送CID(卡ID寄存器)号,进入ident
		Hsmmc_IssueCommand(CMD3, 0, 0, Response_R6); // 请求卡发布新的RCA(卡相对地址),Stand-by状态
		RCA = (rHM1_RSPREG0 >> 16) & 0xffff; // 从卡回复中得到卡相对地址

		Hsmmc_IssueCommand(CMD7, RCA<<16, 0, Response_R1); // 选择已标记的卡,transfer状态
		Debug("Enter to the transfer state\n\r");		
		Hsmmc_SetClock(0x2); // 设置SDCLK = 96M/4 = 24M
		
		if (!Hsmmc_SetBusWidth(4)) 
		{
			Debug("Set bus width error\n\r");
			return 1; // 位宽设置出错
		}
		if (Hsmmc_GetCardState() == 4) 
		{ // 此时卡应在transfer态
			if (!Hsmmc_IssueCommand(CMD16, 512, 0, Response_R1)) 
			{ // 设置块长度为512字节
				rHM1_NORINTSTS = 0xffff; // 清除中断标志
				Hsmmc_init_flag = 1;
				Debug("Card Initialization succeed\n\r");
				return 0; // 初始化成功
			}
		}
	}
	Debug("Card Initialization failed\n\r");
	return 1; // 卡工作异常
}

int Hsmmc_exist (void)
{
	return Hsmmc_init_flag;
}

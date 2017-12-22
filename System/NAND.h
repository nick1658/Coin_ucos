#ifndef __NAND_H__
#define __NAND_H__

#ifdef __cplusplus
extern "C" {
#endif

// Nand flash 5字节ID
typedef struct Nand_ID_Info {
	unsigned char Maker;
	unsigned char Device;
	unsigned char ID_Data3;
	unsigned char ID_Data4;
	unsigned char ID_Data5;
} Nand_ID_Info;





/*
 * Standard NAND flash commands
 */
// 页读周期1(命令1)
#define NAND_CMD_READ0		0x00
// 页读周期1(命令2)
#define NAND_CMD_READ1		0x01
// 随机地址读周期1
#define NAND_CMD_RNDOUT		0x05
// 页写周期2
#define NAND_CMD_PAGEPROG	0x10
// OOB区读命令
#define NAND_CMD_READOOB	0x50
// 块擦除命令周期1
#define NAND_CMD_ERASE1		0x60
// 读Nand状态命令
#define NAND_CMD_STATUS		0x70
// 读多层状态命令
#define NAND_CMD_STATUS_MULTI	0x71
// 页写周期1
#define NAND_CMD_SEQIN		0x80
// 随机地址写命令
#define NAND_CMD_RNDIN		0x85
// 读ID命令
#define NAND_CMD_READID		0x90
// 块擦除命令周期2
#define NAND_CMD_ERASE2		0xd0
// Nand复位
#define NAND_CMD_RESET		0xff

/* Extended commands for large page devices */
// 页读周期2
#define NAND_CMD_READSTART	0x30
// 随机地址读周期2
#define NAND_CMD_RNDOUTSTART	0xE0
#define NAND_CMD_CACHEDPROG	0x15

// 发送命令
#define NF_CMD(Data) 		{rNFCMD  = (Data);}
// 写地址
#define NF_ADDR(Addr)       {rNFADDR = (Addr);}
// 读字(4字节)
#define NF_READ_WORD()      (rNFDATA)
// 读一字节
#define NF_READ_BYTE()      (rNFDATA8)
// 写字(4字节)
#define NF_WRITE_WORD(Data) {rNFDATA = (Data);}
// 写一字节
#define NF_WRITE_BYTE(Data) {rNFDATA8 = (Data);} 

// 使能片选
#define NF_CE_ENABLE() 		{rNFCONT &= ~(1<<1);}
// 关闭片选
#define NF_CE_DISABLE() 	{rNFCONT |= (1<<1);}
// 清空spare区ECC校验值
#define NF_INIT_SECC() 		{rNFCONT |= (1<<4);}
// 清空main区ECC校验值
#define NF_INIT_MECC() 		{rNFCONT |= (1<<5);}
// 解锁spare区ECC校验值
#define NF_SECC_UNLOCK() 	{rNFCONT &= ~(1<<6);}
// 锁定spare区ECC校验值
#define NF_SECC_LOCK() 		{rNFCONT |= (1<<6);}
// 解锁main区ECC校验值
#define NF_MECC_UNLOCK() 	{rNFCONT &= ~(1<<7);}
// 锁定main区ECC校验值
#define NF_MECC_LOCK() 		{rNFCONT |= (1<<7);}

// nand传输完会置位NFSTAT[4],若开启中断,会同时发送中断请求
#define NF_WAIT_READY()     {while(!(rNFSTAT & (1<<4)));}
// 获得nand RnB引脚状态,1为准备好,0为忙
#define NF_GET_STATE_RB()   {(rNFSTAT & (1<<0))}
// 清除nand传输完标志
#define NF_CLEAR_RB()       {rNFSTAT |= (1<<4);}

/* 模块接口函数 */
extern unsigned char Nand_ReadSkipBad(unsigned int Address, 
				unsigned char *Buffer, unsigned int Length);
extern unsigned char Nand_WriteSkipBad(unsigned int Address,
				unsigned char *Buffer, unsigned int Length);
extern void Nand_Init(void);
extern int Nand_ReadID(Nand_ID_Info *pInfo);
int Nand_ReadPage(unsigned int Block, unsigned int Page,
							unsigned char *Buffer);
int Nand_WritePage(unsigned int Block, unsigned int Page, 
							unsigned char *Buffer);
extern unsigned char Nand_EraseBlock(unsigned int Block);
extern unsigned char Nand_MarkBadBlock(unsigned int Block);
extern unsigned char Nand_IsBadBlock(unsigned int Block);
extern unsigned char WriteCodeToNand(void);
unsigned char WriteAppToNand(unsigned int CodeAddr, unsigned int CodeSize);
unsigned char WriteAppToAppSpace(unsigned int CodeAddr, unsigned int CodeSize);
////////////////////////////////////////////////////////////////

#define NF_NFCE0		(0x1<<1)
#define CMD_READ			0x00	//  Read


#define NF_CE_L()			{rNFCONT &= ~NF_NFCE0;}
#define NF_CE_H()			{rNFCONT |= NF_NFCE0;}
#define NF_DETECT_RB()			{while(!(rNFSTAT&(1<<4)));}
#define NF_nFCE_H()			{rNFCONT |= (1<<1); }
#define NF_nFCE_L()			{rNFCONT &= ~(1<<1); }

#define NF_DATA_R()			(rNFDATA8)
#define NF_DATA_W(n)			(rNFDATA8 = (unsigned char)n)
#define NF_RDDATA8()			(rNFDATA8)



void rNF_Init(void);
unsigned char BlockErase(unsigned int pageaddr);
unsigned char rNF_WritePage(unsigned int page_num, unsigned char *buf);

void rLB_ReadPage(unsigned int addr,unsigned char * to);
unsigned char rNF_RamdomRead(unsigned int page_num, unsigned int addr);

#ifdef __cplusplus
}
#endif

#endif /*__NAND_H__*/

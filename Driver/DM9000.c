#include "S3C2416.h"
#include "DM9000.h"

//#define DM9000_IO		(0x20000000)
//#define DM9000_DATA		(0x20000008)
#define DM9000_IO		(0x20000000)
#define DM9000_DATA		(0x20000002)

#define NET_ERROR		-1
#define NET_TX_OK		1
#define NET_RX_OK		2

static volatile int8_t DM9000_LinkStatus;
static volatile int8_t DM9000_TxStatus;
static volatile int8_t DM9000_RxStatus;

uint8_t default_enetaddr[6] = {0x00, 0x22, 0x12, 0x34, 0x56, 0x90}; 

static void DM9000_WriteReg(uint8_t Reg, uint8_t Value)
{
	DM9000_CS(0);
	__REGb(DM9000_IO) = Reg;
	__REGb(DM9000_DATA) = Value;
	DM9000_CS(1);
}

static uint8_t DM9000_ReadReg(uint8_t Reg)
{
	uint8_t temp;
	DM9000_CS(0);
	__REGb(DM9000_IO) = Reg;
	temp = __REGb(DM9000_DATA);
	DM9000_CS(1);
	return temp;
}

static void DM9000_WritePhy(uint8_t Reg, uint16_t Value)
{
	int32_t Timeout;
	// Fill the phyxcer register into REG_0C
	DM9000_WriteReg(DM9000_EPAR, DM9000_PHY | Reg);	
	// Fill the written data into REG_0D & REG_0E
	DM9000_WriteReg(DM9000_EPDRL, (Value & 0xff));
	DM9000_WriteReg(DM9000_EPDRH, ((Value >> 8) & 0xff));
	DM9000_WriteReg(DM9000_EPCR, 0x0A);	// Issue phyxcer write command
	// Wait write complete
	Timeout = 1000;
	while (DM9000_ReadReg(DM9000_EPCR)&0x1) {
		Delay_us(1);
		Timeout--;
		if (Timeout == 0) {
			break;
		}
	}	
	DM9000_WriteReg(DM9000_EPCR, 0x0);	// Clear phyxcer write command	
}

//static uint16_t DM9000_ReadPhy(uint8_t Reg)
//{
//	int32_t Timeout;
//	uint16_t Value;
//	// Fill the phyxcer register into REG_0C
//	DM9000_WriteReg(DM9000_EPAR, DM9000_PHY | Reg);
//	DM9000_WriteReg(DM9000_EPCR, 0x0C);	// Issue phyxcer read command
//	// Wait write complete
//	Timeout = 1000;
//	while (DM9000_ReadReg(DM9000_EPCR)&0x1) {
//		Delay_us(1);
//		Timeout--;
//		if (Timeout == 0) {
//			break;
//		}
//	}
//	DM9000_WriteReg(DM9000_EPCR, 0x0);	// Clear phyxcer read command
//	Value = (DM9000_ReadReg(DM9000_EPDRH)<<8) | DM9000_ReadReg(DM9000_EPDRL);
//	return Value;
//}

int DM90000_ReceivePacket(void *pBuffer)
{
	int Status = 0;	
	int i;
	int RxLen;
	uint8_t RxReady;

	if (!DM9000_RxStatus) {
		return 0;
	}
	
	// 禁止所有中断
	DM9000_WriteReg(DM9000_IMR, 0x80);
	DM9000_RxStatus = 0;
	
	for (;;){
		// 接收到包
		RxReady = DM9000_ReadReg(DM9000_MRCMDX); // Dummy read
		//save_mrr = (DM9000_ReadReg(0xf5) << 8) | DM9000_ReadReg(0xf4);
		RxReady = DM9000_ReadReg(DM9000_MRCMDX); // Got most updated data 
	
		// 0:not packet, 1:packet received, other:error
		if (RxReady != 0x01) {
			if (RxReady) {
				DM9000_WriteReg(DM9000_RCR, 0x00); // Stop Device
				DM9000_WriteReg(DM9000_IMR, 0x80); // mask int
				dbg("DM9000 error: status check fail: 0x%x\n", RxReady);
				return -1;
			}
			DM9000_WriteReg(DM9000_IMR, 0xa3);
			return Status;
		}
		
		// A packet ready now  & Get status/length
		DM9000_CS (0);
		__REGb(DM9000_IO) = DM9000_MRCMD;
		__REGw(DM9000_DATA); // status
		RxLen = __REGw(DM9000_DATA);
		DM9000_CS (1);
		
		if ((RxLen>DM9000_PKT_MAX) || (RxLen<0x40)) {
			dbg("DM9000 error packet: RX Len 0x%x\n", RxLen);
			Status = -1;
		} else {
			Status = RxLen;
		}
		if ((RxLen > 1500)) {
			Status = -1;
		}
		// Read received packet from RX SRAM
		RxLen = (RxLen+1) / 2;
		for (i=0; i<RxLen; i++) {
			DM9000_CS (0);
			((uint16_t *)pBuffer)[i] = __REGw(DM9000_DATA);
			DM9000_CS (1);
		}
		if (Status < 0){
			DM9000_WriteReg(DM9000_IMR, 0xa3);
			return Status;
		}
	}
}

int DM9000_SendPacket(void *pBuffer, int32_t Len)
{
	uint16_t *pTemp;
	int32_t TempLen;
	int32_t i;
	int8_t TempStatus;
	if ((pBuffer == NULL) || (Len==0)) {
		return -1;
	}
	
	// 禁止所有中断
	DM9000_WriteReg(DM9000_IMR, 0x80);
	// Move data to DM9000 TX RAM
	DM9000_CS (0);
	__REGb(DM9000_IO) = DM9000_MWCMD;
	DM9000_CS (1);
	pTemp = (uint16_t *)pBuffer;
	TempLen = (Len+1) / 2;
	for (i=0; i<TempLen; i++) {
		DM9000_CS (0);
		__REGw(DM9000_DATA) = pTemp[i];
		DM9000_CS (1);
	}
	// Set TX length to DM9000
	DM9000_WriteReg(DM9000_TXPLL, Len&0xff);
	DM9000_WriteReg(DM9000_TXPLH, (Len>>8)&0xff);
	// Issue TX request command
	DM9000_WriteReg(DM9000_TCR, TCR_TXREQ);
		// 重新使能中断
	DM9000_WriteReg(DM9000_IMR, 0xa3);
	
	DM9000_TxStatus = 0;
	time_out = 1;
	while ((TempStatus = DM9000_TxStatus) != NET_TX_OK) {
		time_out++;
		if (TempStatus != 0) {
			return TempStatus;
		}
		if (time_out > 200){
			return -1;
		}
	}
	
	return 0;
}


extern OS_EVENT * net_msg;
static void INT_EINT4_7_Handler(void)
{	
	uint8_t IntStatus;
	
	if (rEINTPEND & (1<<4)) { // EINT4
		rEINTPEND |= (1 << 4); // Clear pending bit	
		// 禁止所有中断
		DM9000_WriteReg(DM9000_IMR, 0x80);	
		IntStatus = DM9000_ReadReg(DM9000_ISR);
		DM9000_WriteReg(DM9000_ISR, IntStatus);	

		if (IntStatus & (1<<0)) {
			DM9000_RxStatus = NET_RX_OK;
//			OSQPost(net_msg, (void *) 0x55);//发送消息	 	
		}
		if (IntStatus & (1<<1)) {		
			// 发送完成
			DM9000_TxStatus = NET_TX_OK;
		}
		if (IntStatus & (1<<5)) {
			// 连接改变
			if (DM9000_ReadReg(DM9000_NSR) & (1<<6)) {
				dbg("Link OK\n");			
				DM9000_LinkStatus = 1;
			} else {
				dbg("Link failed\n");	
				DM9000_LinkStatus = 0;
			}
		}
	}
	
	IRQ_ClearInt(INT_EINT4_7);// clear pending
	// 重新使能中断
	DM9000_WriteReg(DM9000_IMR, 0xa3);
}

int DM9000_GetLinkStatus(void)
{
	return DM9000_LinkStatus;
}

int DM9000_GetReceiveStatus(void)
{
	return DM9000_RxStatus;
}


/* General Purpose dm9000 reset routine */
static void
dm9000_reset(void)
{
	dbg("resetting DM9000\n");

	/* Reset DM9000,
   see DM9000 Application Notes V1.22 Jun 11, 2004 page 29 */

	/* DEBUG: Make all GPIO0 outputs, all others inputs */
	DM9000_WriteReg(DM9000_GPCR, GPCR_GPIO0_OUT);
	/* Step 1: Power internal PHY by writing 0 to GPIO0 pin */
	DM9000_WriteReg(DM9000_GPR, 0);
	/* Step 2: Software reset */
	DM9000_WriteReg(DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST));

	do {
		dbg("resetting the DM9000, 1st reset\n");
		Delay_us(25); /* Wait at least 20 us */
	} while (DM9000_ReadReg(DM9000_NCR) & 1);

	DM9000_WriteReg(DM9000_NCR, 0);
	DM9000_WriteReg(DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST)); /* Issue a second reset */

	do {
		dbg("resetting the DM9000, 2nd reset\n");
		Delay_us(25); /* Wait at least 20 us */
	} while (DM9000_ReadReg(DM9000_NCR) & 1);

	/* Check whether the ethernet controller is present */
	if ((DM9000_ReadReg(DM9000_PIDL) != 0x0) ||
	    (DM9000_ReadReg(DM9000_PIDH) != 0x90))
		dbg("ERROR: resetting DM9000 -> not responding.\n");
}


int DM9000_Init(void)
{
	uint32_t Id;
	int32_t i;
	int32_t oft;
	
	// DM9000A最大的连续读/写时间为4clk(20ns/clk),一次读/写周期设置在100ns, 7 clk@66M	
	rGPACON = 0x07ffdff;   // GPA24 GPA23 LED / GPA9 DM900_CS
	rSMBIDCYR4 = 0xF;
	rSMBWSTWRR4 = 12;
	rSMBWSTRDR4 = 12;	
	rSMBWSTOENR4 = 2;
	rSMBWSTWENR4 = 2;
	
	rSMBCR4 |= (1<<15) | (1<<7);
	rSMBCR4 |= (1<<2) | (1<<0);
	rSMBCR4 &= ~((3<<20) | (3<<12));
	rSMBCR4 &= ~(3<<4);
	rSMBCR4 |= (1<<4); // Memory Bank4, 16 bit
	
	// set pin interrupt
	rGPFCON &= ~(3<<(4<<1));
	rGPFCON |= (2<<(4<<1)); // EINT4
	rEXTINT0 &= ~(0x7<<(4<<2));
	rEXTINT0 |= (4<<(4<<2));	// rising edge
	
	IRQ_Register(INT_EINT4_7, INT_EINT4_7_Handler);
	IRQ_EnableInt(INT_EINT4_7);
	rEINTPEND |= (1 << 4); // Clear pending bit	
	rEINTMASK &= ~(1 << 4);
	
	// RESET device
//	DM9000_WriteReg(DM9000_NCR, NCR_RST);
//	Delay_us(100);
//	DM9000_WriteReg(DM9000_NCR, NCR_RST);
//	Delay_us(100);
	
	dm9000_reset ();
	
	// Read id
	Id = DM9000_ReadReg(DM9000_VIDL);
	Id |= DM9000_ReadReg(DM9000_VIDH) << 8;
	Id |= DM9000_ReadReg(DM9000_PIDL) << 16;
	Id |= DM9000_ReadReg(DM9000_PIDH) << 24;
	
	if (Id == DM9000_ID) {
		dbg("DM9000 found at 0x%08x, id: 0x%08x\n", DM9000_IO, Id);		
	} else {
		dbg("DM9000 not found at 0x%08x, id: 0x%08x\n", DM9000_IO, Id);
		return -1;
	}
	
	DM9000_WriteReg(DM9000_GPR, 0x00);	// Enable PHY	
	Delay_us(1000);
	
	// Set PHY
	DM9000_WritePhy(0x04, 0x101); // Set PHY media mode
	DM9000_WritePhy(0x00, 0x3100);

	// Program operating register
	DM9000_WriteReg(DM9000_NCR, 0x0);	// only intern phy supported by now
	DM9000_WriteReg(DM9000_TCR, 0);	// TX Polling clear
	DM9000_WriteReg(DM9000_BPTR, 0x3f);	// Less 3Kb, 600us
	DM9000_WriteReg(DM9000_FCTR, FCTR_HWOT(3)|FCTR_LWOT(8));// Flow Control : High/Low Water
	DM9000_WriteReg(DM9000_FCR, 0x08);
	DM9000_WriteReg(DM9000_SMCR, 0);	// Special Mode
	DM9000_WriteReg(DM9000_NSR, NSR_WAKEST|NSR_TX2END|NSR_TX1END);	// clear TX status
	DM9000_WriteReg(DM9000_ISR, 0x0F);	// Clear interrupt status
	
	// Set Node address
	oft = 0x10;
	for (i=0;  i<6; i++) {
		DM9000_WriteReg(oft, default_enetaddr[i]);
		oft++;
	}
	oft = 0x16;
	// 设置过滤组播地址
	for (i=0; i<8; i++) {
		if (i == 7) {
			DM9000_WriteReg(oft, 0x80); // 接收广播地址
		} else {
			DM9000_WriteReg(oft, 0);
		}
		oft++;		
	}
	oft = 0x10;
	cy_print("MAC: ");
	for (i=0; i<5; i++) {
		cy_print("%02x:", DM9000_ReadReg(oft));
		oft++;
	}
	cy_print("%02x\n", DM9000_ReadReg(oft));

	// Activate DM9000
	DM9000_WriteReg(DM9000_RCR, RCR_DIS_LONG|RCR_DIS_CRC|RCR_RXEN);	// RX enable 	
	DM9000_WriteReg(DM9000_IMR, 0xa3);	// Enable TX/RX/Link interrupt
	return 0;
}

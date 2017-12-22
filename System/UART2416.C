#include "S3C2416.h"


void HandleUART0(void);
void HandleUART2(void);

//////////////////////////////////////串口0  现只用PC显示 以后如不用实时与PC通信可用智能屏通信/////////////////////////////////////////////
void uart_init(void)
{
	unsigned char i;
	rUFCON0 = 0x0;       //UART channel 0 FIFO control register, FIFO disable
	rUMCON0 = 0x0;      // AFC disable

	rULCON0 = 0x03;      // Normal,No parity,1 stop,8 bits
	rUCON0  = 0x05;  //允许 发送 接收 PCLK作时钟 中断给脉冲

	rUBRDIV0=((unsigned int)(PCLK/16/115200)-1);//波特率设置57600  115200


	rSUBSRCPND |= 0x3;   //清除中断标志  发送 接收都清除
	rSRCPND1 |= 1<<28;	
	rINTPND1 |= 1<<28;

	rINTMSK1 &= ~(1<<28);
	rINTSUBMSK &= ~(1<<0);
	//pISR_UART0 = (unsigned int) HandleUART0;   //中断处理函数  本程序里串口0不需要中断 只需要发送
	
	IRQ_Register(INT_UART0,HandleUART0);

	for (i=0; i<100; i++)
		;
}
void Uart0_SendByte(int data)
{
		if(data=='\n')
		{
			while(!(rUTRSTAT0 & 0x2));
			// Delay(1);                 //because the slow response of hyper_terminal 
			WrUTXH0('\r');
		}
		while(!(rUTRSTAT0 & 0x2));   //Wait until THR is empty.
		//  Delay(1);
		WrUTXH0(data);
} 

void Uart0_sendchar(U8 data)
{
//	if(data=='\n')
//	{
//		while(!(rUTRSTAT0 & 0x2));
		// Delay(1);                 //because the slow response of hyper_terminal 
//		WrUTXH0('\r');
//	}
	while(!(rUTRSTAT0 & 0x2));   //Wait until THR is empty.
	//  Delay(1);
	WrUTXH0(data);
} 

void Uart0_SendString(const char *pt)
{
    while(*pt)
        Uart0_SendByte(*pt++);
}
void Uart0_Printf(const char *fmt,...)
{
	va_list ap;
	char string[256];

	va_start(ap,fmt);
	vsprintf(string,fmt,ap);
	Uart0_SendString(string);
	va_end(ap);
}




//void Uart0_sendchar(U8 data)
//{
//	if(data=='\n')
//	{
//		while(!(rUTRSTAT0 & 0x2));
//		// Delay(1);                 //because the slow response of hyper_terminal 
//		WrUTXH0('\r');
//	}
//	while(!(rUTRSTAT0 & 0x2));   //Wait until THR is empty.
//	//  Delay(1);
//	WrUTXH0(data);
//} 
/////////////////////////////////串口1  打印机 只输出///////////////////////////////////////////////
void uart1_init(void)
{
	unsigned char i;
	rUFCON1 = 0x0;       //UART channel 1 FIFO control register, FIFO disable
	rUMCON1 = 0x0;      // AFC disable

	rULCON1 = 0x03;      // Normal,No parity,1 stop,8 bits
	rUCON1  = 0x05;  //允许 发送 接收 PCLK作时钟 中断给脉冲

	rUBRDIV1=((unsigned int)(PCLK/16/9600)-1);//波特率设置   打印机 9600


//	rSUBSRCPND |= 0x3;   //清除中断标志  发送 接收都清除
//	rSRCPND1 |= 1<<28;	
//	rINTPND1 |= 1<<28;

//	rINTMSK1 &= ~(1<<28);
//	rINTSUBMSK &= ~(1<<0);
//	pISR_UART0 = (unsigned int) HandleUART0;   //中断处理函数  本程序里串口0不需要中断 只需要发送

	for (i=0; i<100; i++)
		;
}
void Uart1_SendByte(int data)
{
		if(data=='\n')
		{
			while(!(rUTRSTAT1 & 0x2)){;}
			// Delay(1);                 //because the slow response of hyper_terminal 
			WrUTXH1('\r');
		}
		while(!(rUTRSTAT1 & 0x2));   //Wait until THR is empty.
		//  Delay(1);
		WrUTXH1(data);
} 
void Uart1_SendString(const char *pt)
{
    while(*pt)
        Uart1_SendByte(*pt++);
}
void Uart1_Printf(const char *fmt,...)
{
	va_list ap;
	char string[256];

	va_start(ap,fmt);
	vsprintf(string,fmt,ap);
	Uart1_SendString(string);
	va_end(ap);
}

/////////////////////////////////串口2  智能屏 中断输入///////////////////////////////////////////////
void uart2_init(void)
{
	unsigned char i;
	rUFCON2 = 0x0;       //UART channel 1 FIFO control register, FIFO disable
	rUMCON2 = 0x0;      // AFC disable

	rULCON2 = 0x03;      // Normal,No parity,1 stop,8 bits
	rUCON2  = 0x05;  //允许 发送 接收 PCLK作时钟 中断给脉冲

	rUBRDIV2=((unsigned int)(PCLK/16/115200)-1);//波特率设置 智能屏 115200


	rSUBSRCPND |= 0x3<<6;   //清除中断标志  发送 接收都清除
	rSRCPND1 |= 1<<15;	//串口2 第15位
	rINTPND1 |= 1<<15;

	rINTMSK1 &= ~(1<<15);
	rINTSUBMSK &= ~(1<<6);   //接收中断
//	pISR_UART2 = (unsigned int) HandleUART2;   //中断处理函数 
	IRQ_Register(INT_UART2,HandleUART2);
	for (i=0; i<100; i++)
		;
}
void Uart2_sendchar(U8 data)
{
	while(!(rUTRSTAT2 & 0x2))
	{		
    }   //Wait until
	WrUTXH2(data);
} 

//void __irq HandleUART2(void)   //串口2 中断接收函数 
void HandleUART2(void)   //串口2 中断接收函数 
{
	rSUBSRCPND |= 0x3<<6;   //清除中断标志  发送 接收都清除
	rSRCPND1 |= 1<<15;	//串口2 第15位
	rINTPND1 |= 1<<15;
	
	touchget_from_uart2();    //receive the touch from dgus at real time 
}


volatile U16 uart0_count = 0;
void HandleUART0 (void)
{
	rSUBSRCPND |= 0x3<<0;   //清除中断标志  发送 接收都清除
	rSRCPND1 |= 1<<28;	//串口2 第15位
	rINTPND1 |= 1<<28;
	LED1_NOT;

	fill_rec_buf((char)rURXH0);
}

#define CMD_DEBUG 0x02




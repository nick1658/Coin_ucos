#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#ifdef __cplusplus
extern "C" {
#endif

// PENDING INT1
#define INT_EINT0		(0)
#define INT_EINT1		(1)
#define INT_EINT2		(2)
#define INT_EINT3		(3)
#define INT_EINT4_7		(4)
#define INT_EINT8_15	(5)

#define INT_BAT_FLT		(7)
#define INT_TICK		(8)
#define INT_WDT_AC97	(9)
#define INT_TIMER0		(10)
#define INT_TIMER1		(11)
#define INT_TIMER2		(12)
#define INT_TIMER3		(13)
#define INT_TIMER4		(14)
#define INT_UART2		(15)
#define INT_LCD			(16)
#define INT_DMA			(17)
#define INT_UART3		(18)

#define INT_SDI1		(20)
#define INT_SDI0		(21)
#define INT_SPI0		(22)
#define INT_UART1		(23)
#define INT_NAND		(24)
#define INT_USBD		(25)
#define INT_USBH		(26)
#define INT_IIC0		(27)
#define INT_UART0		(28)

#define INT_RTC			(30)
#define INT_ADC			(31)

// PENDING INT2
#define INT_2D			(0)
#define INT_PCM0		(4)
#define INT_I2S0		(6)

// SUB PENDING INT
#define SUBINT_AC97		(28)
#define SUBINT_WDT		(27)
#define SUBINT_ERR3		(26)
#define SUBINT_TXD3		(25)
#define SUBINT_RXD3		(24)
#define SUBINT_DMA5		(23)
#define SUBINT_DMA4		(22)
#define SUBINT_DMA3		(21)
#define SUBINT_DMA2		(20)
#define SUBINT_DMA1		(19)
#define SUBINT_DMA0		(18)
#define SUBINT_LCD4		(17)
#define SUBINT_LCD3		(16)
#define SUBINT_LCD2		(15)
#define SUBINT_LCD1		(14)

#define SUBINT_ADC		(10)
#define SUBINT_TC		(9)
#define SUBINT_ERR2		(8)
#define SUBINT_TXD2		(7)
#define SUBINT_RXD2		(6)
#define SUBINT_ERR1		(5)
#define SUBINT_TXD1		(4)
#define SUBINT_RXD1		(3)
#define SUBINT_ERR0		(2)
#define SUBINT_TXD0		(1)
#define SUBINT_RXD0		(0)
	
extern void Undef_Handler(void);
extern unsigned int SWI_Handler(unsigned int p);
extern void PAbt_Handler(void);
extern void DAbt_Handler(void);
extern void IRQ_Handler(void);
extern void FIQ_Handler(void);	
void IRQ_Register(unsigned char Channel, void (*Func)(void));
void IRQ_ClearInt(unsigned char Channel);
void IRQ_EnableInt(unsigned char Channel);
void IRQ_DisableInt(unsigned char Channel);
	
	
#ifdef __cplusplus
}
#endif

#endif /*__EXCEPTION_H__*/

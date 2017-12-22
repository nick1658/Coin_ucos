/*---------------------------µ◊≤„«˝∂Ø∫Ø ˝------------------------------------*/
#include "def.h"
#include "S3C2416.h"

void port_Init(void)
{
    //CAUTION:Follow the configuration order for setting the ports. 
    // 1) setting value(GPnDAT) 
    // 2) setting control register  (GPnCON)
    // 3) configure pull-up resistor(GPnUP)  

    //32bit data bus configuration  
    //*** PORT A GROUP
    //*** PORT A GROUP
    //Ports  :GPA26 GPA25 GPA24  GPA23 GPA22 GPA21  GPA20 GPA19 GPA18 GPA17 GPA16 GPA15 GPA14 GPA13 GPA12  
    //Signal :DQM3  DQM2  RSMAVD RSMCLK nFCE nRSTOUT nFRE   nFWE  ALE   CLE  nGCS5 nGCS4 nGCS3 nGCS2 nGCS1 
    //Binary : 0     0      0  ,   0    1     1     1 ,   1    1      1      1,   1    1      1    1
    //Ports  : GPA11   GPA10  GPA9   GPA8   GPA7   GPA6   GPA5   GPA4   GPA3   GPA2   GPA1  GPA0
    //Signal : ADDR26 ADDR25 ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0 
    //Binary :  1       1      1      1   , 1       1      1      1   ,  1       1     1      1         
    rGPACON = 0x07fffff;   // GPA24 GPA23 LED

    //**** PORT B GROUP
    //Ports  : GPB10    GPB9    GPB8    GPB7      GPB6     GPB5    GPB4   GPB3   GPB2     GPB1      GPB0
    //Signal : nXDREQ0 nXDACK0 reserved reserved  nXBREQ  nXBACK   TCLK   TOUT3  TOUT2    TOUT1     TOUT0
    //Setting: INPUT  OUTPUT   reserved reserved   OUTPUT   OUTPUT  OUTPUT OUTPUT OUTPUT   OUTPUT    OUTPUT 
    //Binary :   00  ,  01       00  ,   01        01   ,  01       01  ,   01     01   ,  01        01  
    rGPBCON = 0x045555;   //(GPB6 GPB5 LED)  (GPB0 BEEP)
    rGPBUDP  = 0x0;     // The pull up function is disabled GPB[10:0]
	rGPBSEL = 0;
//	rGPBSEL = 0; 			//≈‰÷√GPB6 GPB1 GPB0

    //*** PORT C GROUP for youlong
    //Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8  GPC7   GPC6   GPC5 GPC4 GPC3  GPC2  GPC1 GPC0
    //Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND  
    //Binary :  10   10  , 10    10  , 10    10  , 10   10  , 10     10  ,  10   10 , 10     10 , 10   10
    //rGPCCON = 0xaaaaaaaa;       
    //rGPCUP  = 0xffff;     // The pull up function is disabled GPC[15:0] 
    
    //*** PORT C GROUP
    //Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8  GPC7   GPC6   GPC5 GPC4 GPC3  GPC2  GPC1 GPC0
    //Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND  
    //Binary :  10   10  , 10    10  , 10    10  , 10   01  , 01     01  ,  01   10 , 10     10 , 10   10
	//                                                       output  output output
    rGPCCON = 0xaaa956aa;       
    rGPCUDP  = 0xffff;     // The pull up function is disabled GPC[15:0] 
	//≈‰÷√GPC 5 6 7     

    //*** PORT E GROUP
    //Ports  : GPE15  GPE14 GPE13   GPE12   GPE11   GPE10   GPE9    GPE8     GPE7  GPE6  GPE5   GPE4  
    //Signal : IICSDA IICSCL SPICLK SPIMOSI SPIMISO SDDATA3 SDDATA2 SDDATA1 SDDATA0 SDCMD SDCLK IN 
    //Binary :  10     10  ,  10      10  ,  10      00   , 00      00   ,   00    00  , 01     00  ,  
	//                                              INPUT  INPUT  INPUT     INPUT INPUT  OUTPUT   
    //-------------------------------------------------------------------------------------------------------
    //Ports  :  GPE3   GPE2  GPE1    GPE0    
    //Signal :  IN     IN    IN      IN  
    //Binary :  00     00  ,  00      00 
    //rGPECON = 0xaaaaaaaa;       
    //rGPEUP  = 0xffff;     // The pull up function is disabled GPE[15:0]
	rGPECON = 0xaa800400; // IIC≈‰÷√ IICSDA IICSCL      
    rGPEUDP  = 0x00000000;    //pullup pulldown disable    
	 //≈‰÷√GPE5 6 7 8 9 10 
       //*** PORT F GROUP
    //Ports  : GPF7   GPF6   GPF5   GPF4      GPF3     GPF2  GPF1   GPF0
    //Signal : nLED_8 nLED_4 nLED_2 nLED_1 nIRQ_PCMCIA EINT2 KBDINT EINT0
    //Setting: Output Output Output Output    EINT3    EINT2 EINT1  EINT0
    //Binary :  01      01 ,  01     01  ,     10       10  , 01     10
	//                                                        OUTPUT
    rGPFCON = 0x55a6;
    rGPFUDP  = 0xff;     // The pull up function is disabled GPF[7:0]
				   //≈‰÷√GPF1 

    //*** PORT G GROUP
    //Ports  :     GPG7      GPG6    
    //Signal : KBDSPICLK KBDSPIMOSI
    //Setting:    EINT15    EINT14
    //Binary :      01         00
    //-----------------------------------------------------------------------------------------
    //Ports  :    GPG5       GPG4    GPG3    GPG2    GPG1    GPG0    
    //Signal : KBDSPIMISO LCD_PWREN EINT11 nSS_SPI IRQ_LAN IRQ_PCMCIA
    //Setting:  EINT13   EINT12 EINT11   EINT10   EINT9    EINT8
    //Binary :     00         00   ,  01      10 ,  10        10
    rGPGCON = 0x406a;// GPG9 input without pull-up
    rGPGUDP  = 0xffff;    // The pull up function is disabled GPG[15:0]
	 //≈‰÷√GPG 3 4 5 6 7 ‰»Î



  //*** PORT H GROUP   
    //Ports  :  GPH14  GPH13   GPH12      GPH11  GPH10 GPH9  GPH8  GPH7 GPH6 GPH5 GPH4 GPH3 GPH2 GPH1  GPH0 
    //Signal : CLKOUT1 CLKOUT0 EXTUARTCLK nRTS1  nCTS1 nRTS0 nCTS0 RXD3 TXD3 RXD2 TXD2 RXD1 TXD1 RXD0  TXD0
    //Binary :   01      01     01        10  ,  10    10    10    10 , 10   10   10,  10   10   10    10
	//			 output	 output output
     //up		 00      00     00         00    00
    rGPHCON = 0x15AAAAAA;   //¥Æø⁄≈‰÷√ RXD3 TXD3 RXD2 TXD2 RXD1 TXD1 RXD0  TXD0
    rGPHUDP  = 0x00000000;    // The pull up function is disabled 00 : pull-up/down disable
							 //≈‰÷√GPH14 PGH13 GPH12
	
	LED1_OFF; LED2_OFF;	//LED3_OFF; LED4_OFF;
	//LED1_ON; LED2_ON;	//LED3_OFF; LED4_OFF;
	ALL_STOP();

}


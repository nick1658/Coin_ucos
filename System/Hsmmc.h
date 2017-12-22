#ifndef __HSMMC_H__
#define __HSMMC_H__

#ifdef __cplusplus
extern "C" {
#endif
	
#define	CMD0	0
#define	CMD1	1
#define	CMD2	2	
#define	CMD3	3	
#define	CMD6	6
#define	CMD7	7
#define	CMD8	8
#define	CMD9	9	
#define	CMD13	13
#define	CMD16	16
#define	CMD17	17
#define	CMD18	18
#define	CMD23	23
#define	CMD24	24
#define	CMD25	25	
#define	CMD32	32	
#define	CMD33	33	
#define	CMD38	38	
#define	CMD41	41	
#define	CMD55	55	

// ø®¿‡–Õ
#define UnusableCard	0
#define SD_V1			1	
#define	SD_V2			2	
#define	SD_HC			3
#define	MMC				4

#define	Response_NONE	0	
#define	Response_R1		1
#define	Response_R2		2
#define	Response_R3		3
#define	Response_R4		4
#define	Response_R5		5
#define	Response_R6		6
#define	Response_R7		7
#define	Response_R1b	8

int Hsmmc_Init(void);
int Hsmmc_GetCardState(void);
int Hsmmc_GetSdState(unsigned char *pState);
int Hsmmc_Get_CSD(unsigned char *pCSD);
int Hsmmc_EraseBlock(unsigned int StartBlock, unsigned int EndBlock);
int Hsmmc_WriteBlock(unsigned char *pBuffer, unsigned int BlockAddr, unsigned int BlockNumber);
int Hsmmc_ReadBlock(unsigned char *pBuffer, unsigned int BlockAddr, unsigned int BlockNumber);
int Hsmmc_exist (void);

	
#ifdef __cplusplus
}
#endif

#endif /*__HSMMC_H__*/

#ifndef __MMU_H__
#define __MMU_H__

#ifdef __cplusplus
extern "C" {
#endif

// ICache使能控制位
#define R1_I		(1<<12)
// DCache使能控制位
#define R1_C		(1<<2)
// 地址对齐检查使能控制位
#define R1_A		(1<<1)
// MMU使能控制位
#define R1_M   		(1<<0)

// 段标识符，以段(1MB)方式进行映射
#define DESC_SEC	((1<<1)|(1<<4))
// cache_on, write_back cache会出现命中的情况	
#define CB			(3<<2)
// cache_on, write_through 	
#define CNB			(2<<2)
// cache_off,WR_BUF on cache不会出现命中的情况	
#define NCB         	(1<<2)  
// cache_off,WR_BUF off
#define NCNB		(0<<2)

// supervisor=RW, user=RW
#define AP_RW		(3<<10)
// supervisor=RW, user=RO
#define AP_RO		(2<<10) 
// supervisor=RW, user=No access
#define AP_NO		(1<<10) 

// 任何访问都将导致"Domain fault"
#define DOMAIN_FAULT	(0x0)
// 使用描述符中的设置进行权限检查
#define DOMAIN_CHK	(0x1) 
// 不进行权限检查，允许任何访问
#define DOMAIN_NOTCHK	(0x3)
// 在域0检查权限
#define DOMAIN0		(0x0<<5)
// 在域1检查权限
#define DOMAIN1		(0x1<<5)

// 域0属性为用描述符权限进行权限检查
#define DOMAIN0_ATTR	(DOMAIN_CHK<<0) 
// 域1属性为不可访问
#define DOMAIN1_ATTR	(DOMAIN_FAULT<<2) 

// 段描述符域0可读写，开cache 写缓存
#define RW_CB		(AP_RW|DOMAIN0|CB|DESC_SEC)
#define RW_CNB		(AP_RW|DOMAIN0|CNB|DESC_SEC)
#define RW_NCB		(AP_RW|DOMAIN0|NCB|DESC_SEC)
#define RW_NCNB		(AP_RW|DOMAIN0|NCNB|DESC_SEC)
// 段描述符域1不可访问
#define RW_FAULT	(AP_RW|DOMAIN1|NCNB|DESC_SEC)

void MMU_EnableICache(void); // 使能ICache
void MMU_DisableICache(void); // 禁止ICache
void MMU_EnableDCache(void); // 使能DCache
void MMU_DisableDCache(void); // 禁止DCache
void MMU_EnableAlignFault(void); // 使能对齐检查
void MMU_EnableMMU(void); // 使能MMU
void MMU_DisableMMU(void); // 禁止MMU
void MMU_SetTTBase(int Base); // Set TTBase
void MMU_SetDomain(int Domain); // Set Domain
void MMU_InvalidateICache(void); // 无效ICache
void MMU_InvalidateDCache(void); // 无效DCache
void MMU_InvalidateTLB(void); // 无效TLB
void MMU_SetProcessId(unsigned int pid); // Process ID

void MMU_Init(void); // MMU页表初始化
unsigned int *MMU_GetTableBase(void); // 获取MMU页表内存地址
void MMU_SetMTT(unsigned int vaddrStart,unsigned int vaddrEnd,
				unsigned int paddrStart,unsigned int attr);


#ifdef __cplusplus
}
#endif

#endif /*__MMU_H__*/

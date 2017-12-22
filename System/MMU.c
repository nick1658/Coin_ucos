
#include "MMU.h"

// MMU页表在编译器初始化代码之前调用,分配一个16k对齐16k大小MMU内存段,
// 这里让编译器分配这个全局内存块，一定不能让编译器初始化这部分内存
// 用__attribute__限制为未初始化段，不要让编译器初始化
__align(0x4000) static unsigned char MMU_PageTable[0x4000] 
	__attribute__((section("MMU_Mem"), zero_init));

unsigned int *MMU_GetTableBase(void)
{
	return (unsigned int *)MMU_PageTable;
}

void MMU_Init(void)
{
	// 引入用户代码运行基址，0x0处向量表重映射到用户代码向量表位置处
	extern unsigned int __CodeAddr__;	
	MMU_DisableDCache(); // 禁用DCache
	MMU_DisableICache(); // 禁用ICache
	MMU_InvalidateDCache(); // 使16K DCache无效
	MMU_InvalidateICache(); // 使16K ICache无效
	MMU_EnableICache(); // 加快执行MMU_Init
	MMU_DisableMMU(); // 禁用MMU
	MMU_InvalidateTLB(); // 使无效转换表
	
	extern unsigned int Image$$RW_NO_CACHE$$Base;
	extern unsigned int Image$$RW_MMU$$Base;
	
	unsigned int No_Cache_Start = (unsigned int)&Image$$RW_NO_CACHE$$Base;
	unsigned int No_Cache_End = (unsigned int)&Image$$RW_MMU$$Base;
	
	//MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr)
	// 0处异常向量表映射到__CodeAddr__处，代码搬移时己复制用户向量表
	// 0地址中断向量表重新映射到代码运行RAM基址，其余默认1:1映射
	// 寄存器空间不能开启cache，写缓存，寄存器应为volatile变量
	// steppingstone(Nand Boot)
    MMU_SetMTT(0x00000000, 0x00000000, __CodeAddr__, RW_CB);
	//SROM Bank0 Reserve
    MMU_SetMTT(0x00100000, 0x07f00000, 0x00100000, RW_NCNB);
	//SROM Bank1 Reserve
    MMU_SetMTT(0x08000000, 0x0ff00000, 0x08000000, RW_NCNB);
	//SROM Bank2 Reserve
    MMU_SetMTT(0x10000000, 0x17f00000, 0x10000000, RW_NCNB);
	//SROM Bank3 Reserve
    MMU_SetMTT(0x18000000, 0x1ff00000, 0x18000000, RW_NCNB);
	//SROM Bank4 Reserve
    MMU_SetMTT(0x20000000, 0x27f00000, 0x20000000, RW_NCNB);
	//SROM Bank5 Reserve
    MMU_SetMTT(0x28000000, 0x2ff00000, 0x28000000, RW_NCNB);
	//Bank6 DDR2 64M
    MMU_SetMTT(0x30000000, 0x33f00000, 0x30000000, RW_CB);  
	//SDRAM Bank6-2 Reserve
    MMU_SetMTT(0x34000000, 0x37f00000, 0x34000000, RW_NCNB);
	//SDRAM Bank7 Reserve
    MMU_SetMTT(0x38000000, 0x3ff00000, 0x38000000, RW_NCNB);
	// steppingstone(IROM boot)
    MMU_SetMTT(0x40000000, 0x40000000, 0x40000000, RW_NCNB);
	// 特殊功能寄存器
    MMU_SetMTT(0x40100000, 0x47f00000, 0x40100000, RW_NCNB);//SFR
    MMU_SetMTT(0x48000000, 0x5af00000, 0x48000000, RW_NCNB);//SFR
    MMU_SetMTT(0x5b000000, 0x5c000000, 0x5b000000, RW_NCNB);//SFR
    MMU_SetMTT(0x5c100000, 0xfff00000, 0x5c100000, RW_FAULT);//not used  
	// no cache内存段设置为关cache,不写缓存
	if (No_Cache_End > No_Cache_Start) {
		MMU_SetMTT(No_Cache_Start, No_Cache_End-1, No_Cache_Start, RW_NCB);
	}
    MMU_SetTTBase((int)MMU_PageTable); // 设置MMU的页表地址，16K对齐
    //DOMAIN1: no_access, DOMAIN0,2~15=client(AP is checked)	
    MMU_SetDomain(0x55555550|DOMAIN1_ATTR|DOMAIN0_ATTR);//域访问权限
    MMU_SetProcessId(0x0);
    MMU_EnableAlignFault(); // 使能对齐检查

    MMU_EnableMMU(); // 开启MMU后用的是虚拟地址
    MMU_EnableICache(); // 开启ICache
    MMU_EnableDCache(); // MMU开启后开启DCache
}

/* 使用段映射 */
void MMU_SetMTT(unsigned int vaddrStart,unsigned int vaddrEnd,
					unsigned int paddrStart,unsigned int attr)
{
    unsigned int *pTT;
    unsigned int i, nSec;
    
    pTT = (unsigned int *)MMU_PageTable + (vaddrStart>>20);
    nSec = (vaddrEnd>>20) - (vaddrStart>>20);
    
    for(i=0; i<=nSec; i++) {
		*pTT++ = attr | (((paddrStart>>20)+i)<<20);
	}
}

__asm void MMU_DisableMMU(void)
{
	mrc p15,0,r0,c1,c0,0
	bic r0,r0,#R1_M
	mcr p15,0,r0,c1,c0,0
	bx lr
}

__asm void MMU_EnableMMU(void)
{
	mrc p15,0,r0,c1,c0,0
	orr r0,r0,#R1_M
	mcr p15,0,r0,c1,c0,0
	bx lr
}
__asm void MMU_DisableICache(void)
{
	mrc p15,0,r0,c1,c0,0
	bic r0,r0,#R1_I
	mcr p15,0,r0,c1,c0,0
	bx lr
}

__asm void MMU_EnableICache(void)
{
	mrc p15,0,r0,c1,c0,0
	orr r0,r0,#R1_I
	mcr p15,0,r0,c1,c0,0
	bx lr
}

__asm void MMU_DisableDCache(void)
{
	mrc p15,0,r0,c1,c0,0
	bic r0,r0,#R1_C
	mcr p15,0,r0,c1,c0,0
	bx lr
}

__asm void MMU_EnableDCache(void)
{
	mrc p15,0,r0,c1,c0,0
	orr r0,r0,#R1_C
	mcr p15,0,r0,c1,c0,0
	bx lr
}

__asm void MMU_InvalidateDCache(void)
{
	mcr p15,0,r0,c7,c6,0
	bx lr
}

__asm void MMU_InvalidateICache(void)
{
	mcr p15,0,r0,c7,c5,0
	bx lr
}

__asm void MMU_InvalidateTLB(void)
{
	mcr p15,0,r0,c8,c7,0
	bx lr
}

__asm void MMU_SetTTBase(int Base)
{
	mcr p15,0,r0,c2,c0,0
	bx lr
}

__asm void MMU_SetDomain(int Domain)
{
	mcr p15,0,r0,c3,c0,0
	bx lr
}

__asm void MMU_SetProcessId(unsigned int Pid)
{
	mcr p15,0,r0,c13,c0,0
	bx lr
}

__asm void MMU_EnableAlignFault(void)
{
	mrc p15,0,r0,c1,c0,0
	orr r0,r0,#R1_A
	mcr p15,0,r0,c1,c0,0
	bx lr
}


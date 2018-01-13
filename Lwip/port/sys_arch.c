#include "sys_arch.h"
#include "stdint.h"

extern volatile uint32_t SystemTick;

uint32_t sys_now(void)
{
	return SystemTick;
}

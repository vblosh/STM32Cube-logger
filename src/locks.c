#include <stdint.h>
#include <cmsis_gcc.h>

void lock(uint32_t* interrupts_enabled)
{
	*interrupts_enabled = (__get_PRIMASK() == 0);
	__disable_irq();
}

void unlock(uint32_t* interrupts_enabled)
{
	if (*interrupts_enabled) {
		__enable_irq();
	}
}

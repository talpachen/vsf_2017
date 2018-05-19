#include "vsf.h"
#include "core.h"

static struct vsfhal_info_t vsfhal_info =
{
	0, CORE_VECTOR_TABLE, CPU_FREQ_HZ
};

vsf_err_t vsfhal_core_get_info(struct vsfhal_info_t **info)
{
	*info = &vsfhal_info;
	return VSFERR_NONE;
}

// Pendsv
struct vsfhal_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static vsfhal_pendsv;

ROOTFUNC void PendSV_Handler(void)
{
	if (vsfhal_pendsv.on_pendsv != NULL)
	{
		vsfhal_pendsv.on_pendsv(vsfhal_pendsv.param);
	}
}

vsf_err_t vsfhal_core_pendsv_config(void (*on_pendsv)(void *), void *param)
{
	vsfhal_pendsv.on_pendsv = on_pendsv;
	vsfhal_pendsv.param = param;

	if (vsfhal_pendsv.on_pendsv != NULL)
	{
		SCB->SHP[10] = 0xFF;
	}
	return VSFERR_NONE;
}

vsf_err_t vsfhal_core_pendsv_trigger(void)
{
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	return VSFERR_NONE;
}

uint32_t vsfhal_core_get_stack(void)
{
	return __get_MSP();
}

vsf_err_t vsfhal_core_set_stack(uint32_t sp)
{
	__set_MSP(sp);
	return VSFERR_NONE;
}

vsf_err_t vsfhal_core_fini(void *p)
{
	return VSFERR_NONE;
}

vsf_err_t vsfhal_core_reset(void *p)
{
	// TODO
	return VSFERR_NONE;
}

uint8_t vsfhal_core_set_intlevel(uint8_t level)
{
	uint8_t origlevel = __get_BASEPRI();
	__set_BASEPRI(level);
	return origlevel;
}

// sleep will enable interrupt
// for cortex processor, if an interrupt occur between enable the interrupt
// 		and __WFI, wfi will not make the core sleep
void vsfhal_core_sleep(uint32_t mode)
{
	vsf_leave_critical();
	// TODO
	__WFI();
}

vsf_err_t vsfhal_core_init(void *p)
{
	GLOBAL_CTRL->CLK_SEL_1_b.SYS_CLK = 3;
	GLOBAL_CTRL->NMI_SEL_b.NMI = PAD_INT_IRQn;

	SCB->VTOR = vsfhal_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | vsfhal_info.priority_group | (0x7ul << 8);
	return VSFERR_NONE;
}

static void (*tickclk_callback)(void *param) = NULL;
static void *tickclk_param = NULL;
static volatile uint32_t tickcnt = 0;

static uint32_t tickclk_get_ms_local(void)
{
	return tickcnt;
}

uint32_t vsfhal_tickclk_get_ms(void)
{
	uint32_t count1, count2;

	do {
		count1 = tickclk_get_ms_local();
		count2 = tickclk_get_ms_local();
	} while (count1 != count2);
	return count1;
}

uint32_t vsfhal_tickclk_get_us(void)
{
	uint32_t val, load;
	load = SysTick->LOAD;
	val = (load - SysTick->VAL) * 1000;
	return tickcnt * 1000 + val / load;
}

ROOTFUNC void SysTick_Handler(void)
{
	tickcnt++;
	if (tickclk_callback != NULL)
	{
		tickclk_callback(tickclk_param);
	}
}

vsf_err_t vsfhal_tickclk_config_cb(void (*callback)(void*), void *param)
{
	tickclk_callback = callback;
	tickclk_param = param;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_tickclk_stop(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_tickclk_init(int32_t int_priority)
{
	SysTick->LOAD = vsfhal_info.cpu_freq_hz / 1000;
	if (int_priority >= 0)
	{
		SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
		NVIC_SetPriority(SysTick_IRQn, (uint32_t)int_priority);
	}
	else
	{
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;
	}
	return VSFERR_NONE;
}

vsf_err_t vsfhal_tickclk_fini(void)
{
	return vsfhal_tickclk_stop();
}

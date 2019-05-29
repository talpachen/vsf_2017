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

#if defined(VSFHAL_SWI_NUMBER) && (VSFHAL_SWI_NUMBER > 0)
struct vsfhal_swi_t
{
	void (*handler)(void *);
	void *param;
} static vsfhal_swi[VSFHAL_SWI_NUMBER];

#define SWI0_IRQn			USERADD0_IRQn
#define SWI0_IRQHandler		USERADD0_IRQHandler
#define SWI0_Trigger()		NVIC_SetPendingIRQ(USERADD0_IRQn)
ROOT void SWI0_IRQHandler(void)
{
	if (vsfhal_swi[0].handler)
		vsfhal_swi[0].handler(vsfhal_swi[0].param);
}
#if VSFHAL_SWI_NUMBER >= 2
#define SWI1_IRQn			USERADD1_IRQn
#define SWI1_IRQHandler		USERADD1_IRQHandler
#define SWI1_Trigger()		NVIC_SetPendingIRQ(USERADD1_IRQn)
ROOT void SWI1_IRQHandler(void)
{
	if (vsfhal_swi[1].handler)
		vsfhal_swi[1].handler(vsfhal_swi[1].param);
}
#endif
#if VSFHAL_SWI_NUMBER >= 3
#define SWI2_IRQn			USERADD2_IRQn
#define SWI2_IRQHandler		USERADD2_IRQHandler
#define SWI2_Trigger()		NVIC_SetPendingIRQ(USERADD2_IRQn)
ROOT void SWI2_IRQHandler(void)
{
	if (vsfhal_swi[2].handler)
		vsfhal_swi[2].handler(vsfhal_swi[2].param);
}
#endif
#if VSFHAL_SWI_NUMBER >= 4
#error "Need select IRQ and replace it"
#define SWI3_IRQn			PendSV_IRQn
#define SWI3_IRQHandler		PendSV_Handler
#define SWI3_Trigger()		do {SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;} while(0)
ROOT void SWI3_IRQHandler(void)
{
	if (vsfhal_swi[3].handler)
		vsfhal_swi[3].handler(vsfhal_swi[3].param);
}
#endif
#endif

vsf_err_t vsfhal_swi_config_priority(uint8_t index, int32_t int_priority)
{
#if defined(VSFHAL_SWI_NUMBER) && (VSFHAL_SWI_NUMBER > 0)
	switch (index)
	{
	case 0:
		NVIC_SetPriority(SWI0_IRQn, int_priority);
		NVIC_EnableIRQ(SWI0_IRQn);
		break;
#if VSFHAL_SWI_NUMBER >= 2
	case 1:
		NVIC_SetPriority(SWI1_IRQn, int_priority);
		NVIC_EnableIRQ(SWI1_IRQn);
		break;
#endif
#if VSFHAL_SWI_NUMBER >= 3
	case 2:
		NVIC_SetPriority(SWI2_IRQn, int_priority);
		NVIC_EnableIRQ(SWI2_IRQn);
		break;
#endif
#if VSFHAL_SWI_NUMBER >= 4
	case 3:
		NVIC_SetPriority(SWI3_IRQn, int_priority);
		NVIC_EnableIRQ(SWI3_IRQn);
		break;
#endif
	}
	return VSFERR_NONE;
#else
	return VSFERR_NOT_SUPPORT;
#endif
}

vsf_err_t vsfhal_swi_init(uint8_t index, int32_t int_priority,
		void (*handler)(void *), void *param) @ "VSF_INIT_CODE"
{
#if defined(VSFHAL_SWI_NUMBER) && (VSFHAL_SWI_NUMBER > 0)
	vsfhal_swi[index].handler = handler;
	vsfhal_swi[index].param = param;
	vsfhal_swi_config_priority(index, int_priority);
#else
	return VSFERR_NOT_SUPPORT;
#endif
}

void vsfhal_swi_trigger(uint8_t index)
{
#if defined(VSFHAL_SWI_NUMBER) && (VSFHAL_SWI_NUMBER > 0)
	switch (index)
	{
	case 0:
		SWI0_Trigger();
		break;
#if VSFHAL_SWI_NUMBER >= 2
	case 1:
		SWI1_Trigger();
		break;
#endif
#if VSFHAL_SWI_NUMBER >= 3
	case 2:
		SWI0_Trigger();
		SWI1_Trigger();
		SWI2_Trigger();
		break;
#endif
#if VSFHAL_SWI_NUMBER >= 4
	case 3:
		SWI3_Trigger();
		break;
#endif
	}
#endif
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

void vsfhal_core_reset(void *p)
{
	// TODO
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
	ENABLE_GLOBAL_INTERRUPT();
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

ROOT void SysTick_Handler(void)
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

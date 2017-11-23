#include "vsf.h"
#include "core.h"

static struct vsfhal_info_t vsfhal_info =
{
	0, CORE_VECTOR_TABLE,
	CORE_CLKEN,
	CORE_HCLKSRC,
	OSC_FREQ_HZ, OSC32_FREQ_HZ, LIRC_FREQ_HZ,
	CORE_PLL_FREQ_HZ, CORE_APLL_FREQ_HZ, CPU_FREQ_HZ, HCLK_FREQ_HZ, PCLK_FREQ_HZ,
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
	SYS->IPRST0 |= SYS_IPRST0_CHIPRST_Msk;
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
	if (mode == VSFHAL_SLEEP_WFI) // sleep
	{
		SCB->SCR &= ~0x4ul;
	}
	else // power down
	{
		SCB->SCR |= 0x4ul;
		CLK->PWRCTL |= CLK_PWRCTL_PDWKIF_Msk;
	}
	CLK->PWRCTL &= ~CLK_PWRCTL_HXTEN_Msk;
	__WFI();
}

vsf_err_t vsfhal_core_init(void *p)
{
	uint32_t temp32, reg32;

	if (p != NULL)
	{
		vsfhal_info = *(struct vsfhal_info_t *)p;
	}

	//set spim with highspeed 4bit operation
	//SPIM->CTL0 = 0xEBF00003;

	if (vsfhal_info.osc_freq_hz != (12 * 1000 * 1000))
		return VSFERR_INVALID_PARAMETER;

	CLK->PWRCTL |= CLK_PWRCTL_PDWKIEN_Msk | CLK_PWRCTL_PDWTCPU_Msk;

	// switch HCLK to HXT
	CLK->CLKDIV0 &= ~CLK_CLKDIV0_HCLKSEL_Msk;

	if (vsfhal_info.clk_enable & NUC505_CLK_LXT)
	{
		RTC->SET = RTC_SET_CBEN_Msk | RTC_SET_IOMSEL_Msk;
		//RTC->SET = RTC_SET_CBEN_Msk;
		RTC->CLKSRC &= ~RTC_CLKSRC_CKSRC_Msk;
	}
	else if (vsfhal_info.clk_enable & NUC505_CLK_LIRC)
	{
		RTC->SET = 0;
		RTC->SET = RTC_CLKSRC_CKSRC_Msk;
	}

	if (vsfhal_info.clk_enable & NUC505_CLK_PLL)
	{
		uint32_t n, m, p;

		for (p = 1; p <= 8; p++)
		{
			if ((vsfhal_info.pll_freq_hz * p >= (300 * 1000 * 1000)) &&
					(vsfhal_info.pll_freq_hz * p < (1000 * 1000 * 1000)))
			break;
		}
		if ((p == 8) &&
			((vsfhal_info.pll_freq_hz * 8 < (300 * 1000 * 1000)) ||
				(vsfhal_info.pll_freq_hz * 8 >= (1000 * 1000 * 1000))))
			return VSFERR_INVALID_PARAMETER;

		if (vsfhal_info.pll_freq_hz * p > (768 * 1000 * 1000))
			m = 1;
		else
			m = 2;

		n = vsfhal_info.pll_freq_hz * p * m / vsfhal_info.osc_freq_hz;
		if ((n < 1) || (n > 128))
			return VSFERR_INVALID_PARAMETER;

		CLK->PLLCTL = (n - 1) + ((m - 1) << 7) + ((p - 1) << 13);
	}
	else
	{
		CLK->PLLCTL |= CLK_PLLCTL_PD_Msk;
	}

	// TODO
	if (vsfhal_info.clk_enable & NUC505_CLK_APLL)
	{

	}
	else
	{
		CLK->APLLCTL |= CLK_APLLCTL_PD_Msk;
	}

    // set pclk
	temp32 = vsfhal_info.hclk_freq_hz / vsfhal_info.pclk_freq_hz;
	if ((temp32 < 1) || (temp32 > 16))
		return VSFERR_INVALID_PARAMETER;
	CLK->CLKDIV0 = (CLK->CLKDIV0 & (~CLK_CLKDIV0_PCLKDIV_Msk)) |
					((temp32 - 1) << CLK_CLKDIV0_PCLKDIV_Pos);

	// set hclk
	switch (vsfhal_info.hclksrc)
	{
	case NUC505_HCLKSRC_PLLFOUT:
		temp32 = vsfhal_info.pll_freq_hz;
		reg32 = CLK_CLKDIV0_HCLKSEL_Msk;
		break;
	case NUC505_HCLKSRC_HXT:
		temp32 = vsfhal_info.osc_freq_hz;
		reg32 = 0;
		break;
	default:
		return VSFERR_INVALID_PARAMETER;
	}
	temp32 /= vsfhal_info.hclk_freq_hz;
	if ((temp32 < 1) || (temp32 > 16))
		return VSFERR_INVALID_PARAMETER;
	CLK->CLKDIV0 = ((CLK->CLKDIV0 | reg32) & ~CLK_CLKDIV0_HCLKDIV_Msk) | (temp32 - 1);

	SCB->VTOR = vsfhal_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | vsfhal_info.priority_group;
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

uint16_t vsfhal_tickclk_get_us(void)
{
	uint32_t temp = SysTick->VAL * 1000;
	return temp / SysTick->LOAD;
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

// special
uint32_t vsfhal_get_pc(void)
{
	uint32_t pc;
	asm("MOV	%0,	pc" : "=r" (pc));
	return pc;
}

int32_t vsfhal_is_running_on_ram(void)
{
	uint32_t vecmap_addr = SYS->RVMPADDR, vecmap_len = SYS->RVMPLEN >> 14;
	uint32_t pc = vsfhal_get_pc();

	if (((pc >= 0x20000000) && (pc < 0x20020000)) ||
		((pc >= 0x1ff00000) && (pc < 0x1ff20000)) ||
		((vecmap_addr >= 0x20000000) && (vecmap_addr < 0x20020000) && (vecmap_len > 0)))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

vsf_err_t vsfhal_code_map(uint8_t en, uint8_t rst, uint8_t len_kb, uint32_t addr)
{
	if (!en)
	{
		SYS->LVMPADDR = 0;
		SYS->LVMPLEN = 0;
	}
	else if ((len_kb > 0) && (len_kb <= 128))
	{
		memcpy((void *)0x20000000, (void *)addr, 1024ul * len_kb);
		SYS->LVMPADDR = 0x20000000;
		SYS->LVMPLEN = len_kb;
	}
	else
	{
		return VSFERR_INVALID_PARAMETER;
	}

	if (rst)
	{
		SYS->IPRST0 |= SYS_IPRST0_CPURST_Msk;
	}
	else
	{
		SYS->RVMPLEN |= 0x1;
	}
	return VSFERR_NONE;
}


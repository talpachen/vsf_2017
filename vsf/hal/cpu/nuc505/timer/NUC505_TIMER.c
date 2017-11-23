#include "vsf.h"
#include "core.h"

#if VSFHAL_TIMER_EN

#define VSFHAL_TIMER_NUM		4

static TIMER_T * const timer_list[VSFHAL_TIMER_NUM] =
{
	TIMER0, TIMER1, TIMER2, TIMER3,
};

struct timer_info_t
{
	void (*cb)(void *);
	void *param;
	int32_t trigger_cnt;
} static timer_info[VSFHAL_TIMER_NUM];

vsf_err_t vsfhal_timer_init(uint8_t index, int32_t int_priority)
{
	switch (index)
	{
	case 0:
		CLK->CLKDIV4 &= ~CLK_CLKDIV4_TMR0DIV_Msk;
		CLK->CLKDIV4 |= CLK_CLKDIV4_TMR0SEL_Msk;
		CLK->APBCLK |= CLK_APBCLK_TMR0CKEN_Msk;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TMR0_IRQn, int_priority);
			NVIC_EnableIRQ(TMR0_IRQn);
		}
		break;
	case 1:
		CLK->CLKDIV4 &= ~CLK_CLKDIV4_TMR1DIV_Msk;
		CLK->CLKDIV4 |= CLK_CLKDIV4_TMR1SEL_Msk;
		CLK->APBCLK |= CLK_APBCLK_TMR1CKEN_Msk;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TMR1_IRQn, int_priority);
			NVIC_EnableIRQ(TMR1_IRQn);
		}
		break;
	case 2:
		CLK->CLKDIV4 &= ~CLK_CLKDIV4_TMR2DIV_Msk;
		CLK->CLKDIV4 |= CLK_CLKDIV4_TMR2SEL_Msk;
		CLK->APBCLK |= CLK_APBCLK_TMR2CKEN_Msk;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TMR2_IRQn, int_priority);
			NVIC_EnableIRQ(TMR2_IRQn);
		}
		break;
	case 3:
		CLK->CLKDIV5 &= ~CLK_CLKDIV5_TMR3DIV_Msk;
		CLK->CLKDIV5 |= CLK_CLKDIV5_TMR3SEL_Msk;
		CLK->APBCLK |= CLK_APBCLK_TMR3CKEN_Msk;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TMR3_IRQn, int_priority);
			NVIC_EnableIRQ(TMR3_IRQn);
		}
		break;
	default:
		return VSFERR_FAIL;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_timer_fini(uint8_t index)
{
	switch (index)
	{
	case 0:
		CLK->APBCLK &= ~CLK_APBCLK_TMR0CKEN_Msk;
		break;
	case 1:
		CLK->APBCLK &= ~CLK_APBCLK_TMR1CKEN_Msk;
		break;
	case 2:
		CLK->APBCLK &= ~CLK_APBCLK_TMR2CKEN_Msk;
		break;
	case 3:
		CLK->APBCLK &= ~CLK_APBCLK_TMR3CKEN_Msk;
		break;
	default:
		return VSFERR_FAIL;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_timer_callback_config(uint8_t index, void (*cb)(void *),
		void *param)
{
	timer_info[index].cb = cb;
	timer_info[index].param = param;	
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_timer_callback_start(uint8_t index, uint32_t interval_us,
		int32_t trigger_cnt)
{
	TIMER_T * const timer = timer_list[index];
	
	timer_info[index].trigger_cnt = trigger_cnt;
	if (interval_us)
		timer->CMP = interval_us * 12;
	else
		timer->CMP = 2;	
	timer->CTL = (0x1ul << TIMER_CTL_OPMODE_Pos) | TIMER_CTL_INTEN_Msk | TIMER_CTL_CNTEN_Msk;
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_timer_stop(uint8_t index)
{
	TIMER_T * const timer = timer_list[index];
	timer->CTL = 0;
	
	return VSFERR_NONE;
}

void timer_irq(uint8_t index)
{
	TIMER_T * const timer = timer_list[index];

	if (timer_info[index].trigger_cnt > 0)
		timer_info[index].trigger_cnt--;
	if (timer_info[index].trigger_cnt == 0)
		timer->CTL = 0;
	
	if (timer_info[index].cb)
		timer_info[index].cb(timer_info[index].param);
	
	timer->INTSTS = TIMER_INTSTS_TIF_Msk;
}

ROOTFUNC void TMR0_IRQHandler(void)
{
	timer_irq(0);
}

ROOTFUNC void TMR1_IRQHandler(void)
{
	timer_irq(1);
}

ROOTFUNC void TMR2_IRQHandler(void)
{
	timer_irq(2);
}

ROOTFUNC void TMR3_IRQHandler(void)
{
	timer_irq(3);
}

#endif	// VSFHAL_TIMER_EN


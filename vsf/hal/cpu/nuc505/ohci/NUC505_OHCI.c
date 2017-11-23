/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       GPIO.h                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    GPIO interface header file                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/
#include "vsf.h"

#if VSFHAL_HCD_EN

#include "NUC505_OHCI.h"
#include "core.h"

struct vsfhal_ohci_irq_t
{
	void *param;
	vsf_err_t (*irq)(void*);
} static vsfhal_ohci_irq;

ROOTFUNC void USBH_IRQHandler(void)
{
	if (vsfhal_ohci_irq.irq != NULL)
	{
		vsfhal_ohci_irq.irq(vsfhal_ohci_irq.param);
	}
}

vsf_err_t vsfhal_hcd_init(uint32_t index, int32_t int_priority, vsf_err_t (*ohci_irq)(void *), void *param)
{
	uint8_t div;
	struct vsfhal_info_t *info;

	if (ohci_irq != NULL)
	{
		vsfhal_ohci_irq.irq = ohci_irq;
		vsfhal_ohci_irq.param = param;
	}
	
	// usbh clock 48M
	vsfhal_core_get_info(&info);
	div = info->pll_freq_hz / 48000000;
	CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_USBHDIV_Msk) | 
			CLK_CLKDIV0_USBHSEL_Msk | ((div - 1) << CLK_CLKDIV0_USBHDIV_Pos);
	CLK->AHBCLK |= CLK_AHBCLK_USBHCKEN_Msk;

	if (int_priority >= 0)
	{
		NVIC_SetPriority(USBH_IRQn, int_priority);
		NVIC_EnableIRQ(USBH_IRQn);
	}
	
	if (index & VSFHAL_HCD_PORT1_MASK)
	{
		if (index & VSFHAL_HCD_PORT1_DP_PB12_MASK)
		{
			PB->PUEN = (PB->PUEN & ~GPIO_PUEN_PULLSEL12_Msk) | (0x2ul << GPIO_PUEN_PULLSEL12_Pos);
			SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB12MFP_Msk) |
					(2 << SYS_GPB_MFPH_PB12MFP_Pos);
		}
		if (index & VSFHAL_HCD_PORT1_DM_PB13_MASK)
		{
			PB->PUEN = (PB->PUEN & ~GPIO_PUEN_PULLSEL13_Msk) | (0x2ul << GPIO_PUEN_PULLSEL13_Pos);
			SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB13MFP_Msk) |
					(2 << SYS_GPB_MFPH_PB13MFP_Pos);
		}
		if (index & VSFHAL_HCD_PORT1_DP_PB14_MASK)
		{
			PB->PUEN = (PB->PUEN & ~GPIO_PUEN_PULLSEL14_Msk) | (0x2ul << GPIO_PUEN_PULLSEL14_Pos);
			SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB14MFP_Msk) |
					(1 << SYS_GPB_MFPH_PB14MFP_Pos);
		}
		if (index & VSFHAL_HCD_PORT1_DM_PB15_MASK)
		{
			PB->PUEN = (PB->PUEN & ~GPIO_PUEN_PULLSEL15_Msk) | (0x2ul << GPIO_PUEN_PULLSEL15_Pos);
			SYS->GPB_MFPH = (SYS->GPB_MFPH & ~SYS_GPB_MFPH_PB15MFP_Msk) |
					(1 << SYS_GPB_MFPH_PB15MFP_Pos);
		}
	}
	
	if (index & VSFHAL_HCD_PORT2_MASK)
	{
		PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL13_Msk) | (0x2ul << GPIO_PUEN_PULLSEL13_Pos);
		PC->PUEN = (PC->PUEN & ~GPIO_PUEN_PULLSEL14_Msk) | (0x2ul << GPIO_PUEN_PULLSEL14_Pos);
		SYS->GPC_MFPH = (SYS->GPC_MFPH & ~(SYS_GPC_MFPH_PC13MFP_Msk | SYS_GPC_MFPH_PC14MFP_Msk)) |
				(1 << SYS_GPC_MFPH_PC13MFP_Pos) | (1 << SYS_GPC_MFPH_PC14MFP_Pos);
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_hcd_fini(uint32_t index)
{
	switch (index & 0x0f)
	{
	case 0:
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}

void* vsfhal_hcd_regbase(uint32_t index)
{
	switch (index & 0x0f)
	{
	case 0:
		return (void*)USBH;
	default:
		return NULL;
	}
}

#endif

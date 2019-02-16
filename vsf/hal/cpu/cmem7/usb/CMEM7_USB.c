#include "vsf.h"

#if VSFHAL_USB_EN

struct
{
	void *param;
	void (*irq)(void*);
} static vsfhal_usb_irq[VSFHAL_USB_NUM];

#ifdef VSFHAL_USB_HS_INDEX
ROOT void USB_IRQHandler(void)
{
	if (vsfhal_usb_irq[VSFHAL_USB_HS_INDEX].irq != NULL)
	{
		vsfhal_usb_irq[VSFHAL_USB_HS_INDEX].irq(vsfhal_usb_irq[VSFHAL_USB_HS_INDEX].param);
	}
}
#endif

vsf_err_t vsfhal_hcd_init(uint32_t index, int32_t int_priority, void (*irq)(void *), void *param)
{
	if (index >= VSFHAL_USB_NUM)
		return VSFERR_NOT_SUPPORT;
	
	vsfhal_usb_irq[index].irq = irq;
	vsfhal_usb_irq[index].param = param;
	
	GLOBAL_CTRL->USB_PHY_CTRL_b.CKISEL = 1;
	while (!GLOBAL_CTRL->USB_PHY_CTRL_b.CKISEL);
	GLOBAL_CTRL->USB_PHY_CTRL_b.RSTPRT = 1;
	while (!GLOBAL_CTRL->USB_PHY_CTRL_b.RSTPRT);
	GLOBAL_CTRL->USB_PHY_CTRL_b.RSTPRT = 0;
	while (GLOBAL_CTRL->USB_PHY_CTRL_b.RSTPRT);

	NVIC_SetPriority(USB_INT_IRQn, int_priority);
	NVIC_EnableIRQ(USB_INT_IRQn);	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_hcd_fini(uint32_t index)
{
	if (index >= VSFHAL_USB_NUM)
		return VSFERR_NOT_SUPPORT;
	
	vsfhal_usb_irq[index].irq = NULL;
	vsfhal_usb_irq[index].param = NULL;
	
	NVIC_DisableIRQ(USB_INT_IRQn);
	return VSFERR_NONE;
}

void *vsfhal_hcd_regbase(uint32_t index)
{
	switch (index)
	{
#ifdef VSFHAL_USB_HS_INDEX
	case VSFHAL_USB_HS_INDEX:
		return (void*)USB;
#endif
	default:
		return NULL;
	}
}

void * cmem7_hcd_regbase(uint32_t index)
{
	switch (index >> 16)
	{
	default:
		return NULL;
	}
}

#endif // IFS_HCD_EN


#if IFS_USB_DCD_EN

#endif // IFS_HCD_EN


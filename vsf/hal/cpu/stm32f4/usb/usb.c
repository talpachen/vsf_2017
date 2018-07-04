#include "vsf.h"

#if VSFHAL_USBD_EN

struct
{
	void *param;
	void (*irq)(void*);
} static vsfhal_usb_irq[VSFHAL_USB_NUM];

#ifdef VSFHAL_USB_FS_INDEX
ROOTFUNC void OTG_FS_IRQHandler(void)
{
	if (vsfhal_usb_irq[VSFHAL_USB_FS_INDEX].irq != NULL)
	{
		vsfhal_usb_irq[VSFHAL_USB_FS_INDEX].irq(vsfhal_usb_irq[VSFHAL_USB_FS_INDEX].param);
	}
}
#endif
#ifdef VSFHAL_USB_HS_INDEX
ROOTFUNC void OTG_HS_IRQHandler(void)
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
	
#ifdef VSFHAL_USB_FS_INDEX
	if (index == VSFHAL_USB_FS_INDEX)
	{
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
		SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
		// PA10 id AF OD
		GPIOA->MODER &= ~(3 << (10 * 2));
		GPIOA->MODER |= 2 << (10 * 2);
		GPIOA->AFR[1] |= 10 << (10 * 4 - 32);
		// GPIOA->OSPEEDR |= 3 << (10 * 2);
		GPIOA->OTYPER |= 1 << 10;
		GPIOA->PUPDR &= ~(3 << (10 * 2));
		GPIOA->PUPDR |= 1 << (10 * 2);

		//  PA11
		GPIOA->MODER &= ~(3 << (11 * 2));
		GPIOA->MODER |= 2 << (11 * 2);
		GPIOA->AFR[1] |= 10 << (11 * 4 - 32);
		GPIOA->OSPEEDR |= 3 << (11 * 2);
		GPIOA->PUPDR &= ~(3 << (11 * 2));
		
		// PA12
		GPIOA->MODER &= ~(3 << (12 * 2));
		GPIOA->MODER |= 2 << (12 * 2);
		GPIOA->AFR[1] |= 10 << (12 * 4 - 32);
		GPIOA->OSPEEDR |= 3 << (12 * 2);
		GPIOA->PUPDR &= ~(3 << (12 * 2));		
		
		SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);
		NVIC_SetPriority(OTG_FS_IRQn, int_priority);
		NVIC_EnableIRQ(OTG_FS_IRQn);
	}
#endif
#ifdef VSFHAL_USB_HS_INDEX
	if (index == VSFHAL_USB_HS_INDEX)
	{
		// TODO
	}
#endif

	return VSFERR_NONE;
}

vsf_err_t vsfhal_hcd_fini(uint32_t index)
{
	if (index >= VSFHAL_USB_NUM)
		return VSFERR_NOT_SUPPORT;
	
	vsfhal_usb_irq[index].irq = NULL;
	vsfhal_usb_irq[index].param = NULL;
	
#ifdef VSFHAL_USB_FS_INDEX
	if (index == VSFHAL_USB_FS_INDEX)
	{
		CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);
		NVIC_DisableIRQ(OTG_FS_IRQn);
	}
#endif
#ifdef VSFHAL_USB_HS_INDEX
	if (usb_id == VSFHAL_USB_HS_INDEX)
	{
		// TODO
	}
#endif

	return VSFERR_NONE;
}


void *vsfhal_hcd_regbase(uint32_t index)
{
	switch (index)
	{
#ifdef VSFHAL_USB_FS_INDEX
	case VSFHAL_USB_FS_INDEX:
		return (void*)USB_OTG_FS;
#endif
#ifdef VSFHAL_USB_HS_INDEX
	case VSFHAL_USB_HS_INDEX:
		return (void*)USB_OTG_HS;
#endif
	default:
		return NULL;
	}
}

#endif // VSFHAL_USBD_EN


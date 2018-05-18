#include "vsf.h"

#if VSFHAL_USB_EN

#define USB_NUM			1
#define USB_FS			0
//#define USB_HS			1

struct vsfhal_ohci_irq_t
{
	void *param;
	void (*irq)(void*);
} static vsfhal_usb_irq[USB_NUM];

#ifdef USB_FS
ROOTFUNC void OTG_FS_IRQHandler(void)
{
	if (vsfhal_usb_irq[USB_FS].irq != NULL)
	{
		vsfhal_usb_irq[USB_FS].irq(vsfhal_usb_irq[USB_FS].param);
	}
}
#endif
#ifdef USB_HS
ROOTFUNC void OTG_HS_IRQHandler(void)
{
	if (vsfhal_usb_irq[USB_HS].irq != NULL)
	{
		vsfhal_usb_irq[USB_HS].irq(vsfhal_usb_irq[USB_HS].param);
	}
}
#endif

vsf_err_t vsfhal_hcd_init(uint32_t index, int32_t int_priority, void (*ohci_irq)(void *), void *param)
{
	if (usb_id >= USB_NUM)
		return VSFERR_NOT_SUPPORT;
	
	// enable 48M clock
	// see core.c
	
	vsfhal_usb_irq[index].irq = irq;
	vsfhal_usb_irq[index].param = param;
	
#ifdef USB_FS
	if (usb_id == USB_FS)
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
		NVIC_EnableIRQ(OTG_FS_IRQn);
	}
#endif
#ifdef USB_HS
	if (usb_id == USB_HS)
	{
		// TODO
	}
#endif

	return VSFERR_NONE;
}

vsf_err_t stm32f4_hcd_fini(uint32_t index)
{
	if (usb_id >= USB_NUM)
		return VSFERR_NOT_SUPPORT;
	
	vsfhal_usb_irq[index].irq = NULL;
	vsfhal_usb_irq[index].param = NULL;
	
#ifdef USB_FS
	if (usb_id == USB_FS)
	{
		CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);
		NVIC_DisableIRQ(OTG_FS_IRQn);
	}
#endif
#ifdef USB_HS
	if (usb_id == USB_HS)
	{
		// TODO
	}
#endif

	return VSFERR_NONE;
}


void* vsfhal_hcd_regbase(uint32_t index)
{
	switch (index)
	{
#ifdef USB_FS
	case USB_FS:
		return (void*)USB_OTG_FS;
#endif
#ifdef USB_HS
	case USB_HS:
		return (void*)USB_OTG_HS;
#endif
	default:
		return NULL;
	}
}

#endif // VSFHAL_USB_EN


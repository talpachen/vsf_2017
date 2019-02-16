#include "vsf.h"

#if VSFHAL_USBH_EN

struct
{
	void *param;
	void (*irq)(void*);
} static vsfhal_usb_irq[VSFHAL_USB_NUM];

#ifdef VSFHAL_USB_FS_INDEX
ROOT void OTG_FS_IRQHandler(void)
{
	if (vsfhal_usb_irq[VSFHAL_USB_FS_INDEX].irq != NULL)
	{
		vsfhal_usb_irq[VSFHAL_USB_FS_INDEX].irq(vsfhal_usb_irq[VSFHAL_USB_FS_INDEX].param);
	}
}
#endif
#ifdef VSFHAL_USB_HS_INDEX
ROOT void OTG_HS_IRQHandler(void)
{
	if (vsfhal_usb_irq[VSFHAL_USB_HS_INDEX].irq != NULL)
	{
		vsfhal_usb_irq[VSFHAL_USB_HS_INDEX].irq(vsfhal_usb_irq[VSFHAL_USB_HS_INDEX].param);
	}
}
#endif

static void ulpi_io_config(GPIO_TypeDef *gpiox, uint8_t io)
{
	gpiox->MODER &= ~(GPIO_MODER_MODER0 << (io * 2));
	gpiox->MODER |= GPIO_MODER_MODER0_1 << (io * 2);
	gpiox->OTYPER &= ~(GPIO_OTYPER_OT_0 << (io * 2));
	gpiox->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0 << (io * 2);
	gpiox->PUPDR &= ~(GPIO_PUPDR_PUPDR0 << (io * 2));
	if (io < 8)
	{
		gpiox->AFR[0] &= ~(0xful << (io * 4));
		gpiox->AFR[0] |= 0xaul << (io * 4);
	}
	else
	{
		gpiox->AFR[1] &= ~(0xful << ((io - 8) * 4));
		gpiox->AFR[1] |= 0xaul << ((io - 8) * 4);
	}
}

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
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN |
				RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOHEN | RCC_AHB1ENR_GPIOIEN;

		ulpi_io_config(VSFHAL_USBHS_ULPI_CK_GPIO, VSFHAL_USBHS_ULPI_CK_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_D0_GPIO, VSFHAL_USBHS_ULPI_D0_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_D1_GPIO, VSFHAL_USBHS_ULPI_D1_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_D2_GPIO, VSFHAL_USBHS_ULPI_D2_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_D3_GPIO, VSFHAL_USBHS_ULPI_D3_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_D4_GPIO, VSFHAL_USBHS_ULPI_D4_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_D5_GPIO, VSFHAL_USBHS_ULPI_D5_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_D6_GPIO, VSFHAL_USBHS_ULPI_D6_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_D7_GPIO, VSFHAL_USBHS_ULPI_D7_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_STP_GPIO, VSFHAL_USBHS_ULPI_STP_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_NXT_GPIO, VSFHAL_USBHS_ULPI_NXT_IO);
		ulpi_io_config(VSFHAL_USBHS_ULPI_DIR_GPIO, VSFHAL_USBHS_ULPI_DIR_IO);

		SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_OTGHSULPIEN);
		SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_OTGHSEN);
		NVIC_EnableIRQ(OTG_HS_IRQn);
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
		RCC->AHB2ENR &= ~(RCC_AHB2ENR_OTGFSEN);
		NVIC_DisableIRQ(OTG_FS_IRQn);
	}
#endif
#ifdef VSFHAL_USB_HS_INDEX
	if (index == VSFHAL_USB_HS_INDEX)
	{
		RCC->AHB1ENR &= ~(RCC_AHB1ENR_OTGHSULPIEN);
		RCC->AHB1ENR &= ~(RCC_AHB1ENR_OTGHSEN);
		NVIC_DisableIRQ(OTG_HS_IRQn);
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

#endif // VSFHAL_USBH_EN


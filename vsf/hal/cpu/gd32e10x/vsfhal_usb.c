#include "vsf.h"
#include "core.h"

#if VSFHAL_USB_EN

#define VSFHAL_USB_NUM				1

struct
{
	void *param;
	void (*irq)(void*);
} static vsfhal_usb_irq[VSFHAL_USB_NUM];

vsf_err_t vsfhal_usb_init(uint32_t index, int32_t int_priority,
		void (*irq)(void *), void *param)
{
	struct vsfhal_info_t *vsfhal_info;
	
	if (index >= VSFHAL_USB_NUM)
		return VSFERR_NOT_SUPPORT;

	if (vsfhal_core_get_info(&vsfhal_info))
		return VSFERR_FAIL;
	if (vsfhal_info->clk_enable & GD32E10X_CLK_HSI48M)
	{
		RCU_ADDAPB1EN |= RCU_ADDAPB1EN_CTCEN;
		CTC_CTL1 = (0x2ul << 28) | (0x1cul << 16) | (48000 - 1);
		CTC_CTL0 |= CTC_CTL0_AUTOTRIM | CTC_CTL0_CNTEN;

		RCU_ADDCTL |= RCU_ADDCTL_CK48MSEL;
	}
	else if (vsfhal_info->clk_enable & GD32E10X_CLK_PLL)
	{
		switch (vsfhal_info->pll_freq_hz)
		{
		case 48000000:	// div 1
			RCU_CFG0 &= ~(RCU_CFG0_USBFSPSC | RCU_CFG0_USBFSPSC_2);
			RCU_CFG0 |= 0x1ul << 22;
			break;
		case 72000000:	// div 1.5
			RCU_CFG0 &= ~(RCU_CFG0_USBFSPSC | RCU_CFG0_USBFSPSC_2);
			break;
		case 96000000:	// div 2
			RCU_CFG0 &= ~(RCU_CFG0_USBFSPSC | RCU_CFG0_USBFSPSC_2);
			RCU_CFG0 |= 0x3ul << 22;
			break;
		case 120000000:	// div 2.5
			RCU_CFG0 &= ~(RCU_CFG0_USBFSPSC | RCU_CFG0_USBFSPSC_2);
			RCU_CFG0 |= 0x2ul << 22;
			break;
		case 144000000:	// div 3
			RCU_CFG0 &= ~(RCU_CFG0_USBFSPSC | RCU_CFG0_USBFSPSC_2);
			RCU_CFG0 |= RCU_CFG0_USBFSPSC_2;
			break;
		case 168000000:	// div 3.5
			RCU_CFG0 &= ~(RCU_CFG0_USBFSPSC | RCU_CFG0_USBFSPSC_2);
			RCU_CFG0 |= (0x1ul << 22) | RCU_CFG0_USBFSPSC_2;
			break;
		case 192000000:	// div 4
			RCU_CFG0 &= ~(RCU_CFG0_USBFSPSC | RCU_CFG0_USBFSPSC_2);
			RCU_CFG0 |= (0x2ul << 22) | RCU_CFG0_USBFSPSC_2;
			break;
		default:
			return VSFERR_FAIL;
		}
		RCU_ADDCTL &= ~RCU_ADDCTL_CK48MSEL;
	}
	else
		return VSFERR_FAIL;
	RCU_AHBEN |= RCU_AHBEN_USBFSEN;

	// disable USB global interrupt
	USB_GAHBCS &= ~GAHBCS_GINTEN;
	
	// soft reset the core
	USB_GRSTCTL |= GRSTCTL_CSRST;
	while (USB_GRSTCTL & GRSTCTL_CSRST);	
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");

	USB_GCCFG |= GCCFG_PWRON | GCCFG_VBUSACEN | GCCFG_VBUSBCEN | GCCFG_VBUSIG;

	if (vsfhal_info->clk_enable & GD32E10X_CLK_HSI48M)
		USB_GCCFG |= GCCFG_SOFOEN;

	vsfhal_usb_irq[index].irq = irq;
	vsfhal_usb_irq[index].param = param;

	if (int_priority >= 0)
	{
		NVIC_SetPriority(USBFS_IRQn, int_priority);
		NVIC_EnableIRQ(USBFS_IRQn);
	}
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usb_fini(uint32_t index)
{
	if (index >= VSFHAL_USB_NUM)
		return VSFERR_NOT_SUPPORT;
	
	vsfhal_usb_irq[index].irq = NULL;
	vsfhal_usb_irq[index].param = NULL;
	
	NVIC_DisableIRQ(USBFS_IRQn);
	return VSFERR_NONE;
}

void *vsfhal_usb_regbase(uint32_t index)
{
	if (index >= VSFHAL_USB_NUM)
		return NULL;
	
	return (void *)USBFS_BASE;
}

ROOT void USBFS_IRQHandler(void)
{
	if (vsfhal_usb_irq[0].irq)
		vsfhal_usb_irq[0].irq(vsfhal_usb_irq[0].param);
}


#ifdef VSFDWC2_DEVICE_ENABLE
static void vsfhal_usbd_cb(enum vsfhal_usbd_evt_t evt, uint32_t value);
const uint8_t vsfhal_usbd_ep_num = VSFUSBD_CFG_EPMAXNO;
struct vsfhal_usbd_callback_t vsfhal_usbd_callback;
static struct vsfdwcotg_device_rx_ep_pkt_t rx_ep_pkt[VSFUSBD_CFG_EPMAXNO];
struct vsfdwc2_device_t vsfhal_usb_dwc2_device;

vsf_err_t vsfhal_usb_dwc2_device_init(void *p, uint32_t *rx_buff, uint32_t rx_buffer_size)
{
	struct vsfdwc2_hcd_param_t *hc_param = p;
	uint32_t regbase;

	if (!hc_param || !rx_buff)
		return VSFERR_FAIL;

	regbase = (uint32_t)vsfhal_usb_regbase(hc_param->index);
	if (!regbase)
		return VSFERR_FAIL;

	memset(rx_ep_pkt, 0, sizeof(struct vsfdwcotg_device_rx_ep_pkt_t) * VSFUSBD_CFG_EPMAXNO);
	
	vsfhal_usb_dwc2_device.hc_param = hc_param;
	vsfhal_usb_dwc2_device.cb = vsfhal_usbd_cb;
	vsfhal_usb_dwc2_device.rx_ep_pkt = rx_ep_pkt;
	vsfhal_usb_dwc2_device.device_rx_buffer = rx_buff;
	vsfhal_usb_dwc2_device.device_rx_buffer_size = rx_buffer_size;
	vsfhal_usb_dwc2_device.device_rx_buffer_pos = 0;

	vsfhal_usb_dwc2_device.global_reg = (void *)(regbase + 0x000);
	vsfhal_usb_dwc2_device.host_global_regs = (void *)(regbase + 0x400);
	vsfhal_usb_dwc2_device.hprt0 = (void *)(regbase + 0x440);
	vsfhal_usb_dwc2_device.hc_regs = (void *)(regbase + 0x500);
	vsfhal_usb_dwc2_device.dev_global_regs = (void *)(regbase + 0x800);
	vsfhal_usb_dwc2_device.in_ep_regs = (void *)(regbase + 0x900);
	vsfhal_usb_dwc2_device.out_ep_regs = (void *)(regbase + 0xB00);
#if VSFDWCOTG_HC_EP_AMOUNT_LIMIT >= 1
	vsfhal_usb_dwc2_device.dfifo[0] = (void *)(regbase + 0x1000);
#endif
#if VSFDWCOTG_HC_EP_AMOUNT_LIMIT >= 2
	vsfhal_usb_dwc2_device.dfifo[1] = (void *)(regbase + 0x2000);
#endif
#if VSFDWCOTG_HC_EP_AMOUNT_LIMIT >= 3
	vsfhal_usb_dwc2_device.dfifo[2] = (void *)(regbase + 0x3000);
#endif
#if VSFDWCOTG_HC_EP_AMOUNT_LIMIT >= 4
	vsfhal_usb_dwc2_device.dfifo[3] = (void *)(regbase + 0x4000);
#endif
#if VSFDWCOTG_HC_EP_AMOUNT_LIMIT >= 5
	vsfhal_usb_dwc2_device.dfifo[4] = (void *)(regbase + 0x5000);
#endif
#if VSFDWCOTG_HC_EP_AMOUNT_LIMIT >= 6
	vsfhal_usb_dwc2_device.dfifo[5] = (void *)(regbase + 0x6000);
#endif
#if VSFDWCOTG_HC_EP_AMOUNT_LIMIT >= 7
	vsfhal_usb_dwc2_device.dfifo[6] = (void *)(regbase + 0x7000);
#endif
#if VSFDWCOTG_HC_EP_AMOUNT_LIMIT >= 8
	vsfhal_usb_dwc2_device.dfifo[7] = (void *)(regbase + 0x8000);
#endif

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usb_dwc2_device_fini(void *p)
{
	//struct vsfdwc2_hcd_param_t *hc_param = p;

	RCU_AHBEN &= ~RCU_AHBEN_USBFSEN;
	return VSFERR_NONE;
}

static void vsfhal_usbd_cb(enum vsfhal_usbd_evt_t evt, uint32_t value)
{
	if (vsfhal_usbd_callback.on_event != NULL)
		vsfhal_usbd_callback.on_event(vsfhal_usbd_callback.param, evt, value);
}
#endif

#ifdef VSFDWC2_HOST_ENABLE
// TODO
#endif

#endif

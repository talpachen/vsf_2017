#include "vsf.h"
#include "vsfdwcotg_priv.h"

#ifdef VSFDWC2_DEVICE_ENABLE
static void vsfdwc2_usbd_interrupt(void *p);

static void vsfdwc2_core_reset(struct vsfdwc2_device_t *otgd)
{
	otgd->global_reg->grstctl |= USB_OTG_GRSTCTL_CSRST;
	while (otgd->global_reg->grstctl & USB_OTG_GRSTCTL_CSRST);
	while ((otgd->global_reg->grstctl & USB_OTG_GRSTCTL_AHBIDL) == 0U);
}

vsf_err_t vsfdwc2_usbd_init(struct vsfdwc2_device_t *otgd, int32_t int_priority)
{
	if (vsfhal_usb_init(otgd->hc_param->index, int_priority,
			vsfdwc2_usbd_interrupt, otgd) != VSFERR_NONE)
		return VSFERR_FAIL;

	if (otgd->hc_param->ulpi_en)
	{
		// Init The ULPI Interface
		otgd->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_TSDPS | USB_OTG_GUSBCFG_ULPIFSLS | USB_OTG_GUSBCFG_PHYSEL);
	
		// Select vbus source
		otgd->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_ULPIEVBUSD | USB_OTG_GUSBCFG_ULPIEVBUSI);
		if (otgd->hc_param->vbus_en)
			otgd->global_reg->gusbcfg |= USB_OTG_GUSBCFG_ULPIEVBUSD;
		
		// Reset after a PHY select
		vsfdwc2_core_reset(otgd);
	}
	else if (otgd->hc_param->utmi_en)
	{
		// Init The UTMI Interface: UTMI, 8bit, High Speed
		otgd->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_PHYSEL | USB_OTG_GUSBCFG_ULPI_UTMI_SEL |
				USB_OTG_GUSBCFG_PHYIF | USB_OTG_GUSBCFG_ULPIFSLS);

		// Select vbus source
		otgd->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_ULPIEVBUSD | USB_OTG_GUSBCFG_ULPIEVBUSI);

		// Reset after a PHY select
		vsfdwc2_core_reset(otgd);
	}
	else
	{
		// Select FS Embedded PHY
		otgd->global_reg->gusbcfg |= USB_OTG_GUSBCFG_PHYSEL;
		
		// Reset after a PHY select
		vsfdwc2_core_reset(otgd);
	}

	otgd->global_reg->gahbcfg |= USB_OTG_GAHBCFG_TXFELVL;
	if (otgd->hc_param->dma_en)
		otgd->global_reg->gahbcfg |= USB_OTG_GAHBCFG_HBSTLEN_0 | USB_OTG_GAHBCFG_DMAEN;

	// set device mode
	otgd->global_reg->gusbcfg &= ~USB_OTG_GUSBCFG_FHMOD;
	otgd->global_reg->gusbcfg |= USB_OTG_GUSBCFG_FDMOD;

	if (otgd->hc_param->speed == USB_SPEED_HIGH)
		otgd->dev_global_regs->dcfg &= ~USB_OTG_DCFG_DSPD;
	else if (otgd->hc_param->ulpi_en || otgd->hc_param->utmi_en)
		otgd->dev_global_regs->dcfg = (otgd->dev_global_regs->dcfg & ~USB_OTG_DCFG_DSPD) |
			USB_OTG_DCFG_DSPD_0;
	else	// set full speed PHY, config 80% periodic frmae interval to default
		otgd->dev_global_regs->dcfg = USB_OTG_DCFG_DSPD_0 | USB_OTG_DCFG_DSPD_1;

	// disconnect
	otgd->dev_global_regs->dctl |= USB_OTG_DCTL_SDIS;

	otgd->dev_global_regs->doepmsk = USB_OTG_DOEPMSK_XFRCM | USB_OTG_DOEPMSK_EPDM | USB_OTG_DOEPMSK_STUPM;
	otgd->dev_global_regs->diepmsk = USB_OTG_DIEPMSK_XFRCM | USB_OTG_DIEPMSK_EPDM | USB_OTG_DIEPMSK_TOM;
	otgd->dev_global_regs->daint = 0xffffffff;
	otgd->dev_global_regs->daintmsk = 0;

	otgd->global_reg->gintsts = 0xbfffffff;
	otgd->global_reg->gotgint = 0xffffffff;

	otgd->global_reg->gintmsk = USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM |
			USB_OTG_GINTMSK_IEPINT | USB_OTG_GINTMSK_OEPINT |
			USB_OTG_GINTMSK_IISOIXFRM | USB_OTG_GINTMSK_PXFRM_IISOOXFRM |
			USB_OTG_GINTMSK_RXFLVLM;

	otgd->global_reg->gahbcfg |= USB_OTG_GAHBCFG_GINT;

	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_fini(struct vsfdwc2_device_t *otgd)
{
	return vsfhal_usb_dwc2_device_fini(otgd->hc_param);
}

vsf_err_t vsfdwc2_usbd_reset(struct vsfdwc2_device_t *otgd)
{
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_connect(struct vsfdwc2_device_t *otgd)
{
	otgd->dev_global_regs->dctl &= ~USB_OTG_DCTL_SDIS;
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_disconnect(struct vsfdwc2_device_t *otgd)
{
	otgd->dev_global_regs->dctl |= USB_OTG_DCTL_SDIS;
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_set_address(struct vsfdwc2_device_t *otgd, uint8_t address)
{
	uint32_t v = address & 0x7f;
	v <<= 4;
	otgd->dev_global_regs->dcfg = (otgd->dev_global_regs->dcfg & ~USB_OTG_DCFG_DAD) | v;
	return VSFERR_NONE;
}

uint8_t vsfdwc2_usbd_get_address(struct vsfdwc2_device_t *otgd)
{
	return (otgd->dev_global_regs->dcfg & USB_OTG_DCFG_DAD) >> 4;
}

vsf_err_t vsfdwc2_usbd_wakeup(struct vsfdwc2_device_t *otgd)
{
	return VSFERR_NONE;
}

uint32_t vsfdwc2_usbd_get_frame_number(struct vsfdwc2_device_t *otgd)
{
	return (otgd->dev_global_regs->dsts & USB_OTG_DSTS_FNSOF) >> 8;
}

vsf_err_t vsfdwc2_usbd_get_setup(struct vsfdwc2_device_t *otgd, uint8_t *buffer)
{
	if (otgd->rx_ep_pkt[0].packet_size == 8)
	{
		memcpy(buffer, otgd->rx_ep_pkt[0].buf, 8);
		otgd->rx_ep_pkt[0].packet_size = 0;
		return VSFERR_NONE;
	}
	else
		return VSFERR_FAIL;
}

vsf_err_t vsfdwc2_usbd_prepare_buffer(struct vsfdwc2_device_t *otgd)
{
	uint16_t i, depth;

	otgd->global_reg->grxfsiz = otgd->hc_param->device_rx_fifo_depth;
	depth = otgd->hc_param->device_rx_fifo_depth;
	otgd->global_reg->gnptxfsiz = ((uint32_t)otgd->hc_param->device_tx_fifo_depth[0] << 16) | depth;
	depth += otgd->hc_param->device_tx_fifo_depth[0];
	for (i = 0; i < (otgd->hc_param->hc_ep_amount - 1); i++)
	{
		otgd->global_reg->dtxfsiz[i] = ((uint32_t)otgd->hc_param->device_tx_fifo_depth[i + 1] << 16) + depth;
		depth += otgd->hc_param->device_tx_fifo_depth[i + 1];
	}

	otgd->global_reg->grstctl = USB_OTG_GRSTCTL_TXFNUM | USB_OTG_GRSTCTL_TXFFLSH | USB_OTG_GRSTCTL_RXFFLSH;
	while (otgd->global_reg->grstctl & (USB_OTG_GRSTCTL_TXFFLSH | USB_OTG_GRSTCTL_RXFFLSH));
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_reset(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_set_type(struct vsfdwc2_device_t *otgd, uint8_t idx, enum vsfhal_usbd_eptype_t type)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	if (idx == 0)
	{
		otgd->in_ep_regs[0].diepctl |= USB_OTG_DIEPCTL_USBAEP;
		otgd->out_ep_regs[0].doepctl |= USB_OTG_DOEPCTL_USBAEP;
	}
	else
	{
		switch (type)
		{
		case USB_EP_TYPE_CONTROL:
			if (otgd->in_ep_regs[idx].diepctl & USB_OTG_DIEPCTL_MPSIZ)
				otgd->in_ep_regs[idx].diepctl = (otgd->in_ep_regs[idx].diepctl & ~USB_OTG_DIEPCTL_EPTYP) | USB_OTG_DIEPCTL_USBAEP;
			if (otgd->out_ep_regs[idx].doepctl & USB_OTG_DOEPCTL_MPSIZ)
				otgd->out_ep_regs[idx].doepctl = (otgd->out_ep_regs[idx].doepctl & ~USB_OTG_DOEPCTL_EPTYP) | USB_OTG_DOEPCTL_USBAEP;
			break;
		case USB_EP_TYPE_INTERRUPT:
			if (otgd->in_ep_regs[idx].diepctl & USB_OTG_DIEPCTL_MPSIZ)
				otgd->in_ep_regs[idx].diepctl = (otgd->in_ep_regs[idx].diepctl & ~USB_OTG_DIEPCTL_EPTYP) | (0x3ul << 18) | USB_OTG_DIEPCTL_USBAEP;
			if (otgd->out_ep_regs[idx].doepctl & USB_OTG_DOEPCTL_MPSIZ)
				otgd->out_ep_regs[idx].doepctl = (otgd->out_ep_regs[idx].doepctl & ~USB_OTG_DOEPCTL_EPTYP) | (0x3ul << 18) | USB_OTG_DOEPCTL_USBAEP;
			break;
		case USB_EP_TYPE_BULK:
			if (otgd->in_ep_regs[idx].diepctl & USB_OTG_DIEPCTL_MPSIZ)
				otgd->in_ep_regs[idx].diepctl = (otgd->in_ep_regs[idx].diepctl & ~USB_OTG_DIEPCTL_EPTYP) | (0x2ul << 18) | USB_OTG_DIEPCTL_USBAEP;
			if (otgd->out_ep_regs[idx].doepctl & USB_OTG_DOEPCTL_MPSIZ)
				otgd->out_ep_regs[idx].doepctl = (otgd->out_ep_regs[idx].doepctl & ~USB_OTG_DOEPCTL_EPTYP) | (0x2ul << 18) | USB_OTG_DOEPCTL_USBAEP;
			break;
		case USB_EP_TYPE_ISO:
			if (otgd->in_ep_regs[idx].diepctl & USB_OTG_DIEPCTL_MPSIZ)
				otgd->in_ep_regs[idx].diepctl = (otgd->in_ep_regs[idx].diepctl & ~USB_OTG_DIEPCTL_EPTYP) | (0x1ul << 18) | USB_OTG_DIEPCTL_USBAEP;
			if (otgd->out_ep_regs[idx].doepctl & USB_OTG_DOEPCTL_MPSIZ)
				otgd->out_ep_regs[idx].doepctl = (otgd->out_ep_regs[idx].doepctl & ~USB_OTG_DOEPCTL_EPTYP) | (0x1ul << 18) | USB_OTG_DOEPCTL_USBAEP;
			break;
		}
	}
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_set_IN_epsize(struct vsfdwc2_device_t *otgd, uint8_t idx, uint16_t epsize)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	if (idx == 0)
	{
		switch (epsize)
		{
		case 64:
			otgd->in_ep_regs[0].diepctl = otgd->in_ep_regs[0].diepctl & ~USB_OTG_DIEPCTL_MPSIZ;
			break;
		case 32:
			otgd->in_ep_regs[0].diepctl = (otgd->in_ep_regs[0].diepctl & ~USB_OTG_DIEPCTL_MPSIZ) | 1;
			break;
		case 16:
			otgd->in_ep_regs[0].diepctl = (otgd->in_ep_regs[0].diepctl & ~USB_OTG_DIEPCTL_MPSIZ) | 2;
			break;
		case 8:
			otgd->in_ep_regs[0].diepctl = (otgd->in_ep_regs[0].diepctl & ~USB_OTG_DIEPCTL_MPSIZ) | 3;
			break;
		}
	}
	else
	{
		otgd->in_ep_regs[idx].diepctl = epsize | ((uint32_t)idx << 22);
	}
	otgd->dev_global_regs->daintmsk |= 0x1ul << idx;
	
	return VSFERR_NONE;
}

uint16_t vsfdwc2_usbd_ep_get_IN_epsize(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx == 0)
	{
		switch (otgd->in_ep_regs[0].diepctl & USB_OTG_DIEPCTL_MPSIZ)
		{
		case 0:
			return 64;
		case 1:
			return 32;
		case 2:
			return 16;
		case 3:
			return 8;
		}
	}
	else if (idx < otgd->hc_param->hc_ep_amount)
		return otgd->in_ep_regs[idx].diepctl & USB_OTG_DIEPCTL_MPSIZ;

	return 0;
}

vsf_err_t vsfdwc2_usbd_ep_set_IN_stall(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	otgd->in_ep_regs[idx].diepctl |= USB_OTG_DIEPCTL_STALL;	
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_clear_IN_stall(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	otgd->in_ep_regs[idx].diepctl &= ~USB_OTG_DIEPCTL_STALL;	
	return VSFERR_NONE;
}

bool vsfdwc2_usbd_ep_is_IN_stall(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return false;

	return otgd->in_ep_regs[idx].diepctl & USB_OTG_DIEPCTL_STALL ? true : false;
}

vsf_err_t vsfdwc2_usbd_ep_reset_IN_toggle(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	otgd->in_ep_regs[idx].diepctl &= ~USB_OTG_DIEPCTL_EONUM_DPID;
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_toggle_IN_toggle(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;
	
	if (otgd->in_ep_regs[idx].diepctl & USB_OTG_DIEPCTL_EONUM_DPID)
		otgd->in_ep_regs[idx].diepctl &= ~USB_OTG_DIEPCTL_EONUM_DPID;
	else
		otgd->in_ep_regs[idx].diepctl |= USB_OTG_DIEPCTL_EONUM_DPID;
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_set_IN_count(struct vsfdwc2_device_t *otgd, uint8_t idx, uint16_t size)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	if (!size)
	{
		otgd->in_ep_regs[idx].dieptsiz = 0x1ul << 19;
		otgd->in_ep_regs[idx].diepctl |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK;
	}
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_write_IN_buffer(struct vsfdwc2_device_t *otgd, uint8_t idx, uint8_t *buffer, uint16_t size)
{
	uint16_t i;

	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	otgd->in_ep_regs[idx].dieptsiz = (0x1ul << 19) | size;
	otgd->in_ep_regs[idx].diepctl |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK;

	for (i = 0; i < size; i += 4)
	{
		*otgd->dfifo[idx] = *(uint32_t *)buffer;
		buffer += 4;
	}
	otgd->dev_global_regs->dtknqr4_fifoemptymsk |= 0x1ul << idx;

	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_set_OUT_epsize(struct vsfdwc2_device_t *otgd, uint8_t idx, uint16_t epsize)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	if (idx == 0)
	{
		otgd->rx_ep_pkt[0].buf = otgd->device_rx_buffer;
		otgd->rx_ep_pkt[0].packet_size = 0;
		otgd->rx_ep_pkt[0].max_packet_size = epsize;
		switch (epsize)
		{
		case 64:
			otgd->out_ep_regs[0].doepctl = otgd->out_ep_regs[0].doepctl & ~USB_OTG_DOEPCTL_MPSIZ;
			break;
		case 32:
			otgd->out_ep_regs[0].doepctl = (otgd->out_ep_regs[0].doepctl & ~USB_OTG_DOEPCTL_MPSIZ) | 1;
			break;
		case 16:
			otgd->out_ep_regs[0].doepctl = (otgd->out_ep_regs[0].doepctl & ~USB_OTG_DOEPCTL_MPSIZ) | 2;
			break;
		case 8:
			otgd->out_ep_regs[0].doepctl = (otgd->out_ep_regs[0].doepctl & ~USB_OTG_DOEPCTL_MPSIZ) | 3;
			break;
		}
	}
	else
	{
		uint16_t rx_buff_free_size = otgd->device_rx_buffer_size - otgd->device_rx_buffer_pos;
		if (rx_buff_free_size < epsize)
			return VSFERR_NOT_ENOUGH_RESOURCES;
		
		otgd->rx_ep_pkt[idx].buf = otgd->device_rx_buffer + otgd->device_rx_buffer_pos / 4;
		otgd->device_rx_buffer_pos += epsize;
		otgd->rx_ep_pkt[idx].packet_size = 0;
		otgd->rx_ep_pkt[idx].max_packet_size = epsize;
		
		otgd->out_ep_regs[idx].doepctl = (otgd->out_ep_regs[idx].doepctl & ~USB_OTG_DOEPCTL_MPSIZ) | epsize;
	}
	otgd->dev_global_regs->daintmsk |= 0x10000ul << idx;

	return VSFERR_NONE;
}

uint16_t vsfdwc2_usbd_ep_get_OUT_epsize(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx < otgd->hc_param->hc_ep_amount)
		return otgd->rx_ep_pkt[idx].max_packet_size;
	return 0;
}

vsf_err_t vsfdwc2_usbd_ep_set_OUT_stall(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	otgd->out_ep_regs[idx].doepctl |= USB_OTG_DOEPCTL_STALL;	
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_clear_OUT_stall(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	otgd->out_ep_regs[idx].doepctl &= ~USB_OTG_DOEPCTL_STALL;	
	return VSFERR_NONE;
}

bool vsfdwc2_usbd_ep_is_OUT_stall(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return false;

	return otgd->out_ep_regs[idx].doepctl & USB_OTG_DOEPCTL_STALL ? true : false;
}

vsf_err_t vsfdwc2_usbd_ep_reset_OUT_toggle(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	otgd->out_ep_regs[idx].doepctl |= USB_OTG_DOEPCTL_SD0PID_SEVNFRM;
	return VSFERR_NONE;
}

vsf_err_t vsfdwc2_usbd_ep_toggle_OUT_toggle(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	if (otgd->out_ep_regs[idx].doepctl & 0x00010000)
		otgd->out_ep_regs[idx].doepctl |= USB_OTG_DOEPCTL_SODDFRM;
	else
		otgd->out_ep_regs[idx].doepctl |= USB_OTG_DOEPCTL_SD0PID_SEVNFRM;
	return VSFERR_NONE;
}

uint16_t vsfdwc2_usbd_ep_get_OUT_count(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx < otgd->hc_param->hc_ep_amount)
		return otgd->rx_ep_pkt[idx].packet_size;
	return 0;
}

vsf_err_t vsfdwc2_usbd_ep_read_OUT_buffer(struct vsfdwc2_device_t *otgd, uint8_t idx, uint8_t *buffer, uint16_t size)
{
	if (idx < otgd->hc_param->hc_ep_amount)
	{
		size = min(otgd->rx_ep_pkt[idx].packet_size, size);
		if (size && buffer)
		{
			memcpy(buffer, otgd->rx_ep_pkt[idx].buf, size);
			otgd->rx_ep_pkt[idx].packet_size -= size;
			return VSFERR_NONE;
		}
	}
	return VSFERR_FAIL;
}

vsf_err_t vsfdwc2_usbd_ep_enable_OUT(struct vsfdwc2_device_t *otgd, uint8_t idx)
{
	if (idx >= otgd->hc_param->hc_ep_amount)
		return VSFERR_FAIL;

	otgd->out_ep_regs[idx].doeptsiz = (otgd->out_ep_regs[idx].doeptsiz & ~(USB_OTG_DOEPTSIZ_XFRSIZ | USB_OTG_DOEPTSIZ_PKTCNT)) |
			(0x1ul << 19) | otgd->rx_ep_pkt[idx].max_packet_size;
	otgd->out_ep_regs[idx].doepctl |= USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK;	
	return VSFERR_NONE;
}

static void ep_read(struct vsfdwc2_device_t *otgd, uint8_t ep_num, uint16_t size)
{
	size = min(otgd->rx_ep_pkt[ep_num].max_packet_size, size);

	if (size)
	{
		uint16_t i;
		uint32_t *dest = otgd->rx_ep_pkt[ep_num].buf;
		otgd->rx_ep_pkt[ep_num].packet_size = size;
		size = (size + 3) / 4;
		for (i = 0; i < size; i++)
			dest[i] = *otgd->dfifo[0];
	}
}

static void vsfdwc2_usbd_interrupt(void *p)
{
	struct vsfdwc2_device_t *otgd = p;
	
	uint32_t intsts = otgd->global_reg->gintmsk | USB_OTG_GINTSTS_CMOD;
	intsts &= otgd->global_reg->gintsts;

	if (!(intsts & USB_OTG_GINTSTS_CMOD)) // device mode
	{
		if (intsts & USB_OTG_GINTSTS_MMIS)
			otgd->global_reg->gintsts = USB_OTG_GINTSTS_MMIS;

		if (intsts & USB_OTG_GINTSTS_USBRST)
		{
			otgd->cb(VSFHAL_USBD_ON_RESET, 0);
			otgd->global_reg->gintsts = USB_OTG_GINTSTS_USBRST;
		}

		if (intsts & USB_OTG_GINTSTS_ENUMDNE)
		{
			uint8_t speed = (otgd->dev_global_regs->dsts & USB_OTG_DSTS_ENUMSPD) >> 1;
			otgd->global_reg->gusbcfg &= ~USB_OTG_GUSBCFG_TRDT;

			switch (speed)
			{
			case 0: //DSTAT_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ:
				otgd->out_ep_regs[0].doepctl = otgd->out_ep_regs[0].doepctl & ~USB_OTG_DOEPCTL_MPSIZ;
				otgd->global_reg->gusbcfg |= 0x09U << 10;
				break;
			case 1: //DSTAT_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ:
				otgd->out_ep_regs[0].doepctl = otgd->out_ep_regs[0].doepctl & ~USB_OTG_DOEPCTL_MPSIZ;
				otgd->global_reg->gusbcfg |= 0x05U << 10;
				break;
			case 3: //DSTAT_ENUMSPD_FS_PHY_48MHZ:
				otgd->out_ep_regs[0].doepctl = otgd->out_ep_regs[0].doepctl & ~USB_OTG_DOEPCTL_MPSIZ;
				otgd->global_reg->gusbcfg |= 0x05U << 10;
				break;
			case 2: //DSTAT_ENUMSPD_LS_PHY_6MHZ:
				otgd->out_ep_regs[0].doepctl = (otgd->out_ep_regs[0].doepctl & ~USB_OTG_DOEPCTL_MPSIZ) | 3;
				otgd->global_reg->gusbcfg |= 0x05U << 10;
				break;
			}
			otgd->dev_global_regs->dctl |= USB_OTG_DCTL_CGINAK;
			otgd->global_reg->gintsts = USB_OTG_GINTSTS_ENUMDNE;
		}

		if (intsts & USB_OTG_GINTSTS_ESUSP)
		{
			otgd->global_reg->gintmsk &= ~USB_OTG_GINTMSK_ESUSPM;
			otgd->global_reg->gintsts = USB_OTG_GINTSTS_ESUSP;
		}

		if (intsts & USB_OTG_GINTSTS_USBSUSP)
		{
			otgd->cb(VSFHAL_USBD_ON_SUSPEND, 0);
			otgd->global_reg->gintsts = USB_OTG_GINTSTS_USBSUSP;
		}

		if (intsts & USB_OTG_GINTSTS_WKUINT)
		{
			otgd->cb(VSFHAL_USBD_ON_RESUME, 0);
			otgd->global_reg->gintsts = USB_OTG_GINTSTS_WKUINT;
		}

		if (intsts & USB_OTG_GINTSTS_SOF)
		{
			otgd->cb(VSFHAL_USBD_ON_SOF, 0);
			otgd->global_reg->gintsts = USB_OTG_GINTSTS_SOF;
		}

		if (intsts & USB_OTG_GINTSTS_OEPINT)
		{
			uint8_t ep_num = 0;
			uint32_t ep_int = otgd->dev_global_regs->daint;
			ep_int = (ep_int & otgd->dev_global_regs->daintmsk) >> 16;

			while (ep_int)
			{
				if (ep_int & 0x1)
				{
					uint32_t int_status = otgd->out_ep_regs[ep_num].doepint;
					int_status &= otgd->dev_global_regs->doepmsk;
					
					// transfer complete interrupt
					if (int_status & USB_OTG_DOEPINT_XFRC)
					{
						otgd->cb(VSFHAL_USBD_ON_OUT, ep_num);
						otgd->out_ep_regs[ep_num].doepint = USB_OTG_DOEPINT_XFRC;
					}

					// endpoint disable interrupt
					if (int_status & USB_OTG_DOEPINT_EPDISD)
						otgd->out_ep_regs[ep_num].doepint = USB_OTG_DOEPINT_EPDISD;

					// setup phase finished interrupt (just for control endpoints)
					if (int_status & USB_OTG_DOEPINT_STUP)
					{
						// need update address immediately
						if (((otgd->rx_ep_pkt[0].buf[0] & 0xff00ffff) == 0x00000500) &&
								(otgd->rx_ep_pkt[0].buf[1] == 0x0))
							vsfdwc2_usbd_set_address(otgd, otgd->rx_ep_pkt[0].buf[0] >> 16);
						
						otgd->cb(VSFHAL_USBD_ON_SETUP, 0);
						otgd->out_ep_regs[ep_num].doeptsiz |= 0x3ul <<  29;
						otgd->out_ep_regs[ep_num].doepint = USB_OTG_DOEPINT_STUP;
					}

					// back to back setup packets received
					if (int_status & USB_OTG_DOEPINT_B2BSTUP)
						otgd->out_ep_regs[ep_num].doepint = USB_OTG_DOEPINT_B2BSTUP;
				}
				ep_int >>= 1;
				ep_num++;
			}
		}

		if (intsts & USB_OTG_GINTSTS_IEPINT)
		{
			uint8_t ep_num = 0;
			uint32_t ep_int = otgd->dev_global_regs->daint;
			ep_int = (ep_int & otgd->dev_global_regs->daintmsk) & 0xffff;
			
			while (ep_int)
			{
				if (ep_int & 0x1)
				{
					uint32_t int_status = otgd->in_ep_regs[ep_num].diepint;
					uint32_t int_msak = otgd->dev_global_regs->diepmsk;
					int_msak |= ((otgd->dev_global_regs->dtknqr4_fifoemptymsk >> ep_num) & 0x1) << 7;
					int_status &= int_msak;
					
					if (int_status & USB_OTG_DIEPINT_XFRC)
					{
						otgd->cb(VSFHAL_USBD_ON_IN, ep_num);
						otgd->in_ep_regs[ep_num].diepint = USB_OTG_DIEPINT_XFRC;
					}
					
					if (int_status & USB_OTG_DIEPINT_EPDISD)
						otgd->in_ep_regs[ep_num].diepint = USB_OTG_DIEPINT_EPDISD;
					
					if (int_status & USB_OTG_DIEPINT_TOC)
						otgd->in_ep_regs[ep_num].diepint = USB_OTG_DIEPINT_TOC;
					
					if (int_status & USB_OTG_DIEPINT_INEPNE)
						otgd->in_ep_regs[ep_num].diepint = USB_OTG_DIEPINT_INEPNE;
					
					if (int_status & USB_OTG_DIEPINT_TXFE)
					{
						otgd->dev_global_regs->dtknqr4_fifoemptymsk &= ~(0x1ul << ep_num);
						otgd->in_ep_regs[ep_num].diepint = USB_OTG_DIEPINT_TXFE;
					}
				}
				ep_int >>= 1;
				ep_num++;
			}
		}

		if (intsts & USB_OTG_GINTSTS_RXFLVL)
		{
			uint8_t ep_num, pid;
			uint16_t size;
			uint32_t rx_status;
			
			otgd->global_reg->gintmsk &= ~USB_OTG_GINTMSK_RXFLVLM;
			
			rx_status = otgd->global_reg->grxstsp;
			ep_num = rx_status & USB_OTG_GRXSTSP_EPNUM;
			size = (rx_status & USB_OTG_GRXSTSP_BCNT) >> 4;
			pid = (rx_status & USB_OTG_GRXSTSP_DPID) >> 15;

			switch ((rx_status & USB_OTG_GRXSTSP_PKTSTS) >> 17)
			{
			case 6: //RXSTAT_SETUP_UPDT:
				if (ep_num || (size != 8) || (pid != 0/*DPID_DATA0*/))
					break;
			case 2: //RXSTAT_DATA_UPDT:
				ep_read(otgd, ep_num, size);
				break;
			//case RXSTAT_GOUT_NAK:
			//case RXSTAT_SETUP_COMP:
			default:
				break;
			}

			otgd->global_reg->gintmsk |= USB_OTG_GINTMSK_RXFLVLM;
		}
	}
}
#endif

#ifdef VSFDWC2_HOST_ENABLE
static struct hc_t *alloc_hc(struct vsfdwcotg_host_t *otgh)
{
	uint8_t i;
	for (i = 0; i < otgh->hc_ep_amount; i++)
	{
		if (otgh->hc_pool[i].alloced == 0)
		{
			memset(&otgh->hc_pool[i], 0, sizeof(struct hc_t));
			otgh->hc_pool[i].hc_num = i;
			otgh->hc_pool[i].alloced = 1;
			return &otgh->hc_pool[i];
		}
	}
	return NULL;
}

static void free_hc(struct hc_t *hc)
{
	// TODO reset hc reg

	hc->alloced = 0;
}

static struct hc_t *hc_init(struct vsfdwcotg_host_t *otgh,
		struct dwcotg_device_t *dev_priv)
{
	struct hc_t *hc;

	if (dev_priv->hc_num < MAX_HC_NUM_EACH_DEVICE)
	{
		hc = alloc_hc(otgh);
		if (hc == NULL)
			return NULL;
		else
		{
			uint8_t i;
			for (i = 0; i < MAX_HC_NUM_EACH_DEVICE; i++)
			{
				if (dev_priv->hc[i] == NULL)
				{
					dev_priv->hc[i] = hc;
					dev_priv->hc_num++;
					hc->owner_dev = dev_priv;
					return hc;
				}
			}
		}
		free_hc(hc);
	}
	return NULL;
}

static void hc_fini(struct hc_t **hcp)
{
	uint8_t i;
	struct hc_t *hc;
	struct dwcotg_device_t *dev_priv;

	if (*hcp == NULL)
		return;

	hc = *hcp;
	hcp = NULL;
	dev_priv = hc->owner_dev;

	for (i = 0; i < MAX_HC_NUM_EACH_DEVICE; i++)
	{
		if (dev_priv->hc[i] == hc)
		{
			dev_priv->hc[i] = NULL;
			dev_priv->hc_num--;
			free_hc(hc);
			break;
		}
	}
}

static void hc_halt(struct vsfdwcotg_host_t *otgh, uint8_t hc_num)
{
	uint32_t count = 0;
	struct dwcotg_hc_regs_t *hc_reg = &otgh->hc_regs[hc_num];
	uint8_t type = (hc_reg->hcchar >> 18) & 0x3;

	hc_reg->hcchar |= USB_OTG_HCCHAR_CHDIS;

	if ((type == 0) || (type == 2))
	{
		if (otgh->global_reg->gnptxsts & 0xffff)
		{
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_CHENA;
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_EPDIR;
			do
			{
				if (count++ > 1000)
					break;
			}
			while ((hc_reg->hcchar & USB_OTG_HCCHAR_CHENA) == USB_OTG_HCCHAR_CHENA);
		}
		else
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
	}
	else
	{
		if (otgh->host_global_regs->hptxsts & 0xffff)
		{
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_CHENA;
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_EPDIR;
			do
			{
				if (count++ > 1000)
					break;
			}
			while ((hc_reg->hcchar & USB_OTG_HCCHAR_CHENA) == USB_OTG_HCCHAR_CHENA);
		}
		else
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
	}
}

const uint8_t pipetype_to_dwctype[4] = {1, 3, 0, 2};
static vsf_err_t submit_hc(struct vsfdwcotg_host_t *otgh, struct hc_t *hc,
		uint32_t *buf, uint16_t size)
{
	uint32_t pkt_num, tmp;
	struct urb_priv_t *urb_priv = hc->owner_priv;
	struct dwcotg_hc_regs_t *reg = &otgh->hc_regs[hc->hc_num];

	switch (urb_priv->type)
	{
	case URB_PRIV_TYPE_ISO:
		reg->hcintmsk = USB_OTG_HCINTMSK_XFRCM | USB_OTG_HCINTMSK_ACKM |
				USB_OTG_HCINTMSK_AHBERR | USB_OTG_HCINTMSK_FRMORM ;
		if (hc->dir_o0_i1)
			reg->hcintmsk |= USB_OTG_HCINTMSK_TXERRM;
		break;
	case URB_PRIV_TYPE_INT:
		reg->hcintmsk = USB_OTG_HCINTMSK_XFRCM | USB_OTG_HCINTMSK_STALLM |
				USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_DTERRM|
				USB_OTG_HCINTMSK_NAKM | USB_OTG_HCINTMSK_AHBERR |
				USB_OTG_HCINTMSK_FRMORM;
		break;
	case URB_PRIV_TYPE_CTRL:
	case URB_PRIV_TYPE_BULK:
		reg->hcintmsk = USB_OTG_HCINTMSK_XFRCM  | USB_OTG_HCINTMSK_STALLM |
				USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_DTERRM |
				USB_OTG_HCINTMSK_AHBERR | USB_OTG_HCINTMSK_NAKM;
		if (!hc->dir_o0_i1)
			reg->hcintmsk |= USB_OTG_HCINTMSK_NYET;
		break;
	}
	if (hc->dir_o0_i1)
		reg->hcintmsk |= USB_OTG_HCINTMSK_BBERRM;

	otgh->host_global_regs->haintmsk |= 0x1ul << hc->hc_num;

	reg->hcchar = (hc->dev_addr << 22) | (hc->ep_num << 11) |
			(hc->dir_o0_i1 ? USB_OTG_HCCHAR_EPDIR : 0) |
			((hc->speed == USB_SPEED_LOW) << 17) |
			((uint32_t)pipetype_to_dwctype[urb_priv->type] << 18) |
			(urb_priv->packet_size & USB_OTG_HCCHAR_MPSIZ);

	if (urb_priv->type == URB_PRIV_TYPE_INT)
	{
		reg->hcchar |= USB_OTG_HCCHAR_ODDFRM;
	}

	// transfer size
	if (size > 0)
	{
		pkt_num = (size + urb_priv->packet_size - 1) / urb_priv->packet_size;
		if (pkt_num > 256)
		{
			pkt_num = 256;
			size = urb_priv->packet_size * 256;
		}
	}
	else
		pkt_num = 1;
	urb_priv->toggle_next = urb_priv->toggle_start ^ (pkt_num & 0x1);

	if (hc->dir_o0_i1)
		size = urb_priv->packet_size * pkt_num;
	hc->transfer_size = size;

	reg->hctsiz = ((pkt_num << 19) & USB_OTG_HCTSIZ_PKTCNT) |
			((uint32_t)hc->dpid << 29) | size;

	if (otgh->dma_en)
		reg->hcdma = (uint32_t)buf;

	tmp = otgh->host_global_regs->hfnum & 0x1 ? 0x0ul : 0x1ul;
	reg->hcchar |= tmp << 29;

	// enable hc, TODO merge up code
	tmp = (reg->hcchar & (~USB_OTG_HCCHAR_CHDIS)) | USB_OTG_HCCHAR_CHENA;
	reg->hcchar = tmp;

	if ((otgh->dma_en == 0) && (hc->dir_o0_i1 == 0) && (hc->transfer_size))
	{
		// TODO: check if there is enough space in FIFO space
		switch (urb_priv->type)
		{
		case URB_PRIV_TYPE_CTRL:
		case URB_PRIV_TYPE_BULK:
			break;
		case URB_PRIV_TYPE_ISO:
		case URB_PRIV_TYPE_INT:
			break;
		}
		//size = (size + 3) >> 2;
		tmp = 0;
		while (tmp < size)
		{
			otgh->dfifo[hc->hc_num] = buf[tmp >> 2];
			tmp += 4;
		}
	}

	return VSFERR_NONE;
}

static vsf_err_t submit_priv_urb(struct vsfdwcotg_host_t *otgh,
		struct urb_priv_t *urb_priv)
{
	uint32_t interval;
	struct vsfusbh_urb_t *vsfurb = container_of(urb_priv, struct vsfusbh_urb_t,
			urb_priv);
	struct hc_t *hc = urb_priv->hc;

	switch (urb_priv->phase)
	{
	case URB_PRIV_PHASE_PERIOD_WAIT:
		if (otgh->speed == USB_SPEED_HIGH)
			interval = vsfurb->interval * 8;
		else
			interval = vsfurb->interval;
		if (otgh->softick % interval == 0)
		{
			hc->hc_state = HC_START;
			urb_priv->phase = URB_PRIV_PHASE_DATA_WAIT;
			return submit_priv_urb(otgh, urb_priv);
		}
		else
		{
			hc->hc_state = HC_WAIT;
		}
		return VSFERR_NONE;
	case URB_PRIV_PHASE_SETUP_WAIT:
		hc->dir_o0_i1 = 0;
		hc->dpid = HC_DPID_SETUP;

		return submit_hc(otgh, hc, (uint32_t *)(&vsfurb->setup_packet),
				sizeof(struct usb_ctrlrequest_t));
	case URB_PRIV_PHASE_DATA_WAIT:
		hc->dir_o0_i1 = urb_priv->dir_o0_i1;
		switch (urb_priv->type)
		{
		case URB_PRIV_TYPE_ISO:
			hc->dpid = HC_DPID_DATA0;
			break;
		case URB_PRIV_TYPE_INT:
		case URB_PRIV_TYPE_BULK:
			hc->dpid = urb_priv->toggle_start ? HC_DPID_DATA1 : HC_DPID_DATA0;
			break;
		case URB_PRIV_TYPE_CTRL:
			hc->dpid = HC_DPID_DATA1;
			break;
		}
		return submit_hc(otgh, hc, (uint32_t *)urb_priv->transfer_buffer,
				urb_priv->transfer_length);
	case URB_PRIV_PHASE_STATE_WAIT:
		hc->dir_o0_i1 = !urb_priv->dir_o0_i1;
		hc->dpid = HC_DPID_DATA1;

		return submit_hc(otgh, hc, NULL, 0);
	default :
		return VSFERR_FAIL;
	}
}

static void free_priv_urb(struct urb_priv_t *urb_priv)
{
	struct vsfusbh_urb_t *vsfurb = container_of(urb_priv, struct vsfusbh_urb_t,
			urb_priv);

	hc_fini(&urb_priv->hc);
	if (vsfurb->transfer_buffer != NULL)
		vsf_bufmgr_free(vsfurb->transfer_buffer);
	vsf_bufmgr_free(vsfurb);
}

static void vsfdwcotg_hc_done_handler(struct vsfdwcotg_host_t *otgh,
		struct urb_priv_t *urb_priv, struct hc_t *hc)
{
	struct vsfusbh_urb_t *vsfurb = container_of(urb_priv, struct vsfusbh_urb_t,
			urb_priv);
	struct dwcotg_device_t *dev_priv = hc->owner_dev;
	struct vsfusbh_device_t *vsfdev = dev_priv->vsfdev;

	if ((vsfdev == NULL) || (urb_priv->discarded))
	{
		free_priv_urb(urb_priv);
		if (vsfdev == NULL)
		{
			if (dev_priv->hc_num == 0)
				vsf_bufmgr_free(dev_priv);
		}
		return;
	}

	switch (urb_priv->state)
	{
	case URB_PRIV_STATE_NAK:
		hc->hc_state = HC_WAIT;
		urb_priv->phase = URB_PRIV_PHASE_PERIOD_WAIT;
		submit_priv_urb(otgh, urb_priv);
		break;
	case URB_PRIV_STATE_NOTREADY:
		submit_priv_urb(otgh, urb_priv);
		break;
	case URB_PRIV_STATE_DONE:
		switch (urb_priv->type)
		{
		case URB_PRIV_TYPE_INT:
			urb_priv->toggle_start = urb_priv->toggle_next;
			if (urb_priv->toggle_next)
			{
				vsfdev->toggle[hc->dir_o0_i1] |= 0x1ul << hc->ep_num;
			}
			else
			{
				vsfdev->toggle[hc->dir_o0_i1] &= ~(0x1ul << hc->ep_num);
			}
		case URB_PRIV_TYPE_ISO:
			vsfurb->actual_length = urb_priv->actual_length;
			vsfurb->status = URB_OK;
			vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
			break;
		case URB_PRIV_TYPE_BULK:
			if (urb_priv->toggle_next)
			{
				vsfdev->toggle[hc->dir_o0_i1] |= 0x1ul << hc->ep_num;
			}
			else
			{
				vsfdev->toggle[hc->dir_o0_i1] &= ~(0x1ul << hc->ep_num);
			}
			vsfurb->actual_length = urb_priv->actual_length;
			hc_fini(&urb_priv->hc);
			vsfurb->status = URB_OK;
			vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
			break;
		case URB_PRIV_TYPE_CTRL:
			if ((urb_priv->phase == URB_PRIV_PHASE_SETUP_WAIT) &&
					(urb_priv->transfer_length == 0))
			{
				urb_priv->phase = URB_PRIV_PHASE_STATE_WAIT;
			}
			else if (urb_priv->phase == URB_PRIV_PHASE_DATA_WAIT)
			{
				vsfurb->actual_length = urb_priv->actual_length;
				urb_priv->phase++;
			}
			else
				urb_priv->phase++;
			if (urb_priv->phase == URB_PRIV_PHASE_DONE)
			{
				hc_fini(&urb_priv->hc);
				vsfurb->status = URB_OK;
				vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
			}
			else
			{
				submit_priv_urb(otgh, urb_priv);
			}
			break;
		}
		break;
	default:
		hc_fini(&urb_priv->hc);
		vsfurb->status = VSFERR_FAIL;
		vsfsm_post_evt_pending(vsfurb->sm, VSFSM_EVT_URB_COMPLETE);
		break;
	}
}

static void vsfdwcotg_hc_in_handler(struct vsfdwcotg_host_t *otgh, uint8_t hc_num)
{
	struct dwcotg_hc_regs_t *hc_reg = &otgh->hc_regs[hc_num];
	struct hc_t *hc = &otgh->hc_pool[hc_num];
	struct urb_priv_t *urb_priv = hc->owner_priv;
	uint32_t intsts = hc_reg->hcintmsk;
	intsts &= hc_reg->hcint;

	if (intsts & USB_OTG_HCINT_AHBERR)
	{
		hc_reg->hcint = USB_OTG_HCINT_AHBERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
	}
	else if (intsts & USB_OTG_HCINT_STALL)
	{
		hc->hc_state = HC_STALL;
		hc_reg->hcint = USB_OTG_HCINT_NAK | USB_OTG_HCINT_STALL;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_DTERR)
	{
		hc->hc_state = HC_DATATGLERR;
		hc_reg->hcint = USB_OTG_HCINT_NAK | USB_OTG_HCINT_DTERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_FRMOR)
	{
		hc_reg->hcint = USB_OTG_HCINT_FRMOR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_TXERR)
	{
		hc_reg->hcint = USB_OTG_HCINT_TXERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc->hc_state = HC_XACTERR;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_ACK)
	{
		hc_reg->hcint = USB_OTG_HCINT_ACK;
	}

	if (intsts & USB_OTG_HCINT_XFRC)
	{
		if (otgh->dma_en)
		{
			urb_priv->actual_length = hc->transfer_size -
					(hc_reg->hctsiz & USB_OTG_HCTSIZ_XFRSIZ);
		}

		hc->hc_state = HC_XFRC;
		hc_reg->hcint = USB_OTG_HCINT_XFRC;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;

		if ((urb_priv->type == URB_PRIV_TYPE_CTRL) ||
			(urb_priv->type == URB_PRIV_TYPE_BULK))
		{
			hc_halt(otgh, hc_num);
			hc_reg->hcint = USB_OTG_HCINT_NAK;
		}
		else if (urb_priv->type == URB_PRIV_TYPE_INT)
		{
			hc_reg->hcchar |= USB_OTG_HCCHAR_ODDFRM;
			urb_priv->state = URB_PRIV_STATE_DONE;
		}
	}
	else if (intsts & USB_OTG_HCINT_NAK)
	{
		hc->hc_state = HC_NAK;
		hc_reg->hcint = USB_OTG_HCINT_NAK;

		if (urb_priv->type == URB_PRIV_TYPE_INT)
		{
			hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
			hc_halt(otgh, hc_num);
		}
		else if ((urb_priv->type == URB_PRIV_TYPE_CTRL) ||
				(urb_priv->type == URB_PRIV_TYPE_BULK))
		{
			// re-activate the channel
			hc_reg->hcchar &= ~USB_OTG_HCCHAR_CHDIS;
			hc_reg->hcchar |= USB_OTG_HCCHAR_CHENA;
		}
	}

	intsts = hc_reg->hcintmsk;
	intsts &= hc_reg->hcint;

	if (intsts & USB_OTG_HCINT_CHH)
	{
		hc_reg->hcintmsk &= ~USB_OTG_HCINTMSK_CHHM;
		hc_reg->hcint = USB_OTG_HCINT_CHH;

		if (hc->hc_state == HC_XFRC)
			urb_priv->state = URB_PRIV_STATE_DONE;
		else if (hc->hc_state == HC_STALL)
			urb_priv->state = URB_PRIV_STATE_STALL;
		else if ((hc->hc_state == HC_XACTERR) ||
				(hc->hc_state == HC_DATATGLERR))
		{
			if (++hc->err_cnt > 8)
			{
				hc->err_cnt = 0;
				urb_priv->state = URB_PRIV_STATE_ERROR;
			}
			else
			{
				urb_priv->state = URB_PRIV_STATE_NOTREADY;
			}
		}
		else if (hc->hc_state == HC_NAK)
		{
			urb_priv->state = URB_PRIV_STATE_NAK;
		}
		else
		{
			urb_priv->state = URB_PRIV_STATE_ERROR;
		}

		vsfdwcotg_hc_done_handler(otgh, urb_priv, hc);
	}
}
static void vsfdwcotg_hc_out_handler(struct vsfdwcotg_host_t *otgh, uint8_t hc_num)
{
	struct dwcotg_hc_regs_t *hc_reg = &otgh->hc_regs[hc_num];
	struct hc_t *hc = &otgh->hc_pool[hc_num];
	struct urb_priv_t *urb_priv = hc->owner_priv;
	uint32_t intsts = hc_reg->hcintmsk;
	intsts &= hc_reg->hcint;

	if (intsts & USB_OTG_HCINT_AHBERR)
	{
		hc_reg->hcint = USB_OTG_HCINT_AHBERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
	}
	else if (intsts & USB_OTG_HCINT_NYET)
	{
		hc->hc_state = HC_NYET;
		hc->err_cnt = 0;
		hc_reg->hcint = USB_OTG_HCINT_NYET;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_FRMOR)
	{
		hc_reg->hcint = USB_OTG_HCINT_FRMOR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_STALL)
	{
		hc->hc_state = HC_STALL;
		hc_reg->hcint = USB_OTG_HCINT_STALL;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_TXERR)
	{
		hc->hc_state = HC_XACTERR;
		hc_reg->hcint = USB_OTG_HCINT_TXERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_DTERR)
	{
		hc->hc_state = HC_DATATGLERR;
		hc_reg->hcint = USB_OTG_HCINT_NAK | USB_OTG_HCINT_DTERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_ACK)
	{
		hc_reg->hcint = USB_OTG_HCINT_ACK;

		if (urb_priv->do_ping)
		{
			hc->hc_state = HC_NYET;
			hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
			hc_halt(otgh, hc_num);
			urb_priv->state = URB_PRIV_STATE_NOTREADY;
		}
	}

	if (intsts & USB_OTG_HCINT_XFRC)
	{
		if (otgh->dma_en)
		{
			urb_priv->actual_length = hc->transfer_size;
		}

		hc->hc_state = HC_XFRC;
		hc_reg->hcint = USB_OTG_HCINT_XFRC;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}
	else if (intsts & USB_OTG_HCINT_NAK)
	{
		hc->hc_state = HC_NAK;
		hc_reg->hcint = USB_OTG_HCINT_NAK;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(otgh, hc_num);
	}

	intsts = hc_reg->hcintmsk;
	intsts &= hc_reg->hcint;

	if (intsts & USB_OTG_HCINT_CHH)
	{
		hc_reg->hcintmsk &= ~USB_OTG_HCINTMSK_CHHM;
		hc_reg->hcint = USB_OTG_HCINT_CHH;

		if (hc->hc_state == HC_XFRC)
		{
			urb_priv->state = URB_PRIV_STATE_DONE;
			if (urb_priv->type == URB_PRIV_TYPE_BULK)
			{
				// TODO
				//hc->toggle ^= 1;
			}

		}
		else if (hc->hc_state == HC_NAK)
			urb_priv->state = URB_PRIV_STATE_NOTREADY;
		else if (hc->hc_state == HC_NYET)
		{
			urb_priv->state = URB_PRIV_STATE_NOTREADY;
			urb_priv->do_ping = 0;
		}
		else if (hc->hc_state == HC_STALL)
			urb_priv->state = URB_PRIV_STATE_STALL;
		else if ((hc->hc_state == HC_XACTERR) ||
				(hc->hc_state == HC_DATATGLERR))
		{
			if (++hc->err_cnt > 8)
			{
				hc->err_cnt = 0;
				urb_priv->state = URB_PRIV_STATE_ERROR;
			}
			else
			{
				urb_priv->state = URB_PRIV_STATE_NOTREADY;
			}
		}
		else if (hc->hc_state == HC_NAK)
		{
			urb_priv->state = URB_PRIV_STATE_NAK;
		}
		else
		{
			urb_priv->state = URB_PRIV_STATE_ERROR;
		}

		vsfdwcotg_hc_done_handler(otgh, urb_priv, hc);
	}
}
static vsf_err_t vsfdwcotg_interrupt(void *param)
{
	struct vsfdwcotg_host_t *otgh = (struct vsfdwcotg_host_t *)param;
	uint32_t intsts = otgh->global_reg->gintmsk;
	intsts &= otgh->global_reg->gintsts;

	if (!intsts)
		return VSFERR_NONE;

	if (otgh->global_reg->gintsts & USB_OTG_GINTSTS_CMOD) // host mode
	{
		if (intsts & USB_OTG_GINTSTS_SOF)
		{
			uint8_t i;
			struct hc_t *hc = otgh->hc_pool;

			otgh->softick++;
			otgh->global_reg->gintsts = USB_OTG_GINTSTS_SOF;

			for (i = 0; i < otgh->hc_ep_amount; i++)
			{
				if ((hc[i].alloced) && (hc[i].hc_state == HC_WAIT))
				{
					submit_priv_urb(otgh, hc[i].owner_priv);
				}
			}
		}
		if (intsts & USB_OTG_GINTSTS_RXFLVL)
		{
			// TODO
		}
		if (intsts & USB_OTG_GINTSTS_NPTXFE)
		{
			// TODO
		}
		if (intsts & USB_OTG_GINTSTS_HPRTINT)
		{
			// TODO
		}
		if (intsts & USB_OTG_GINTSTS_HCINT)
		{
			uint8_t i;
			uint32_t hc_intsts = otgh->host_global_regs->haint;

			for (i = 0; i < otgh->hc_ep_amount; i++)
			{
				if (hc_intsts & (0x1ul << i))
				{
					if (otgh->hc_regs[i].hcchar & USB_OTG_HCCHAR_EPDIR)
						vsfdwcotg_hc_in_handler(otgh, i);
					else
						vsfdwcotg_hc_out_handler(otgh, i);
				}
			}

			otgh->global_reg->gintsts = USB_OTG_GINTSTS_HCINT;
		}
		if (intsts & USB_OTG_GINTSTS_PTXFE)
		{
			// TODO
		}
	}
	else // device mode
	{
		// TODO
	}

	return VSFERR_NONE;
}

static vsf_err_t dwcotg_init_get_resource(struct vsfusbh_t *usbh,
		uint32_t reg_base)
{
	struct vsfdwcotg_host_t *otgh;
	struct vsfdwc2_hcd_param_t *hcd_param = usbh->hcd_param;

	otgh = vsf_bufmgr_malloc(sizeof(struct vsfdwcotg_host_t));
	if (otgh == NULL)
		return VSFERR_NOT_ENOUGH_RESOURCES;
	memset(otgh, 0, sizeof(struct vsfdwcotg_host_t));
	usbh->hcd_data = otgh;

	// config init
	otgh->speed = hcd_param->speed;
	otgh->dma_en = hcd_param->dma_en;
	otgh->ulpi_en = hcd_param->ulpi_en;
	otgh->external_vbus_en = hcd_param->vbus_en;
	otgh->hc_ep_amount = hcd_param->hc_ep_amount;

	//otgh->ep_in_amount = VSFUSBD_CFG_MAX_IN_EP;
	//otgh->ep_out_amount = VSFUSBD_CFG_MAX_OUT_EP;
	otgh->ep_in_amount = 0;
	otgh->ep_out_amount = 0;

	// reg addr init
	otgh->global_reg =
			(struct dwcotg_core_global_regs_t *)(reg_base + 0);
	otgh->host_global_regs =
			(struct dwcotg_host_global_regs_t *)(reg_base + 0x400);
	otgh->hprt0 =
			(volatile uint32_t *)(reg_base + 0x440);
	otgh->hc_regs =
			(struct dwcotg_hc_regs_t *)(reg_base + 0x500);
	otgh->dev_global_regs =
			(struct dwcotg_dev_global_regs_t *)(reg_base + 0x800);
	otgh->in_ep_regs =
			(struct dwcotg_dev_in_ep_regs_t *)(reg_base + 0x900);
	otgh->out_ep_regs =
			(struct dwcotg_dev_out_ep_regs_t *)(reg_base + 0xb00);
	otgh->dfifo = (uint32_t *)(reg_base + 0x1000);

	otgh->hc_pool = vsf_bufmgr_malloc(sizeof(struct hc_t) *
			otgh->hc_ep_amount);
	if (otgh->hc_pool == NULL)
	{
		vsf_bufmgr_free(otgh);
		usbh->hcd_data = NULL;
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}
	memset(otgh->hc_pool, 0, sizeof(struct hc_t) * otgh->hc_ep_amount);

	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_init_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	uint8_t i;
	vsf_err_t err;
	struct vsfusbh_t *usbh = (struct vsfusbh_t *)pt->user_data;
	struct vsfdwc2_hcd_param_t *hcd_param = usbh->hcd_param;
	struct vsfdwcotg_host_t *otgh = (struct vsfdwcotg_host_t *)usbh->hcd_data;

	vsfsm_pt_begin(pt);

	usbh->hcd_rh_speed = hcd_param->speed;
	err = dwcotg_init_get_resource(usbh,
			(uint32_t)vsfhal_hcd_regbase(hcd_param->index));
	if (err)
		return err;
	otgh = (struct vsfdwcotg_host_t *)usbh->hcd_data;

	vsfhal_hcd_init(hcd_param->index, vsfdwcotg_interrupt, usbh->hcd_data);

	// disable global int
	otgh->global_reg->gahbcfg &= ~USB_OTG_GAHBCFG_GINT;

	if (otgh->ulpi_en)
	{
		// GCCFG &= ~(USB_OTG_GCCFG_PWRDWN)
		otgh->global_reg->gccfg &= ~USB_OTG_GCCFG_PWRDWN;

		// Init The ULPI Interface
		otgh->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_TSDPS |
				USB_OTG_GUSBCFG_ULPIFSLS | USB_OTG_GUSBCFG_PHYSEL);
		// Select vbus source
		otgh->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_ULPIEVBUSD |
				USB_OTG_GUSBCFG_ULPIEVBUSI);
		if (otgh->external_vbus_en)
		{
			otgh->global_reg->gusbcfg |= USB_OTG_GUSBCFG_ULPIEVBUSD;
		}

	}
	else
	{
		otgh->global_reg->gusbcfg |= USB_OTG_GUSBCFG_PHYSEL;
	}

	// Core Reset
	otgh->retry = 0;
	while ((otgh->global_reg->grstctl & USB_OTG_GRSTCTL_AHBIDL) == 0)
	{
		if (otgh->retry > 10)
			return VSFERR_FAIL;

		vsfsm_pt_delay(pt, 2);
		otgh->retry++;
	}
	otgh->global_reg->grstctl |= USB_OTG_GRSTCTL_CSRST;
	otgh->retry = 0;
	while ((otgh->global_reg->grstctl & USB_OTG_GRSTCTL_CSRST) ==
			USB_OTG_GRSTCTL_CSRST)
	{
		if (otgh->retry > 10)
			return VSFERR_FAIL;

		vsfsm_pt_delay(pt, 2);
		otgh->retry++;
	}

	if (otgh->ulpi_en == 0)
	{
		otgh->global_reg->gccfg = USB_OTG_GCCFG_PWRDWN;
	}

	if (otgh->dma_en)
	{
		otgh->global_reg->gahbcfg |= USB_OTG_GAHBCFG_DMAEN |
				USB_OTG_GAHBCFG_HBSTLEN_1 | USB_OTG_GAHBCFG_HBSTLEN_2;
	}

	// Force Host Mode
	otgh->global_reg->gusbcfg &= ~USB_OTG_GUSBCFG_FDMOD;
	otgh->global_reg->gusbcfg |= USB_OTG_GUSBCFG_FHMOD;

	vsfsm_pt_delay(pt, 50);

	// Enable Core
	// USBx->GCCFG |= USB_OTG_GCCFG_VBDEN;
	otgh->global_reg->gccfg |= USB_OTG_GCCFG_VBDEN;

	if (otgh->speed == USB_SPEED_HIGH)
	{
		otgh->host_global_regs->hcfg &= ~USB_OTG_HCFG_FSLSS;
	}
	else
	{
		otgh->host_global_regs->hcfg |= USB_OTG_HCFG_FSLSS;
	}

	// Flush FIFO
	otgh->global_reg->grstctl = USB_OTG_GRSTCTL_TXFFLSH |
			USB_OTG_GRSTCTL_TXFNUM_4;
	otgh->retry = 0;
	while ((otgh->global_reg->grstctl & USB_OTG_GRSTCTL_TXFFLSH) ==
			USB_OTG_GRSTCTL_TXFFLSH)
	{
		if (otgh->retry > 10)
			return VSFERR_FAIL;
		vsfsm_pt_delay(pt, 2);
		otgh->retry++;
	}
	otgh->global_reg->grstctl = USB_OTG_GRSTCTL_RXFFLSH;
	otgh->retry = 0;
	while ((otgh->global_reg->grstctl & USB_OTG_GRSTCTL_RXFFLSH) ==
			USB_OTG_GRSTCTL_RXFFLSH)
	{
		if (otgh->retry > 10)
			return VSFERR_FAIL;
		vsfsm_pt_delay(pt, 2);
		otgh->retry++;
	}

	// Clear all pending HC Interrupts
	for (i = 0; i < otgh->hc_ep_amount; i++)
	{
		otgh->hc_regs[i].hcint = 0xffffffff;
		otgh->hc_regs[i].hcintmsk = 0;
	}

	vsfsm_pt_delay(pt, 10);

	// Disable all interrupts
	otgh->global_reg->gintmsk = 0;

	// Clear any pending interrupts
	otgh->global_reg->gintsts = 0xffffffff;

	// fifo size
	otgh->global_reg->grxfsiz = (((uint32_t)hcd_param->host_rx_fifo_depth + 1) * 2);
	otgh->global_reg->gnptxfsiz = ((uint32_t)hcd_param->host_non_periodic_tx_fifo_depth << 16) |
			(otgh->global_reg->grxfsiz & 0xffff);
	otgh->global_reg->hptxfsiz = ((uint32_t)hcd_param->host_periodic_tx_fifo_depth << 16) |
			(otgh->global_reg->gnptxfsiz & 0xffff);

	if (otgh->dma_en == 0)
		otgh->global_reg->gintmsk |= USB_OTG_GINTMSK_RXFLVLM;

	// Enable interrupts matching to the Host mode ONLY
	otgh->global_reg->gintmsk |= USB_OTG_GINTMSK_HCIM | USB_OTG_GINTMSK_SOFM |
			USB_OTG_GINTMSK_PXFRM_IISOOXFRM;

	vsfsm_pt_delay(pt, 10);

	// enable global int
	otgh->global_reg->gahbcfg |= USB_OTG_GAHBCFG_GINT;

	otgh->dwcotg_state = DWCOTG_WORKING;

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_fini(void *param)
{
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_suspend(void *param)
{
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_resume(void *param)
{
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_alloc_device(void *param, struct vsfusbh_device_t *dev)
{
	struct dwcotg_device_t *dev_priv = vsf_bufmgr_malloc(sizeof(struct dwcotg_device_t));
	if (dev_priv == NULL)
		return VSFERR_FAIL;
	memset(dev_priv, 0, sizeof(struct dwcotg_device_t));
	dev->priv = dev_priv;
	dev_priv->vsfdev = dev;
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_free_device(void *param, struct vsfusbh_device_t *dev)
{
	struct dwcotg_device_t *dev_priv = dev->priv;
	if (dev_priv->hc_num == 0)
		vsf_bufmgr_free(dev_priv);
	dev_priv->vsfdev = NULL;
	dev->priv = NULL;
	return VSFERR_NONE;
}

static struct vsfusbh_urb_t *dwcotgh_alloc_urb(void)
{
	uint32_t size;
	struct vsfusbh_urb_t *vsfurb;
	size = sizeof(struct vsfusbh_urb_t) - 4 + sizeof(struct urb_priv_t);
	vsfurb = vsf_bufmgr_malloc_aligned(size, 16);
	if (vsfurb == NULL)
		return NULL;
	memset(vsfurb, 0, size);
	return vsfurb;
}

static vsf_err_t dwcotgh_free_urb(void *param, struct vsfusbh_urb_t **vsfurbp)
{
	struct vsfusbh_urb_t *vsfurb = *vsfurbp;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)vsfurb->urb_priv;

	if (vsfurb == NULL)
		return VSFERR_FAIL;
	*vsfurbp = NULL;

	if (urb_priv->hc)
	{
		urb_priv->discarded = 1;
	}
	else
	{
		if (vsfurb->transfer_buffer != NULL)
			vsf_bufmgr_free(vsfurb->transfer_buffer);
		vsf_bufmgr_free(vsfurb);
	}
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_submit_urb(void *param, struct vsfusbh_urb_t *vsfurb)
{
	vsf_err_t err;
	struct hc_t *hc;
	struct vsfdwcotg_host_t *otgh = (struct vsfdwcotg_host_t *)param;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)vsfurb->urb_priv;
	uint32_t pipe = vsfurb->pipe;

	// rh address check
	if (usb_pipedevice(pipe) == 1)
		return VSFERR_FAIL;

	if (otgh->dwcotg_state == DWCOTG_DISABLE)
		return VSFERR_FAIL;

	memset(urb_priv, 0, sizeof(struct urb_priv_t));

	urb_priv->type = usb_pipetype(pipe);
	if (urb_priv->type == URB_PRIV_TYPE_CTRL)
		urb_priv->phase = URB_PRIV_PHASE_SETUP_WAIT;
	else if (urb_priv->type == URB_PRIV_TYPE_BULK)
		urb_priv->phase = URB_PRIV_PHASE_DATA_WAIT;
	else
		urb_priv->phase = URB_PRIV_PHASE_PERIOD_WAIT;

	if ((urb_priv->type == URB_PRIV_TYPE_INT) ||
		(urb_priv->type == URB_PRIV_TYPE_BULK))
	{
		urb_priv->toggle_start = vsfurb->vsfdev->toggle[usb_pipein(pipe)] &
				(0x1ul << usb_pipeendpoint(pipe));

		urb_priv->do_ping = otgh->dma_en ? 0 :
				(usb_pipespeed(pipe) == USB_SPEED_HIGH);
	}

	urb_priv->transfer_buffer = vsfurb->transfer_buffer;
	urb_priv->transfer_length = vsfurb->transfer_length;
	urb_priv->packet_size = vsfurb->packet_size;
	urb_priv->dir_o0_i1 = usb_pipein(pipe);

	hc = hc_init(otgh, (struct dwcotg_device_t *)vsfurb->vsfdev->priv);
	if (hc)
	{
		hc->dev_addr = usb_pipedevice(pipe);
		hc->ep_num = usb_pipeendpoint(pipe);
		hc->speed = usb_pipespeed(pipe);
		hc->owner_priv = urb_priv;
		urb_priv->hc = hc;

		err = submit_priv_urb(otgh, urb_priv);
		if (err == VSFERR_NONE)
			return VSFERR_NONE;
	}
	else
	{
		err = VSFERR_NOT_ENOUGH_RESOURCES;
	}

	vsfurb->status = URB_FAIL;
	return err;
}

// use for int/iso urb
static vsf_err_t dwcotgh_relink_urb(void *param, struct vsfusbh_urb_t *vsfurb)
{
	return submit_priv_urb((struct vsfdwcotg_host_t *)param,
			(struct urb_priv_t *)vsfurb->urb_priv);
}



const static uint8_t root_hub_str_index0[] =
{
	0x04,				/* u8  bLength; */
	0x03,				/* u8  bDescriptorType; String-descriptor */
	0x09,				/* u8  lang ID */
	0x04,				/* u8  lang ID */
};
const static uint8_t root_hub_str_index1[] =
{
	30,					/* u8  bLength; */
	0x03,				/* u8  bDescriptorType; String-descriptor */
	'D',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'W',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'C',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'O',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'T',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'G',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'-',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'R',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'o',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'o',				/* u8  Unicode */
	0,					/* u8  Unicode */
	't',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'H',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'u',				/* u8  Unicode */
	0,					/* u8  Unicode */
	'b',				/* u8  Unicode */
	0,					/* u8  Unicode */
};

static uint32_t rd_rh_portstat(volatile uint32_t *port)
{
	uint32_t dwcotg_port = *port;
	uint32_t value = 0;

	if ((dwcotg_port & USB_OTG_HPRT_PSPD) == 0)
		value |= USB_PORT_STAT_HIGH_SPEED;
	else if ((dwcotg_port & USB_OTG_HPRT_PSPD) == USB_OTG_HPRT_PSPD_1)
		value |= USB_PORT_STAT_LOW_SPEED;

	if (dwcotg_port & USB_OTG_HPRT_PCDET)
		value |= (USB_PORT_STAT_C_CONNECTION << 16);

	if (dwcotg_port & USB_OTG_HPRT_PENA)
		value |= USB_PORT_STAT_ENABLE;

	if (dwcotg_port & USB_OTG_HPRT_PCSTS)
		value |= USB_PORT_STAT_CONNECTION;

	return value;
}

static vsf_err_t dwcotgh_rh_control(void *param,
		struct vsfusbh_urb_t *vsfurb)
{
	//uint16_t typeReq, wValue, wIndex, wLength;
	uint16_t typeReq, wValue, wLength;
	struct vsfdwcotg_host_t *otgh = (struct vsfdwcotg_host_t *)param;
	struct usb_ctrlrequest_t *cmd = &vsfurb->setup_packet;
	uint32_t datadw[4];
	uint8_t *data = (uint8_t*)datadw;
	uint8_t len = 0;


	typeReq = (cmd->bRequestType << 8) | cmd->bRequest;
	wValue = cmd->wValue;
	//wIndex = cmd->wIndex;
	wLength = cmd->wLength;

	switch (typeReq)
	{
	case GetHubStatus:
		datadw[0] = 0;
		len = 4;
		break;
	case GetPortStatus:
		datadw[0] = rd_rh_portstat(otgh->hprt0);
		len = 4;
		break;
	case SetPortFeature:
		switch (wValue)
		{
		case(USB_PORT_FEAT_ENABLE):
			*otgh->hprt0 |= USB_OTG_HPRT_PENA;
			len = 0;
			break;
		case(USB_PORT_FEAT_RESET):
			if (*otgh->hprt0 & USB_OTG_HPRT_PCSTS)
				*otgh->hprt0 |= USB_OTG_HPRT_PRST;
			len = 0;
			break;
		case(USB_PORT_FEAT_POWER):
			*otgh->hprt0 |= USB_OTG_HPRT_PPWR;
			len = 0;
			break;
		default:
			goto error;
		}
		break;
	case ClearPortFeature:
		switch (wValue)
		{
		case(USB_PORT_FEAT_ENABLE):
			*otgh->hprt0 &= ~USB_OTG_HPRT_PENA;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_RESET):
			*otgh->hprt0 &= ~USB_OTG_HPRT_PRST;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_CONNECTION):
			*otgh->hprt0 |= USB_OTG_HPRT_PCDET;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_ENABLE):
			*otgh->hprt0 &= ~USB_OTG_HPRT_PENCHNG;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_SUSPEND):
			*otgh->hprt0 &= ~USB_OTG_HPRT_PSUSP;
			len = 0;
			break;
		case(USB_PORT_FEAT_C_OVER_CURRENT):
			*otgh->hprt0 &= ~USB_OTG_HPRT_POCA;
			len = 0;
			break;
		default:
			goto error;
		}
		break;
	case GetHubDescriptor:
		data[0] = 9;			// min length;
		data[1] = 0x29;
		data[2] = 1;
		data[3] = 0x8;
		datadw[1] = 0;
		data[5] = 0;
		data[8] = 0xff;
		len = min(data[0], wLength);
		break;
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (wValue & 0xff00)
		{
		case USB_DT_STRING << 8:
			if (wValue == 0x0300)
			{
				data = (uint8_t *)root_hub_str_index0;
				len = min(sizeof(root_hub_str_index0), wLength);
			}
			if (wValue == 0x0301)
			{
				data = (uint8_t *)root_hub_str_index1;
				len = min(sizeof(root_hub_str_index0), wLength);
			}
			break;
		default:
			goto error;
		}
		break;
	default:
		goto error;
	}

	if (len)
	{
		if (vsfurb->transfer_length < len)
			len = vsfurb->transfer_length;
		vsfurb->actual_length = len;

		memcpy (vsfurb->transfer_buffer, data, len);
	}

	vsfusbh_rh_urb_complete(vsfurb, URB_OK);
	return VSFERR_NONE;

error:
	vsfurb->status = URB_FAIL;
	return VSFERR_FAIL;
}

const struct vsfusbh_hcddrv_t vsfdwcotgh_drv =
{
	.init_thread = dwcotgh_init_thread,
	.fini = dwcotgh_fini,
	.suspend = dwcotgh_suspend,
	.resume = dwcotgh_resume,
	.alloc_device = dwcotgh_alloc_device,
	.free_device = dwcotgh_free_device,
	.alloc_urb = dwcotgh_alloc_urb,
	.free_urb = dwcotgh_free_urb,
	.submit_urb = dwcotgh_submit_urb,
	.relink_urb = dwcotgh_relink_urb,
	.rh_control = dwcotgh_rh_control,
};
#endif


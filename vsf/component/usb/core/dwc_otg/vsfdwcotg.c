#include "vsf.h"
#include "vsfdwcotg_priv.h"

static struct hc_t *hc_alloc(struct dwcotg_t *dwcotg)
{
	uint8_t i;
	for (i = 0; i < dwcotg->hc_amount; i++)
	{
		if (dwcotg->hc_pool[i].alloced == 0)
		{
			memset(&dwcotg->hc_pool[i], 0, sizeof(struct hc_t));
			dwcotg->hc_pool[i].hc_num = i;
			dwcotg->hc_pool[i].alloced = 1;
			return &dwcotg->hc_pool[i];
		}
	}
	return NULL;
}

static void hc_free(struct hc_t **hcp)
{
	struct hc_t *hc = *hcp;
	
	if (hc)
	{
		*hcp = NULL;
		hc->alloced = 0;
	}
}

static void hc_halt(struct dwcotg_t *dwcotg, uint8_t hc_num)
{
	uint32_t count = 0;
	struct dwcotg_hc_regs_t *hc_reg = &dwcotg->hc_regs[hc_num];
	uint8_t type = (hc_reg->hcchar >> 18) & 0x3;

	hc_reg->hcchar |= USB_OTG_HCCHAR_CHDIS;
	if ((type == 0) || (type == 2)) // ctrl or bulk
	{
		if (!(dwcotg->global_reg->gnptxsts & 0xffff))
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
		if (!(dwcotg->host_global_regs->hptxsts & 0xffff))
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

static void hc_manual_write(uint32_t *dfifo, uint8_t hc_num, uint32_t *buf, uint16_t size)
{
	uint16_t i;
	dfifo = dfifo + 0x1000 * hc_num;
	for (i = 0; i < size; i += 4)
		*dfifo = buf[i >> 2];
}

static void hc_manual_read(uint32_t *dfifo, uint8_t hc_num, uint32_t *buf, uint16_t size)
{
	uint16_t i;
	for (i = 0; i < size; i += 4)
		buf[i >> 2] = *dfifo;
}

const uint8_t pipetype_to_dwctype[4] = {1, 3, 0, 2};
static vsf_err_t hc_submit(struct dwcotg_t *dwcotg, struct hc_t *hc,
		uint32_t *buf, uint16_t size)
{
	uint32_t pkt_num, tmp;
	struct urb_priv_t *urb_priv = hc->owner_priv;
	struct dwcotg_hc_regs_t *reg = &dwcotg->hc_regs[hc->hc_num];

	reg->hcint = 0xffffffff;
	
	switch (urb_priv->type)
	{
	case URB_PRIV_TYPE_CTRL:
	case URB_PRIV_TYPE_BULK:
		reg->hcintmsk = USB_OTG_HCINTMSK_XFRCM  | USB_OTG_HCINTMSK_STALLM |
				USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_DTERRM |
				USB_OTG_HCINTMSK_AHBERR | USB_OTG_HCINTMSK_NAKM;
#if 0
		if (!hc->dir_o0_i1 && (hc->speed == USB_SPEED_HIGH))
			reg->hcintmsk |= USB_OTG_HCINTMSK_NYET | USB_OTG_HCINTMSK_ACKM;
#endif
		break;
	case URB_PRIV_TYPE_INT:
		reg->hcintmsk = USB_OTG_HCINTMSK_XFRCM | USB_OTG_HCINTMSK_STALLM |
				USB_OTG_HCINTMSK_TXERRM | USB_OTG_HCINTMSK_DTERRM|
				USB_OTG_HCINTMSK_NAKM | USB_OTG_HCINTMSK_AHBERR |
				USB_OTG_HCINTMSK_FRMORM;
		break;
	case URB_PRIV_TYPE_ISO:
		reg->hcintmsk = USB_OTG_HCINTMSK_XFRCM | USB_OTG_HCINTMSK_ACKM |
				USB_OTG_HCINTMSK_AHBERR | USB_OTG_HCINTMSK_FRMORM ;
		if (hc->dir_o0_i1)
			reg->hcintmsk |= USB_OTG_HCINTMSK_TXERRM;
		break;
	}
	if (hc->dir_o0_i1)
		reg->hcintmsk |= USB_OTG_HCINTMSK_BBERRM;

	dwcotg->host_global_regs->haintmsk |= 0x1ul << hc->hc_num;

	reg->hcchar = ((hc->dev_addr << 22) & USB_OTG_HCCHAR_DAD) |
			((hc->ep_num << 11) & USB_OTG_HCCHAR_EPNUM) |
			(hc->dir_o0_i1 ? USB_OTG_HCCHAR_EPDIR : 0) |
			((urb_priv->speed == USB_SPEED_LOW) ? USB_OTG_HCCHAR_LSDEV : 0) |
			(((uint32_t)pipetype_to_dwctype[urb_priv->type] << 18) & USB_OTG_HCCHAR_EPTYP) |
			(urb_priv->packet_size & USB_OTG_HCCHAR_MPSIZ);
	if (urb_priv->type == URB_PRIV_TYPE_INT)
		reg->hcchar |= USB_OTG_HCCHAR_ODDFRM;

#if 0
	if (hc->speed == USB_SPEED_HIGH)
	{
		if (!dwcotg->dma_en && hc->do_ping)
		{
			// TODO
			// USB_DoPing
		}
		else if (dwcotg->dma_en)
		{
			reg->hcintmsk &= ~(USB_OTG_HCINTMSK_NYET | USB_OTG_HCINTMSK_ACKM);
			hc->do_ping = 0;
		}			
	}
#endif

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
	if (hc->dir_o0_i1)
		size = urb_priv->packet_size * pkt_num;
	hc->transfer_size = size;

	reg->hctsiz = ((pkt_num << 19) & USB_OTG_HCTSIZ_PKTCNT) |
			(((uint32_t)hc->dpid << 29) & USB_OTG_HCTSIZ_DPID) |
			(size & USB_OTG_HCTSIZ_XFRSIZ);

	if (dwcotg->dma_en)
		reg->hcdma = (uint32_t)buf;

	tmp = (dwcotg->host_global_regs->hfnum & 0x1) ? 0x0 : USB_OTG_HCCHAR_ODDFRM;
	reg->hcchar &= ~USB_OTG_HCCHAR_ODDFRM;
	reg->hcchar |= tmp;

	// Set host channel enable
	tmp = (reg->hcchar & ~USB_OTG_HCCHAR_CHDIS) | USB_OTG_HCCHAR_CHENA;
	reg->hcchar = tmp;

	if (!dwcotg->dma_en && (hc->dir_o0_i1 == 0) && (hc->transfer_size))
	{
		tmp = (hc->transfer_size + 3) / 4;
		switch (urb_priv->type)
		{
		case URB_PRIV_TYPE_CTRL:
		case URB_PRIV_TYPE_BULK:
			if (tmp > (dwcotg->global_reg->gnptxsts & 0xffff))
			{
				tmp = dwcotg->global_reg->gnptxsts & 0xfffc;
				dwcotg->global_reg->gintmsk |= USB_OTG_GINTMSK_PTXFEM;
			}
			break;
		case URB_PRIV_TYPE_ISO:
		case URB_PRIV_TYPE_INT:
			if (tmp > (dwcotg->host_global_regs->hptxsts & 0xffff))
			{
				tmp = dwcotg->global_reg->gnptxsts & 0xfffc;
				dwcotg->global_reg->gintmsk |= USB_OTG_GINTMSK_PTXFEM;
			}
			break;
		}		
		hc_manual_write(dwcotg->dfifo, hc->hc_num, buf, tmp);
	}

	return VSFERR_NONE;
}

static vsf_err_t submit_priv_urb(struct dwcotg_t *dwcotg,
		struct urb_priv_t *urb_priv)
{
	uint32_t interval;
	struct vsfhcd_urb_t *urb = container_of(urb_priv, struct vsfhcd_urb_t, priv);
	struct hc_t *hc = urb_priv->hc;

	switch (urb_priv->phase)
	{
	case URB_PRIV_PHASE_PERIOD_WAIT:
		if (dwcotg->speed == USB_SPEED_HIGH)
			interval = urb->interval * 8;
		else
			interval = urb->interval;
		if (!interval || (dwcotg->softick % interval == 0))
		{
			hc->hc_state = HC_START;
			urb_priv->phase = URB_PRIV_PHASE_DATA_WAIT;
			return submit_priv_urb(dwcotg, urb_priv);
		}
		else
			hc->hc_state = HC_WAIT;
		return VSFERR_NONE;
	case URB_PRIV_PHASE_SETUP_WAIT:
		hc->dir_o0_i1 = 0;
		hc->dpid = HC_DPID_SETUP;
		return hc_submit(dwcotg, hc, (uint32_t *)(&urb->setup_packet), sizeof(struct usb_ctrlrequest_t));
	case URB_PRIV_PHASE_DATA_WAIT:
		hc->dir_o0_i1 = urb_priv->dir_o0_i1;
		switch (urb_priv->type)
		{
		case URB_PRIV_TYPE_ISO:
			hc->dpid = HC_DPID_DATA0;
			break;
		// TODO
		case URB_PRIV_TYPE_CTRL:
			hc->dpid = HC_DPID_DATA1;
			break;
		case URB_PRIV_TYPE_INT:
		case URB_PRIV_TYPE_BULK:
			hc->dpid = urb_priv->toggle ? HC_DPID_DATA1 : HC_DPID_DATA0;
			break;
		}
		return hc_submit(dwcotg, hc, urb_priv->transfer_buffer, urb_priv->transfer_length);
	case URB_PRIV_PHASE_STATE_WAIT:
		hc->dir_o0_i1 = !urb_priv->dir_o0_i1;
		hc->dpid = HC_DPID_DATA1;
		return hc_submit(dwcotg, hc, NULL, 0);
	default:
		return VSFERR_FAIL;
	}
}

static void free_priv_urb(struct urb_priv_t *urb_priv)
{
	struct vsfhcd_urb_t *urb = container_of(urb_priv, struct vsfhcd_urb_t, priv);

	hc_free(&urb_priv->hc);
	if (urb->transfer_buffer != NULL)
		vsf_bufmgr_free(urb->transfer_buffer);
	vsf_bufmgr_free(urb);
}

static void vsfdwcotg_hc_done_handler(struct dwcotg_t *dwcotg,
		struct urb_priv_t *urb_priv, struct hc_t *hc)
{
	struct vsfhcd_urb_t *urb = container_of(urb_priv, struct vsfhcd_urb_t, priv);
	struct vsfhcd_device_t *hcddev = urb->hcddev;

	if (urb_priv->discarded)
	{
		free_priv_urb(urb_priv);
		return;
	}

	switch (urb_priv->state)
	{
	case URB_PRIV_STATE_NAK:
		hc->hc_state = HC_WAIT;
		urb_priv->phase = URB_PRIV_PHASE_PERIOD_WAIT;
		submit_priv_urb(dwcotg, urb_priv);
		break;
	case URB_PRIV_STATE_NOTREADY:
		submit_priv_urb(dwcotg, urb_priv);
		break;
	case URB_PRIV_STATE_DONE:
		switch (urb_priv->type)
		{
		case URB_PRIV_TYPE_INT:
			if (urb_priv->toggle)
				hcddev->toggle[hc->dir_o0_i1] |= 0x1ul << hc->ep_num;
			else
				hcddev->toggle[hc->dir_o0_i1] &= ~(0x1ul << hc->ep_num);
		case URB_PRIV_TYPE_ISO:
			urb->actual_length = urb_priv->actual_length;
			urb->status = URB_OK;
			vsfsm_post_evt_pending(urb->notifier_sm, VSFSM_EVT_URB_COMPLETE);
			break;
		case URB_PRIV_TYPE_BULK:
			if (urb_priv->toggle)
				hcddev->toggle[hc->dir_o0_i1] |= 0x1ul << hc->ep_num;
			else
				hcddev->toggle[hc->dir_o0_i1] &= ~(0x1ul << hc->ep_num);
			urb->actual_length = urb_priv->actual_length;
			hc_free(&urb_priv->hc);
			urb->status = URB_OK;
			vsfsm_post_evt_pending(urb->notifier_sm, VSFSM_EVT_URB_COMPLETE);
			break;
		case URB_PRIV_TYPE_CTRL:
			if ((urb_priv->phase == URB_PRIV_PHASE_SETUP_WAIT) &&
					(urb_priv->transfer_length == 0))
			{
				urb_priv->phase = URB_PRIV_PHASE_STATE_WAIT;
			}
			else if (urb_priv->phase == URB_PRIV_PHASE_DATA_WAIT)
			{
				urb->actual_length = urb_priv->actual_length;
				urb_priv->phase++;
			}
			else
				urb_priv->phase++;
			if (urb_priv->phase == URB_PRIV_PHASE_DONE)
			{
				hc_free(&urb_priv->hc);
				urb->status = URB_OK;
				vsfsm_post_evt_pending(urb->notifier_sm, VSFSM_EVT_URB_COMPLETE);
			}
			else
			{
				submit_priv_urb(dwcotg, urb_priv);
			}
			break;
		}
		break;
	default:
		hc_free(&urb_priv->hc);
		urb->status = VSFERR_FAIL;
		vsfsm_post_evt_pending(urb->notifier_sm, VSFSM_EVT_URB_COMPLETE);
		break;
	}
}

static void vsfdwcotg_hc_in_handler(struct dwcotg_t *dwcotg, uint8_t hc_num)
{
	struct dwcotg_hc_regs_t *hc_reg = &dwcotg->hc_regs[hc_num];
	struct hc_t *hc = &dwcotg->hc_pool[hc_num];
	struct urb_priv_t *urb_priv = hc->owner_priv;

	if (hc_reg->hcint & USB_OTG_HCINT_AHBERR)
	{
		hc_reg->hcint = USB_OTG_HCINT_AHBERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_ACK)
	{
		hc_reg->hcint = USB_OTG_HCINT_ACK;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_STALL)
	{
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc->hc_state = HC_STALL;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_NAK | USB_OTG_HCINT_STALL;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_DTERR)
	{
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc->hc_state = HC_DATATGLERR;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_NAK | USB_OTG_HCINT_DTERR;
	}	
	else if (hc_reg->hcint & USB_OTG_HCINT_FRMOR)
	{
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_FRMOR;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_TXERR)
	{
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		if (hc->err_cnt < 3)
			hc->err_cnt++;
		hc->hc_state = HC_XACTERR;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_TXERR;
	}
	
	if (hc_reg->hcint & USB_OTG_HCINT_XFRC)
	{
		if (dwcotg->dma_en)
			urb_priv->actual_length = hc->transfer_size -
					(hc_reg->hctsiz & USB_OTG_HCTSIZ_XFRSIZ);

		hc->hc_state = HC_XFRC;
		hc->err_cnt = 0;
		urb_priv->toggle ^= 1;
		hc_reg->hcint = USB_OTG_HCINT_XFRC;

		if ((urb_priv->type == URB_PRIV_TYPE_CTRL) || (urb_priv->type == URB_PRIV_TYPE_BULK))
		{
			hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
			hc_halt(dwcotg, hc_num);
			hc_reg->hcint = USB_OTG_HCINT_NAK;
		}
		else if (urb_priv->type == URB_PRIV_TYPE_INT)
		{
			hc_reg->hcchar |= USB_OTG_HCCHAR_ODDFRM;
			urb_priv->state = URB_PRIV_STATE_DONE;
			//vsfdwcotg_hc_done_handler(dwcotg, urb_priv, hc);
		}
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_NAK)
	{
		if (urb_priv->type == URB_PRIV_TYPE_INT)
		{
			hc->err_cnt = 0;
			hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
			hc_halt(dwcotg, hc_num);
		}
		else if ((urb_priv->type == URB_PRIV_TYPE_CTRL) || (urb_priv->type == URB_PRIV_TYPE_BULK))
		{
			hc->err_cnt = 0;
			if (!dwcotg->dma_en)
			{
				hc->hc_state = HC_NAK;
				hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
				hc_halt(dwcotg, hc_num);
			}
		}
		hc_reg->hcint = USB_OTG_HCINT_NAK;
	}

	if (hc_reg->hcint & USB_OTG_HCINT_CHH)
	{
		hc_reg->hcintmsk &= ~USB_OTG_HCINTMSK_CHHM;
		
		if (hc->hc_state == HC_XFRC)
			urb_priv->state = URB_PRIV_STATE_DONE;
		else if (hc->hc_state == HC_STALL)
			urb_priv->state = URB_PRIV_STATE_STALL;
		else if ((hc->hc_state == HC_XACTERR) || (hc->hc_state == HC_DATATGLERR))
		{
			if (hc->err_cnt == 3)
			{
				hc->err_cnt = 0;
				urb_priv->state = URB_PRIV_STATE_ERROR;
			}
			else
			{
				hc->err_cnt++;
				urb_priv->state = URB_PRIV_STATE_NOTREADY;
			}
			// Maybe TODO : re-activate the channel
		}
		else if (hc->hc_state == HC_NAK)
		{
			urb_priv->state = URB_PRIV_STATE_NAK;
			// Maybe TODO : re-activate the channel
		}
		else
			urb_priv->state = URB_PRIV_STATE_ERROR;

		hc_reg->hcint = USB_OTG_HCINT_CHH;
		vsfdwcotg_hc_done_handler(dwcotg, urb_priv, hc);
	}
}

static void vsfdwcotg_hc_out_handler(struct dwcotg_t *dwcotg, uint8_t hc_num)
{
	struct dwcotg_hc_regs_t *hc_reg = &dwcotg->hc_regs[hc_num];
	struct hc_t *hc = &dwcotg->hc_pool[hc_num];
	struct urb_priv_t *urb_priv = hc->owner_priv;

	if (hc_reg->hcint & USB_OTG_HCINT_AHBERR)
	{
		hc_reg->hcint = USB_OTG_HCINT_AHBERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_ACK)
	{
		hc_reg->hcint = USB_OTG_HCINT_ACK;
		if (hc->do_ping)
		{
			hc->do_ping = 0;
			urb_priv->state = URB_PRIV_STATE_NOTREADY;
			hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
			hc_halt(dwcotg, hc_num);
		}
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_NYET)
	{
		hc->hc_state = HC_NYET;
		hc->do_ping = 1;
		hc->err_cnt = 0;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_NYET;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_FRMOR)
	{
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_FRMOR;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_STALL)
	{
		hc->hc_state = HC_STALL;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_STALL;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_TXERR)
	{
		hc->hc_state = HC_XACTERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_TXERR;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_DTERR)
	{
		hc->hc_state = HC_DATATGLERR;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_NAK | USB_OTG_HCINT_DTERR;
	}

	if (hc_reg->hcint & USB_OTG_HCINT_XFRC)
	{
		if (dwcotg->dma_en)
			urb_priv->actual_length = hc->transfer_size;
		hc->err_cnt = 0;
		hc->hc_state = HC_XFRC;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_XFRC;
	}
	else if (hc_reg->hcint & USB_OTG_HCINT_NAK)
	{
		hc->err_cnt = 0;
		hc->hc_state = HC_NAK;
		if (!hc->do_ping  && (urb_priv->speed == USB_SPEED_HIGH))
			hc->do_ping = 1;
		hc_reg->hcintmsk |= USB_OTG_HCINTMSK_CHHM;
		hc_halt(dwcotg, hc_num);
		hc_reg->hcint = USB_OTG_HCINT_NAK;
	}
	
	if (hc_reg->hcint & USB_OTG_HCINT_CHH)
	{
		hc_reg->hcintmsk &= ~USB_OTG_HCINTMSK_CHHM;

		if (hc->hc_state == HC_XFRC)
		{
			urb_priv->state = URB_PRIV_STATE_DONE;
			if (urb_priv->type == URB_PRIV_TYPE_BULK)
				urb_priv->toggle ^= 1;
		}
		else if (hc->hc_state == HC_NAK)
			urb_priv->state = URB_PRIV_STATE_NOTREADY;
		else if (hc->hc_state == HC_NYET)
			urb_priv->state = URB_PRIV_STATE_NOTREADY;
		else if (hc->hc_state == HC_STALL)
			urb_priv->state = URB_PRIV_STATE_STALL;
		else if ((hc->hc_state == HC_XACTERR) || (hc->hc_state == HC_DATATGLERR))
		{
			if (hc->err_cnt == 3)
			{
				hc->err_cnt = 0;
				urb_priv->state = URB_PRIV_STATE_ERROR;
			}
			else
			{
				hc->err_cnt++;
				urb_priv->state = URB_PRIV_STATE_NOTREADY;
			}
			// Maybe TODO : re-activate the channel
		}
		else
			urb_priv->state = URB_PRIV_STATE_ERROR;

		hc_reg->hcint = USB_OTG_HCINT_CHH;
		vsfdwcotg_hc_done_handler(dwcotg, urb_priv, hc);
	}
}

static void vsfdwcotg_interrupt(void *param)
{
	struct dwcotg_t *dwcotg = (struct dwcotg_t *)param;
	uint32_t intsts = dwcotg->global_reg->gintmsk;
	intsts &= dwcotg->global_reg->gintsts;

	if (!intsts)
		return;

	if (dwcotg->global_reg->gintsts & USB_OTG_GINTSTS_CMOD) // host mode
	{
#if 0
		if (intsts & USB_OTG_GINTSTS_DISCINT)
		{
			//dwcotg->host_global_regs->hcfg &= ~USB_OTG_HCFG_FSLSPCS;
			//dwcotg->host_global_regs->hcfg |= USB_OTG_HCFG_FSLSPCS_0;
			// dwcotg->host_global_regs->hfir = 48000;
			dwcotg->global_reg->gintsts = USB_OTG_GINTSTS_DISCINT;
		}
		if (intsts & USB_OTG_GINTSTS_HPRTINT)
		{
			// TODO
			dwcotg->global_reg->gintsts = USB_OTG_GINTSTS_HPRTINT;
		}
#endif
		if (intsts & USB_OTG_GINTSTS_SOF)	// Handle Host SOF Interrupts
		{
#if 0 // TODO
			uint8_t i;
			struct hc_t *hc = dwcotg->hc_pool;
			for (i = 0; i < dwcotg->hc_amount; i++)
			{
				if ((hc[i].alloced) && (hc[i].hc_state == HC_WAIT))
					submit_priv_urb(dwcotg, hc[i].owner_priv);
			}
#endif
			dwcotg->softick++;
			dwcotg->global_reg->gintsts = USB_OTG_GINTSTS_SOF;
		}
		if (intsts & USB_OTG_GINTSTS_HCINT)	// Handle Host channel Interrupts
		{
			uint8_t i;
			uint32_t haint = dwcotg->host_global_regs->haint;
			for (i = 0; i < dwcotg->hc_amount; i++)
			{
				if (haint & (0x1ul << i))
				{
					if (dwcotg->hc_regs[i].hcchar & USB_OTG_HCCHAR_EPDIR)
						vsfdwcotg_hc_in_handler(dwcotg, i);
					else
						vsfdwcotg_hc_out_handler(dwcotg, i);
				}
			}
			dwcotg->global_reg->gintsts = USB_OTG_GINTSTS_HCINT;
		}
		if (intsts & USB_OTG_GINTSTS_RXFLVL)
		{
			uint8_t channelnum;
			uint32_t pktsts, pktcnt, temp;
			
			dwcotg->global_reg->gintmsk &= ~USB_OTG_GINTMSK_RXFLVLM;
			
			temp = dwcotg->global_reg->grxstsp;
			channelnum = temp &  USB_OTG_GRXSTSP_EPNUM;
			pktsts = (temp & USB_OTG_GRXSTSP_PKTSTS) >> 17;
			pktcnt = (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
			if (pktsts == 2)	// GRXSTS_PKTSTS_IN
			{
				if (pktcnt > 0)
				{
					// TODO
					//hc_manual_read()
				}
			}
			
			dwcotg->global_reg->gintmsk |= USB_OTG_GINTMSK_RXFLVLM;
		}
		if (intsts & USB_OTG_GINTSTS_NPTXFE)
		{
			// TODO
			dwcotg->global_reg->gintsts = USB_OTG_GINTSTS_NPTXFE;
		}
		if (intsts & USB_OTG_GINTSTS_HPRTINT)
		{
			// TODO
			dwcotg->global_reg->gintsts = USB_OTG_GINTSTS_HPRTINT;
		}
		if (intsts & USB_OTG_GINTSTS_PTXFE)
		{
			// TODO
			dwcotg->global_reg->gintsts = USB_OTG_GINTSTS_PTXFE;
		}
	}
	else // device mode
	{
		// TODO
	}

	return;
}

static vsf_err_t dwcotg_init_get_resource(struct vsfusbh_t *usbh,
		uint32_t reg_base)
{
	struct vsfdwcotg_hcd_param_t *hcd_param = usbh->hcd.param;
	struct dwcotg_t *dwcotg = vsf_bufmgr_malloc(sizeof(struct dwcotg_t));
	if (dwcotg)
		memset(dwcotg, 0, sizeof(struct dwcotg_t));	
	usbh->hcd.priv = dwcotg;
	
	// config init
	dwcotg->speed = hcd_param->speed;
	dwcotg->dma_en = hcd_param->dma_en;
	dwcotg->ulpi_en = hcd_param->ulpi_en;
	dwcotg->utmi_en = hcd_param->utmi_en;
	dwcotg->vbus_en = hcd_param->vbus_en;
	dwcotg->hc_amount = hcd_param->hc_amount;
	
	//dwcotg->ep_in_amount = VSFUSBD_CFG_MAX_IN_EP;
	//dwcotg->ep_out_amount = VSFUSBD_CFG_MAX_OUT_EP;
	dwcotg->ep_in_amount = 0;
	dwcotg->ep_out_amount = 0;
	
	// reg addr init
	dwcotg->global_reg =
			(struct dwcotg_core_global_regs_t *)(reg_base + 0);
	dwcotg->host_global_regs =
			(struct dwcotg_host_global_regs_t *)(reg_base + 0x400);
	dwcotg->hprt0 =
			(volatile uint32_t *)(reg_base + 0x440);
	dwcotg->hc_regs =
			(struct dwcotg_hc_regs_t *)(reg_base + 0x500);
	dwcotg->dev_global_regs =
			(struct dwcotg_dev_global_regs_t *)(reg_base + 0x800);
	dwcotg->in_ep_regs =
			(struct dwcotg_dev_in_ep_regs_t *)(reg_base + 0x900);
	dwcotg->out_ep_regs =
			(struct dwcotg_dev_out_ep_regs_t *)(reg_base + 0xb00);
	dwcotg->dfifo = (uint32_t *)(reg_base + 0x1000);

	dwcotg->hc_pool = vsf_bufmgr_malloc(sizeof(struct hc_t) *
			dwcotg->hc_amount);
	if (dwcotg->hc_pool == NULL)
	{
		vsf_bufmgr_free(dwcotg);
		usbh->hcd.priv = NULL;
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}
	memset(dwcotg->hc_pool, 0, sizeof(struct hc_t) * dwcotg->hc_amount);
	
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_init_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	uint8_t i;
	vsf_err_t err;
	struct vsfusbh_t *usbh = pt->user_data;
	struct dwcotg_t *dwcotg = usbh->hcd.priv;
	struct vsfdwcotg_hcd_param_t *hcd_param = usbh->hcd.param;

	vsfsm_pt_begin(pt);

	usbh->hcd.rh_speed = hcd_param->speed;
	err = dwcotg_init_get_resource(usbh,
			(uint32_t)vsfhal_hcd_regbase(hcd_param->index));
	if (err)
		return err;
	dwcotg = usbh->hcd.priv;
	
	vsfhal_hcd_init(hcd_param->index, hcd_param->int_priority, vsfdwcotg_interrupt, dwcotg);

	// disable global int
	dwcotg->global_reg->gahbcfg &= ~USB_OTG_GAHBCFG_GINT;

	if (dwcotg->ulpi_en)
	{
		dwcotg->global_reg->gccfg &= ~USB_OTG_GCCFG_PWRDWN;
		// Init The ULPI Interface
		dwcotg->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_TSDPS | USB_OTG_GUSBCFG_ULPIFSLS | USB_OTG_GUSBCFG_PHYSEL);
		// Select vbus source
		dwcotg->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_ULPIEVBUSD | USB_OTG_GUSBCFG_ULPIEVBUSI);
		if (dwcotg->vbus_en)
			dwcotg->global_reg->gusbcfg |= USB_OTG_GUSBCFG_ULPIEVBUSD;
	}
	else if (dwcotg->utmi_en)
	{
		dwcotg->global_reg->gccfg &= ~USB_OTG_GCCFG_PWRDWN;
		// Init The UTMI Interface
		dwcotg->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_TSDPS | USB_OTG_GUSBCFG_ULPIFSLS | USB_OTG_GUSBCFG_PHYSEL);
		// Select vbus source
		dwcotg->global_reg->gusbcfg &= ~(USB_OTG_GUSBCFG_ULPIEVBUSD | USB_OTG_GUSBCFG_ULPIEVBUSI);
		// Select UTMI Interace
		dwcotg->global_reg->gusbcfg &= ~USB_OTG_GUSBCFG_ULPI_UTMI_SEL;
		if (dwcotg->vbus_en)
			dwcotg->global_reg->gusbcfg |= USB_OTG_GUSBCFG_ULPIEVBUSD;
	}
	else
	{
		// Select FS Embedded PHY
		dwcotg->global_reg->gusbcfg |= USB_OTG_GUSBCFG_PHYSEL;
	}

	// Core Reset
	dwcotg->retry = 0;
	while ((dwcotg->global_reg->grstctl & USB_OTG_GRSTCTL_AHBIDL) == 0)
	{
		if (dwcotg->retry > 10)
			return VSFERR_FAIL;

		vsfsm_pt_delay(pt, 2);
		dwcotg->retry++;
	}
	dwcotg->global_reg->grstctl |= USB_OTG_GRSTCTL_CSRST;
	dwcotg->retry = 0;
	while ((dwcotg->global_reg->grstctl & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST)
	{
		if (dwcotg->retry > 10)
			return VSFERR_FAIL;

		vsfsm_pt_delay(pt, 2);
		dwcotg->retry++;
	}

	// Deactivate the power down
	if (!dwcotg->ulpi_en && !dwcotg->utmi_en)
		dwcotg->global_reg->gccfg = USB_OTG_GCCFG_PWRDWN;

	if (dwcotg->dma_en)
		dwcotg->global_reg->gahbcfg |= USB_OTG_GAHBCFG_DMAEN | USB_OTG_GAHBCFG_HBSTLEN_1 | USB_OTG_GAHBCFG_HBSTLEN_2;

	// Force Host Mode
	dwcotg->global_reg->gusbcfg &= ~USB_OTG_GUSBCFG_FDMOD;
	dwcotg->global_reg->gusbcfg |= USB_OTG_GUSBCFG_FHMOD;
	vsfsm_pt_delay(pt, 50);

	// Enable Core
	dwcotg->global_reg->gccfg |= USB_OTG_GCCFG_VBDEN;
	vsfsm_pt_delay(pt, 50);
	
	if (dwcotg->speed == USB_SPEED_HIGH)
		dwcotg->host_global_regs->hcfg &= ~USB_OTG_HCFG_FSLSS;
	else
		dwcotg->host_global_regs->hcfg |= USB_OTG_HCFG_FSLSPCS;

	// Flush FIFO
	dwcotg->global_reg->grstctl = USB_OTG_GRSTCTL_TXFFLSH | USB_OTG_GRSTCTL_TXFNUM_4;
	dwcotg->retry = 0;
	while ((dwcotg->global_reg->grstctl & USB_OTG_GRSTCTL_TXFFLSH) ==
			USB_OTG_GRSTCTL_TXFFLSH)
	{
		if (dwcotg->retry > 10)
			return VSFERR_FAIL;
		vsfsm_pt_delay(pt, 2);
		dwcotg->retry++;
	}
	dwcotg->global_reg->grstctl = USB_OTG_GRSTCTL_RXFFLSH;
	dwcotg->retry = 0;
	while ((dwcotg->global_reg->grstctl & USB_OTG_GRSTCTL_RXFFLSH) ==
			USB_OTG_GRSTCTL_RXFFLSH)
	{
		if (dwcotg->retry > 10)
			return VSFERR_FAIL;
		vsfsm_pt_delay(pt, 2);
		dwcotg->retry++;
	}

	// Clear all pending HC Interrupts
	for (i = 0; i < dwcotg->hc_amount; i++)
	{
		dwcotg->hc_regs[i].hcint = 0xffffffff;
		dwcotg->hc_regs[i].hcintmsk = 0;
	}
	
	vsfsm_pt_delay(pt, 10);

	// Disable all interrupts
	dwcotg->global_reg->gintmsk = 0;

	// Clear any pending interrupts
	dwcotg->global_reg->gintsts = 0xffffffff;

	// fifo size
	dwcotg->global_reg->grxfsiz = hcd_param->rx_fifo_size;
	dwcotg->global_reg->gnptxfsiz = hcd_param->rx_fifo_size +
			((uint32_t)hcd_param->non_periodic_tx_fifo_size << 16);
	dwcotg->global_reg->hptxfsiz = hcd_param->rx_fifo_size + hcd_param->non_periodic_tx_fifo_size +
			((uint32_t)hcd_param->periodic_tx_fifo_size << 16);

	// Enable the common interrupts
	if (!dwcotg->dma_en)
		dwcotg->global_reg->gintmsk |= USB_OTG_GINTMSK_RXFLVLM;

	// Enable interrupts matching to the Host mode ONLY
	dwcotg->global_reg->gintmsk |= USB_OTG_GINTMSK_HCIM | USB_OTG_GINTMSK_SOFM;
#if 0
	dwcotg->global_reg->gintmsk |= USB_OTG_GINTMSK_PRTIM | USB_OTG_GINTSTS_DISCINT;
#endif
	
	vsfsm_pt_delay(pt, 10);

	// enable global int
	dwcotg->global_reg->gahbcfg |= USB_OTG_GAHBCFG_GINT;

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_fini(struct vsfhcd_t *hcd)
{
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_suspend(struct vsfhcd_t *hcd)
{
	return VSFERR_NONE;
}

static vsf_err_t dwcotgh_resume(struct vsfhcd_t *hcd)
{
	return VSFERR_NONE;
}

static struct vsfhcd_urb_t *dwcotgh_alloc_urb(struct vsfhcd_t *hcd)
{
	uint32_t size;
	struct vsfhcd_urb_t *urb;
	
	size = sizeof(struct vsfhcd_urb_t) - 4 + sizeof(struct urb_priv_t);
	urb = vsf_bufmgr_malloc_aligned(size, 16);
	if (urb)
		memset(urb, 0, size);
	return urb;
}

static void dwcotgh_free_urb(struct vsfhcd_t *hcd, struct vsfhcd_urb_t *urb)
{
	//struct dwcotg_t *dwcotg = hcd->priv;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)urb->priv;
	
	if (!urb)
		return;

	if (urb_priv->hc)
		urb_priv->discarded = 1;
	else
	{
		if (urb->transfer_buffer != NULL)
			vsf_bufmgr_free(urb->transfer_buffer);
		vsf_bufmgr_free(urb);
	}
}

static vsf_err_t dwcotgh_submit_urb(struct vsfhcd_t *hcd, struct vsfhcd_urb_t *urb)
{
	vsf_err_t err;
	struct hc_t *hc;
	struct dwcotg_t *dwcotg = hcd->priv;
	struct urb_priv_t *urb_priv = (struct urb_priv_t *)urb->priv;
	uint32_t pipe = urb->pipe;

	memset(urb_priv, 0, sizeof(struct urb_priv_t));

	urb_priv->type = usb_pipetype(pipe);
	if (urb_priv->type == URB_PRIV_TYPE_CTRL)
		urb_priv->phase = URB_PRIV_PHASE_SETUP_WAIT;
	else if (urb_priv->type == URB_PRIV_TYPE_BULK)
		urb_priv->phase = URB_PRIV_PHASE_DATA_WAIT;
	else
		urb_priv->phase = URB_PRIV_PHASE_PERIOD_WAIT;

	urb_priv->speed = usb_pipespeed(pipe);
	
	if ((urb_priv->type == URB_PRIV_TYPE_INT) || (urb_priv->type == URB_PRIV_TYPE_BULK))
	{
		urb_priv->toggle = urb->hcddev->toggle[usb_pipein(pipe)] &
				(0x1ul << usb_pipeendpoint(pipe));
		urb_priv->do_ping = dwcotg->dma_en ? 0 :
				(usb_pipespeed(pipe) == USB_SPEED_HIGH);
	}

	urb_priv->transfer_buffer = urb->transfer_buffer;
	urb_priv->transfer_length = urb->transfer_length;
	urb_priv->packet_size = urb->packet_size;
	urb_priv->dir_o0_i1 = usb_pipein(pipe);

	hc = hc_alloc(dwcotg);
	if (hc)
	{
		hc->dev_addr = usb_pipedevice(pipe);
		hc->ep_num = usb_pipeendpoint(pipe);
		hc->owner_priv = urb_priv;
		urb_priv->hc = hc;

		err = submit_priv_urb(dwcotg, urb_priv);
		if (err == VSFERR_NONE)
			return VSFERR_NONE;
	}
	else
		err = VSFERR_NOT_ENOUGH_RESOURCES;

	urb->status = URB_FAIL;
	return err;
}

// use for int/iso urb
static vsf_err_t dwcotgh_relink_urb(struct vsfhcd_t *hcd, struct vsfhcd_urb_t *urb)
{
	return submit_priv_urb(hcd->priv, (struct urb_priv_t *)urb->priv);
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

static int dwcotgh_rh_control(struct vsfhcd_t *hcd, struct vsfhcd_urb_t *urb)
{
	uint16_t typeReq, wValue, wLength;
	struct dwcotg_t *dwcotg = hcd->priv;
	struct usb_ctrlrequest_t *cmd = &urb->setup_packet;
	uint32_t datadw[4];
	uint8_t *data = (uint8_t*)datadw;
	uint8_t len = 0;

	typeReq = (cmd->bRequestType << 8) | cmd->bRequest;
	wValue = cmd->wValue;
	wLength = cmd->wLength;

	switch (typeReq)
	{
	case GetHubStatus:
		datadw[0] = 0;
		len = 4;
		break;
	case GetPortStatus:
		datadw[0] = rd_rh_portstat(dwcotg->hprt0);
		len = 4;
		break;
	case SetPortFeature:
		switch (wValue)
		{
		case(USB_PORT_FEAT_RESET):
			if (*dwcotg->hprt0 & USB_OTG_HPRT_PCSTS)
			{
				uint32_t hprt0 = *dwcotg->hprt0 & ~(USB_OTG_HPRT_PENA |
						USB_OTG_HPRT_PCDET | USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);
				*dwcotg->hprt0 = hprt0 | USB_OTG_HPRT_PRST;
				
				if (dwcotg->ulpi_en || dwcotg->utmi_en)
				{
					if (dwcotg->speed == USB_SPEED_FULL)
						dwcotg->host_global_regs->hfir = 60000;
				}
				else
				{
					if ((hprt0 & USB_OTG_HPRT_PSPD) == USB_OTG_HPRT_PSPD_1)	// USB_SPEED_LOW
					{
						dwcotg->host_global_regs->hcfg &= ~USB_OTG_HCFG_FSLSPCS;
						dwcotg->host_global_regs->hcfg |= USB_OTG_HCFG_FSLSPCS_1;
						dwcotg->host_global_regs->hfir = 6000;
					}
					else
					{
						dwcotg->host_global_regs->hcfg &= ~USB_OTG_HCFG_FSLSPCS;
						dwcotg->host_global_regs->hcfg |= USB_OTG_HCFG_FSLSPCS_0;
						dwcotg->host_global_regs->hfir = 48000;
					}
				}				
			}
			len = 0;
			break;
		case(USB_PORT_FEAT_POWER):
			{
				uint32_t hprt0 = *dwcotg->hprt0 & ~(USB_OTG_HPRT_PENA |
						USB_OTG_HPRT_PCDET | USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);
				*dwcotg->hprt0 = hprt0 | USB_OTG_HPRT_PPWR;
			}
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
			*dwcotg->hprt0 &= ~(USB_OTG_HPRT_PENA | USB_OTG_HPRT_PCDET |
					USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);
			len = 0;
			break;
		case(USB_PORT_FEAT_C_RESET):
			*dwcotg->hprt0 &= ~(USB_OTG_HPRT_PRST |
					USB_OTG_HPRT_PCDET | USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);
			len = 0;
			break;
		case(USB_PORT_FEAT_C_CONNECTION):
			{
				uint32_t hprt0 = *dwcotg->hprt0 & ~(USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);
				*dwcotg->hprt0 = hprt0 | USB_OTG_HPRT_PCDET;
			}
			len = 0;
			break;
		case(USB_PORT_FEAT_C_ENABLE):
			{
				uint32_t hprt0 = *dwcotg->hprt0 & ~(USB_OTG_HPRT_PCDET | USB_OTG_HPRT_POCCHNG);
				*dwcotg->hprt0 = hprt0 | USB_OTG_HPRT_PENCHNG;
			}
			len = 0;
			break;
		case(USB_PORT_FEAT_C_SUSPEND):
			*dwcotg->hprt0 &= ~(USB_OTG_HPRT_PSUSP | USB_OTG_HPRT_PCDET |
					USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);
			len = 0;
			break;
		case(USB_PORT_FEAT_C_OVER_CURRENT):
			*dwcotg->hprt0 &= ~(USB_OTG_HPRT_POCA | USB_OTG_HPRT_PCDET |
					USB_OTG_HPRT_PENCHNG | USB_OTG_HPRT_POCCHNG);
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
		if (urb->transfer_length < len)
			len = urb->transfer_length;
		urb->actual_length = len;
		memcpy(urb->transfer_buffer, data, len);
	}
	return len;

error:
	urb->status = URB_FAIL;
	return VSFERR_FAIL;
}

const struct vsfhcd_drv_t vsfdwcotgh_drv =
{
	.init_thread = dwcotgh_init_thread,
	.fini = dwcotgh_fini,
	.suspend = dwcotgh_suspend,
	.resume = dwcotgh_resume,
	.alloc_urb = dwcotgh_alloc_urb,
	.free_urb = dwcotgh_free_urb,
	.submit_urb = dwcotgh_submit_urb,
	.relink_urb = dwcotgh_relink_urb,
	.rh_control = dwcotgh_rh_control,
};


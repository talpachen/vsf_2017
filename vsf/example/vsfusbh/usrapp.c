#include "vsf.h"
#include "usrapp.h"

static struct vsfdwc2_hcd_param_t dwc2_hcd_param = 
{
	.index = 0,
	
#if defined(VSFUSBD_CFG_HIGHSPEED)
	.speed = USB_SPEED_HIGH,
#elif defined(VSFUSBD_CFG_FULLSPEED)
	.speed = USB_SPEED_FULL,
#elif defined(VSFUSBD_CFG_LOWSPEED)
	.speed = USB_SPEED_LOW,
#endif
	.dma_en = true,
	.ulpi_en = false,
	.utmi_en = true,
	.vbus_en = false,
	
	.hc_ep_amount = VSFUSBD_CFG_EPMAXNO,
	.fifo_depth = (4096) / 4,		// FIXME

#ifdef VSFDWC2_OTG_ENABLE
#endif

#ifdef VSFDWC2_DEVICE_ENABLE
	.device_rx_fifo_depth = (64 * 4) / 4,	// FIXME
	.device_tx_fifo_depth[0] = (64 * 2) / 4,
	.device_tx_fifo_depth[1] = (64 * 2) / 4,
	.device_tx_fifo_depth[2] = (64 * 2) / 4,
	.device_tx_fifo_depth[3] = (64 * 2) / 4,
	.device_tx_fifo_depth[4] = (64 * 2) / 4,
	.device_tx_fifo_depth[5] = (64 * 2) / 4,
	.device_tx_fifo_depth[6] = (64 * 2) / 4,
	.device_tx_fifo_depth[7] = (64 * 2) / 4,
#endif

#ifdef VSFDWC2_HOST_ENABLE
	.host_rx_fifo_depth = (512 * 4) /4,
	.host_non_periodic_tx_fifo_depth = (512 * 2) /4,
	.host_periodic_tx_fifo_depth = (512 * 2) /4,
#endif
};
static uint32_t dwc2_hcd_rx_buff[(512 * 4) / 4];

struct usrapp_t usrapp;

static void usrapp_srt0_do(void *p)
{
	struct usrapp_t *app = p;

	// usbh
	vsfusbh_init(&app->usbh);
	vsfusbh_register_driver(&app->usbh, &vsfusbh_hub_drv);
}

void usrapp_initial_init(struct usrapp_t *app)
{
	// null
}

void usrapp_nrt_init(struct usrapp_t *app)
{
}

void usrapp_srt0_init(struct usrapp_t *app)
{
	vsftimer_create_cb(10, 1, usrapp_srt0_do, app);
}

//void usrapp_srt1_init(struct usrapp_t *app);
//void usrapp_srt2_init(struct usrapp_t *app);
//void usrapp_srt3_init(struct usrapp_t *app);

int main()
{
	return vsfmain();
}


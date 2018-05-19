#include "vsf.h"
#include "usrapp.h"

#if defined(SOC_TYPE_STM32F411)
static struct vsfdwcotg_hcd_param_t fs_dwcotg_param = 
{
	.index = VSFHAL_USB_FS_INDEX,
	.int_priority = VSFHAL_USB_FS_PRIORITY,

	.speed = USB_SPEED_FULL,
	.dma_en = 0,
	.ulpi_en = 0,
	.vbus_en = 0,
	.hc_amount = 8,
	.fifo_size = 0x500,
	.periodic_out_packet_size_max = 256,
	.non_periodic_out_packet_size_max = 256,
	.in_packet_size_max = 256,
};
#define USBH_HCDDRV		&vsfdwcotgh_drv
#define USBH_HCDPARAM	&fs_dwcotg_param
#elif defined(SOC_TYPE_CMEM7)
uint8_t heap_buf[4096];

static struct vsfdwcotg_hcd_param_t hs_dwcotg_param = 
{
	.index = VSFHAL_USB_HS_INDEX,
	.int_priority = VSFHAL_USB_HS_PRIORITY,

	.speed = USB_SPEED_HIGH,
	.dma_en = 1,
	.ulpi_en = 1,
	.vbus_en = 0,
	.hc_amount = 8,
	.fifo_size = 0x500,
	.periodic_out_packet_size_max = 256,
	.non_periodic_out_packet_size_max = 256,
	.in_packet_size_max = 256,
};
#define USBH_HCDDRV		&vsfdwcotgh_drv
#define USBH_HCDPARAM	&hs_dwcotg_param
#endif


struct usrapp_t usrapp =
{
	.usbh = 
	{
		.hcd.drv = USBH_HCDDRV,
		.hcd.param = USBH_HCDPARAM,
	},
};

static void usrapp_pendsv_do(void *p)
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

void usrapp_srt_init(struct usrapp_t *app)
{
	// Increase the difficulty of disassembly
	vsftimer_create_cb(1000, 1, usrapp_pendsv_do, app);
}

void usrapp_nrt_init(struct usrapp_t *app)
{

}


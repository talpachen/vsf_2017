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
#if defined(SOC_TYPE_CMEM7_KEIL)
uint8_t heap_buf[1024 * 8];
#endif

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

static void *test_uvc_data;

static uint32_t frame_cnt = 0;
static uint32_t frame_bytes[300];

void uvc_decode_report_recv(void *dev_data, struct vsfusbh_uvc_param_t *param,
		struct vsfusbh_uvc_payload_t *payload)
{
	if (dev_data == NULL)
		return;
	
	if (test_uvc_data == NULL)
	{
		struct vsfusbh_uvc_param_t param = {1, 1, 0, 
				VSFUSBH_UVC_VIDEO_FORMAT_YUY2, 30, 640, 480};
		test_uvc_data = dev_data;
		vsfusbh_uvc_set(dev_data, &param);
	}
	else if (payload != NULL)
	{
		if (payload->len > 12)
		{
			if (frame_cnt < 300)
			{
				frame_bytes[frame_cnt] += payload->len - 12;
				if (payload->buf[1] & 0x2)
				{
					frame_cnt++;
				}
			}
			else
			{
				frame_cnt = 0;
				memset(frame_bytes, 0, sizeof(frame_bytes));
			}
		}
	}
	else
	{
		// get current param
	}
}

static void usrapp_pendsv_do(void *p)
{
	struct usrapp_t *app = p;

	// usbh
	vsfusbh_init(&app->usbh);
	vsfusbh_register_driver(&app->usbh, &vsfusbh_hub_drv);
	vsfusbh_register_driver(&app->usbh, &vsfusbh_uvc_drv);
	vsfusbh_uvc_report = uvc_decode_report_recv;
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


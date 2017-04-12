#ifndef __USRAPP_H__
#define __USRAPP_H__

#include "dal/usart_stream/usart_stream.h"

struct usrapp_t
{
	struct
	{
		struct vsfusbd_device_t device;
		struct vsfusbd_config_t config[1];
		struct vsfusbd_iface_t ifaces[8];

		struct usrapp_cdc_uart_t
		{
			struct usart_stream_info_t usart_stream;

			struct vsfusbd_CDCACM_param_t param;
			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[256 + 4];
			uint8_t rxbuff[256 + 4];
		} cdc[4];
	} usbd;
};

extern struct usrapp_t usrapp;

void usrapp_init(struct usrapp_t *app);

#endif // __USRAPP_H__


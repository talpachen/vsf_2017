#ifndef __USRAPP_H__
#define __USRAPP_H__

#include "shell_cmd/help.h"
#include "shell_cmd/cdcuart.h"
#include "shell_cmd/ota.h"

struct usrapp_t
{
	struct
	{
		struct vsfusbd_device_t device;
		struct vsfusbd_config_t config[1];
		struct vsfusbd_iface_t ifaces[2];

		struct
		{
			struct vsfusbd_CDCACM_param_t param;
			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[256 + 4];
			uint8_t rxbuff[256 + 4];
		} cdc;
	} usbd;
	
	struct vsfshell_t shell;
	struct cdcuart_param_t cdcuart_param;
	struct ota_param_t ota_param;
};

extern struct usrapp_t usrapp;

void usrapp_init(struct usrapp_t *app, int32_t level);

#endif // __USRAPP_H__


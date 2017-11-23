#ifndef __USRAPP_H__
#define __USRAPP_H__

#include "shell_cmd/help.h"
#include "shell_cmd/cdcuart.h"
#include "shell_cmd/sq.h"
#include "component/stream/usart_stream/usart_stream.h"

struct usrapp_t
{
	struct usrapp_usbd_t
	{
		struct vsfusbd_device_t device;
		struct vsfusbd_config_t config[1];
		struct vsfusbd_iface_t ifaces[7];
		struct
		{
			struct vsfusbd_VLLINK_param_t param;
			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[512 + 4];
			uint8_t rxbuff[512 + 4];
		} vllink;
		struct
		{
			struct vsfusbd_CDCACM_param_t param;
			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[256 + 4];
			uint8_t rxbuff[256 + 4];
		} shell_cdc;
		struct
		{
			struct usart_stream_info_t usart_stream;
			struct vsfusbd_CDCACM_param_t param;
			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[256 + 4];
			uint8_t rxbuff[256 + 4];
		} wifi_uart_cdc;
		struct
		{
			struct usart_stream_info_t usart_stream;
			struct vsfusbd_CDCACM_param_t param;
			struct vsf_fifostream_t stream_tx;
			struct vsf_fifostream_t stream_rx;
			uint8_t txbuff[256 + 4];
			uint8_t rxbuff[256 + 4];
		} ext_uart_cdc;
	} usbd;
	
	struct vsfshell_t shell;
	struct sq_param_t sq_param;
	struct cdcuart_param_t wifi_uart_param;
	struct cdcuart_param_t ext_uart_param;
};

extern struct usrapp_t usrapp;

void usrapp_initial_init(struct usrapp_t *app);
void usrapp_srt_init(struct usrapp_t *app);
void usrapp_nrt_init(struct usrapp_t *app);

#endif // __USRAPP_H__


#include "vsf.h"
#include "usrapp.h"

#include "usbd_config.c"

static vsf_err_t wifi_uart_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding);
static vsf_err_t ext_uart_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding);

struct usrapp_t usrapp =
{	
	.usbd.device.num_of_configuration		= dimof(usrapp.usbd.config),
	.usbd.device.config						= usrapp.usbd.config,
	.usbd.device.desc_filter				= (struct vsfusbd_desc_filter_t *)USB_descriptors,
	.usbd.device.device_class_iface			= 0,
	.usbd.device.drv						= (struct vsfhal_usbd_t *)&vsfhal_usbd,
	.usbd.device.int_priority				= APPCFG_USBD_PRIORITY,
	.usbd.config[0].num_of_ifaces			= dimof(usrapp.usbd.ifaces),
	.usbd.config[0].iface					= usrapp.usbd.ifaces,
	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_VLLINK_class,
	.usbd.ifaces[0].protocol_param			= &usrapp.usbd.vllink.param,
	.usbd.ifaces[1].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[1].protocol_param			= &usrapp.usbd.shell_cdc.param,
	.usbd.ifaces[2].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[2].protocol_param			= &usrapp.usbd.shell_cdc.param,
	.usbd.ifaces[3].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[3].protocol_param			= &usrapp.usbd.wifi_uart_cdc.param,
	.usbd.ifaces[4].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[4].protocol_param			= &usrapp.usbd.wifi_uart_cdc.param,
	.usbd.ifaces[5].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[5].protocol_param			= &usrapp.usbd.ext_uart_cdc.param,
	.usbd.ifaces[6].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[6].protocol_param			= &usrapp.usbd.ext_uart_cdc.param,

	.usbd.vllink.param.ep_in							= 1,
	.usbd.vllink.param.ep_out							= 2,
	.usbd.vllink.param.stream_tx						= (struct vsf_stream_t *)&usrapp.usbd.vllink.stream_tx,
	.usbd.vllink.param.stream_rx						= (struct vsf_stream_t *)&usrapp.usbd.vllink.stream_rx,
	.usbd.vllink.stream_tx.stream.op					= &fifostream_op,
	.usbd.vllink.stream_tx.mem.buffer.buffer			= (uint8_t *)&usrapp.usbd.vllink.txbuff,
	.usbd.vllink.stream_tx.mem.buffer.size				= sizeof(usrapp.usbd.vllink.txbuff),
	.usbd.vllink.stream_rx.stream.op					= &fifostream_op,
	.usbd.vllink.stream_rx.mem.buffer.buffer			= (uint8_t *)&usrapp.usbd.vllink.rxbuff,
	.usbd.vllink.stream_rx.mem.buffer.size				= sizeof(usrapp.usbd.vllink.rxbuff),
	
	.usbd.shell_cdc.param.CDC.ep_notify					= 3,
	.usbd.shell_cdc.param.CDC.ep_in						= 4,
	.usbd.shell_cdc.param.CDC.ep_out					= 5,
	.usbd.shell_cdc.param.CDC.stream_tx					= (struct vsf_stream_t *)&usrapp.usbd.shell_cdc.stream_tx,
	.usbd.shell_cdc.param.CDC.stream_rx					= (struct vsf_stream_t *)&usrapp.usbd.shell_cdc.stream_rx,
	.usbd.shell_cdc.param.line_coding.bitrate			= 115200,
	.usbd.shell_cdc.param.line_coding.stopbittype		= 0,
	.usbd.shell_cdc.param.line_coding.paritytype		= 0,
	.usbd.shell_cdc.param.line_coding.datatype			= 8,
	.usbd.shell_cdc.stream_tx.stream.op					= &fifostream_op,
	.usbd.shell_cdc.stream_tx.mem.buffer.buffer			= (uint8_t *)&usrapp.usbd.wifi_uart_cdc.txbuff,
	.usbd.shell_cdc.stream_tx.mem.buffer.size			= sizeof(usrapp.usbd.wifi_uart_cdc.txbuff),
	.usbd.shell_cdc.stream_rx.stream.op					= &fifostream_op, 
	.usbd.shell_cdc.stream_rx.mem.buffer.buffer			= (uint8_t *)&usrapp.usbd.wifi_uart_cdc.rxbuff,
	.usbd.shell_cdc.stream_rx.mem.buffer.size			= sizeof(usrapp.usbd.wifi_uart_cdc.rxbuff),

	.usbd.wifi_uart_cdc.usart_stream.index				= WIFI_UART_INDEX,
	.usbd.wifi_uart_cdc.usart_stream.mode				= VSFHAL_USART_STOPBITS_1 | VSFHAL_USART_PARITY_NONE,
	.usbd.wifi_uart_cdc.usart_stream.int_priority		= 0,
	.usbd.wifi_uart_cdc.usart_stream.baudrate			= 115200,
	.usbd.wifi_uart_cdc.usart_stream.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.wifi_uart_cdc.stream_tx,
	.usbd.wifi_uart_cdc.usart_stream.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.wifi_uart_cdc.stream_rx,
	.usbd.wifi_uart_cdc.param.CDC.ep_notify				= 6,
	.usbd.wifi_uart_cdc.param.CDC.ep_in					= 7,
	.usbd.wifi_uart_cdc.param.CDC.ep_out				= 8,
	.usbd.wifi_uart_cdc.param.CDC.stream_tx				= (struct vsf_stream_t *)&usrapp.usbd.wifi_uart_cdc.stream_rx,
	.usbd.wifi_uart_cdc.param.CDC.stream_rx				= (struct vsf_stream_t *)&usrapp.usbd.wifi_uart_cdc.stream_tx,
	.usbd.wifi_uart_cdc.param.line_coding.bitrate		= 115200,
	.usbd.wifi_uart_cdc.param.line_coding.stopbittype	= 0,
	.usbd.wifi_uart_cdc.param.line_coding.paritytype	= 0,
	.usbd.wifi_uart_cdc.param.line_coding.datatype		= 8,
	.usbd.wifi_uart_cdc.param.callback.set_line_coding	= wifi_uart_set_line_coding,
	.usbd.wifi_uart_cdc.stream_tx.stream.op				= &fifostream_op,
	.usbd.wifi_uart_cdc.stream_tx.mem.buffer.buffer		= (uint8_t *)&usrapp.usbd.wifi_uart_cdc.txbuff,
	.usbd.wifi_uart_cdc.stream_tx.mem.buffer.size		= sizeof(usrapp.usbd.wifi_uart_cdc.txbuff),
	.usbd.wifi_uart_cdc.stream_rx.stream.op				= &fifostream_op, 
	.usbd.wifi_uart_cdc.stream_rx.mem.buffer.buffer		= (uint8_t *)&usrapp.usbd.wifi_uart_cdc.rxbuff,
	.usbd.wifi_uart_cdc.stream_rx.mem.buffer.size		= sizeof(usrapp.usbd.wifi_uart_cdc.rxbuff),

	.usbd.ext_uart_cdc.usart_stream.index				= EXT_UART_INDEX,
	.usbd.ext_uart_cdc.usart_stream.mode				= VSFHAL_USART_STOPBITS_1 | VSFHAL_USART_PARITY_NONE,
	.usbd.ext_uart_cdc.usart_stream.int_priority		= 0,
	.usbd.ext_uart_cdc.usart_stream.baudrate			= 115200,
	.usbd.ext_uart_cdc.usart_stream.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.ext_uart_cdc.stream_tx,
	.usbd.ext_uart_cdc.usart_stream.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.ext_uart_cdc.stream_rx,
	.usbd.ext_uart_cdc.param.CDC.ep_notify				= 9,
	.usbd.ext_uart_cdc.param.CDC.ep_in					= 10,
	.usbd.ext_uart_cdc.param.CDC.ep_out					= 11,
	.usbd.ext_uart_cdc.param.CDC.stream_tx				= (struct vsf_stream_t *)&usrapp.usbd.ext_uart_cdc.stream_rx,
	.usbd.ext_uart_cdc.param.CDC.stream_rx				= (struct vsf_stream_t *)&usrapp.usbd.ext_uart_cdc.stream_tx,
	.usbd.ext_uart_cdc.param.line_coding.bitrate		= 115200,
	.usbd.ext_uart_cdc.param.line_coding.stopbittype	= 0,
	.usbd.ext_uart_cdc.param.line_coding.paritytype		= 0,
	.usbd.ext_uart_cdc.param.line_coding.datatype		= 8,
	.usbd.ext_uart_cdc.param.callback.set_line_coding	= ext_uart_set_line_coding,
	.usbd.ext_uart_cdc.stream_tx.stream.op				= &fifostream_op,
	.usbd.ext_uart_cdc.stream_tx.mem.buffer.buffer		= (uint8_t *)&usrapp.usbd.ext_uart_cdc.txbuff,
	.usbd.ext_uart_cdc.stream_tx.mem.buffer.size		= sizeof(usrapp.usbd.ext_uart_cdc.txbuff),
	.usbd.ext_uart_cdc.stream_rx.stream.op				= &fifostream_op, 
	.usbd.ext_uart_cdc.stream_rx.mem.buffer.buffer		= (uint8_t *)&usrapp.usbd.ext_uart_cdc.rxbuff,
	.usbd.ext_uart_cdc.stream_rx.mem.buffer.size		= sizeof(usrapp.usbd.ext_uart_cdc.rxbuff),

	.shell.echo											= true,
	.shell.stream_tx									= (struct vsf_stream_t *)&usrapp.usbd.shell_cdc.stream_tx,
	.shell.stream_rx									= (struct vsf_stream_t *)&usrapp.usbd.shell_cdc.stream_rx,
};

static vsf_err_t wifi_uart_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding)
{
	usrapp.usbd.wifi_uart_cdc.usart_stream.baudrate = line_coding->bitrate;	
	return usart_stream_init(&usrapp.usbd.wifi_uart_cdc.usart_stream);
}

static vsf_err_t ext_uart_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding)
{
	usrapp.usbd.ext_uart_cdc.usart_stream.baudrate = line_coding->bitrate;	
	return usart_stream_init(&usrapp.usbd.ext_uart_cdc.usart_stream);
}

static void usrapp_usbd_conn(void *p)
{
	struct usrapp_t *app = (struct usrapp_t *)p;
	
	vsfusbd_device_init(&app->usbd.device);
	app->usbd.device.drv->connect();
}

void usrapp_initial_init(struct usrapp_t *app)
{
	// null
}

void usrapp_srt_init(struct usrapp_t *app)
{
	STREAM_INIT(&app->usbd.shell_cdc.stream_rx);
	STREAM_INIT(&app->usbd.shell_cdc.stream_tx);
	STREAM_INIT(&app->usbd.wifi_uart_cdc.stream_rx);
	STREAM_INIT(&app->usbd.wifi_uart_cdc.stream_tx);
	STREAM_INIT(&app->usbd.ext_uart_cdc.stream_rx);
	STREAM_INIT(&app->usbd.ext_uart_cdc.stream_tx);

	vsfshell_init(&app->shell);
	help_init(&app->shell);
	//sq_init(&app->shell, &usrapp.sq_param);
	
	// usbd init
	app->usbd.device.drv->disconnect();
	vsftimer_create_cb(200, 1, usrapp_usbd_conn, app);
}

void usrapp_nrt_init(struct usrapp_t *app)
{

}

void HardFault_Handler(void)
{
	while (1);
}


#include "vsf.h"
#include "usrapp.h"

#include "usbd_config.c"

struct usrapp_t usrapp =
{
	.usbd.device.num_of_configuration		= dimof(usrapp.usbd.config),
	.usbd.device.config						= usrapp.usbd.config,
	.usbd.device.desc_filter				= (struct vsfusbd_desc_filter_t *)USB_descriptors,
	.usbd.device.device_class_iface			= 0,
	.usbd.device.drv						= (struct vsfhal_usbd_t *)&vsfhal_usbd,
	.usbd.device.int_priority				= 0,
	.usbd.config[0].num_of_ifaces			= dimof(usrapp.usbd.ifaces),
	.usbd.config[0].iface					= usrapp.usbd.ifaces,
	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[0].protocol_param			= &usrapp.usbd.cdc.param,
	.usbd.ifaces[1].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[1].protocol_param			= &usrapp.usbd.cdc.param,

	.usbd.cdc.param.CDC.ep_notify			= 1,
	.usbd.cdc.param.CDC.ep_out				= 2,
	.usbd.cdc.param.CDC.ep_in				= 2,
	.usbd.cdc.param.CDC.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc.stream_tx,
	.usbd.cdc.param.CDC.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc.stream_rx,
	.usbd.cdc.param.line_coding.bitrate		= 115200,
	.usbd.cdc.param.line_coding.stopbittype	= 0,
	.usbd.cdc.param.line_coding.paritytype	= 0,
	.usbd.cdc.param.line_coding.datatype	= 8,
	.usbd.cdc.stream_tx.stream.op			= &fifostream_op,
	.usbd.cdc.stream_tx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc.txbuff,
	.usbd.cdc.stream_tx.mem.buffer.size		= sizeof(usrapp.usbd.cdc.txbuff),
	.usbd.cdc.stream_rx.stream.op			= &fifostream_op,
	.usbd.cdc.stream_rx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc.rxbuff,
	.usbd.cdc.stream_rx.mem.buffer.size		= sizeof(usrapp.usbd.cdc.rxbuff),

	.shell.echo								= true,
	.shell.stream_tx						= (struct vsf_stream_t *)&usrapp.usbd.cdc.stream_tx,
	.shell.stream_rx						= (struct vsf_stream_t *)&usrapp.usbd.cdc.stream_rx,

	.cdcuart_param.index_table				= {0x3 << 4, 0x0 << 4, 0x1 << 4, 0x4 << 4},
	.cdcuart_param.index_num				= 4,
	.cdcuart_param.mode						= 0,
	.cdcuart_param.int_priority				= 0,
	.cdcuart_param.baudrate					= 115200,
	.cdcuart_param.param 					= &usrapp.usbd.cdc.param,
	
	.ota_param.recovery.index				= 0,
	.ota_param.recovery.addr				= 0x000a0000,
	.ota_param.recovery.size				= 0x1000,
	.ota_param.recovery.character			= 0x12345678,
	.ota_param.recovery.playload_size_max	= 252,
};

static void usbd_connect(void *p)
{
	struct usrapp_t *app = p;
	
	app->usbd.device.drv->connect();
}


void usrapp_init(struct usrapp_t *app)
{
	// vsfcdc init
	STREAM_INIT(&app->usbd.cdc.stream_rx);
	STREAM_INIT(&app->usbd.cdc.stream_tx);	
	vsfshell_init(&app->shell);
	help_init(&app->shell);
	cdcuart_init(&app->shell, &usrapp.cdcuart_param);
	ota_init(&app->shell, &usrapp.ota_param);
	
	// usbd init
	vsfusbd_device_init(&app->usbd.device);		
	app->usbd.device.drv->disconnect();
	vsftimer_create_cb(200, 1, usbd_connect, app);
}


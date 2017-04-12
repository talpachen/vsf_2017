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
	.usbd.ifaces[0].protocol_param			= &usrapp.usbd.cdc[0].param,
	.usbd.ifaces[1].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[1].protocol_param			= &usrapp.usbd.cdc[0].param,
	.usbd.ifaces[2].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[2].protocol_param			= &usrapp.usbd.cdc[1].param,
	.usbd.ifaces[3].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[3].protocol_param			= &usrapp.usbd.cdc[1].param,
	.usbd.ifaces[4].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[4].protocol_param			= &usrapp.usbd.cdc[2].param,
	.usbd.ifaces[5].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[5].protocol_param			= &usrapp.usbd.cdc[2].param,
	.usbd.ifaces[6].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMControl_class,
	.usbd.ifaces[6].protocol_param			= &usrapp.usbd.cdc[3].param,
	.usbd.ifaces[7].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_CDCACMData_class,
	.usbd.ifaces[7].protocol_param			= &usrapp.usbd.cdc[3].param,
	
	.usbd.cdc[0].usart_stream.index				= CDCUART_1_INDEX,
	.usbd.cdc[0].usart_stream.mode				= VSFHAL_USART_STOPBITS_1 | VSFHAL_USART_PARITY_NONE,
	.usbd.cdc[0].usart_stream.int_priority		= 0,
	.usbd.cdc[0].usart_stream.baudrate			= 115200,
	.usbd.cdc[0].usart_stream.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[0].stream_tx,
	.usbd.cdc[0].usart_stream.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[0].stream_rx,
	.usbd.cdc[0].param.CDC.ep_notify			= 1,
	.usbd.cdc[0].param.CDC.ep_out				= 2,
	.usbd.cdc[0].param.CDC.ep_in				= 2,
	.usbd.cdc[0].param.CDC.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[0].stream_tx,
	.usbd.cdc[0].param.CDC.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[0].stream_rx,
	.usbd.cdc[0].param.line_coding.bitrate		= 115200,
	.usbd.cdc[0].param.line_coding.stopbittype	= 0,
	.usbd.cdc[0].param.line_coding.paritytype	= 0,
	.usbd.cdc[0].param.line_coding.datatype		= 8,
	.usbd.cdc[0].stream_tx.stream.op			= &fifostream_op,
	.usbd.cdc[0].stream_tx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[0].txbuff,
	.usbd.cdc[0].stream_tx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[0].txbuff),
	.usbd.cdc[0].stream_rx.stream.op			= &fifostream_op, 
	.usbd.cdc[0].stream_rx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[0].rxbuff,
	.usbd.cdc[0].stream_rx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[0].rxbuff),
	
	.usbd.cdc[1].usart_stream.index				= CDCUART_2_INDEX,
	.usbd.cdc[1].usart_stream.mode				= VSFHAL_USART_STOPBITS_1 | VSFHAL_USART_PARITY_NONE,
	.usbd.cdc[1].usart_stream.int_priority		= 0,
	.usbd.cdc[1].usart_stream.baudrate			= 115200,
	.usbd.cdc[1].usart_stream.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[1].stream_tx,
	.usbd.cdc[1].usart_stream.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[1].stream_rx,
	.usbd.cdc[1].param.CDC.ep_notify			= 1,
	.usbd.cdc[1].param.CDC.ep_out				= 2,
	.usbd.cdc[1].param.CDC.ep_in				= 2,
	.usbd.cdc[1].param.CDC.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[1].stream_tx,
	.usbd.cdc[1].param.CDC.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[1].stream_rx,
	.usbd.cdc[1].param.line_coding.bitrate		= 115200,
	.usbd.cdc[1].param.line_coding.stopbittype	= 0,
	.usbd.cdc[1].param.line_coding.paritytype	= 0,
	.usbd.cdc[1].param.line_coding.datatype		= 8,
	.usbd.cdc[1].stream_tx.stream.op			= &fifostream_op,
	.usbd.cdc[1].stream_tx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[1].txbuff,
	.usbd.cdc[1].stream_tx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[1].txbuff),
	.usbd.cdc[1].stream_rx.stream.op			= &fifostream_op, 
	.usbd.cdc[1].stream_rx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[1].rxbuff,
	.usbd.cdc[1].stream_rx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[1].rxbuff),
};

static void usbd_connect(void *p)
{
	struct usrapp_t *app = p;
	
	app->usbd.device.drv->connect();
}

static void cdc_uart_init(struct usrapp_cdc_uart_t *usrapp_cdc_uart)
{
	STREAM_INIT(&usrapp_cdc_uart->stream_rx);
	STREAM_INIT(&usrapp_cdc_uart->stream_tx);	
}

void usrapp_init(struct usrapp_t *app)
{
	// vsfcdc init
	cdc_uart_init(&app->usbd.cdc[0]);
	//cdc_uart_init(&app->usbd.cdc[1]);
	//cdc_uart_init(&app->usbd.cdc[2]);
	//cdc_uart_init(&app->usbd.cdc[3]);
	
	// usbd init
	vsfusbd_device_init(&app->usbd.device);		
	app->usbd.device.drv->disconnect();
	vsftimer_create_cb(200, 1, usbd_connect, app);
}


#include "vsf.h"
#include "usrapp.h"

#include "usbd_config.c"

static vsf_err_t cdcuart1_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding);
static vsf_err_t cdcuart2_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding);
static vsf_err_t cdcuart3_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding);
static vsf_err_t cdcuart4_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding);

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
	.usbd.cdc[0].param.CDC.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[0].stream_rx,
	.usbd.cdc[0].param.CDC.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[0].stream_tx,
	.usbd.cdc[0].param.line_coding.bitrate		= 115200,
	.usbd.cdc[0].param.line_coding.stopbittype	= 0,
	.usbd.cdc[0].param.line_coding.paritytype	= 0,
	.usbd.cdc[0].param.line_coding.datatype		= 8,
	.usbd.cdc[0].param.callback.set_line_coding = cdcuart1_set_line_coding,
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
	.usbd.cdc[1].param.CDC.ep_notify			= 3,
	.usbd.cdc[1].param.CDC.ep_out				= 4,
	.usbd.cdc[1].param.CDC.ep_in				= 4,
	.usbd.cdc[1].param.CDC.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[1].stream_rx,
	.usbd.cdc[1].param.CDC.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[1].stream_tx,
	.usbd.cdc[1].param.line_coding.bitrate		= 115200,
	.usbd.cdc[1].param.line_coding.stopbittype	= 0,
	.usbd.cdc[1].param.line_coding.paritytype	= 0,
	.usbd.cdc[1].param.line_coding.datatype		= 8,
	.usbd.cdc[1].param.callback.set_line_coding = cdcuart2_set_line_coding,
	.usbd.cdc[1].stream_tx.stream.op			= &fifostream_op,
	.usbd.cdc[1].stream_tx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[1].txbuff,
	.usbd.cdc[1].stream_tx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[1].txbuff),
	.usbd.cdc[1].stream_rx.stream.op			= &fifostream_op, 
	.usbd.cdc[1].stream_rx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[1].rxbuff,
	.usbd.cdc[1].stream_rx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[1].rxbuff),

	.usbd.cdc[2].usart_stream.index				= CDCUART_3_INDEX,
	.usbd.cdc[2].usart_stream.mode				= VSFHAL_USART_STOPBITS_1 | VSFHAL_USART_PARITY_NONE,
	.usbd.cdc[2].usart_stream.int_priority		= 0,
	.usbd.cdc[2].usart_stream.baudrate			= 115200,
	.usbd.cdc[2].usart_stream.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[2].stream_tx,
	.usbd.cdc[2].usart_stream.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[2].stream_rx,
	.usbd.cdc[2].param.CDC.ep_notify			= 5,
	.usbd.cdc[2].param.CDC.ep_out				= 6,
	.usbd.cdc[2].param.CDC.ep_in				= 6,
	.usbd.cdc[2].param.CDC.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[2].stream_rx,
	.usbd.cdc[2].param.CDC.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[2].stream_tx,
	.usbd.cdc[2].param.line_coding.bitrate		= 115200,
	.usbd.cdc[2].param.line_coding.stopbittype	= 0,
	.usbd.cdc[2].param.line_coding.paritytype	= 0,
	.usbd.cdc[2].param.line_coding.datatype		= 8,
	.usbd.cdc[2].param.callback.set_line_coding = cdcuart3_set_line_coding,
	.usbd.cdc[2].stream_tx.stream.op			= &fifostream_op,
	.usbd.cdc[2].stream_tx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[2].txbuff,
	.usbd.cdc[2].stream_tx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[2].txbuff),
	.usbd.cdc[2].stream_rx.stream.op			= &fifostream_op, 
	.usbd.cdc[2].stream_rx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[2].rxbuff,
	.usbd.cdc[2].stream_rx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[2].rxbuff),
	
	.usbd.cdc[3].usart_stream.index				= CDCUART_4_INDEX,
	.usbd.cdc[3].usart_stream.mode				= VSFHAL_USART_STOPBITS_1 | VSFHAL_USART_PARITY_NONE,
	.usbd.cdc[3].usart_stream.int_priority		= 0,
	.usbd.cdc[3].usart_stream.baudrate			= 115200,
	.usbd.cdc[3].usart_stream.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[3].stream_tx,
	.usbd.cdc[3].usart_stream.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[3].stream_rx,
	.usbd.cdc[3].param.CDC.ep_notify			= 7,
	.usbd.cdc[3].param.CDC.ep_out				= 8,
	.usbd.cdc[3].param.CDC.ep_in				= 8,
	.usbd.cdc[3].param.CDC.stream_tx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[3].stream_rx,
	.usbd.cdc[3].param.CDC.stream_rx			= (struct vsf_stream_t *)&usrapp.usbd.cdc[3].stream_tx,
	.usbd.cdc[3].param.line_coding.bitrate		= 115200,
	.usbd.cdc[3].param.line_coding.stopbittype	= 0,
	.usbd.cdc[3].param.line_coding.paritytype	= 0,
	.usbd.cdc[3].param.line_coding.datatype		= 8,
	.usbd.cdc[3].param.callback.set_line_coding = cdcuart4_set_line_coding,
	.usbd.cdc[3].stream_tx.stream.op			= &fifostream_op,
	.usbd.cdc[3].stream_tx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[3].txbuff,
	.usbd.cdc[3].stream_tx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[3].txbuff),
	.usbd.cdc[3].stream_rx.stream.op			= &fifostream_op, 
	.usbd.cdc[3].stream_rx.mem.buffer.buffer	= (uint8_t *)&usrapp.usbd.cdc[3].rxbuff,
	.usbd.cdc[3].stream_rx.mem.buffer.size		= sizeof(usrapp.usbd.cdc[3].rxbuff),
};

static vsf_err_t cdcuart1_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding)
{
	usrapp.usbd.cdc[0].usart_stream.baudrate = line_coding->bitrate;	
	// TODO
	//usrapp.usbd.cdc[0].usart_stream.mode = VSFHAL_USART_STOPBITS_1 | VSFHAL_USART_PARITY_NONE,
	return usart_stream_init(&usrapp.usbd.cdc[0].usart_stream);
}

static vsf_err_t cdcuart2_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding)
{
	usrapp.usbd.cdc[1].usart_stream.baudrate = line_coding->bitrate;	
	return usart_stream_init(&usrapp.usbd.cdc[1].usart_stream);
}

static vsf_err_t cdcuart3_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding)
{
	usrapp.usbd.cdc[2].usart_stream.baudrate = line_coding->bitrate;	
	return usart_stream_init(&usrapp.usbd.cdc[2].usart_stream);
}

static vsf_err_t cdcuart4_set_line_coding(struct usb_CDCACM_line_coding_t *line_coding)
{
	usrapp.usbd.cdc[3].usart_stream.baudrate = line_coding->bitrate;	
	return usart_stream_init(&usrapp.usbd.cdc[3].usart_stream);
}

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

static void usrapp_init_initial(struct usrapp_t *app)
{
	// null
}

static void usrapp_init_pendsv(struct usrapp_t *app)
{
	// null
}

static void usrapp_init_main(struct usrapp_t *app)
{
	// vsfcdc init
	cdc_uart_init(&app->usbd.cdc[0]);
	cdc_uart_init(&app->usbd.cdc[1]);
	cdc_uart_init(&app->usbd.cdc[2]);
	cdc_uart_init(&app->usbd.cdc[3]);
	
	// usbd init
	vsfusbd_device_init(&app->usbd.device);		
	app->usbd.device.drv->disconnect();
	vsftimer_create_cb(200, 1, usbd_connect, app);
}

void usrapp_init(struct usrapp_t *app, int32_t level)
{
	if (level == VSFMAIN_PHASE_INITIAL)
		usrapp_init_initial(app);
	else if (level == VSFMAIN_PHASE_PENDSV)
		usrapp_init_pendsv(app);
	else if (level == VSFMAIN_PHASE_MAIN)
		usrapp_init_main(app);
	else
		return;
}


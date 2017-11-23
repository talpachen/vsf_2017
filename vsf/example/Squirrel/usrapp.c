#include "vsf.h"
#include "usrapp.h"

struct usrapp_t usrapp =
{
	.usbd_pullup = 
	{
		USBD_PULLUP_PORT,
		USBD_PULLUP_PIN,
	},
};


static void usrapp_usbd_conn(void *p)
{
	struct usrapp_t *app = (struct usrapp_t *)p;
	
	vsfusbd_device_init(&app->usbd.device);
	app->usbd.device.drv->connect();
	vsfhal_gpio_set(app->usbd_pullup.port, 1 << app->usbd_pullup.pin);
}

static void usrapp_usbd_disconn(void *p)
{
	struct usrapp_t *app = (struct usrapp_t *)p;
	
	vsfhal_gpio_clear(app->usbd_pullup.port, 1 << app->usbd_pullup.pin);
	app->usbd.device.drv->disconnect();
	vsfusbd_device_fini(&app->usbd.device);
}

void usrapp_initial_init(struct usrapp_t *app)
{

}

void usrapp_srt_init(struct usrapp_t *app)
{
	
}

void usrapp_nrt_init(struct usrapp_t *app)
{

}

void HardFault_Handler(void)
{
	while (1);
}


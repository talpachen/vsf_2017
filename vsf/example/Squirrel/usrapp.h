#ifndef __USRAPP_H__
#define __USRAPP_H__



struct usrapp_t
{
	struct vsfhal_gpio_pin_t usbd_pullup;

	struct usrapp_usbd_t
	{
		struct vsfusbd_iface_t ifaces[3];
		struct vsfusbd_config_t config[1];
		struct vsfusbd_device_t device;
	} usbd;

};

extern struct usrapp_t usrapp;

void usrapp_initial_init(struct usrapp_t *app);
void usrapp_srt_init(struct usrapp_t *app);
void usrapp_nrt_init(struct usrapp_t *app);

#endif // __USRAPP_H__


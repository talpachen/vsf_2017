#ifndef __USRAPP_H__
#define __USRAPP_H__

struct usrapp_t
{
	struct vsfusbh_t usbh;
};

extern struct usrapp_t usrapp;

void usrapp_initial_init(struct usrapp_t *app);
void usrapp_nrt_init(struct usrapp_t *app);
void usrapp_srt0_init(struct usrapp_t *app);
void usrapp_srt1_init(struct usrapp_t *app);
void usrapp_srt2_init(struct usrapp_t *app);
void usrapp_srt3_init(struct usrapp_t *app);

#endif // __USRAPP_H__


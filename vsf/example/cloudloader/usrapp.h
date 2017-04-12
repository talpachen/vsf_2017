#ifndef __USRAPP_H__
#define __USRAPP_H__

struct usrapp_t
{
	uint32_t d;
};

extern struct usrapp_t usrapp;

void usrapp_init(struct usrapp_t *app);

#endif // __USRAPP_H__


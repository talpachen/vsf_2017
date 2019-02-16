#ifndef __USRAPP_H__
#define __USRAPP_H__

struct usrapp_t
{
	uint8_t dummy;
};

extern struct usrapp_t usrapp;
#if defined(SOC_TYPE_CMEM7_KEIL)
extern uint8_t heap_buf[1024 * 8];
#endif

void usrapp_initial_init(struct usrapp_t *app);
void usrapp_srt_init(struct usrapp_t *app);
void usrapp_nrt_init(struct usrapp_t *app);

#endif // __USRAPP_H__


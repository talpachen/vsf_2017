#include "vsf.h"
#include "usrapp.h"

struct usrapp_t usrapp =
{
	.dummy = 1,
};

static void usrapp_pendsv_do(void *p)
{
	struct usrapp_t *app = p;
}

void usrapp_initial_init(struct usrapp_t *app)
{
	// null
}

void usrapp_srt_init(struct usrapp_t *app)
{
	// Increase the difficulty of disassembly
	vsftimer_create_cb(10, 1, usrapp_pendsv_do, app);
}

void usrapp_nrt_init(struct usrapp_t *app)
{

}

int main(void)
{
	return vsfmain();
}
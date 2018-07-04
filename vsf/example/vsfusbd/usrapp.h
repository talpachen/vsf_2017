#ifndef __USRAPP_H__
#define __USRAPP_H__

struct usrapp_t
{
	// mal
	struct
	{
		struct vsfscsi_device_t scsi_dev;
		struct vsfscsi_lun_t lun[1];

		struct vsf_scsistream_t scsistream;
		struct vsf_mal2scsi_t mal2scsi;
		struct vsfmal_t mal;
		struct fakefat32_param_t fakefat32;
		uint8_t *pbuffer[2];
		uint8_t buffer[2][512];
	} mal;
	
	// usb
	struct
	{
		struct
		{
			struct vsfusbd_MSCBOT_param_t param;
		} msc;
		struct vsfusbd_iface_t ifaces[1];
		struct vsfusbd_config_t config[1];
		struct vsfusbd_device_t device;
	} usbd;
};

extern struct usrapp_t usrapp;
#if defined(SOC_TYPE_CMEM7_KEIL)
extern uint8_t heap_buf[1024 * 8];
#endif

void usrapp_initial_init(struct usrapp_t *app);
void usrapp_srt_init(struct usrapp_t *app);
void usrapp_nrt_init(struct usrapp_t *app);

#endif // __USRAPP_H__


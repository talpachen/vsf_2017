#ifndef __VSFUSBH_H_INCLUDED__
#define __VSFUSBH_H_INCLUDED__

#ifndef USB_MAX_CHILDREN
#define USB_MAX_CHILDREN				4
#endif
#ifndef USB_MAX_ENDPOINTS
#define USB_MAX_ENDPOINTS				16
#endif

#define USBH_INTERFACE_RAM_OPTIMIZE		1
#define USBH_INTERFACE_MULTI_SUPPORT	0
#define USBH_ID_TABLE_OPTIMIZE			0

#include "component/usb/common/usb_common.h"
#include "component/usb/common/usb_ch11.h"
#include "hcd/vsfhcd.h"

#define VSFSM_EVT_URB_COMPLETE	(VSFSM_EVT_USER_LOCAL + 1)
#define VSFSM_EVT_NEW_DEVICE	(VSFSM_EVT_USER_LOCAL + 2)
#define VSFSM_EVT_EP0_CRIT		(VSFSM_EVT_USER_LOCAL + 3)

#define DEFAULT_TIMEOUT			50	// 50ms

struct vsfusbh_ifs_alt_t
{
	struct usb_interface_desc_t *ifs_desc;
	struct usb_endpoint_desc_t *ep_desc;
	uint16_t desc_size;
};

struct vsfusbh_class_drv_t;
struct vsfusbh_ifs_t
{
	struct vsfusbh_ifs_alt_t *alt;
	const struct vsfusbh_class_drv_t *drv;
	void *priv;
	uint8_t num_of_alt;
	uint8_t cur_alt;
};

struct vsfusbh_cfg_t
{
	struct vsfusbh_ifs_t *ifs;
	uint8_t num_of_ifs;
};

struct vsfusbh_device_t
{
	struct vsfhcd_device_t hcddev;

	struct usb_device_descriptor_t *device_desc;
	struct usb_config_descriptor_t *config_desc;
	struct vsfusbh_cfg_t config;

#ifndef VSFCFG_FUNC_USBH_TINY
	struct vsfusbh_device_t *parent;
	struct vsfusbh_device_t *children[USB_MAX_CHILDREN];
	uint8_t children_reset_request[USB_MAX_CHILDREN];
#endif

	struct vsfsm_crit_t ep0_crit;
	uint16_t ep_mps_in[USB_MAX_ENDPOINTS];
	uint16_t ep_mps_out[USB_MAX_ENDPOINTS];

	uint8_t cur_config;
	uint8_t devnum_temp;
	uint8_t maxchild;
};

#define USB_DEVICE_ID_MATCH_VENDOR			0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT			0x0002
#define USB_DEVICE_ID_MATCH_DEV_LO			0x0004
#define USB_DEVICE_ID_MATCH_DEV_HI			0x0008
#define USB_DEVICE_ID_MATCH_DEV_CLASS		0x0010
#define USB_DEVICE_ID_MATCH_DEV_SUBCLASS	0x0020
#define USB_DEVICE_ID_MATCH_DEV_PROTOCOL	0x0040
#define USB_DEVICE_ID_MATCH_INT_CLASS		0x0080
#define USB_DEVICE_ID_MATCH_INT_SUBCLASS	0x0100
#define USB_DEVICE_ID_MATCH_INT_PROTOCOL	0x0200
struct vsfusbh_device_id_t
{
	uint16_t match_flags;
	uint16_t idVendor;
	uint16_t idProduct;
#if USBH_ID_TABLE_OPTIMIZE == 0
	uint16_t bcdDevice_lo, bcdDevice_hi;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
#endif
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
#if USBH_ID_TABLE_OPTIMIZE == 0
	uint8_t bInterfaceProtocol;
	//uint32_t driver_info;
#endif
};

struct vsfusbh_t;
struct vsfusbh_class_drv_t
{
	const char *name;
	const struct vsfusbh_device_id_t *id_table;
	void * (*probe)(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev,
			struct vsfusbh_ifs_t *ifs, const struct vsfusbh_device_id_t *id);
	void (*disconnect)(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev,
			void *priv);
};

struct vsfusbh_t
{
	struct vsfhcd_t hcd;
	
	// private
#ifndef VSFCFG_FUNC_USBH_TINY
	uint32_t device_bitmap[4];
	struct vsfusbh_device_t *rh_dev;
	struct sllist drv_list;
#else
	const struct vsfusbh_class_drv_t *drv;
#endif
	
	struct vsfusbh_device_t *new_dev;
	
	struct vsfsm_t sm;
	struct vsfsm_pt_t pt;

	struct vsfhcd_urb_t *probe_urb;
};

#ifndef VSFCFG_EXCLUDE_USBH
struct vsfhcd_urb_t *vsfusbh_alloc_urb(struct vsfusbh_t *usbh);
void vsfusbh_free_urb(struct vsfusbh_t *usbh, struct vsfhcd_urb_t **purb);
uint8_t * vsfusbh_alloc_urb_buffer(struct vsfhcd_urb_t *urb, uint16_t size);
void vsfusbh_free_urb_buffer(struct vsfhcd_urb_t *urb);

vsf_err_t vsfusbh_submit_urb(struct vsfusbh_t *usbh, struct vsfhcd_urb_t *urb);
vsf_err_t vsfusbh_relink_urb(struct vsfusbh_t *usbh, struct vsfhcd_urb_t *urb);

vsf_err_t vsfusbh_init(struct vsfusbh_t *usbh);
vsf_err_t vsfusbh_fini(struct vsfusbh_t *usbh);
vsf_err_t vsfusbh_register_driver(struct vsfusbh_t *usbh,
		const struct vsfusbh_class_drv_t *drv);

struct vsfusbh_device_t *vsfusbh_alloc_device(struct vsfusbh_t *usbh);
void vsfusbh_clean_device(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev);
void vsfusbh_disconnect_device(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t **pdev);
void vsfusbh_remove_interface(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct vsfusbh_ifs_t *ifs);

vsf_err_t vsfusbh_control_msg(struct vsfusbh_t *usbh, struct vsfhcd_urb_t *urb,
		uint8_t bRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex);

vsf_err_t vsfusbh_get_descriptor(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint8_t type, uint8_t index);
vsf_err_t vsfusbh_get_class_descriptor(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint16_t ifnum, uint8_t type, uint8_t id);
vsf_err_t vsfusbh_set_configuration(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint8_t configuration);
vsf_err_t vsfusbh_set_interface(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint16_t ifnum, uint16_t alternate);

vsf_err_t vsfusbh_get_extra_descriptor(uint8_t *buf, uint16_t size,
		uint8_t type, void **ptr);
#endif

#endif	// __VSFUSBH_H_INCLUDED__

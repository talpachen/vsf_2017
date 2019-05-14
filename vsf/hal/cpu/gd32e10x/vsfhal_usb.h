#ifndef __VSFHAL_USB_H_INCLUDED__
#define __VSFHAL_USB_H_INCLUDED__

extern struct vsfdwc2_device_t vsfhal_usb_dwc2_device;
extern const uint8_t vsfhal_usbd_ep_num;
extern struct vsfhal_usbd_callback_t vsfhal_usbd_callback;

#define vsfhal_usbd_init(int_priority)				vsfdwc2_usbd_init(&vsfhal_usb_dwc2_device, int_priority)
#define vsfhal_usbd_fini()							vsfdwc2_usbd_fini(&vsfhal_usb_dwc2_device)
#define vsfhal_usbd_poll()							vsfdwc2_usbd_poll(&vsfhal_usb_dwc2_device)
#define vsfhal_usbd_reset()							vsfdwc2_usbd_reset(&vsfhal_usb_dwc2_device)
#define vsfhal_usbd_connect()						vsfdwc2_usbd_connect(&vsfhal_usb_dwc2_device)
#define vsfhal_usbd_disconnect()					vsfdwc2_usbd_disconnect(&vsfhal_usb_dwc2_device)
#define vsfhal_usbd_set_address(address)			vsfdwc2_usbd_set_address(&vsfhal_usb_dwc2_device, address)
#define vsfhal_usbd_get_address()					vsfdwc2_usbd_get_address(&vsfhal_usb_dwc2_device)
#define vsfhal_usbd_wakeup()						vsfdwc2_usbd_wakeup(&vsfhal_usb_dwc2_device)
#define vsfhal_usbd_get_frame_number()				vsfdwc2_usbd_get_frame_number(&vsfhal_usb_dwc2_device)
#define vsfhal_usbd_get_setup(buffer)				vsfdwc2_usbd_get_setup(&vsfhal_usb_dwc2_device, buffer)
#define vsfhal_usbd_prepare_buffer()				vsfdwc2_usbd_prepare_buffer(&vsfhal_usb_dwc2_device)
#define vsfhal_usbd_ep_reset(idx)					vsfdwc2_usbd_ep_reset(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_set_type(idx, type)			vsfdwc2_usbd_ep_set_type(&vsfhal_usb_dwc2_device, idx, type)
#define vsfhal_usbd_ep_set_IN_epsize(idx, epsize)	vsfdwc2_usbd_ep_set_IN_epsize(&vsfhal_usb_dwc2_device, idx, epsize)
#define vsfhal_usbd_ep_get_IN_epsize(idx)			vsfdwc2_usbd_ep_get_IN_epsize(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_set_IN_stall(idx)			vsfdwc2_usbd_ep_set_IN_stall(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_clear_IN_stall(idx)			vsfdwc2_usbd_ep_clear_IN_stall(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_is_IN_stall(idx)				vsfdwc2_usbd_ep_is_IN_stall(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_reset_IN_toggle(idx)			vsfdwc2_usbd_ep_reset_IN_toggle(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_set_IN_count(idx, size)		vsfdwc2_usbd_ep_set_IN_count(&vsfhal_usb_dwc2_device, idx, size)
#define vsfhal_usbd_ep_write_IN_buffer(idx, buffer, size)	vsfdwc2_usbd_ep_write_IN_buffer(&vsfhal_usb_dwc2_device, idx, buffer, size)
#define vsfhal_usbd_ep_set_OUT_epsize(idx, epsize)	vsfdwc2_usbd_ep_set_OUT_epsize(&vsfhal_usb_dwc2_device, idx, epsize)
#define vsfhal_usbd_ep_get_OUT_epsize(idx)			vsfdwc2_usbd_ep_get_OUT_epsize(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_set_OUT_stall(idx)			vsfdwc2_usbd_ep_set_OUT_stall(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_clear_OUT_stall(idx)			vsfdwc2_usbd_ep_clear_OUT_stall(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_is_OUT_stall(idx)			vsfdwc2_usbd_ep_is_OUT_stall(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_reset_OUT_toggle(idx)		vsfdwc2_usbd_ep_reset_OUT_toggle(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_get_OUT_count(idx)			vsfdwc2_usbd_ep_get_OUT_count(&vsfhal_usb_dwc2_device, idx)
#define vsfhal_usbd_ep_read_OUT_buffer(idx, buffer, size)	vsfdwc2_usbd_ep_read_OUT_buffer(&vsfhal_usb_dwc2_device, idx, buffer, size)
#define vsfhal_usbd_ep_enable_OUT(idx)				vsfdwc2_usbd_ep_enable_OUT(&vsfhal_usb_dwc2_device, idx)

vsf_err_t vsfhal_usb_init(uint32_t index, int32_t int_priority,
		void (*irq)(void *), void *param);

#ifdef VSFDWC2_DEVICE_ENABLE
vsf_err_t vsfhal_usb_dwc2_device_init(void *p, uint32_t *rx_buff, uint32_t rx_buffer_size);
vsf_err_t vsfhal_usb_dwc2_device_fini(void *p);
#endif

#ifdef VSFDWC2_HOST_ENABLE
#endif

#endif	// __VSFHAL_USB_H_INCLUDED__

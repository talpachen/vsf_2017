/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       interfaces.h                                              *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    interfaces header file                                    *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/

#ifndef __INTERFACES_H_INCLUDED__
#define __INTERFACES_H_INCLUDED__

#include "app_type.h"
#include "compiler.h"
#include "interfaces_cfg.h"
#include "interfaces_const.h"

/*******************************************************************************
CORE
*******************************************************************************/
vsf_err_t vsfhal_core_init(void *p);
vsf_err_t vsfhal_core_fini(void *p);
vsf_err_t vsfhal_core_reset(void *p);
uint32_t vsfhal_core_get_stack(void);
vsf_err_t vsfhal_core_set_stack(uint32_t sp);
void vsfhal_core_sleep(uint32_t mode);
vsf_err_t vsfhal_core_pendsv_config(void (*on_pendsv)(void *), void *param);
vsf_err_t vsfhal_core_pendsv_trigger(void);

/*******************************************************************************
UID
*******************************************************************************/
uint32_t vsfhal_uid_get(uint8_t *buffer, uint32_t size);

/*******************************************************************************
tickclk
*******************************************************************************/
vsf_err_t vsfhal_tickclk_init(void);
vsf_err_t vsfhal_tickclk_fini(void);
vsf_err_t vsfhal_tickclk_start(void);
vsf_err_t vsfhal_tickclk_stop(void);
uint32_t vsfhal_tickclk_get_ms(void);
uint16_t vsfhal_tickclk_get_us(void);
vsf_err_t vsfhal_tickclk_config_cb(void (*)(void*), void*);

/*******************************************************************************
FLASH
*******************************************************************************/
vsf_err_t vsfhal_flash_init(uint8_t index);
vsf_err_t vsfhal_flash_fini(uint8_t index);
vsf_err_t vsfhal_flash_capacity(uint8_t index, uint32_t *pagesize, uint32_t *pagenum);
uint32_t vsfhal_flash_baseaddr(uint8_t index);
uint32_t vsfhal_flash_blocksize(uint8_t index, uint32_t addr, uint32_t size, int op);
vsf_err_t vsfhal_flash_config_cb(uint8_t index, uint32_t int_priority, void *param, void (*onfinish)(void*, vsf_err_t));
vsf_err_t vsfhal_flash_erase(uint8_t index, uint32_t addr);
vsf_err_t vsfhal_flash_read(uint8_t index, uint32_t addr, uint8_t *buff);
vsf_err_t vsfhal_flash_write(uint8_t index, uint32_t addr, uint8_t *buff);

/*******************************************************************************
USART
*******************************************************************************/
vsf_err_t vsfhal_usart_init(uint8_t index);
vsf_err_t vsfhal_usart_fini(uint8_t index);
vsf_err_t vsfhal_usart_config(uint8_t index, uint32_t baudrate, uint32_t mode);
vsf_err_t vsfhal_usart_config_cb(uint8_t index, uint32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *, uint16_t));
uint16_t vsfhal_usart_tx_bytes(uint8_t index, uint8_t *data, uint16_t size);
uint16_t vsfhal_usart_tx_get_free_size(uint8_t index);
uint16_t vsfhal_usart_rx_bytes(uint8_t index, uint8_t *data, uint16_t size);
uint16_t vsfhal_usart_rx_get_data_size(uint8_t index);

/*******************************************************************************
GPIO
*******************************************************************************/
vsf_err_t vsfhal_gpio_init(uint8_t index);
vsf_err_t vsfhal_gpio_fini(uint8_t index);
vsf_err_t vsfhal_gpio_config(uint8_t index, uint8_t pin_idx, uint32_t mode);
vsf_err_t vsfhal_gpio_set(uint8_t index, uint32_t pin_mask);
vsf_err_t vsfhal_gpio_clear(uint8_t index, uint32_t pin_mask);
vsf_err_t vsfhal_gpio_out(uint8_t index, uint32_t pin_mask, uint32_t value);
uint32_t vsfhal_gpio_get(uint8_t index, uint32_t pin_mask);

/*******************************************************************************
USBD
*******************************************************************************/
enum interface_usbd_eptype_t
{
	USB_EP_TYPE_CONTROL,
	USB_EP_TYPE_INTERRUPT,
	USB_EP_TYPE_BULK,
	USB_EP_TYPE_ISO
};
enum interface_usbd_error_t
{
	USBERR_ERROR,
	USBERR_INVALID_CRC,
	USBERR_SOF_TO,
};
struct vsfhal_usbd_callback_t
{
	void *param;
	vsf_err_t (*on_attach)(void*);
	vsf_err_t (*on_detach)(void*);
	vsf_err_t (*on_reset)(void*);
	vsf_err_t (*on_setup)(void*);
	vsf_err_t (*on_error)(void*, enum interface_usbd_error_t error);
	vsf_err_t (*on_wakeup)(void*);
	vsf_err_t (*on_suspend)(void*);
	vsf_err_t (*on_sof)(void*);
	vsf_err_t (*on_underflow)(void*, uint8_t);
	vsf_err_t (*on_overflow)(void*, uint8_t);
	vsf_err_t (*on_in)(void*, uint8_t);
	vsf_err_t (*on_out)(void*, uint8_t);
};
struct vsfhal_usbd_t
{
	vsf_err_t (*init)(int32_t int_priority);
	vsf_err_t (*fini)(void);
	vsf_err_t (*reset)(void);

	vsf_err_t (*connect)(void);
	vsf_err_t (*disconnect)(void);

	vsf_err_t (*set_address)(uint8_t addr);
	uint8_t (*get_address)(void);

	vsf_err_t (*suspend)(void);
	vsf_err_t (*resume)(void);
	vsf_err_t (*lowpower)(uint8_t level);

	uint32_t (*get_frame_number)(void);

	vsf_err_t (*get_setup)(uint8_t *buffer);
	vsf_err_t (*prepare_buffer)(void);
	struct usbd_endpoint_t
	{
		const uint8_t *num_of_ep;

		vsf_err_t (*reset)(uint8_t idx);
		vsf_err_t (*set_type)(uint8_t idx, enum interface_usbd_eptype_t type);

		vsf_err_t (*set_IN_dbuffer)(uint8_t idx);
		bool (*is_IN_dbuffer)(uint8_t idx);
		vsf_err_t (*switch_IN_buffer)(uint8_t idx);
		vsf_err_t (*set_IN_epsize)(uint8_t idx, uint16_t size);
		uint16_t (*get_IN_epsize)(uint8_t idx);
		vsf_err_t (*set_IN_stall)(uint8_t idx);
		vsf_err_t (*clear_IN_stall)(uint8_t idx);
		bool (*is_IN_stall)(uint8_t idx);
		vsf_err_t (*reset_IN_toggle)(uint8_t idx);
		vsf_err_t (*toggle_IN_toggle)(uint8_t idx);
		vsf_err_t (*set_IN_count)(uint8_t idx, uint16_t size);
		vsf_err_t (*write_IN_buffer)(uint8_t idx, uint8_t *buffer, uint16_t size);

		vsf_err_t (*set_OUT_dbuffer)(uint8_t idx);
		bool (*is_OUT_dbuffer)(uint8_t idx);
		vsf_err_t (*switch_OUT_buffer)(uint8_t idx);
		vsf_err_t (*set_OUT_epsize)(uint8_t idx, uint16_t size);
		uint16_t (*get_OUT_epsize)(uint8_t idx);
		vsf_err_t (*set_OUT_stall)(uint8_t idx);
		vsf_err_t (*clear_OUT_stall)(uint8_t idx);
		bool (*is_OUT_stall)(uint8_t idx);
		vsf_err_t (*reset_OUT_toggle)(uint8_t idx);
		vsf_err_t (*toggle_OUT_toggle)(uint8_t idx);
		uint16_t (*get_OUT_count)(uint8_t idx);
		vsf_err_t (*read_OUT_buffer)(uint8_t idx, uint8_t *buffer, uint16_t size);
		vsf_err_t (*enable_OUT)(uint8_t idx);
	} ep;
	struct vsfhal_usbd_callback_t *callback;
};
vsf_err_t vsfhal_usbd_init(int32_t int_priority);
vsf_err_t vsfhal_usbd_fini(void);
vsf_err_t vsfhal_usbd_reset(void);
vsf_err_t vsfhal_usbd_connect(void);
vsf_err_t vsfhal_usbd_disconnect(void);
vsf_err_t vsfhal_usbd_set_address(uint8_t addr);
uint8_t vsfhal_usbd_get_address(void);
vsf_err_t vsfhal_usbd_suspend(void);
vsf_err_t vsfhal_usbd_resume(void);
vsf_err_t vsfhal_usbd_lowpower(uint8_t level);
uint32_t vsfhal_usbd_get_frame_number(void);
vsf_err_t vsfhal_usbd_get_setup(uint8_t *buffer);
vsf_err_t vsfhal_usbd_prepare_buffer(void);
vsf_err_t vsfhal_usbd_ep_reset(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_type(uint8_t idx, enum interface_usbd_eptype_t type);
vsf_err_t vsfhal_usbd_ep_set_IN_dbuffer(uint8_t idx);
bool vsfhal_usbd_ep_is_IN_dbuffer(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_switch_IN_buffer(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_IN_epsize(uint8_t idx, uint16_t size);
uint16_t vsfhal_usbd_ep_get_IN_epsize(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_IN_stall(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_clear_IN_stall(uint8_t idx);
bool vsfhal_usbd_ep_is_IN_stall(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_reset_IN_toggle(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_toggle_IN_toggle(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_IN_count(uint8_t idx, uint16_t size);
vsf_err_t vsfhal_usbd_ep_write_IN_buffer(uint8_t idx, uint8_t *buffer, uint16_t size);
vsf_err_t vsfhal_usbd_ep_set_OUT_dbuffer(uint8_t idx);
bool vsfhal_usbd_ep_is_OUT_dbuffer(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_switch_OUT_buffer(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_OUT_epsize(uint8_t idx, uint16_t size);
uint16_t vsfhal_usbd_ep_get_OUT_epsize(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_set_OUT_stall(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_clear_OUT_stall(uint8_t idx);
bool vsfhal_usbd_ep_is_OUT_stall(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_reset_OUT_toggle(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_toggle_OUT_toggle(uint8_t idx);
uint16_t vsfhal_usbd_ep_get_OUT_count(uint8_t idx);
vsf_err_t vsfhal_usbd_ep_read_OUT_buffer(uint8_t idx, uint8_t *buffer, uint16_t size);
vsf_err_t vsfhal_usbd_ep_enable_OUT(uint8_t idx);
#if VSFHAL_USBD_EN
extern const uint8_t vsfhal_usbd_ep_num;
extern struct vsfhal_usbd_callback_t vsfhal_usbd_callback;
extern const struct vsfhal_usbd_t vsfhal_usbd;
#endif

#endif	// __INTERFACES_H_INCLUDED__


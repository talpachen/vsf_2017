/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       interfaces.c                                              *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    interfaces implementation file                            *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2010-12-05:     created(by SimonQian)                             *
 **************************************************************************/
#include "interfaces.h"

#if VSFHAL_USBD_EN
const struct vsfhal_usbd_t vsfhal_usbd =
{
	vsfhal_usbd_init,
	vsfhal_usbd_fini,
	vsfhal_usbd_reset,
	vsfhal_usbd_connect,
	vsfhal_usbd_disconnect,
	vsfhal_usbd_set_address,
	vsfhal_usbd_get_address,
	vsfhal_usbd_suspend,
	vsfhal_usbd_resume,
	vsfhal_usbd_lowpower,
	vsfhal_usbd_get_frame_number,
	vsfhal_usbd_get_setup,
	vsfhal_usbd_prepare_buffer,

	&vsfhal_usbd_ep_num,
	vsfhal_usbd_ep_reset,
	vsfhal_usbd_ep_set_type,
	vsfhal_usbd_ep_set_IN_dbuffer,
	vsfhal_usbd_ep_is_IN_dbuffer,
	vsfhal_usbd_ep_switch_IN_buffer,
	vsfhal_usbd_ep_set_IN_epsize,
	vsfhal_usbd_ep_get_IN_epsize,
	vsfhal_usbd_ep_set_IN_stall,
	vsfhal_usbd_ep_clear_IN_stall,
	vsfhal_usbd_ep_is_IN_stall,
	vsfhal_usbd_ep_reset_IN_toggle,
	vsfhal_usbd_ep_toggle_IN_toggle,
	vsfhal_usbd_ep_set_IN_count,
	vsfhal_usbd_ep_write_IN_buffer,
	vsfhal_usbd_ep_set_OUT_dbuffer,
	vsfhal_usbd_ep_is_OUT_dbuffer,
	vsfhal_usbd_ep_switch_OUT_buffer,
	vsfhal_usbd_ep_set_OUT_epsize,
	vsfhal_usbd_ep_get_OUT_epsize,
	vsfhal_usbd_ep_set_OUT_stall,
	vsfhal_usbd_ep_clear_OUT_stall,
	vsfhal_usbd_ep_is_OUT_stall,
	vsfhal_usbd_ep_reset_OUT_toggle,
	vsfhal_usbd_ep_toggle_OUT_toggle,
	vsfhal_usbd_ep_get_OUT_count,
	vsfhal_usbd_ep_read_OUT_buffer,
	vsfhal_usbd_ep_enable_OUT,
	&vsfhal_usbd_callback,
};
#endif


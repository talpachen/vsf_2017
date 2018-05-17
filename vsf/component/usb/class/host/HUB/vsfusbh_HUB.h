/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __VSFUSBH_HUB_H_INCLUDED__
#define __VSFUSBH_HUB_H_INCLUDED__

#ifndef VSFCFG_EXCLUDE_USBH_HUB
extern const struct vsfusbh_class_drv_t vsfusbh_hub_drv;

bool vsfusbh_hub_dev_is_reset(struct vsfusbh_device_t *dev);
vsf_err_t vsfusbh_hub_reset_dev(struct vsfusbh_device_t *dev);
vsf_err_t vsfusbh_hub_reconnect_dev(struct vsfusbh_device_t *dev);
#endif

#endif

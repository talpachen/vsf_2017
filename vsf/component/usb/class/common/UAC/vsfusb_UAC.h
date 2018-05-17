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

#ifndef __VSFUSB_UAC_H_INCLUDED__
#define __VSFUSB_UAC_H_INCLUDED__

enum UAC_control_t
{
	UAC_CTRL_MUTE			= 1,
	UAC_CTRL_VOLUME,
};

enum UAC_channel_t
{
	UAC_CHANNEL_LFRONT		= 1,
	UAC_CHANNEL_RFRONT,
};

enum UAC_req_t
{
	UAC_REQ_SET_CUR			= 0x01,
	UAC_REQ_GET_CUR			= 0x81,
	UAC_REQ_SET_MIN			= 0x02,
	UAC_REQ_GET_MIN			= 0x82,
	UAC_REQ_SET_MAX			= 0x03,
	UAC_REQ_GET_MAX			= 0x83,
	UAC_REQ_SET_RES			= 0x04,
	UAC_REQ_GET_RES			= 0x84,
};

#endif	// __VSFUSB_UAC_H_INCLUDED__

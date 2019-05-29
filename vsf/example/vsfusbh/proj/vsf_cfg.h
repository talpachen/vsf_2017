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

// define the address of the api table
#define VSFCFG_API_ADDR				(SYS_MAIN_ADDR + 0x200)

#define VSFCFG_MAX_SRT_PRIO			0xFF

//#define VSFCFG_DEBUG
//#define VSFCFG_DEBUG_BUFLEN			512
//#define VSFCFG_BUFMGR_LOG

#define VSFCFG_BUFFER
#define VSFCFG_LIST
#define VSFCFG_STREAM
//#define VSFCFG_MAL
//#define VSFCFG_SCSI
//#define VSFCFG_FILE

// include VSFCFG_STANDALONE_MODULE to compile the project as a module
//#define VSFCFG_STANDALONE_MODULE

// include VSFCFG_MODULE to enable module support
//#define VSFCFG_MODULE

// define VSFCFG_FUNC_XXXX to include corresponding func
#define VSFCFG_FUNC_USBD
#define VSFUSBD_CFG_HIGHSPEED
#define VSFUSBD_CFG_EPMAXNO				8
//#define VSFCFG_FUNC_USBH
#define VSFCFG_FUNC_SHELL
//#define VSFCFG_FUNC_TCPIP
//#define VSFCFG_FUNC_BCMWIFI


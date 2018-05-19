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
#ifndef CPU_FREQ_HZ
#	define CPU_FREQ_HZ						(200 * 1000 * 1000)
#endif
#ifndef CORE_VECTOR_TABLE
#	define CORE_VECTOR_TABLE				(FLASH_LOAD_OFFSET)
#endif

#define VSFHAL_USB_EN							1

#if VSFHAL_USB_EN
#	define VSFHAL_USB_NUM						1
//#	define VSFHAL_USB_FS_INDEX		0
#	define VSFHAL_USB_HS_INDEX			0
#endif
#if defined(VSFHAL_USB_HS_INDEX)
#	define VSFHAL_USB_HS_PRIORITY			0xfe
#endif


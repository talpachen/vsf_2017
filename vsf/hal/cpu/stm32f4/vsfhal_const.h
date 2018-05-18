/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       interfaces_const.h                                        *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    consts of interface module                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2011-04-08:     created(by SimonQian)                             *
 **************************************************************************/
#ifndef __VSFHAL_CONST_H_INCLUDED__
#define __VSFHAL_CONST_H_INCLUDED__

#include "stm32f4xx.h"

// common
#define VSFHAL_DUMMY_PORT					0xFF

// core
#define VSFHAL_SLEEP_WFI					(0x1ul << 0)
#define VSFHAL_SLEEP_PWRDOWN				(0x1ul << 1)

#endif	// __VSFHAL_CONST_H_INCLUDED__

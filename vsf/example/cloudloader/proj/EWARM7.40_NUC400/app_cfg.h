/**************************************************************************
 *  Copyright (C) 2008 - 2012 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    VSF                                                       *
 *  File:       app_cfg.h                                                 *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    configuration file                                        *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/

// hardware config file
#include "hw_cfg_nuc400.h"

// compiler config
#include "compiler.h"


#define APPCFG_VSFTIMER_NUM				64
#define APPCFG_VSFSM_PENDSVQ_LEN		0
#define APPCFG_VSFSM_MAINQ_LEN			64
#define APPCFG_VSFSM_MAINQ_POLLING

#define APPCFG_BUFMGR_SIZE				(8 * 1024)



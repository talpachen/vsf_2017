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

#ifndef __VSF_H_INCLUDED__
#define __VSF_H_INCLUDED__

#include "compiler.h"
#include "vsf_cfg.h"
#include "vsfhal.h"

#include "app_cfg.h"

// framework
#include "framework/vsfsm/vsfsm.h"
#include "framework/vsftimer/vsftimer.h"

#include "component/fundation/bittool/bittool.h"
#include "component/fundation/buffer/buffer.h"
#include "component/fundation/list/list.h"
#include "component/fundation/stream/stream.h"
#include "component/fundation/bittool/bittool.h"
#include "component/tool/dynarr/vsf_dynarr.h"
#include "component/tool/dynpool/vsf_dynpool.h"
#include "component/tool/dynstack/vsf_dynstack.h"

int vsfmain(void);

#endif		// __VSF_H_INCLUDED__

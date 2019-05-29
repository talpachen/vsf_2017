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
#ifndef __VSFHAL_CFG_H__
#define __VSFHAL_CFG_H__

#if defined(BOARD_TYPE_NUCLED_F411RE)
#include "vsfhal_cfg_nucleo_f411re.h"
#define SOC_TYPE_STM32F411
#elif defined(BOARD_TYPE_TEST_CMEM7)
#include "vsfhal_cfg_test_cmem7.h"
#define SOC_TYPE_CMEM7
#elif defined(BOARD_TYPE_STM32F769I_DISCO)
#include "vsfhal_cfg_stm32f769i_disco.h"
#define SOC_TYPE_STM32F769
#endif

//#define VSF_TASK_PRIORITY_L0				0		// main
#define VSF_TASK_PRIORITY_L1				0xff
#define VSF_TASK_PRIORITY_L2				(VSF_TASK_PRIORITY_L1 - 1)
#define VSF_TASK_PRIORITY_L3				(VSF_TASK_PRIORITY_L2 - 1)
#define VSF_TASK_PRIORITY_L4				(VSF_TASK_PRIORITY_L3 - 1)

#define VSF_HARDWARE_IRQ_PRIORITY			VSF_TASK_PRIORITY_L4

#define VSFHAL_TICKCLK_PRIORITY				(VSF_HARDWARE_IRQ_PRIORITY + 1)
#define VSFHAL_USBD_PRIORITY				VSF_HARDWARE_IRQ_PRIORITY
#define VSFHAL_HCD_PRIORITY					VSF_HARDWARE_IRQ_PRIORITY
#define VSFHAL_USART_PRIORITY				VSF_HARDWARE_IRQ_PRIORITY
#define VSFHAL_I2C_PRIORITY					VSF_HARDWARE_IRQ_PRIORITY
#define VSFHAL_SPI_PRIORITY					VSF_HARDWARE_IRQ_PRIORITY
#define VSFHAL_ADC_PRIORITY					VSF_HARDWARE_IRQ_PRIORITY

#endif // __VSFHAL_CFG_H__


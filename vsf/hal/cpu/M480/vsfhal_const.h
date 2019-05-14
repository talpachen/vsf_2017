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

#ifndef __VSFHAL_CONST_H_INCLUDED__
#define __VSFHAL_CONST_H_INCLUDED__

#include "M480.h"

// common
#define VSFHAL_DUMMY_PORT					0xFF

// core
#define VSFHAL_SLEEP_WFI					(0x1ul << 0)
#define VSFHAL_SLEEP_PWRDOWN				(0x1ul << 1)

// usart
#define VSFHAL_USART_STOPBITS_1				(0x0ul << 4)
#define VSFHAL_USART_STOPBITS_1P5			(0x1ul << 4)
#define VSFHAL_USART_STOPBITS_2				(0x1ul << 4)
#define VSFHAL_USART_PARITY_NONE			(0x0ul << 0)
#define VSFHAL_USART_PARITY_ODD				(0x2ul << 0)
#define VSFHAL_USART_PARITY_EVEN			(0x3ul << 0)

// GPIO
#define VSFHAL_GPIO_INPUT					(0x0ul << 0)
#define VSFHAL_GPIO_OUTPP					(0x1ul << 0)
#define VSFHAL_GPIO_OUTOD					(0x2ul << 0)
#define VSFHAL_GPIO_PULLUP					(0x0ul << 4)
#define VSFHAL_GPIO_PULLDOWN				(0x1ul << 4)

// I2C
#define VSFHAL_I2C_READ						(0x1ul << 0)
#define VSFHAL_I2C_WRITE					(0x0ul << 0)
#define VSFHAL_I2C_NOSTART					(0x1ul << 4)
#define VSFHAL_I2C_ACKLAST					(0x1ul << 5)

// SPI
#define VSFHAL_SPI_MASTER					0x00
#define VSFHAL_SPI_SLAVE					(SPI_CTL_SLAVE_Msk)
#define VSFHAL_SPI_MODE0					(SPI_CTL_TXNEG_Msk)
#define VSFHAL_SPI_MODE1					(SPI_CTL_RXNEG_Msk)
#define VSFHAL_SPI_MODE2					(SPI_CTL_CLKPOL_Msk | SPI_CTL_RXNEG_Msk)
#define VSFHAL_SPI_MODE3					(SPI_CTL_CLKPOL_Msk | SPI_CTL_TXNEG_Msk)
#define VSFHAL_SPI_MSB_FIRST				0x00
#define VSFHAL_SPI_LSB_FIRST				(SPI_CTL_LSB_Msk)

// OHCI
#define VSFHAL_HCD_PORT1					(0x1 << 0)
#define VSFHAL_HCD_PORT2					(0x1 << 1)

#endif	// __VSFHAL_CONST_H_INCLUDED__


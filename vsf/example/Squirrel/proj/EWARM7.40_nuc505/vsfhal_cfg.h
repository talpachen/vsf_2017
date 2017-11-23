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

#ifndef CORE_CLKEN
#	define CORE_CLKEN						(NUC505_CLK_HXT | NUC505_CLK_PLL)
#endif
#ifndef CORE_HCLKSRC
#	define CORE_HCLKSRC						NUC505_HCLKSRC_PLLFOUT
#endif
#ifndef OSC_FREQ_HZ
#	define OSC_FREQ_HZ						(12 * 1000 * 1000)
#endif
#ifndef OSC32_FREQ_HZ
#	define OSC32_FREQ_HZ					(32768)
#endif
#ifndef LIRC_FREQ_HZ
#	define LIRC_FREQ_HZ						(32768)
#endif
#ifndef CORE_PLL_FREQ_HZ
#	define CORE_PLL_FREQ_HZ					(384 * 1000 * 1000)
#endif
#ifndef CORE_APLL_FREQ_HZ
#	define CORE_APLL_FREQ_HZ				(0)
#endif
#ifndef CPU_FREQ_HZ
#	define CPU_FREQ_HZ						(96 * 1000 * 1000)
#endif
#ifndef HCLK_FREQ_HZ
#	define HCLK_FREQ_HZ						CPU_FREQ_HZ
#endif
#ifndef PCLK_FREQ_HZ
#	define PCLK_FREQ_HZ						(96 * 1000 * 1000)
#endif
#ifndef CORE_VECTOR_TABLE
#	define CORE_VECTOR_TABLE				FLASH_LOAD_OFFSET
#endif

#define VSFHAL_GPIO_EN						1
#define VSFHAL_USBD_EN						1
#define VSFHAL_HCD_EN						0
#define VSFHAL_USART_EN						0
#define VSFHAL_FLASH_EN						0
#define VSFHAL_I2C_EN						0
#define VSFHAL_SPI_EN						0
#define VSFHAL_TIMER_EN						0

// I2C config
#if VSFHAL_I2C_EN
#define VSFHAL_I2C_NUM						2
#define VSFHAL_I2C0_ENABLE					1
#define VSFHAL_I2C1_ENABLE					0
#define VSFHAL_I2C0_SCL_PA14_EN				0
#define VSFHAL_I2C0_SCL_PB0_EN				1
#define VSFHAL_I2C0_SCL_PD0_EN				0
#define VSFHAL_I2C0_SDA_PA15_EN				0
#define VSFHAL_I2C0_SDA_PB1_EN				1
#define VSFHAL_I2C0_SDA_PD1_EN				0
#define VSFHAL_I2C1_SCL_PA10_EN				0
#define VSFHAL_I2C1_SCL_PB10_EN				0
#define VSFHAL_I2C1_SCL_PB14_EN				0
#define VSFHAL_I2C1_SDA_PA11_EN				0
#define VSFHAL_I2C1_SDA_PB11_EN				0
#define VSFHAL_I2C1_SDA_PB15_EN				0
#endif


// SPI config
#if VSFHAL_SPI_EN
#define VSFHAL_SPI_NUM						1
#define VSFHAL_SPI0_ENABLE					1
#define VSFHAL_SPI1_ENABLE					0
#endif

#if VSFHAL_HCD_EN
#define VSFHAL_HCD_INDEX					0
#define VSFHAL_HCD_PORT1_MASK				(0x0010 << 0)
#define VSFHAL_HCD_PORT2_MASK				(0x0010 << 1)
#define VSFHAL_HCD_PORT1_DP_PB12_MASK		(0x0100 << 0)
#define VSFHAL_HCD_PORT1_DM_PB13_MASK		(0x0100 << 1)
#define VSFHAL_HCD_PORT1_DP_PB14_MASK		(0x0100 << 2)
#define VSFHAL_HCD_PORT1_DM_PB15_MASK		(0x0100 << 3)
#endif

#if VSFHAL_USART_EN
#define VSFHAL_USART_NUM					3
#define VSFHAL_USART0_ENABLE				0
#define VSFHAL_USART1_ENABLE				1
#define VSFHAL_USART2_ENABLE				0
#define VSFHAL_USART1_TXD_PA8_EN			1
#define VSFHAL_USART1_TXD_PB6_EN			0
#define VSFHAL_USART1_RXD_PA9_EN			1
#define VSFHAL_USART1_RXD_PB7_EN			0
#define VSFHAL_USART1_CTS_PB8_EN			0
#define VSFHAL_USART1_RTS_PB9_EN			0
#define VSFHAL_USART2_CTS_PB12_EN			0
#define VSFHAL_USART2_RTS_PB13_EN			0
#endif

#define USBD_PULLUP_PORT							VSFHAL_DUMMY_PORT
#define USBD_PULLUP_PIN								0
	 
#endif // __VSFHAL_CFG_H__


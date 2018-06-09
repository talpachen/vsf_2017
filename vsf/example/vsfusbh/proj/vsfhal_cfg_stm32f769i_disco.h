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

#ifndef CORE_CLKEN
#	define CORE_CLKEN						(STM32F7_CLK_HSE | STM32F7_CLK_PLL)
#endif
#ifndef CORE_HCLKSRC
#	define CORE_HCLKSRC						STM32F7_HCLKSRC_PLL
#endif
#ifndef CORE_PLLSRC
#	define CORE_PLLSRC						STM32F7_PLLSRC_HSE
#endif
#ifndef HSI_FREQ_HZ
#	define HSI_FREQ_HZ						(16 * 1000 * 1000)
#endif
#ifndef HSE_FREQ_HZ
#	define HSE_FREQ_HZ						(25 * 1000 * 1000)
#endif
#ifndef CORE_PLL_FREQ_HZ
#	define CORE_PLL_FREQ_HZ					(192 * 1000 * 1000)
#endif
#ifndef CORE_HCLK_FREQ_HZ
#	define CORE_HCLK_FREQ_HZ				(192 * 1000 * 1000)
#endif
#ifndef CORE_PCLK1_FREQ_HZ
#	define CORE_PCLK1_FREQ_HZ				(48 * 1000 * 1000)
#endif
#ifndef CORE_PCLK2_FREQ_HZ
#	define CORE_PCLK2_FREQ_HZ				(48 * 1000 * 1000)
#endif
#ifndef CORE_VECTOR_TABLE
#	define CORE_VECTOR_TABLE				(FLASH_LOAD_OFFSET)
#endif

#define VSFHAL_USB_EN						1

#if VSFHAL_USB_EN
#define VSFHAL_USB_NUM						2
#define VSFHAL_USB_FS_INDEX					0
#define VSFHAL_USB_HS_INDEX					1
#define VSFHAL_HCD_ISO_SUPPORT				1
#define VSFHAL_HCD_ISO_PACKET_LIMIT			4
#endif
#if defined(VSFHAL_USB_FS_INDEX)
#define VSFHAL_USB_FS_PRIORITY				0xfe
#endif
#if defined(VSFHAL_USB_HS_INDEX)
#define VSFHAL_USB_HS_PRIORITY				0xfe

#define VSFHAL_USBHS_ULPI_CK_GPIO			GPIOA
#define VSFHAL_USBHS_ULPI_CK_IO				5
#define VSFHAL_USBHS_ULPI_D0_GPIO			GPIOA
#define VSFHAL_USBHS_ULPI_D0_IO				3
#define VSFHAL_USBHS_ULPI_D1_GPIO			GPIOB
#define VSFHAL_USBHS_ULPI_D1_IO				0
#define VSFHAL_USBHS_ULPI_D2_GPIO			GPIOB
#define VSFHAL_USBHS_ULPI_D2_IO				1
#define VSFHAL_USBHS_ULPI_D3_GPIO			GPIOB
#define VSFHAL_USBHS_ULPI_D3_IO				10
#define VSFHAL_USBHS_ULPI_D4_GPIO			GPIOB
#define VSFHAL_USBHS_ULPI_D4_IO				11
#define VSFHAL_USBHS_ULPI_D5_GPIO			GPIOB
#define VSFHAL_USBHS_ULPI_D5_IO				12
#define VSFHAL_USBHS_ULPI_D6_GPIO			GPIOB
#define VSFHAL_USBHS_ULPI_D6_IO				13
#define VSFHAL_USBHS_ULPI_D7_GPIO			GPIOB
#define VSFHAL_USBHS_ULPI_D7_IO				5
#define VSFHAL_USBHS_ULPI_STP_GPIO			GPIOC
#define VSFHAL_USBHS_ULPI_STP_IO			0
#define VSFHAL_USBHS_ULPI_NXT_GPIO			GPIOH
#define VSFHAL_USBHS_ULPI_NXT_IO			4
#define VSFHAL_USBHS_ULPI_DIR_GPIO			GPIOI
#define VSFHAL_USBHS_ULPI_DIR_IO			11
#endif

#define STM32F769xx
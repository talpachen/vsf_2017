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
#ifndef __INTERFACES_CFG_H__
#define __INTERFACES_CFG_H__

#ifndef CORE_CLKEN
#	define CORE_CLKEN						NUC400_CLK_HXT
#endif
#ifndef CORE_HCLKSRC
#	define CORE_HCLKSRC						NUC400_HCLKSRC_PLLFOUT
#endif
#ifndef CORE_PCLKSRC
#	define CORE_PCLKSRC						NUC400_PCLKSRC_HCLK
#endif
#ifndef CORE_PLLSRC
#	define CORE_PLLSRC						NUC400_PLLSRC_HXT
#endif
#ifndef OSC0_FREQ_HZ
#	define OSC0_FREQ_HZ						(12 * 1000 * 1000)
#endif
#ifndef OSC32_FREQ_HZ
#	define OSC32_FREQ_HZ					0
#endif
#ifndef CORE_PLL_FREQ_HZ
#	define CORE_PLL_FREQ_HZ					(144 * 1000 * 1000)
#endif
#ifndef CPU_FREQ_HZ
#	define CPU_FREQ_HZ						(72 * 1000 * 1000)
#endif
#ifndef HCLK_FREQ_HZ
#	define HCLK_FREQ_HZ						(72 * 1000 * 1000)
#endif
#ifndef PCLK_FREQ_HZ
#	define PCLK_FREQ_HZ						(72 * 1000 * 1000)
#endif
#ifndef CORE_VECTOR_TABLE
#	define CORE_VECTOR_TABLE				FLASH_LOAD_OFFSET
#endif


// USART config
#define USART_NUM							5

#define IFS_FLASH_EN						1
#define IFS_USART_EN						1
#define IFS_SPI_EN							0
#define IFS_ADC_EN							0
#define IFS_GPIO_EN							1
#define IFS_I2C_EN							0
#define IFS_PWM_EN							0
#define IFS_MICROWIRE_EN					0
#define IFS_TIMER_EN						0
#define IFS_EINT_EN							0
#define IFS_EBI_EN							0
#define IFS_SDIO_EN							0
#define IFS_USBD_EN							1
#define IFS_USBIO_EN						0
#define IFS_HCD_EN							0

#define USART0_0_ENABLE						1
#define USART0_INT_EN						1
#define USART1_0_ENABLE						1
#define USART1_INT_EN						1
#define USART2_0_ENABLE						0
#define USART2_INT_EN						0
#define USART3_0_ENABLE						1
#define USART3_INT_EN						1
#define USART4_0_ENABLE						1
#define USART4_INT_EN						1

#endif // __INTERFACES_CFG_H__


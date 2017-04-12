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
#ifndef __INTERFACE_CONST_H__
#define __INTERFACE_CONST_H__

#include "NUC472_442.h"

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
#define VSFHAL_GPIO_OUTPP					(0x0ul << 0)
#define VSFHAL_GPIO_OUTOD					(0x0ul << 0)
#define VSFHAL_GPIO_PULLUP					(0x1ul << 0)
#define VSFHAL_GPIO_PULLDOWN				(0x1ul << 0)

#if 0
#define vsfhal_USART_MODE0			
#define vsfhal_USART_MODE1			
#define vsfhal_USART_MODE2			
#define vsfhal_USART_MODE3			
#define vsfhal_USART_CLKEN			
#define vsfhal_USART_STOPBITS_0P5	
#define vsfhal_USART_STOPBITS_1		
#define vsfhal_USART_STOPBITS_1P5	
#define vsfhal_USART_STOPBITS_2		
#define vsfhal_USART_PARITY_NONE	0x0
#define vsfhal_USART_PARITY_ODD		0x2
#define vsfhal_USART_PARITY_EVEN	0x3

#define vsfhal_SPI_MASTER			
#define vsfhal_SPI_SLAVE			
#define vsfhal_SPI_MODE0			
#define vsfhal_SPI_MODE1			
#define vsfhal_SPI_MODE2			
#define vsfhal_SPI_MODE3			
#define vsfhal_SPI_MSB_FIRST		
#define vsfhal_SPI_LSB_FIRST		

#define vsfhal_ADC_ALIGNLEFT		
#define vsfhal_ADC_ALIGNRIGHT		

#define vsfhal_GPIO_INFLOAT			0x00
#define vsfhal_GPIO_INPU			0x00
#define vsfhal_GPIO_INPD			0x00
#define vsfhal_GPIO_OUTPP			0x01
#define vsfhal_GPIO_OUTOD			0x02

#define vsfhal_EINT_ONFALL			
#define vsfhal_EINT_ONRISE			
#define vsfhal_EINT_INT				
#define vsfhal_EINT_EVT				

#define vsfhal_SDIO_RESP_NONE		
#define vsfhal_SDIO_RESP_SHORT		
#define vsfhal_SDIO_RESP_LONG		
#endif
#endif	// __INTERFACE_CONST_H__

/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       GPIO.h                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    GPIO interface header file                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/
#include "vsf.h"

#if VSFHAL_GPIO_EN

#include "NUC505_GPIO.h"

#define NUC505_GPIO_NUM					4

typedef struct
{
    __IO uint32_t MODE;
    __IO uint32_t PUEN;
    __IO uint32_t DOUT;
    __I  uint32_t PIN;
} GPIO_COMMON_T;

vsf_err_t vsfhal_gpio_init(uint8_t index)
{
	if (index == VSFHAL_DUMMY_PORT)
		return VSFERR_NOT_SUPPORT;
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_fini(uint8_t index)
{
	if (index == VSFHAL_DUMMY_PORT)
		return VSFERR_NOT_SUPPORT;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_config(uint8_t index, uint8_t pin_idx, uint32_t mode)
{
	GPIO_COMMON_T *gpio;

	if (index == VSFHAL_DUMMY_PORT)
		return VSFERR_NOT_SUPPORT;
	
	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	
	if (mode & (VSFHAL_GPIO_OUTPP | VSFHAL_GPIO_OUTOD))
		gpio->MODE |= 0x1ul << pin_idx;
	else
		gpio->MODE &= ~(0x1ul << pin_idx);

	gpio->PUEN &= ~(0x3ul << (pin_idx * 2));
	if (mode & VSFHAL_GPIO_PULLUP)
		gpio->PUEN |= 0x1ul << (pin_idx * 2);
	if (mode & VSFHAL_GPIO_PULLDOWN)
		gpio->PUEN |= 0x1ul << (pin_idx * 2 + 1);
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_set(uint8_t index, uint32_t pin_mask)
{
	GPIO_COMMON_T *gpio;
	
	if (index == VSFHAL_DUMMY_PORT)
		return VSFERR_NOT_SUPPORT;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	gpio->DOUT |= pin_mask;

	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_clear(uint8_t index, uint32_t pin_mask)
{
	GPIO_COMMON_T *gpio;
	
	if (index == VSFHAL_DUMMY_PORT)
		return VSFERR_NOT_SUPPORT;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	gpio->DOUT &= ~pin_mask;

	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_out(uint8_t index, uint32_t pin_mask, uint32_t value)
{
	GPIO_COMMON_T *gpio;
	
	if (index == VSFHAL_DUMMY_PORT)
		return VSFERR_NOT_SUPPORT;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	gpio->DOUT |= pin_mask & value;
	gpio->DOUT &= ~(pin_mask & ~value);

	return VSFERR_NONE;
}

uint32_t vsfhal_gpio_get(uint8_t index, uint32_t pin_mask)
{
	GPIO_COMMON_T *gpio;
	
	if (index == VSFHAL_DUMMY_PORT)
		return VSFERR_NOT_SUPPORT;

	gpio = (GPIO_COMMON_T *)(GPIOA_BASE + ((uint32_t)index << 4));
	return gpio->PIN & pin_mask;
}

#endif


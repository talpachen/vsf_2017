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

#define VSFHAL_GPIO_NUM					6

vsf_err_t vsfhal_gpio_init(uint8_t index)
{
	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	RCU_APB2EN |= RCU_APB2EN_PAEN << index;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_fini(uint8_t index)
{
	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	RCU_APB2RST |= RCU_APB2RST_PARST << index;
	RCU_APB2EN &= ~(RCU_APB2EN_PAEN << index);
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_config(uint8_t index, uint8_t pin_idx, uint32_t mode)
{
	uint32_t gpiox;
	uint8_t bit4_pos;
	
	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;

	gpiox = GPIOA + 0x400 * index;

	if (mode == VSFHAL_GPIO_INPUT)
		mode = 0x04;
	else if (mode == (VSFHAL_GPIO_INPUT | VSFHAL_GPIO_ANALOG))
		mode = 0x00;

	if (pin_idx >= 8)
	{
		bit4_pos = (pin_idx & 0x7) * 4;
		GPIO_CTL1(gpiox) = (GPIO_CTL1(gpiox) & ~(0xf << bit4_pos)) | ((mode & 0xf) << bit4_pos);
	}
	else
	{
		bit4_pos = pin_idx * 4;
		GPIO_CTL0(gpiox) = (GPIO_CTL0(gpiox) & ~(0xf << bit4_pos)) | ((mode & 0xf) << bit4_pos);
	}
	
	if (mode == (VSFHAL_GPIO_INPUT | VSFHAL_GPIO_PULLUP))
		GPIO_OCTL(gpiox) |= 0x1ul << pin_idx;
	else if (mode == (VSFHAL_GPIO_INPUT | VSFHAL_GPIO_PULLDOWN))
		GPIO_OCTL(gpiox) &= ~(0x1ul << pin_idx);

	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_set(uint8_t index, uint32_t pin_mask)
{
	uint32_t gpiox;

	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	gpiox = GPIOA + 0x400 * index;
	GPIO_BOP(gpiox) = pin_mask & 0xffff;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_clear(uint8_t index, uint32_t pin_mask)
{
	uint32_t gpiox;

	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	gpiox = GPIOA + 0x400 * index;
	GPIO_BOP(gpiox) = pin_mask << 16;
	return VSFERR_NONE;
}

uint32_t vsfhal_gpio_get(uint8_t index, uint32_t pin_mask)
{
	uint32_t gpiox;

	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	gpiox = GPIOA + 0x400 * index;
	return GPIO_ISTAT(gpiox) & pin_mask;
}


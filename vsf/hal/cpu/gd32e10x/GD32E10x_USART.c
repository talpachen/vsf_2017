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

#include "vsf.h"
#include "core.h"

#if VSFHAL_USART_EN

// usart + dma + HTFIE + FTFIE + 48byte buffer

#define DMA_BUFF_SIZE			48

static uint8_t tx_dma_buff[VSFHAL_USART_NUM][DMA_BUFF_SIZE];

static uint8_t rx_dma_buf_sel[VSFHAL_USART_NUM];
static uint8_t rx_dma_buff_pos[VSFHAL_USART_NUM][2];
static uint8_t rx_dma_buff[VSFHAL_USART_NUM][2][DMA_BUFF_SIZE];

static void (*vsfhal_usart_ontx[VSFHAL_USART_NUM])(void *);
static void (*vsfhal_usart_onrx[VSFHAL_USART_NUM])(void *);
static void *vsfhal_usart_callback_param[VSFHAL_USART_NUM];

vsf_err_t vsfhal_usart_init(vsfhal_usart_t index)
{
	if (index >= VSFHAL_USART_NUM)
		return VSFERR_BUG;

	switch (index)
	{
#if VSFHAL_USART0_ENABLE
	case 0:
#if VSFHAL_USART0_TXD_PA9_EN | VSFHAL_USART0_RXD_PA10_EN | VSFHAL_USART0_CTS_PA11_EN | VSFHAL_USART0_RTS_PA12_EN
		vsfhal_gpio_init(0);
#endif
#if VSFHAL_USART0_TXD_PA9_EN
		vsfhal_gpio_config(0, 9, VSFHAL_GPIO_ALTERNATE_OUTPP);
#endif
#if VSFHAL_USART0_RXD_PA10_EN
		vsfhal_gpio_config(0, 10, VSFHAL_GPIO_ALTERNATE_INPUT | VSFHAL_GPIO_PULLUP);
#endif
#if VSFHAL_USART0_CTS_PA11_EN
		vsfhal_gpio_config(0, 11, VSFHAL_GPIO_ALTERNATE_OUTPP);
#endif
#if VSFHAL_USART0_RTS_PA12_EN
		vsfhal_gpio_config(0, 12, VSFHAL_GPIO_ALTERNATE_INPUT | VSFHAL_GPIO_PULLUP);
#endif
		RCU_AHBEN |= RCU_AHBEN_DMA0EN;
		RCU_APB2EN |= RCU_APB2EN_USART0EN;
		break;
#endif
#if VSFHAL_USART1_ENABLE
	case 1:
#if VSFHAL_USART1_TXD_PA2_EN | VSFHAL_USART1_RXD_PA3_EN | VSFHAL_USART1_CTS_PA0_EN | VSFHAL_USART1_RTS_PA1_EN
		vsfhal_gpio_init(0);
#endif
#if VSFHAL_USART1_TXD_PA2_EN
		vsfhal_gpio_config(0, 2, VSFHAL_GPIO_ALTERNATE_OUTPP);
#endif
#if VSFHAL_USART1_RXD_PA3_EN
		vsfhal_gpio_config(0, 3, VSFHAL_GPIO_ALTERNATE_INPUT | VSFHAL_GPIO_PULLUP);
#endif
#if VSFHAL_USART1_CTS_PA0_EN
		vsfhal_gpio_config(0, 0, VSFHAL_GPIO_ALTERNATE_OUTPP);
#endif
#if VSFHAL_USART1_RTS_PA1_EN
		vsfhal_gpio_config(0, 1, VSFHAL_GPIO_ALTERNATE_INPUT | VSFHAL_GPIO_PULLUP);
#endif
		RCU_AHBEN |= RCU_AHBEN_DMA0EN;
		RCU_APB1EN |= RCU_APB1EN_USART1EN;
		break;
#endif
#if VSFHAL_USART2_ENABLE
	case 2:
#if VSFHAL_USART2_TXD_PB10_EN | VSFHAL_USART2_RXD_PB11_EN | VSFHAL_USART2_CTS_PB13_EN | VSFHAL_USART2_RTS_PB14_EN
		vsfhal_gpio_init(1);
#endif
#if VSFHAL_USART2_TXD_PB10_EN
		vsfhal_gpio_config(1, 10, VSFHAL_GPIO_ALTERNATE_OUTPP);
#endif
#if VSFHAL_USART2_RXD_PB11_EN
		vsfhal_gpio_config(1, 11, VSFHAL_GPIO_ALTERNATE_INPUT | VSFHAL_GPIO_PULLUP);
#endif
#if VSFHAL_USART2_CTS_PB13_EN
		vsfhal_gpio_config(1, 13, VSFHAL_GPIO_ALTERNATE_OUTPP);
#endif
#if VSFHAL_USART2_RTS_PB14_EN
		vsfhal_gpio_config(1, 14, VSFHAL_GPIO_ALTERNATE_INPUT | VSFHAL_GPIO_PULLUP);
#endif
		RCU_AHBEN |= RCU_AHBEN_DMA0EN;
		RCU_APB1EN |= RCU_APB1EN_USART2EN;
		break;
#endif
	}
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_usart_fini(vsfhal_usart_t index)
{
	if (index >= VSFHAL_USART_NUM)
		return VSFERR_BUG;

	switch (index)
	{
#if VSFHAL_USART0_ENABLE
	case 0:
		RCU_APB2EN &= ~RCU_APB2EN_USART0EN;
		break;
#endif
#if VSFHAL_USART1_ENABLE
	case 1:
		RCU_APB1EN &= ~RCU_APB1EN_USART1EN;
		break;
#endif
#if VSFHAL_USART2_ENABLE
	case 2:
		RCU_APB1EN &= ~RCU_APB1EN_USART2EN;
		break;
#endif
	}
	
	return VSFERR_NONE;
}

static void usart_dma_config(vsfhal_usart_t index, uint32_t usartx, uint32_t clk,
		uint32_t baudrate, uint32_t mode, uint32_t dmax, uint8_t tx_ch, uint8_t rx_ch)
{
	DMA_CHCTL(dmax, tx_ch) = 0;
	DMA_CHCTL(dmax, rx_ch) = 0;
	USART_CTL0(usartx) = 0;
	USART_CTL0(usartx) = (mode & 0x1600) | USART_CTL0_TEN | USART_CTL0_REN;
	USART_CTL1(usartx) = (mode >> 16) & 0x3000;
	USART_CTL2(usartx) = ((mode << 8) & 0x0300) | USART_CTL2_DENR | USART_CTL2_DENT;
	USART_CTL3(usartx) = USART_CTL3_RTIE | USART_CTL3_RTEN;
	USART_RT(usartx) = 30;
	
	// dma tx
	DMA_CHCTL(dmax, tx_ch) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_DIR | DMA_CHXCTL_FTFIE;
	DMA_CHPADDR(dmax, tx_ch) = (uint32_t)(usartx + 0x28U);

	// dma rx
	rx_dma_buf_sel[index] = 0;
	DMA_CHPADDR(dmax, rx_ch) = (uint32_t)(usartx + 0x24U);
	DMA_CHMADDR(dmax, rx_ch) = (uint32_t)rx_dma_buff[index][0];
	DMA_CHCNT(dmax, rx_ch) = DMA_BUFF_SIZE;
	DMA_CHCTL(dmax, rx_ch) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_FTFIE | DMA_CHXCTL_CHEN;
	
	USART_BAUD(usartx) = (clk / baudrate) << 1;
	USART_CTL0(usartx) |= USART_CTL0_UEN;
}

vsf_err_t vsfhal_usart_config(vsfhal_usart_t index, uint32_t baudrate, uint32_t mode)
{
	struct vsfhal_info_t *info;

	if (index >= VSFHAL_USART_NUM)
		return VSFERR_BUG;

	if (vsfhal_core_get_info(&info) || (NULL == info))
		return VSFERR_BUG;
	
	switch (index)
	{
#if VSFHAL_USART0_ENABLE
	case 0:
		usart_dma_config(0, USART0, info->apb2_freq_hz, baudrate, mode, DMA0, 3, 4);
		break;
#endif
#if VSFHAL_USART1_ENABLE
	case 1:
		usart_dma_config(1, USART1, info->apb1_freq_hz, baudrate, mode, DMA0, 5, 6);
		break;
#endif
#if VSFHAL_USART2_ENABLE
	case 2:
		usart_dma_config(2, USART2, info->apb1_freq_hz, baudrate, mode, DMA0, 1, 2);
		break;
#endif
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usart_config_cb(vsfhal_usart_t index, int32_t int_priority, void *p,
		void (*ontx)(void *), void (*onrx)(void *))
{
	if (index >= VSFHAL_USART_NUM)
		return VSFERR_BUG;

	vsfhal_usart_ontx[index] = ontx;
	vsfhal_usart_onrx[index] = onrx;
	vsfhal_usart_callback_param[index] = p;

	if (int_priority >= 0)
	{
		switch (index)
		{
#if VSFHAL_USART0_ENABLE
		case 0:
			NVIC_EnableIRQ(USART0_IRQn);
			NVIC_SetPriority(USART0_IRQn, int_priority);
			NVIC_EnableIRQ(DMA0_Channel3_IRQn);
			NVIC_SetPriority(DMA0_Channel3_IRQn, int_priority);
			NVIC_EnableIRQ(DMA0_Channel4_IRQn);
			NVIC_SetPriority(DMA0_Channel4_IRQn, int_priority);
			break;
#endif
#if VSFHAL_USART1_ENABLE
		case 1:
			NVIC_EnableIRQ(USART1_IRQn);
			NVIC_SetPriority(USART1_IRQn, int_priority);
			NVIC_EnableIRQ(DMA0_Channel5_IRQn);
			NVIC_SetPriority(DMA0_Channel5_IRQn, int_priority);
			NVIC_EnableIRQ(DMA0_Channel6_IRQn);
			NVIC_SetPriority(DMA0_Channel6_IRQn, int_priority);
			break;
#endif
#if VSFHAL_USART2_ENABLE
		case 2:
			NVIC_EnableIRQ(USART2_IRQn);
			NVIC_SetPriority(USART2_IRQn, int_priority);
			NVIC_EnableIRQ(DMA0_Channel1_IRQn);
			NVIC_SetPriority(DMA0_Channel1_IRQn, int_priority);
			NVIC_EnableIRQ(DMA0_Channel2_IRQn);
			NVIC_SetPriority(DMA0_Channel2_IRQn, int_priority);
			break;
#endif
		}
	}
	return VSFERR_NONE;
}

static uint16_t usart_dma_tx_bytes(vsfhal_usart_t index, uint32_t dmax, uint8_t ch,
		uint8_t *data, uint16_t size)
{
	uint8_t buf[DMA_BUFF_SIZE], last;
	
	DMA_CHCTL(dmax, ch) &= ~DMA_CHXCTL_CHEN;
	last = DMA_CHCNT(dmax, ch);
	if (last)
		memcpy(buf, (void *)DMA_CHMADDR(dmax, ch), last);
	size = min(sizeof(buf) - last, size);
	if (size)
		memcpy(buf + last, data, size);
	memcpy(tx_dma_buff[index], buf, size + last);
	DMA_CHMADDR(dmax, ch) = (uint32_t)tx_dma_buff[index];
	DMA_CHCNT(dmax, ch) = size + last;
	DMA_CHCTL(dmax, ch) |= DMA_CHXCTL_CHEN;
	return size;
}

uint16_t vsfhal_usart_tx_bytes(vsfhal_usart_t index, uint8_t *data, uint16_t size)
{
	if (!size)
		return 0;

	switch (index)
	{
#if VSFHAL_USART0_ENABLE
	case 0:
		return usart_dma_tx_bytes(0, DMA0, 3, data, size);
#endif
#if VSFHAL_USART1_ENABLE
	case 1:
		return usart_dma_tx_bytes(1, DMA0, 5, data, size);
#endif
#if VSFHAL_USART2_ENABLE
	case 2:
		return usart_dma_tx_bytes(2, DMA0, 1, data, size);
#endif
	default:
		return 0;
	}
}

uint16_t vsfhal_usart_tx_get_data_size(vsfhal_usart_t index)
{
	switch (index)
	{
#if VSFHAL_USART0_ENABLE
	case 0:
		if (DMA_CHCTL(DMA0, 3) & DMA_CHXCTL_CHEN)
			return DMA_CHCNT(DMA0, 3);
		else
			return 0;
#endif
#if VSFHAL_USART1_ENABLE
	case 1:
		if (DMA_CHCTL(DMA0, 5) & DMA_CHXCTL_CHEN)
			return DMA_CHCNT(DMA0, 5);
		else
			return 0;
#endif
#if VSFHAL_USART2_ENABLE
	case 2:
		if (DMA_CHCTL(DMA0, 1) & DMA_CHXCTL_CHEN)
			return DMA_CHCNT(DMA0, 1);
		else
			return 0;
#endif
	default:
		return 0;
	}
}

uint16_t vsfhal_usart_tx_get_free_size(vsfhal_usart_t index)
{
	return DMA_BUFF_SIZE -  vsfhal_usart_tx_get_data_size(index);
}

vsf_err_t vsfhal_usart_tx_int_config(vsfhal_usart_t index, bool enable)
{
	switch (index)
	{
#if VSFHAL_USART0_ENABLE
	case 0:
		if (enable)
			DMA_CHCTL(DMA0, 3) |= DMA_CHXCTL_FTFIE;
		else
			DMA_CHCTL(DMA0, 3) &= ~DMA_CHXCTL_FTFIE;
		break;
#endif
#if VSFHAL_USART1_ENABLE
	case 1:
		if (enable)
			DMA_CHCTL(DMA0, 5) |= DMA_CHXCTL_FTFIE;
		else
			DMA_CHCTL(DMA0, 5) &= ~DMA_CHXCTL_FTFIE;
		break;
#endif
#if VSFHAL_USART2_ENABLE
	case 2:
		if (enable)
			DMA_CHCTL(DMA0, 1) |= DMA_CHXCTL_FTFIE;
		else
			DMA_CHCTL(DMA0, 1) &= ~DMA_CHXCTL_FTFIE;
		break;
#endif
	default:
		return VSFERR_BUG;
	}

	return VSFERR_NONE;
}

static uint16_t usart_dma_rx_bytes(vsfhal_usart_t index, uint32_t dmax, uint8_t ch,
		uint8_t *data, uint16_t size)
{
	uint8_t buf[DMA_BUFF_SIZE];
	
	if (DMA_CHCTL(dmax, ch) & DMA_CHXCTL_CHEN)
	{
		if (rx_dma_buf_sel[index] == 0)
		{
			if (rx_dma_buff_pos[index][1])
			{
				if (size >= rx_dma_buff_pos[index][1])
				{
					size = rx_dma_buff_pos[index][1];
					memcpy(data, rx_dma_buff[index][1], size);
					rx_dma_buff_pos[index][1] = 0;
					return size;
				}
				else
				{
					memcpy(data, rx_dma_buff[index][1], size);
					memcpy(buf, rx_dma_buff[index][1] + size, rx_dma_buff_pos[index][1] - size);
					memcpy(rx_dma_buff[index][1], buf, rx_dma_buff_pos[index][1] - size);
					rx_dma_buff_pos[index][1] -= size;
					return size;
				}
			}
		}
		else
		{
			if (rx_dma_buff_pos[index][0])
			{
				if (size >= rx_dma_buff_pos[index][0])
				{
					size = rx_dma_buff_pos[index][0];
					memcpy(data, rx_dma_buff[index][0], size);
					rx_dma_buff_pos[index][0] = 0;
					return size;
				}
				else
				{
					memcpy(data, rx_dma_buff[index][0], size);
					memcpy(buf, rx_dma_buff[index][0] + size, rx_dma_buff_pos[index][0] - size);
					memcpy(rx_dma_buff[index][0], buf, rx_dma_buff_pos[index][0] - size);
					rx_dma_buff_pos[index][0] -= size;
					return size;
				}
			}
		}
	}
	
	return 0;
}

uint16_t vsfhal_usart_rx_bytes(vsfhal_usart_t index, uint8_t *data, uint16_t size)
{
	switch (index)
	{
#if VSFHAL_USART0_ENABLE
	case 0:
		return usart_dma_rx_bytes(0, DMA0, 4, data, size);
#endif
#if VSFHAL_USART1_ENABLE
	case 1:
		return usart_dma_rx_bytes(1, DMA0, 6, data, size);
#endif
#if VSFHAL_USART2_ENABLE
	case 2:
		return usart_dma_rx_bytes(2, DMA0, 2, data, size);
#endif
	default:
		return 0;
	}
}

uint16_t vsfhal_usart_rx_get_data_size(vsfhal_usart_t index)
{
	switch (index)
	{
#if VSFHAL_USART0_ENABLE
	case 0:
		if (DMA_CHCTL(DMA0, 4) & DMA_CHXCTL_CHEN)
		{
			if (rx_dma_buf_sel[0] == 0)
				return rx_dma_buff_pos[0][1];
			else
				return rx_dma_buff_pos[0][0];
		}
		break;
#endif
#if VSFHAL_USART1_ENABLE
	case 1:
		if (DMA_CHCTL(DMA0, 6) & DMA_CHXCTL_CHEN)
		{
			if (rx_dma_buf_sel[1] == 0)
				return rx_dma_buff_pos[1][1];
			else
				return rx_dma_buff_pos[1][0];
		}
		break;
#endif
#if VSFHAL_USART2_ENABLE
	case 2:
		if (DMA_CHCTL(DMA0, 2) & DMA_CHXCTL_CHEN)
		{
			if (rx_dma_buf_sel[2] == 0)
				return rx_dma_buff_pos[2][1];
			else
				return rx_dma_buff_pos[2][0];
		}
		break;
#endif
	}
	return 0;
}

uint16_t vsfhal_usart_rx_get_free_size(vsfhal_usart_t index)
{
	return DMA_BUFF_SIZE - vsfhal_usart_rx_get_data_size(index);
}

vsf_err_t vsfhal_usart_rx_int_config(vsfhal_usart_t index, bool enable)
{
	switch (index)
	{
#if VSFHAL_USART0_ENABLE
	case 0:
		if (enable)
			DMA_CHCTL(DMA0, 4) |= DMA_CHXCTL_FTFIE;
		else
			DMA_CHCTL(DMA0, 4) &= ~DMA_CHXCTL_FTFIE;
		break;
#endif
#if VSFHAL_USART1_ENABLE
	case 1:
		if (enable)
			DMA_CHCTL(DMA0, 6) |= DMA_CHXCTL_FTFIE;
		else
			DMA_CHCTL(DMA0, 6) &= ~DMA_CHXCTL_FTFIE;
		break;
#endif
#if VSFHAL_USART2_ENABLE
	case 2:
		if (enable)
			DMA_CHCTL(DMA0, 2) |= DMA_CHXCTL_FTFIE;
		else
			DMA_CHCTL(DMA0, 2) &= ~DMA_CHXCTL_FTFIE;
		break;
#endif
	default:
		return VSFERR_BUG;
	}

	return VSFERR_NONE;
}

static void usart_irq_handler(vsfhal_usart_t index, uint32_t dmax, uint8_t tx_ch, uint8_t rx_ch)
{
	// rx dma
	if ((DMA_CHCTL(dmax, rx_ch) & DMA_CHXCTL_CHEN) && (DMA_CHCNT(dmax, rx_ch) != DMA_BUFF_SIZE))
	{
		if (rx_dma_buf_sel[index] == 0)
		{
			DMA_CHCTL(dmax, rx_ch) &= ~DMA_CHXCTL_CHEN;
			rx_dma_buff_pos[index][0] = DMA_BUFF_SIZE - DMA_CHCNT(dmax, rx_ch);
			DMA_CHMADDR(dmax, rx_ch) = (uint32_t)rx_dma_buff[index][1];
			DMA_CHCNT(dmax, rx_ch) = DMA_BUFF_SIZE;
			DMA_CHCTL(dmax, rx_ch) |= DMA_CHXCTL_CHEN;
			rx_dma_buf_sel[index] = 1;
			if (vsfhal_usart_onrx[index])
				vsfhal_usart_onrx[index](vsfhal_usart_callback_param[index]);
		}
		else
		{
			DMA_CHCTL(dmax, rx_ch) &= ~DMA_CHXCTL_CHEN;
			rx_dma_buff_pos[index][1] = DMA_BUFF_SIZE - DMA_CHCNT(dmax, rx_ch);
			DMA_CHMADDR(dmax, rx_ch) = (uint32_t)rx_dma_buff[index][0];
			DMA_CHCNT(dmax, rx_ch) = DMA_BUFF_SIZE;
			DMA_CHCTL(dmax, rx_ch) |= DMA_CHXCTL_CHEN;
			rx_dma_buf_sel[index] = 0;
			if (vsfhal_usart_onrx[index])
				vsfhal_usart_onrx[index](vsfhal_usart_callback_param[index]);
		}
	}
	// tx dma
	if ((DMA_CHCTL(dmax, tx_ch) & DMA_CHXCTL_CHEN) && (DMA_CHCNT(dmax, tx_ch) == 0))
	{
		if (vsfhal_usart_ontx[index])
			vsfhal_usart_ontx[index](vsfhal_usart_callback_param[index]);
	}
}

#if VSFHAL_USART0_ENABLE
// used for usart0 tx/rx
ROOT void USART0_IRQHandler(void)
{
	if (USART_STAT1(USART0) & USART_STAT1_RTF)
	{
		USART_STAT1(USART0) &= ~USART_STAT1_RTF;
		usart_irq_handler(0, DMA0, 3, 4);
	}
}

ROOT void DMA0_Channel3_IRQHandler(void)
{
	DMA_INTC(DMA0) = DMA_INTC_GIFC << (3 * 4);
	usart_irq_handler(0, DMA0, 3, 4);
}

ROOT void DMA0_Channel4_IRQHandler(void)
{
	DMA_INTC(DMA0) = DMA_INTC_GIFC << (4 * 4);
	usart_irq_handler(0, DMA0, 3, 4);
}
#endif

#if VSFHAL_USART1_ENABLE
ROOT void USART1_IRQHandler(void)
{
	if (USART_STAT1(USART1) & USART_STAT1_RTF)
	{
		USART_STAT1(USART1) &= ~USART_STAT1_RTF;
		usart_irq_handler(1, DMA0, 5, 6);
	}
}

ROOT void DMA0_Channel5_IRQHandler(void)
{
	DMA_INTC(DMA0) = DMA_INTC_GIFC << (5 * 4);
	usart_irq_handler(1, DMA0, 5, 6);
}

ROOT void DMA0_Channel6_IRQHandler(void)
{
	DMA_INTC(DMA0) = DMA_INTC_GIFC << (6 * 4);
	usart_irq_handler(1, DMA0, 5, 6);
}
#endif

#if VSFHAL_USART2_ENABLE
ROOT void USART2_IRQHandler(void)
{
	if (USART_STAT1(USART2) & USART_STAT1_RTF)
	{
		USART_STAT1(USART2) &= ~USART_STAT1_RTF;
		usart_irq_handler(2, DMA0, 1, 2);
	}
}

ROOT void DMA0_Channel1_IRQHandler(void)
{
	DMA_INTC(DMA0) = DMA_INTC_GIFC << (1 * 4);
	usart_irq_handler(2, DMA0, 1, 2);
}

ROOT void DMA0_Channel2_IRQHandler(void)
{
	DMA_INTC(DMA0) = DMA_INTC_GIFC << (2 * 4);
	usart_irq_handler(2, DMA0, 1, 2);
}
#endif

#endif

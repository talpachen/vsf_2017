#include "vsf.h"
#include "core.h"

#if IFS_USART_EN

#define UART_IS_RX_READY(uart)		((uart->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) >> UART_FIFOSTS_RXEMPTY_Pos)
#define UART_IS_TX_EMPTY(uart)		((uart->FIFOSTS & UART_FIFOSTS_TXEMPTYF_Msk) >> UART_FIFOSTS_TXEMPTYF_Pos)
#define UART_IS_TX_FIFO_FULL(uart)	((uart->FIFOSTS & UART_FIFOSTS_TXFULL_Msk) >> UART_FIFOSTS_TXFULL_Pos)

static void (*vsfhal_usart_ontx[USART_NUM])(void *);
static void (*vsfhal_usart_onrx[USART_NUM])(void *, uint16_t data);
static void *vsfhal_usart_callback_param[USART_NUM];

vsf_err_t vsfhal_usart_init(uint8_t index)
{
	uint8_t usart_idx = (index >> 4) & 0x0F;
	uint8_t remap_idx = index & 0x0F;

	switch (usart_idx)
	{
	#if USART0_0_ENABLE
	case 0:
		switch (remap_idx)
		{
		case 0:
			// RXD
			SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA13MFP_Msk;
			SYS->GPA_MFPH |= 1 << SYS_GPA_MFPH_PA13MFP_Pos;
			// TXD
			SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA14MFP_Msk;
			SYS->GPA_MFPH |= 1 << SYS_GPA_MFPH_PA14MFP_Pos;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->CLKDIV0 &= ~CLK_CLKDIV0_UARTDIV_Msk;
		CLK->CLKSEL1 &= ~CLK_CLKSEL1_UARTSEL_Msk;
		CLK->APBCLK0 |= CLK_APBCLK0_UART0CKEN_Msk;
		break;
	#endif // USART0_0_ENABLE
	#if USART1_0_ENABLE
	case 1:
		switch (remap_idx)
		{
		case 0:
			// RXD
			SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB2MFP_Msk;
			SYS->GPB_MFPL |= 1 << SYS_GPB_MFPL_PB2MFP_Pos;
			// TXD
			SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB3MFP_Msk;
			SYS->GPB_MFPL |= 1 << SYS_GPB_MFPL_PB3MFP_Pos;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->CLKDIV0 &= ~CLK_CLKDIV0_UARTDIV_Msk;
		CLK->CLKSEL1 &= ~CLK_CLKSEL1_UARTSEL_Msk;
		CLK->APBCLK0 |= CLK_APBCLK0_UART1CKEN_Msk;
		break;
	#endif // USART1_0_ENABLE
	#if USART2_0_ENABLE
	#endif // USART2_0_ENABLE
	#if USART3_0_ENABLE
	case 3:
		switch (remap_idx)
		{
		case 0:
			// RXD
			SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD4MFP_Msk;
			SYS->GPD_MFPL |= 2 << SYS_GPD_MFPL_PD4MFP_Pos;
			// TXD
			SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD5MFP_Msk;
			SYS->GPD_MFPL |= 2 << SYS_GPD_MFPL_PD5MFP_Pos;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->CLKDIV0 &= ~CLK_CLKDIV0_UARTDIV_Msk;
		CLK->CLKSEL1 &= ~CLK_CLKSEL1_UARTSEL_Msk;
		CLK->APBCLK0 |= CLK_APBCLK0_UART3CKEN_Msk;
		break;
	#endif // USART3_0_ENABLE
	#if USART4_0_ENABLE
	case 4:
		switch (remap_idx)
		{
		case 0:
			// RXD
			SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC0MFP_Msk;
			SYS->GPC_MFPL |= 3 << SYS_GPC_MFPL_PC0MFP_Pos;
			// TXD
			SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC1MFP_Msk;
			SYS->GPC_MFPL |= 3 << SYS_GPC_MFPL_PC1MFP_Pos;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->CLKDIV0 &= ~CLK_CLKDIV0_UARTDIV_Msk;
		CLK->CLKSEL1 &= ~CLK_CLKSEL1_UARTSEL_Msk;
		CLK->APBCLK0 |= CLK_APBCLK0_UART4CKEN_Msk;
		break;
	#endif // USART4_0_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usart_fini(uint8_t index)
{
	uint8_t usart_idx = (index >> 4) & 0x0F;
	uint8_t remap_idx = index & 0x0F;

	switch (usart_idx)
	{
	#if USART0_0_ENABLE
	case 0:
		switch (remap_idx)
		{
		case 0:
			SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA13MFP_Msk;
			SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA14MFP_Msk;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->APBCLK0 &= ~CLK_APBCLK0_UART0CKEN_Msk;
		break;
	#endif // USART0_0_ENABLE
	#if USART1_0_ENABLE
	case 1:
		switch (remap_idx)
		{
		case 0:
			SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB2MFP_Msk;
			SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB3MFP_Msk;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->APBCLK0 &= ~CLK_APBCLK0_UART1CKEN_Msk;
		break;
	#endif // USART1_0_ENABLE
	#if USART2_0_ENABLE
	#endif // USART2_0_ENABLE
	#if USART3_0_ENABLE
	case 3:
		switch (remap_idx)
		{
		case 0:
			SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD4MFP_Msk;
			SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD5MFP_Msk;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->APBCLK0 &= ~CLK_APBCLK0_UART3CKEN_Msk;
		break;
	#endif // USART3_0_ENABLE
	#if USART4_0_ENABLE
	case 4:
		switch (remap_idx)
		{
		case 0:
			SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC0MFP_Pos;
			SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC1MFP_Msk;
			break;
		default:
			return VSFERR_NOT_SUPPORT;
		}
		CLK->APBCLK0 &= ~CLK_APBCLK0_UART4CKEN_Msk;
		break;
	#endif // USART4_0_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usart_config(uint8_t index, uint32_t baudrate, uint32_t mode)
{
	UART_T *usart;
	uint8_t usart_idx = (index >> 4) & 0x0F;
	struct vsfhal_info_t *info;
	uint32_t baud_div = 0, reg_line = 0;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	// mode:
	// bit0 - bit1: parity
	// ------------------------------------- bit2 - bit3: mode [nothing]
	// bit4       : stopbits
	reg_line |= (mode << 3) & 0x18;	//parity
	reg_line |= (mode >> 2) & 0x04;	//stopbits

	usart->FUNCSEL = 0;
	usart->LINE = reg_line | 3;
	usart->FIFO = 0x5ul << 4; // 46/14 (64/16)
	usart->TOUT = 60;

	if (vsfhal_interface_get_info(&info))
	{
		return VSFERR_FAIL;
	}

	if(baudrate != 0)
	{
		int32_t error;
		baud_div = info->osc_freq_hz / baudrate;
		if ((baud_div < 11) || (baud_div > (0xffff + 2)))
			return VSFERR_INVALID_PARAMETER;
		error = (info->osc_freq_hz / baud_div) * 1000 / baudrate;
		error -= 1000;
		if ((error > 20) || ((error < -20)))
			return VSFERR_INVALID_PARAMETER;
		if (info->osc_freq_hz * 1000 / baud_div / baudrate)
		usart->BAUD = UART_BAUD_BAUDM0_Msk | UART_BAUD_BAUDM1_Msk |
						(baud_div - 2);
	}


	switch (usart_idx)
	{
	#if USART0_INT_EN
	case 0:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART0_IRQn);
		break;
	#endif
	#if USART1_INT_EN
	case 1:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART1_IRQn);
		break;
	#endif
	#if USART2_INT_EN
	case 2:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART2_IRQn);
		break;
	#endif
	#if USART3_INT_EN
	case 3:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART3_IRQn);
		break;
	#endif
	#if USART4_INT_EN
	case 4:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART4_IRQn);
		break;
	#endif
	default:
		break;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usart_config_cb(uint8_t index, uint32_t int_priority,
				void *p, void (*ontx)(void *), void (*onrx)(void *, uint16_t))
{
	uint8_t usart_idx = (index >> 4) & 0x0F;

	vsfhal_usart_ontx[usart_idx] = ontx;
	vsfhal_usart_onrx[usart_idx] = onrx;
	vsfhal_usart_callback_param[usart_idx] = p;

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usart_tx(uint8_t index, uint16_t data)
{
	UART_T *usart;
	uint8_t usart_idx = (index >> 4) & 0x0F;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	usart->DAT = (uint8_t)data;
	usart->INTEN |= UART_INTEN_THREIEN_Msk;

	return VSFERR_NONE;
}

uint16_t vsfhal_usart_rx(uint8_t index)
{
	UART_T *usart;
	uint8_t usart_idx = (index >> 4) & 0x0F;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	return usart->DAT;
}

uint16_t vsfhal_usart_tx_bytes(uint8_t index, uint8_t *data, uint16_t size)
{
	UART_T *usart;
	uint8_t usart_idx = (index >> 4) & 0x0F;
	uint16_t i;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	for (i = 0; i < size; i++)
	{
		usart->DAT = data[i];
	}
	usart->INTEN |= UART_INTEN_THREIEN_Msk;

	return 0;
}

uint16_t vsfhal_usart_tx_get_free_size(uint8_t index)
{
	UART_T *usart;
	uint8_t usart_idx = (index >> 4) & 0x0F;
	uint32_t fifo_len;

	fifo_len = usart_idx ? 64 : 16;
	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	if (usart->FIFOSTS & UART_FIFOSTS_TXFULL_Msk)
	{
		return 0;
	}
	else
	{
		return fifo_len - ((usart->FIFOSTS & UART_FIFOSTS_TXPTR_Msk) >>
							UART_FIFOSTS_TXPTR_Pos);
	}
}

uint16_t vsfhal_usart_rx_bytes(uint8_t index, uint8_t *data, uint16_t size)
{
	UART_T *usart;
	uint8_t usart_idx = (index >> 4) & 0x0F;
	uint16_t i;

	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	for (i = 0; i < size; i++)
	{
		if (usart->FIFOSTS & (UART_FIFOSTS_RXFULL_Msk | UART_FIFOSTS_RXPTR_Msk))
		{
			data[i] = usart->DAT;
		}
		else
		{
			break;
		}
	}
	return i;
}

uint16_t vsfhal_usart_rx_get_data_size(uint8_t index)
{
	UART_T *usart;
	uint8_t usart_idx = (index >> 4) & 0x0F;
	uint32_t fifo_len;

	fifo_len = usart_idx ? 64 : 16;
	usart = (UART_T *)(UART0_BASE + (usart_idx << 12));

	if (usart->FIFOSTS & UART_FIFOSTS_RXFULL_Msk)
	{
		return fifo_len;
	}
	else
	{
		return (usart->FIFOSTS & UART_FIFOSTS_RXPTR_Msk) >>
				UART_FIFOSTS_RXPTR_Pos;
	}
}

static void uart_handler(uint8_t index)
{
	UART_T *usart = (UART_T *)(UART0_BASE + (index << 12));
	
	if (usart->INTSTS & UART_INTSTS_RDAIF_Msk)
	{
		vsfhal_usart_onrx[index](vsfhal_usart_callback_param[index], usart->DAT);
	}
	else if (usart->INTSTS & UART_INTSTS_RXTOINT_Msk)
	{
		vsfhal_usart_onrx[index](vsfhal_usart_callback_param[index], usart->DAT);
	}
	if (usart->INTSTS & UART_INTSTS_THREINT_Msk)
	{
		usart->INTEN &= ~UART_INTEN_THREIEN_Msk;
		vsfhal_usart_ontx[index](vsfhal_usart_callback_param[index]);
	}
}

#if USART0_INT_EN && USART0_0_ENABLE
ROOTFUNC void UART0_IRQHandler(void)
{
	uart_handler(0);
}
#endif
#if USART1_INT_EN && USART1_0_ENABLE
ROOTFUNC void UART1_IRQHandler(void)
{
	uart_handler(1);
}
#endif
#if USART2_INT_EN && USART2_0_ENABLE
ROOTFUNC void UART2_IRQHandler(void)
{
	uart_handler(2);
}
#endif
#if USART3_INT_EN && USART3_0_ENABLE
ROOTFUNC void UART3_IRQHandler(void)
{
	uart_handler(3);
}
#endif
#if USART4_INT_EN && USART4_0_ENABLE
ROOTFUNC void UART4_IRQHandler(void)
{
	uart_handler(4);
}
#endif


#endif


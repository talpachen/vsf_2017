#include "vsf.h"

#if VSFHAL_USART_EN

#include "core.h"

#define UART_IS_RX_READY(uart)		((uart->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) >> UART_FIFOSTS_RXEMPTY_Pos)
#define UART_IS_TX_EMPTY(uart)		((uart->FIFOSTS & UART_FIFOSTS_TXEMPTYF_Msk) >> UART_FIFOSTS_TXEMPTYF_Pos)
#define UART_IS_TX_FIFO_FULL(uart)	((uart->FIFOSTS & UART_FIFOSTS_TXFULL_Msk) >> UART_FIFOSTS_TXFULL_Pos)

static void (*vsfhal_usart_ontx[VSFHAL_USART_NUM])(void *);
static void (*vsfhal_usart_onrx[VSFHAL_USART_NUM])(void *, uint16_t data);
static void *vsfhal_usart_callback_param[VSFHAL_USART_NUM];

vsf_err_t vsfhal_usart_init(uint8_t index)
{
	switch (index)
	{
	#if VSFHAL_USART0_ENABLE
	case 0:
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk);
		SYS->GPB_MFPL |= 3 << SYS_GPB_MFPL_PB0MFP_Pos;
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB1MFP_Msk);
		SYS->GPB_MFPL |= 3 << SYS_GPB_MFPL_PB1MFP_Pos;
		CLK->CLKDIV3 &= ~(CLK_CLKDIV3_UART0SEL_Msk | CLK_CLKDIV3_UART0DIV_Msk);
		CLK->APBCLK |= CLK_APBCLK_UART0CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_UART0RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_UART0RST_Msk;
		break;
	#endif // VSFHAL_USART0_ENABLE
	#if VSFHAL_USART1_ENABLE
	case 1:
		#if VSFHAL_USART1_TXD_PA8_EN
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA8MFP_Msk);
		SYS->GPA_MFPH |= 3 << SYS_GPA_MFPH_PA8MFP_Pos;
		#endif
		#if VSFHAL_USART1_TXD_PB6_EN
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB6MFP_Msk);
		SYS->GPB_MFPL |= 3 << SYS_GPB_MFPL_PB6MFP_Pos;
		#endif
		#if VSFHAL_USART1_RXD_PA9_EN
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA9MFP_Msk);
		SYS->GPA_MFPH |= 3 << SYS_GPA_MFPH_PA9MFP_Pos;
		#endif
		#if VSFHAL_USART1_RXD_PB7_EN
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB7MFP_Msk);
		SYS->GPB_MFPL |= 3 << SYS_GPB_MFPL_PB7MFP_Msk;
		#endif
		#if VSFHAL_USART1_CTS_PB8_EN
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk);
		SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB8MFP_Pos;
		#endif
		#if VSFHAL_USART1_RTS_PB9_EN
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB9MFP_Msk);
		SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB9MFP_Pos;
		#endif
		CLK->CLKDIV3 &= ~(CLK_CLKDIV3_UART1SEL_Msk | CLK_CLKDIV3_UART1DIV_Msk);
		CLK->APBCLK |= CLK_APBCLK_UART1CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_UART1RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_UART1RST_Msk;
		break;
	#endif // VSFHAL_USART1_ENABLE
	#if VSFHAL_USART2_ENABLE
	case 2:
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB10MFP_Msk);
		SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB10MFP_Pos;
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB11MFP_Msk);
		SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB11MFP_Pos;
		#if VSFHAL_USART2_CTS_PB12_EN
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk);
		SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB12MFP_Pos;
		#endif
		#if VSFHAL_USART2_RTS_PB13_EN
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB13MFP_Msk);
		SYS->GPB_MFPH |= 3 << SYS_GPB_MFPH_PB13MFP_Pos;		
		#endif
		CLK->CLKDIV3 &= ~(CLK_CLKDIV3_UART2SEL_Msk | CLK_CLKDIV3_UART2DIV_Msk);
		CLK->APBCLK |= CLK_APBCLK_UART2CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_UART2RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_UART2RST_Msk;
		break;
	#endif // VSFHAL_USART2_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usart_fini(uint8_t index)
{
	switch (index)
	{
	#if VSFHAL_USART0_ENABLE
	case 0:
		SYS->IPRST1 |= SYS_IPRST1_UART0RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_UART0RST_Msk;
		CLK->APBCLK &= ~CLK_APBCLK_UART0CKEN_Msk;
		break;
	#endif // VSFHAL_USART0_ENABLE
	#if VSFHAL_USART1_ENABLE
	case 1:
		SYS->IPRST1 |= SYS_IPRST1_UART1RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_UART1RST_Msk;
		CLK->APBCLK &= ~CLK_APBCLK_UART1CKEN_Msk;
		break;
	#endif // VSFHAL_USART1_ENABLE
	#if VSFHAL_USART2_ENABLE
	case 2:
		SYS->IPRST1 |= SYS_IPRST1_UART2RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_UART2RST_Msk;
		CLK->APBCLK &= ~CLK_APBCLK_UART2CKEN_Msk;
		break;
	#endif // VSFHAL_USART2_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usart_config(uint8_t index, uint32_t baudrate, uint32_t mode)
{
	UART_T *usart;
	struct vsfhal_info_t *info;
	uint32_t baud_div = 0, reg_line = 0;

	usart = (UART_T *)(UART0_BASE + (index << 12));

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

	if (vsfhal_core_get_info(&info))
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

	switch (index)
	{
	#if VSFHAL_USART0_ENABLE
	case 0:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART0_IRQn);
		break;
	#endif
	#if VSFHAL_USART1_ENABLE
	case 1:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART1_IRQn);
		break;
	#endif
	#if VSFHAL_USART2_ENABLE
	case 2:
		usart->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk |
						UART_INTEN_TOCNTEN_Msk;
		NVIC_EnableIRQ(UART2_IRQn);
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
	vsfhal_usart_ontx[index] = ontx;
	vsfhal_usart_onrx[index] = onrx;
	vsfhal_usart_callback_param[index] = p;

	return VSFERR_NONE;
}

vsf_err_t vsfhal_usart_tx(uint8_t index, uint16_t data)
{
	UART_T *usart;

	usart = (UART_T *)(UART0_BASE + (index << 12));

	usart->DAT = (uint8_t)data;
	usart->INTEN |= UART_INTEN_THREIEN_Msk;

	return VSFERR_NONE;
}

uint16_t vsfhal_usart_rx(uint8_t index)
{
	UART_T *usart;

	usart = (UART_T *)(UART0_BASE + (index << 12));

	return usart->DAT;
}

uint16_t vsfhal_usart_tx_bytes(uint8_t index, uint8_t *data, uint16_t size)
{
	UART_T *usart;
	uint16_t i;

	usart = (UART_T *)(UART0_BASE + (index << 12));

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
	uint32_t fifo_len;

	fifo_len = index ? 64 : 16;
	usart = (UART_T *)(UART0_BASE + (index << 12));

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
	uint16_t i;

	usart = (UART_T *)(UART0_BASE + (index << 12));

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
	uint32_t fifo_len;

	fifo_len = index ? 64 : 16;
	usart = (UART_T *)(UART0_BASE + (index << 12));

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

#if VSFHAL_USART0_ENABLE
ROOTFUNC void UART0_IRQHandler(void)
{
	uart_handler(0);
}
#endif
#if VSFHAL_USART1_ENABLE
ROOTFUNC void UART1_IRQHandler(void)
{
	uart_handler(1);
}
#endif
#if VSFHAL_USART2_ENABLE
ROOTFUNC void UART2_IRQHandler(void)
{
	uart_handler(2);
}
#endif

#endif


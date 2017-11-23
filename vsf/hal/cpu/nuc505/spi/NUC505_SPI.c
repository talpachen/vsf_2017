#include "vsf.h"
#include "core.h"

#if VSFHAL_SPI_EN

struct
{
	void (*onready)(void *);
	void *param;
	void *in;
	void *out;
	uint32_t prtrd;
	uint32_t prtwr;
	uint32_t len;
	uint8_t start;
} static spi_ctrl[VSFHAL_SPI_NUM];

vsf_err_t vsfhal_spi_init(uint8_t index)
{
	struct vsfhal_info_t *info;
	
	if (vsfhal_core_get_info(&info) || (NULL == info))
		return VSFERR_BUG;

	switch (index)
	{
	#if VSFHAL_SPI0_ENABLE
	case 0:
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB3MFP_Msk | SYS_GPB_MFPL_PB4MFP_Msk |
				SYS_GPB_MFPL_PB5MFP_Pos);
		SYS->GPB_MFPL |= (1 << SYS_GPB_MFPL_PB3MFP_Pos) |
				(1 << SYS_GPB_MFPL_PB4MFP_Pos) | (1 << SYS_GPB_MFPL_PB5MFP_Pos);
		if (info->clk_enable & NUC505_CLK_PLL)
			CLK->CLKDIV2 |= CLK_CLKDIV2_SPI0SEL_Msk;
		else
			CLK->CLKDIV2 &= ~CLK_CLKDIV2_SPI0SEL_Msk;
		CLK->APBCLK |= CLK_APBCLK_SPI0CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_SPI0RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_SPI0RST_Msk;
		NVIC_EnableIRQ(SPI0_IRQn);
		break;
	#endif // VSFHAL_SPI0_ENABLE
	#if VSFHAL_SPI1_ENABLE
	case 1:
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB11MFP_Msk | SYS_GPB_MFPH_PB12MFP_Msk |
				SYS_GPB_MFPH_PB13MFP_Msk);
		SYS->GPB_MFPH |= (1 << SYS_GPB_MFPH_PB11MFP_Pos) |
				(1 << SYS_GPB_MFPH_PB12MFP_Pos) | (1 << SYS_GPB_MFPH_PB13MFP_Pos);
		if (info->clk_enable & NUC505_CLK_PLL)
			CLK->CLKDIV2 |= CLK_CLKDIV2_SPI1SEL_Msk;
		else
			CLK->CLKDIV2 &= ~CLK_CLKDIV2_SPI1SEL_Msk;
		CLK->APBCLK |= CLK_APBCLK_SPI1CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_SPI1RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_SPI1RST_Msk;
		NVIC_EnableIRQ(SPI1_IRQn);
		break;
	#endif // VSFHAL_SPI1_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_spi_fini(uint8_t index)
{
	switch (index)
	{
	#if VSFHAL_SPI0_ENABLE
	case 0:
		SYS->IPRST1 |= SYS_IPRST1_SPI0RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_SPI0RST_Msk;
		CLK->APBCLK &= ~CLK_APBCLK_SPI0CKEN_Msk;
		NVIC_DisableIRQ(SPI0_IRQn);
		break;
	#endif // VSFHAL_SPI0_ENABLE
	#if VSFHAL_SPI1_ENABLE
	case 1:
		SYS->IPRST1 |= SYS_IPRST1_SPI1RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_SPI1RST_Msk;
		CLK->APBCLK &= ~CLK_APBCLK_SPI1CKEN_Msk;
		NVIC_DisableIRQ(SPI1_IRQn);
		break;
	#endif // VSFHAL_SPI1_ENABLE
	default:
		return VSFERR_NOT_SUPPORT;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_spi_config(uint8_t index, uint32_t kHz, uint32_t mode)
{
	struct vsfhal_info_t *info;
	uint32_t freq;

	if (vsfhal_core_get_info(&info) || (NULL == info))
		return VSFERR_BUG;

	SPI_T *spi = index ? SPI1 : SPI0;

	if (info->clk_enable & NUC505_CLK_PLL)
		freq = info->pll_freq_hz;
	else
		freq = info->osc_freq_hz;
	spi->CLKDIV = freq / kHz / 1000 - 1;
	spi->CTL = mode |
			(0x1ul << SPI_CTL_SUSPITV_Pos) | 	// suspend interval = 1.5 spiclk
			(0x8ul << SPI_CTL_DWIDTH_Pos) |		// transmit bit width = 8
			SPI_CTL_SPIEN_Msk;
	
	spi->FIFOCTL = 0;

	return VSFERR_NONE;
}

vsf_err_t vsfhal_spi_config_cb(uint8_t index, uint32_t int_priority,
		void *p, void (*callback)(void *))
{
	spi_ctrl[index].onready = callback;
	spi_ctrl[index].param = p;

	return VSFERR_NONE;
}

static void spi_irq(SPI_T *spi, int i)
{
	if(spi_ctrl[i].start != 1)
	{
		return;
	}

	while(!(spi->STATUS&SPI_STATUS_RXEMPTY_Msk))
	{
		if (spi_ctrl[i].in != NULL )
		{
			((uint8_t *)(spi_ctrl[i].in))[spi_ctrl[i].prtrd] = spi->RX;
		}
		else
		{
			volatile uint32_t dummy = spi->RX;
		}
		spi_ctrl[i].prtrd++;
	}

	if(spi_ctrl[i].prtrd == spi_ctrl[i].len)
	{
		spi_ctrl[i].start = 0;
		spi->FIFOCTL = 0;

		if (spi_ctrl[i].onready != NULL)
		{
			spi_ctrl[i].onready(spi_ctrl[i].param);
		}
	}
	else
	{
		uint32_t remain = spi_ctrl[i].len - spi_ctrl[i].prtwr;

		if(remain > 8)
		{
			remain = 8;
		}

		if (remain > 0)
		{
			spi->FIFOCTL &= ~SPI_FIFOCTL_RXTH_Msk;
			spi->FIFOCTL |= (remain - 1) << SPI_FIFOCTL_RXTH_Pos;
			while (remain > 0)
			{
				if (spi_ctrl[i].out != NULL)
				{
					spi->TX = ((uint8_t *)(spi_ctrl[i].out))[spi_ctrl[i].prtwr];
				}
				else
				{
					spi->TX = 0xFF;
				}
				spi_ctrl[i].prtwr++;
				remain--;
			}
		}
	}
}

static uint8_t vsfhal_spi_dummy;
vsf_err_t vsfhal_spi_start(uint8_t index, uint8_t *out, uint8_t *in,
		uint32_t len)
{
	if (spi_ctrl[index].start)
		return VSFERR_NOT_READY;

	spi_ctrl[index].start = 1;
	spi_ctrl[index].out = (out == NULL) ? &vsfhal_spi_dummy : out;
	spi_ctrl[index].in = in;
	spi_ctrl[index].prtwr = 0;
	spi_ctrl[index].prtrd = 0;
	spi_ctrl[index].len = len;

	SPI_T *spi = index ? SPI1 : SPI0;

	if(len)
	{
		spi->FIFOCTL = SPI_FIFOCTL_RXTHIEN_Msk;
		spi_irq(spi, index);
	}

	return VSFERR_NONE;
}

uint32_t vsfhal_spi_stop(uint8_t index)
{
	SPI_T *spi = index ? SPI1 : SPI0;
	spi->FIFOCTL = SPI_FIFOCTL_TXRST_Msk | SPI_FIFOCTL_RXRST_Msk;
	spi_ctrl[index].start = 0;

	return spi_ctrl[index].len - spi_ctrl[index].prtwr;
}

#if VSFHAL_SPI0_ENABLE
ROOTFUNC void SPI0_IRQHandler(void)
{
	spi_irq(SPI0, 0);
}
#endif

#if VSFHAL_SPI1_ENABLE
ROOTFUNC void SPI1_IRQHandler(void)
{
	spi_irq(SPI1, 1);
}
#endif

#endif


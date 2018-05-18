#ifndef __STM32F4_CORE_H_INCLUDED__
#define __STM32F4_CORE_H_INCLUDED__

#define STM32F4_CLK_HSI				(1UL << 0)
#define STM32F4_CLK_HSE				(1UL << 1)
#define STM32F4_CLK_PLL				(1UL << 2)

enum vsfhal_hclksrc_t
{
	STM32F4_HCLKSRC_HSI = 0,
	STM32F4_HCLKSRC_HSE = 1,
	STM32F4_HCLKSRC_PLL = 2
};

enum vsfhal_pllsrc_t
{
	STM32F4_PLLSRC_HSI = 0,
	STM32F4_PLLSRC_HSE = 1,
};

struct vsfhal_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;

	uint32_t clk_enable;

	enum vsfhal_hclksrc_t hclksrc;
	enum vsfhal_pllsrc_t pllsrc;

	uint32_t hsi_freq_hz;
	uint32_t hse_freq_hz;
	uint32_t pll_freq_hz;
	uint32_t hclk_freq_hz;
	uint32_t pclk1_freq_hz;
	uint32_t pclk2_freq_hz;
};

// special
uint32_t vsfhal_get_pc(void);

#endif	// __stm32f4_CORE_H_INCLUDED__

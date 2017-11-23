#ifndef __NUC505_CORE_H_INCLUDED__
#define __NUC505_CORE_H_INCLUDED__

#define NUC505_CLK_HXT				(1UL << 0)
#define NUC505_CLK_LXT				(1UL << 1)
#define NUC505_CLK_LIRC				(1UL << 2)
#define NUC505_CLK_PLL				(1UL << 3)
#define NUC505_CLK_APLL				(1UL << 4)

enum vsfhal_hclksrc_t
{
	NUC505_HCLKSRC_PLLFOUT = 1,
	NUC505_HCLKSRC_HXT = 0,
};

struct vsfhal_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;

	uint32_t clk_enable;

	enum vsfhal_hclksrc_t hclksrc;

	uint32_t osc_freq_hz;
	uint32_t osc32k_freq_hz;
	uint32_t lirc_freq_hz;
	uint32_t pll_freq_hz;
	uint32_t apll_freq_hz;
	uint32_t cpu_freq_hz;
	uint32_t hclk_freq_hz;
	uint32_t pclk_freq_hz;
};

vsf_err_t vsfhal_core_get_info(struct vsfhal_info_t **info);

// special
uint32_t vsfhal_get_pc(void);
int32_t vsfhal_is_running_on_ram(void);
vsf_err_t vsfhal_code_map(uint8_t en, uint8_t rst, uint8_t len_kb, uint32_t addr);


#endif	// __NUC505_CORE_H_INCLUDED__


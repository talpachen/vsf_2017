#include "vsf.h"

#if VSFHAL_ADC_EN

#define NUC505_ADC_NUM				1

struct
{
	void (*callback)(void *, uint32_t);
	void *param;
} static adc_ctrl[NUC505_ADC_NUM];

vsf_err_t vsfhal_adc_init(uint8_t index)
{
	// Selece HXT: 12M
	CLK->CLKDIV1 &= ~(CLK_CLKDIV1_ADCSEL_Msk | CLK_CLKDIV1_ADCDIV_Msk);
	CLK->APBCLK |= CLK_APBCLK_ADCCKEN_Msk;
	SYS->IPRST1 = SYS_IPRST1_ADCRST_Msk;
	SYS->IPRST1 = 0;
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_adc_fini(uint8_t index)
{
	CLK->APBCLK &= ~CLK_APBCLK_ADCCKEN_Msk;
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_adc_config(uint8_t index, uint8_t channel, uint32_t kSPS,
		void (*callback)(void *, uint32_t), void *param, int32_t int_priority)
{
	uint32_t extsmpt;
	
	SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk << (4 * channel));
	SYS->GPA_MFPL |= 0x1ul << (4 * channel);
	
	if (int_priority >= 0)
	{
		NVIC_ClearPendingIRQ(ADC_IRQn);
		NVIC_SetPriority(ADC_IRQn, int_priority);
		NVIC_EnableIRQ(ADC_IRQn);
		adc_ctrl[index].callback = callback;
		adc_ctrl[index].param = param;
		
		ADC->INTCTL = ADC_INTCTL_ADCIEN_Msk;
	}
	else
	{
		ADC->INTCTL = 0;
		adc_ctrl[index].callback = NULL;
		adc_ctrl[index].param = NULL;
	}

	if ((channel == 0) && (kSPS > 100))
		kSPS = 100;
	else if ((channel == 1) && (kSPS > 1000))
		kSPS = 1000;
	else if (kSPS > 200)
		kSPS = 200;
	
	extsmpt = 12000 / kSPS;
	if (extsmpt > (255 + 16))
		extsmpt = 255;
	else if (extsmpt > 16)
		extsmpt -= 16;
	else
		extsmpt = 0;
	
	ADC->CTL = ADC->CTL & (~(ADC_CTL_PD_Msk | ADC_CTL_PDKEY_Msk | ADC_CTL_EXTSMPT_Msk | ADC_CTL_CHSEL_Msk)) |		
		(extsmpt << ADC_CTL_EXTSMPT_Pos) | (((uint32_t)channel) << ADC_CTL_CHSEL_Pos);
	
	return VSFERR_NONE;
}


int32_t vsfhal_adc_start(uint8_t index)
{
	ADC->CTL |= ADC_CTL_SWTRG_Msk;
	
	if (adc_ctrl[index].callback)
		return 0;
	else
	{
		while (!(ADC->INTCTL & ADC_INTCTL_ADCIF_Msk));
		ADC->INTCTL = ADC_INTCTL_ADCIF_Msk;
		return ADC->DAT;
	}
}

vsf_err_t vsfhal_adc_stop(uint8_t index)
{
	return VSFERR_NOT_SUPPORT;
}

ROOTFUNC void ADC_IRQHandler(void)
{
	if (adc_ctrl[0].callback)
		adc_ctrl[0].callback(adc_ctrl[0].param, ADC->DAT);
	ADC->INTCTL |= ADC_INTCTL_ADCIF_Msk + ADC_INTCTL_KEYIF_Msk;
}

#endif


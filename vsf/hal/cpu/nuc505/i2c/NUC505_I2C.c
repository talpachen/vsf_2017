#include "vsf.h"
#include "core.h"

#if VSFHAL_I2C_EN

#define I2C_CON_I2C_STS		I2C_CTL_SI_Msk
#define I2C_CON_START		I2C_CTL_STA_Msk
#define I2C_CON_STOP		I2C_CTL_STO_Msk
#define I2C_CON_ACK			I2C_CTL_AA_Msk
#define I2C_SET_CONTROL_REG(i2c, CTL_Msk) ( (i2c)->CTL = ((i2c)->CTL & ~0x3cul) | (CTL_Msk) )

struct i2c_ctrl_t
{
	uint8_t chip_addr;
	uint8_t msg_len;
	uint8_t msg_prt;
	uint16_t msg_buf_prt;
	struct vsfhal_i2c_msg_t *msg;
	void *param;
	void (*callback)(void *, vsf_err_t);
} static i2c_ctrl[VSFHAL_I2C_NUM];

vsf_err_t vsfhal_i2c_init(uint8_t index)
{
	switch (index)
	{
	#if VSFHAL_I2C0_ENABLE
	case 0:
		#if VSFHAL_I2C0_SCL_PA14_EN
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA14MFP_Msk);
		SYS->GPA_MFPH |= 2 << SYS_GPA_MFPH_PA14MFP_Pos;
		#endif
		#if VSFHAL_I2C0_SCL_PB0_EN
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk);
		SYS->GPB_MFPL |= 2 << SYS_GPB_MFPL_PB0MFP_Pos;
		#endif
		#if VSFHAL_I2C0_SCL_PD0_EN
		SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD0MFP_Msk);
		SYS->GPD_MFPL |= 2 << SYS_GPD_MFPL_PD0MFP_Pos;
		#endif
		#if VSFHAL_I2C0_SDA_PA15_EN
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA15MFP_Msk);
		SYS->GPA_MFPH |= 2 << SYS_GPA_MFPH_PA15MFP_Pos;
		#endif
		#if VSFHAL_I2C0_SDA_PB1_EN
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB1MFP_Msk);
		SYS->GPB_MFPL |= 2 << SYS_GPB_MFPL_PB1MFP_Pos;
		#endif
		#if VSFHAL_I2C0_SDA_PD1_EN
		SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD1MFP_Msk);
		SYS->GPD_MFPL |= 2 << SYS_GPD_MFPL_PD1MFP_Pos;
		#endif
		CLK->APBCLK |= CLK_APBCLK_I2C0CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_I2C0RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_I2C0RST_Msk;
		break;
	#endif // VSFHAL_I2C0_ENABLE
	#if VSFHAL_I2C1_ENABLE
	case 1:
		#if VSFHAL_I2C1_SCL_PA10_EN
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA10MFP_Msk);
		SYS->GPA_MFPH |= 2 << SYS_GPA_MFPH_PA10MFP_Pos;
		#endif
		#if VSFHAL_I2C1_SCL_PB10_EN
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB10MFP_Msk);
		SYS->GPB_MFPH |= 2 << SYS_GPB_MFPH_PB10MFP_Pos;
		#endif
		#if VSFHAL_I2C1_SCL_PB14_EN
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB14MFP_Msk);
		SYS->GPB_MFPH |= 2 << SYS_GPB_MFPH_PB14MFP_Pos;
		#endif		
		#if VSFHAL_I2C1_SDA_PA11_EN
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA11MFP_Msk);
		SYS->GPA_MFPH |= 2 << SYS_GPA_MFPH_PA11MFP_Pos;
		#endif
		#if VSFHAL_I2C1_SDA_PB11_EN
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB11MFP_Msk);
		SYS->GPB_MFPH |= 2 << SYS_GPB_MFPH_PB11MFP_Pos;
		#endif
		#if VSFHAL_I2C1_SDA_PB15_EN
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB15MFP_Msk);
		SYS->GPB_MFPH |= 2 << SYS_GPB_MFPH_PB15MFP_Pos;
		#endif
		CLK->APBCLK |= CLK_APBCLK_I2C1CKEN_Msk;
		SYS->IPRST1 |= SYS_IPRST1_I2C1RST_Msk;
		SYS->IPRST1 &= ~SYS_IPRST1_I2C1RST_Msk;
		break;
	#endif // VSFHAL_I2C1_ENABLE
	default:
		return VSFERR_FAIL;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_i2c_fini(uint8_t index)
{
	switch (index)
	{
	#if VSFHAL_I2C0_ENABLE
	case 0:
		SYS->IPRST1 |= SYS_IPRST1_I2C0RST_Msk;
		CLK->APBCLK &= ~CLK_APBCLK_I2C0CKEN_Msk;
		break;
	#endif // VSFHAL_I2C0_ENABLE
	#if VSFHAL_I2C1_ENABLE
	case 1:
		SYS->IPRST1 |= SYS_IPRST1_I2C1RST_Msk;
		CLK->APBCLK &= ~CLK_APBCLK_I2C1CKEN_Msk;
		break;
	#endif // VSFHAL_I2C1_ENABLE
	default:
		return VSFERR_FAIL;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_i2c_config(uint8_t index, uint16_t kHz)
{
	uint32_t div;
	I2C_T *i2c;
	struct vsfhal_info_t *info;

	if (index >= VSFHAL_I2C_NUM)
		return VSFERR_FAIL;

	if (vsfhal_core_get_info(&info))
		return VSFERR_FAIL;
	
	switch (index)
	{
	case 0:
		i2c = I2C0;
		NVIC_EnableIRQ(I2C0_IRQn);
		break;
	case 1:
		i2c = I2C1;
		NVIC_EnableIRQ(I2C1_IRQn);
		break;
	}

	div = info->pclk_freq_hz / (kHz * 1000 * 4) - 1;
	if (div < 4)
		div = 4;
	else if (div > 255)
		div = 255;
	i2c->CLKDIV = div;
	i2c->CTL = I2C_CTL_INTEN_Msk | I2C_CTL_I2CEN_Msk;

	return VSFERR_NONE;
}

vsf_err_t vsfhal_i2c_config_cb(uint8_t index, void *param,
		void (*cb)(void*, vsf_err_t))
{
	if (index >= VSFHAL_I2C_NUM)
		return VSFERR_FAIL;
	
	i2c_ctrl[index].param = param;
	i2c_ctrl[index].callback = cb;
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_i2c_xfer(uint8_t index, uint16_t addr,
		struct vsfhal_i2c_msg_t *msg, uint8_t msg_len)
{
	I2C_T *i2c;

	if ((index >= VSFHAL_I2C_NUM) || !msg || !msg_len)
		return VSFERR_FAIL;

	switch (index)
	{
	case 0:
		i2c = I2C0;
		break;
	case 1:
		i2c = I2C1;
		break;
	}

	i2c_ctrl[index].chip_addr = addr;
	i2c_ctrl[index].msg = msg;
	i2c_ctrl[index].msg_len = msg_len;
	i2c_ctrl[index].msg_prt = 0;
	i2c_ctrl[index].msg_buf_prt = 0;

	i2c->TOCTL = I2C_TOCTL_TOIF_Msk;
	i2c->TOCTL = I2C_TOCTL_TOCEN_Msk;
	I2C_SET_CONTROL_REG(i2c, I2C_CON_START);

	return VSFERR_NONE;
}

static void i2c_handler(I2C_T *i2c, struct i2c_ctrl_t *i2c_ctrl)
{
	if (i2c->TOCTL & I2C_TOCTL_TOIF_Msk)
		goto error;
	else if (i2c->CTL & I2C_CON_I2C_STS)
	{
		uint32_t status = i2c->STATUS;
		struct vsfhal_i2c_msg_t *msg = &i2c_ctrl->msg[i2c_ctrl->msg_prt];

		if (msg->flag & VSFHAL_I2C_READ)
		{
			if ((status == 0x08) || (status == 0x10))
			{
				i2c->DAT = (i2c_ctrl->chip_addr << 1) + 1;
				I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS);
			}
			else if (status == 0x40)
			{
				if (msg->len > 1)
				{
					// host reply ack
					I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS | I2C_CON_ACK);
				}
				else if (msg->len == 1)
				{
					// host reply nack
					I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS);
				}
				else
				{
					goto error;
				}
			}
			else if (status == 0x50)
			{
				if (i2c_ctrl->msg_buf_prt < msg->len)
					msg->buf[i2c_ctrl->msg_buf_prt++] = i2c->DAT;
				if (i2c_ctrl->msg_buf_prt < msg->len - 1)
				{
					// host reply ack
					I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS | I2C_CON_ACK);
				}
				else
				{
					// host reply nack
					I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS);
				}
			}
			else if (status == 0x58)
			{
				if (i2c_ctrl->msg_buf_prt < msg->len)
					msg->buf[i2c_ctrl->msg_buf_prt++] = i2c->DAT;

				if (++i2c_ctrl->msg_prt < i2c_ctrl->msg_len)
				{
					i2c_ctrl->msg_buf_prt = 0;
					I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS | I2C_CON_START);
				}
				else
				{
					I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS | I2C_CON_STOP);
					i2c->TOCTL = I2C_TOCTL_TOIF_Msk;
					if (i2c_ctrl->callback)
						i2c_ctrl->callback(i2c_ctrl->param, VSFERR_NONE);
				}
			}
			else
			{
				goto error;
			}
		}
		else
		{
			if ((status == 0x08) || (status == 0x10))	// start send finish
			{
				i2c->DAT = i2c_ctrl->chip_addr << 1;
				I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS);
			}
			else if ((status == 0x18) || (status == 0x28))	// addr/data send finish and ACK received
			{
				if (i2c_ctrl->msg_buf_prt < msg->len)
				{
					i2c->DAT = msg->buf[i2c_ctrl->msg_buf_prt++];
					I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS);
				}
				else
				{
					if (++i2c_ctrl->msg_prt < i2c_ctrl->msg_len)
					{
						i2c_ctrl->msg_buf_prt = 0;
						I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS | I2C_CON_START);
					}
					else
					{
						I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS | I2C_CON_STOP);
						i2c->TOCTL = I2C_TOCTL_TOIF_Msk;
						if (i2c_ctrl->callback)
							i2c_ctrl->callback(i2c_ctrl->param, VSFERR_NONE);				
					}
				}
			}
			else
			{
				goto error;
			}
		}
	}
	return;

error:
	I2C_SET_CONTROL_REG(i2c, I2C_CON_I2C_STS | I2C_CON_STOP);
	if (i2c_ctrl->callback)
		i2c_ctrl->callback(i2c_ctrl->param, VSFERR_FAIL);
	i2c->TOCTL = I2C_TOCTL_TOIF_Msk;
}

ROOTFUNC void I2C0_IRQHandler(void)
{
	i2c_handler(I2C0, &i2c_ctrl[0]);
}

ROOTFUNC void I2C1_IRQHandler(void)
{
	i2c_handler(I2C1, &i2c_ctrl[1]);
}

#endif


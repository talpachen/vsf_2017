#include "vsf.h"

#include "NUC472_442.h"
#include "NUC400_FLASH.h"

#define NUC400_FLASH_NUM					1

#define NUC400_FLASH_BASEADDR				0x00000000
#define NUC400_FLASH_ADDR(addr)				(NUC400_FLASH_BASEADDR + (addr))
#define NUC400_FLASH_SIZE_KB				(512)

extern void nuc400_unlock_reg(void);
extern void nuc400_lock_reg(void);

static vsf_err_t nuc400_flash_lock(uint8_t index)
{
	switch (index)
	{
	case 0:
		FMC->ISPCTL &=  ~FMC_ISPCTL_ISPEN_Msk;
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}

static vsf_err_t nuc400_flash_unlock(uint8_t index)
{
	switch (index)
	{
	case 0:
		FMC->ISPCTL |=  FMC_ISPCTL_ISPEN_Msk;
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}
	
vsf_err_t nuc400_flash_checkidx(uint8_t index)
{
	return (index < NUC400_FLASH_NUM) ? VSFERR_NONE : VSFERR_NOT_SUPPORT;
}

vsf_err_t nuc400_flash_capacity(uint8_t index, uint32_t *pagesize, 
		uint32_t *pagenum)
{
	switch (index)
	{
	case 0:
		if (NULL != pagesize)
		{
			*pagesize = 2 * 1024;
		}
		if (NULL != pagenum)
		{
			*pagenum = NUC400_FLASH_SIZE_KB / 2;
		}
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}

uint32_t nuc400_flash_baseaddr(uint8_t index)
{
	return NUC400_FLASH_BASEADDR;
}

// op -- operation: 0(ERASE), 1(READ), 2(WRITE)
uint32_t nuc400_flash_blocksize(uint8_t index, uint32_t addr, uint32_t size,
		int op)
{
	uint32_t pagesize;
	if (nuc400_flash_capacity(index, &pagesize, NULL))
		return 0;
	return !op ? pagesize : 4;
}

vsf_err_t nuc400_flash_init(uint8_t index)
{
	switch (index)
	{
	case 0:
		nuc400_unlock_reg();
		FMC->ISPCTL |= FMC_ISPCTL_APUEN_Msk;
		FMC->ISPCTL |= FMC_ISPCTL_ISPEN_Msk;
		nuc400_lock_reg();
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}

vsf_err_t nuc400_flash_fini(uint8_t index)
{
	switch (index)
	{
	case 0:
		return VSFERR_NONE;
	default:
		return VSFERR_NOT_SUPPORT;
	}
}

vsf_err_t nuc400_flash_erase(uint8_t index, uint32_t addr)
{
	switch (index)
	{
	case 0:
		nuc400_unlock_reg();
		FMC->ISPCMD = FMC_ISPCMD_PAGE_ERASE;
		FMC->ISPADDR = NUC400_FLASH_ADDR(addr);
		FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
		while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk);
		nuc400_lock_reg();
		break;
	default:
		return VSFERR_NOT_SUPPORT;
	}
	return VSFERR_NONE;
}

vsf_err_t nuc400_flash_read(uint8_t index, uint32_t addr, uint8_t *buff)
{
	return VSFERR_NOT_SUPPORT;
}

vsf_err_t nuc400_flash_write(uint8_t index, uint32_t addr, uint8_t *buff)
{
	switch (index)
	{
	case 0:
		nuc400_unlock_reg();
		FMC->ISPCMD = FMC_ISPCMD_WRITE;
		FMC->ISPADDR = NUC400_FLASH_ADDR(addr);
		FMC->MPDAT0 = *(uint32_t *)buff;
		FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
		while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk);
		nuc400_lock_reg();
		break;
	default:
		return VSFERR_NOT_SUPPORT;
	}
	return VSFERR_NONE;
}

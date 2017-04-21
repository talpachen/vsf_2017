#include "vsf.h"
#include "recovery.h"

vsf_err_t vsfhal_flash_init(uint8_t index);
vsf_err_t vsfhal_flash_fini(uint8_t index);
vsf_err_t vsfhal_flash_capacity(uint8_t index, uint32_t *pagesize, uint32_t *pagenum);
uint32_t vsfhal_flash_baseaddr(uint8_t index);
uint32_t vsfhal_flash_blocksize(uint8_t index, uint32_t addr, uint32_t size, int op);
vsf_err_t vsfhal_flash_config_cb(uint8_t index, uint32_t int_priority, void *param, void (*onfinish)(void*, vsf_err_t));
vsf_err_t vsfhal_flash_erase(uint8_t index, uint32_t addr);
vsf_err_t vsfhal_flash_read(uint8_t index, uint32_t addr, uint8_t *buff);
vsf_err_t vsfhal_flash_write(uint8_t index, uint32_t addr, uint8_t *buff);

static vsf_err_t recovery_write(struct recovery_t *info, uint8_t *buf, uint32_t addr, uint32_t size)
{

}

static vsf_err_t recovery_read(struct recovery_t *info, uint8_t *buf, uint32_t addr, uint32_t size)
{
	
}

static vsf_err_t recovery_erase(struct recovery_t *info, uint32_t addr, uint32_t size)
{
	
}

static uint32_t recovery_find_last_element(struct recovery_t *info)
{
	uint32_t group_start, last_element = 0xffffffff;
	
	for (group_start = 0; group_start < info->size; group_start += info->groupsize)
	{
		uint32_t cur, end;
		cur = group_start;
		end = cur + info->groupsize - info->elementsize;
		
		while (cur <= end)
		{
			uint32_t character = 0xffffffff;
			recovery_read(info, (uint8_t *)&character, cur, 4);
			if (character == info->character)
			{
				last_element = cur;
			}
			else if (last_element != 0xffffffff)
			{
				return last_element;
			}

			cur += info->elementsize;
		}
	}
	return last_element;
}

int32_t recovery_load(struct recovery_t *info, uint8_t *payload, int32_t size)
{
	uint32_t last_element = recovery_find_last_element(info);
	
	if (last_element != 0xffffffff)
	{
		recovery_read(info, payload, last_element + 4, size);
		return size;
	}
	else
		return -1;
}

int32_t recovery_store(struct recovery_t *info, uint8_t *payload, int32_t size)
{
	uint32_t last_element, new_element, group;
	
	if (info->playload_size_max < size)
		return -1;

	group = info->size / info->groupsize;
	
	last_element = recovery_find_last_element(info);
	
	if (last_element == 0xffffffff)
	{
		recovery_erase(info, info->addr, info->size);
		new_element = info->addr;
	}
	else
	{
		uint32_t cur_group, free;
		
		cur_group = (last_element - info->addr) / info->groupsize;
		free = (info->groupsize - (last_element - info->addr) % info->groupsize);
		free = free / info->elementsize - 1;
		
		if (free == 1)
		{
			cur_group++;
			if (cur_group >= group)
				cur_group = 0;			
			recovery_erase(info, info->addr + cur_group * info->groupsize,
					info->groupsize);
			new_element = last_element + info->elementsize;
		}
		else if (free >= 2)
		{
			new_element = last_element + info->elementsize;
		}
		else
		{
			cur_group++;
			if (cur_group >= group)
				cur_group = 0;
			new_element = info->addr + cur_group * info->groupsize;
		}
	}
	
	recovery_write(info, (uint8_t *)&info->character, new_element,
			sizeof(info->character));
	recovery_write(info, payload, new_element + info->playload_diff, size);
	return size;
}

vsf_err_t recovery_init(struct recovery_t *info)
{
	uint32_t pagenum, pagesize, temp, write_size;
	
	vsfhal_flash_init(info->index);
	vsfhal_flash_capacity(info->index, &pagesize, &pagenum);
	if (info->addr + info->size > pagesize * pagenum)
		return VSFERR_FAIL;
	
	pagesize = vsfhal_flash_blocksize(info->index, info->addr, info->size, 0);
	
	temp = info->addr + info->size;
	temp &= ~(pagesize - 1);
	info->addr = (info->addr + pagesize - 1) & ~(pagesize - 1);
	info->size = temp - info->addr;
	if (info->size < pagesize * 2)
		return VSFERR_FAIL;
	
	write_size = vsfhal_flash_blocksize(info->index, info->addr, info->size, 2);
	info->playload_diff = max(sizeof(info->character), write_size);
	info->elementsize = info->playload_diff + info->playload_size_max;
	info->elementsize += write_size - 1;
	info->elementsize &= ~(write_size - 1);

	if (pagesize >= info->elementsize)
	{
		info->groupsize = pagesize;
	}
	else
	{
		temp = (info->elementsize + pagesize - 1) / pagesize;
		info->groupsize = pagesize * temp;
		
		temp = info->size / info->groupsize;
		if (temp < 2)
			return VSFERR_FAIL;
		else
			info->size = info->groupsize * temp;
	}

	return VSFERR_NONE;
}


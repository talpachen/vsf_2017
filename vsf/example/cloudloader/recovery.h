#ifndef __RECOVERY_H_INCLUDED__
#define __RECOVERY_H_INCLUDED__

struct recovery_t
{
	uint8_t index;
	uint32_t addr;
	uint32_t size;
	uint32_t character;
	uint32_t playload_size_max;
	
	// private
	uint32_t groupsize;
	uint32_t elementsize;
	uint32_t playload_diff;
};

vsf_err_t recovery_init(struct recovery_t *info);
int32_t recovery_load(struct recovery_t *info, uint8_t *payload, int32_t size);
int32_t recovery_store(struct recovery_t *info, uint8_t *payload, int32_t size);
vsf_err_t recovery_clean(struct recovery_t *info);

#endif	// __RECOVERY_H_INCLUDED__


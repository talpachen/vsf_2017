#ifndef __CMEM7_CORE_H_INCLUDED__
#define __CMEM7_CORE_H_INCLUDED__

struct vsfhal_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;

	uint32_t cpu_freq_hz;
};

#endif	// __CMEM7_CORE_H_INCLUDED__


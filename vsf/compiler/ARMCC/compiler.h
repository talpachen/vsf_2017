#ifndef __COMPILER_H_INCLUDED__
#define __COMPILER_H_INCLUDED__

#include <string.h>

#define ROOTFUNC	
#define PACKED_HEAD	
#define PACKED_MID	__attribute__((packed))
#define PACKED_TAIL	
#define WEAKFUNC	__weak

#define vsf_gint_t						uint32_t
#define vsf_set_gint(gint)				__set_PRIMASK(gint)
#define vsf_get_gint()					__get_PRIMASK()
#define vsf_enter_critical()			__disable_irq()
#define vsf_leave_critical()			__enable_irq()

unsigned char * compiler_get_heap(void);
long compiler_get_heap_size(void);

#define compiler_set_pc(reg)			asm("MOV pc, %0" : :"r"(reg))
#define compiler_get_lr(reg)			asm("MOV %0, lr" : "=r"(reg))

#endif	// __COMPILER_H_INCLUDED__

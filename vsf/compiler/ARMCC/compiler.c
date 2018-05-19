#include "compiler.h"

__asm unsigned int __get_MSP(void)
{
  mrs r0, msp
  bx lr
}

__asm void __set_MSP(unsigned int mainStackPointer)
{
  msr msp, r0
  bx lr
}
#if 0
extern unsigned int __heap_base;
extern unsigned int __heap_limit;

unsigned char * compiler_get_heap(void)
{
	return (unsigned char *)__heap_base;
}

long compiler_get_heap_size(void)
{
	return __heap_limit - __heap_base;
}
#endif
#include "compiler.h"

#pragma segment="HEAP"
unsigned char * compiler_get_heap(void)
{
	return __sfb("HEAP");
}

long compiler_get_heap_size(void)
{
	return (long)__sfe("HEAP") - (long)__sfb("HEAP");
}

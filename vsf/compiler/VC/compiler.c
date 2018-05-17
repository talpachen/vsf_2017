#include "vsf.h"

static char heap[APPCFG_BUFMGR_SIZE];
unsigned char * compiler_get_heap(void)
{
	return heap;
}

long compiler_get_heap_size(void)
{
	return sizeof(heap);
}

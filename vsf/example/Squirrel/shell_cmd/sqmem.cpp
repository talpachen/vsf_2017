#include "vsf.h"
#include "squirrel3/squirrel/sqpcheader.h"

#ifndef SQ_EXCLUDE_DEFAULT_MEMFUNCTIONS
void *sq_vm_malloc(SQUnsignedInteger size)
{
	return vsf_bufmgr_malloc(size);
}

void *sq_vm_realloc(void *p, SQUnsignedInteger SQ_UNUSED_ARG(oldsize), SQUnsignedInteger size)
{
	void *n = vsf_bufmgr_malloc(size); 

	if (n)
		memcpy(n, p, oldsize);

	vsf_bufmgr_free(p);
	return n;
}

void sq_vm_free(void *p, SQUnsignedInteger SQ_UNUSED_ARG(size))
{
	vsf_bufmgr_free(p);
}
#endif

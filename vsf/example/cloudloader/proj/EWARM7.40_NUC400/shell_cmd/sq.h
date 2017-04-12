#ifndef __SQ_H__
#define __SQ_H__

#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>

#define SQ_MAXINPUT			1024
#define SQ_MAXOUTPUT		128

struct sq_param_t
{
	// private
	struct vsfshell_t *shell;
	struct vsf_stream_t *stream_tx;
	struct vsf_stream_t *stream_rx;
	
	SQChar buffer[SQ_MAXINPUT];
	HSQUIRRELVM v;
	SQInteger blocks;
	SQInteger string;
	SQInteger retval;
	SQInteger done;
	SQInteger ptr;
};

vsf_err_t sq_init(struct vsfshell_t *shell, struct sq_param_t *sq_param);

#endif // __SQ_H__
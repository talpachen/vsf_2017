#include "vsf.h"
#include "sq.h"

#define VSFSM_EVT_SQ_ON_RX		(VSFSM_EVT_USER_LOCAL + 1)
#define VSFSM_EVT_SQ_EXIT		(VSFSM_EVT_USER_LOCAL + 2)

static void sq_on_rx(void *p)
{
	vsfsm_post_evt_pending((struct vsfsm_t *)p, VSFSM_EVT_SQ_ON_RX);
}
static void sq_on_tx(void *p)
{
}

static struct sq_param_t *static_sq = NULL;
int32_t sq_out(uint8_t *buf, uint32_t size)
{
	struct vsf_buffer_t buffer;

	buffer.buffer = buf;
	buffer.size = stream_get_free_size(static_sq->stream_tx);
	if (buffer.size > size)
		buffer.size = size;
	
	return stream_write(static_sq->stream_tx, &buffer);
}

// retarget
int32_t printf(const char *format, ...)
{
	va_list vl;	
	uint8_t buf[SQ_MAXOUTPUT + 4];
	uint32_t size;

	va_start(vl, format);
	size = vsnprintf((char *)(buf + 1), SQ_MAXOUTPUT, format, vl);
	va_end(vl);
	
	if (size)
	{
		buf[0] = '\r';
		return sq_out(buf, size + 1);
	}

	return 0;
}

void sq_printfunc(HSQUIRRELVM SQ_UNUSED_ARG(v),const SQChar *s,...)
{
	va_list vl;	
	uint8_t buf[SQ_MAXOUTPUT + 4];
	uint32_t size;
	
    va_start(vl, s);
    size = vsnprintf((char *)(buf + 1), SQ_MAXOUTPUT, s, vl);
    va_end(vl);

	if (size)
	{
		buf[0] = '\r';
		sq_out(buf, size + 1);
	}
}

SQInteger quit(HSQUIRRELVM v)
{
    int *done;
    sq_getuserpointer(v, -1, (SQUserPointer*)&done);
    *done = 1;
    return 0;
}

static vsf_err_t sq_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
			(struct vsfshell_handler_param_t *)pt->user_data;
	struct sq_param_t *sq_param = param->context;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsf_buffer_t buffer;
	
	vsfsm_pt_begin(pt);

	if ((param->argc == 0x2) && (memcmp("-h", param->argv[1], 2) == 0))
	{
		vsfshell_printf(outpt, "Squirrel REPL\n\r");
		goto exit;
	}
	
	vsfsm_pt_delay(pt, 100);
	
	sq_param->stream_rx->callback_rx.param = pt->sm;
	sq_param->stream_rx->callback_rx.on_inout = sq_on_rx;
	sq_param->stream_rx->callback_rx.on_connect = NULL;
	sq_param->stream_rx->callback_rx.on_disconnect = NULL;
	sq_param->stream_tx->callback_tx.param = pt->sm;
	sq_param->stream_tx->callback_tx.on_inout = sq_on_tx;
	sq_param->stream_tx->callback_tx.on_connect = NULL;
	sq_param->stream_tx->callback_tx.on_disconnect = NULL;

	sq_param->v = sq_open(1024);
	sq_setprintfunc(sq_param->v, sq_printfunc, sq_printfunc);

	scprintf(_SC("%s %s (%d bits)\n"),SQUIRREL_VERSION,SQUIRREL_COPYRIGHT,((int)(sizeof(SQInteger)*8)));
	
	sq_pushroottable(sq_param->v);

	// standard library
	sqstd_register_bloblib(sq_param->v);
	//sqstd_register_iolib(sq_param->v);
	//sqstd_register_systemlib(sq_param->v);
	sqstd_register_mathlib(sq_param->v);
	sqstd_register_stringlib(sq_param->v);

	// custom library


	sqstd_seterrorhandlers(sq_param->v);

	sq_param->blocks = 0;
	sq_param->string = 0;
	sq_param->retval = 0;
	sq_param->done = 0;

	sq_pushroottable(sq_param->v);
	sq_pushstring(sq_param->v, _SC("quit"), -1);
	sq_pushuserpointer(sq_param->v, &sq_param->done);
	sq_newclosure(sq_param->v, quit, 1);
	sq_setparamscheck(sq_param->v, 1, NULL);
	sq_newslot(sq_param->v, -3, SQFalse);
	sq_pop(sq_param->v, 1);

	while (!sq_param->done)
	{
		sq_param->ptr = 0;
		scprintf(_SC("\r\nsq>"));
		
wait:
		vsfsm_pt_wfe(pt, VSFSM_EVT_SQ_ON_RX);
		
		while (1)
		{
			uint8_t c;
			buffer.buffer = &c;
			buffer.size = 1;
			buffer.size = stream_read(sq_param->stream_rx, &buffer);
			if (0 == buffer.size)
				goto wait;
			
			if (c == _SC('\b'))
			{
				if (sq_param->ptr)
				{
					sq_out("\b \b", 3);
					sq_param->ptr--;
				}
				continue;
			}
			
			sq_out(&c, 1);
			
			if ((c == _SC('\n')) || (c == _SC('\r')))
			{
				c = _SC('\n');
				sq_out(&c, 1);

				if (sq_param->ptr > 0 && sq_param->buffer[sq_param->ptr-1] == _SC('\\'))
				{
					sq_param->buffer[sq_param->ptr - 1] = _SC('\n');
				}
				else if(sq_param->blocks == 0)
				{				
					break;
				}
				sq_param->buffer[sq_param->ptr++] = _SC('\n');
			}
			else if (c == _SC('}'))
			{
				sq_param->blocks--;
				sq_param->buffer[sq_param->ptr++] = c;
			}
			else if (c==_SC('{') && !sq_param->string)
			{
				sq_param->blocks++;
				sq_param->buffer[sq_param->ptr++] = c;
			}
			else if(c == _SC('"') || c == _SC('\''))
			{
				sq_param->string = !sq_param->string;
				sq_param->buffer[sq_param->ptr++] = c;
			}
			else if (sq_param->ptr >= SQ_MAXINPUT - 1)
			{
				scprintf(_SC("sq error : input line too long\n"));
				break;
			}
			else
				sq_param->buffer[sq_param->ptr++] = c;
		}
		sq_param->buffer[sq_param->ptr] = _SC('\0');
		
		if(sq_param->buffer[0] == _SC('='))
		{
			scsprintf(sq_getscratchpad(sq_param->v, SQ_MAXINPUT), (size_t)SQ_MAXINPUT, _SC("return (%s)"), &sq_param->buffer[1]);
			memcpy(sq_param->buffer, sq_getscratchpad(sq_param->v,-1), (scstrlen(sq_getscratchpad(sq_param->v, -1)) + 1) * sizeof(SQChar));
			sq_param->retval=1;
		}
		
		sq_param->ptr = scstrlen(sq_param->buffer);
		if (sq_param->ptr)
		{
			SQInteger oldtop = sq_gettop(sq_param->v);
			if (SQ_SUCCEEDED(sq_compilebuffer(sq_param->v, sq_param->buffer, sq_param->ptr, _SC("interactive console"), SQTrue)))
			{
				sq_pushroottable(sq_param->v);
				if (SQ_SUCCEEDED(sq_call(sq_param->v, 1, sq_param->retval, SQTrue)) &&  sq_param->retval)
				{
					scprintf(_SC("\n"));
					sq_pushroottable(sq_param->v);
					sq_pushstring(sq_param->v, _SC("print"),-1);
					sq_get(sq_param->v, -2);
					sq_pushroottable(sq_param->v);
					sq_push(sq_param->v, -4);
					sq_call(sq_param->v, 2, SQFalse, SQTrue);
					sq_param->retval = 0;
					scprintf(_SC("\n"));
				}
			}
			sq_settop(sq_param->v, oldtop);
		}
	}
	
	sq_close(sq_param->v);
	
	vsfsm_pt_wfe(pt, VSFSM_EVT_SQ_EXIT);
exit:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t sq_init(struct vsfshell_t *shell, struct sq_param_t *sq_param)
{
	struct vsfshell_handler_t *handlers;
	
	static_sq = sq_param;
	
	sq_param->shell = shell;
	sq_param->stream_rx = shell->stream_rx;
	sq_param->stream_tx = shell->stream_tx;
	
	handlers = vsf_bufmgr_malloc(sizeof(struct vsfshell_handler_t));
	if (!handlers)
		return VSFERR_FAIL;
	memset(handlers, 0, sizeof(struct vsfshell_handler_t));
	
	handlers[0] = (struct vsfshell_handler_t)\
			{"sq", sq_thread, sq_param};
	vsfshell_register_handlers(shell, handlers, 1);
	
	return VSFERR_NONE;
}


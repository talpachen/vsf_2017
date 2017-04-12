#include "vsf.h"

static vsf_err_t help_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
			(struct vsfshell_handler_param_t *)pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	struct vsfshell_handler_t *handler = param->context;	
	
	vsfsm_pt_begin(pt);
	
	if (param->argc == 0x1)
	{
		vsfshell_printf(outpt, "Command list:" VSFSHELL_LINEEND);

		param->context = param->shell->handlers;
		handler = param->context;
		
		while (handler != NULL)
		{
			vsfshell_printf(outpt, "    %s\n\r", handler->name);		
			handler = handler->next;
		}
	}
	else if ((param->argc == 0x2) && (memcmp("-h", param->argv[1], 2) == 0))
	{
		vsfshell_printf(outpt, "    List all command." VSFSHELL_LINEEND);
	}
	else
	{
		vsfshell_printf(outpt, "    invalid option" VSFSHELL_LINEEND);
	}

	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t help_init(struct vsfshell_t *shell)
{
	struct vsfshell_handler_t *handlers;
	
	handlers = vsf_bufmgr_malloc(sizeof(struct vsfshell_handler_t));
	if (!handlers)
		return VSFERR_FAIL;
	memset(handlers, 0, sizeof(struct vsfshell_handler_t));
	
	handlers[0] = (struct vsfshell_handler_t){"help", help_thread};
	vsfshell_register_handlers(shell, handlers, 1);
	
	return VSFERR_NONE;
}


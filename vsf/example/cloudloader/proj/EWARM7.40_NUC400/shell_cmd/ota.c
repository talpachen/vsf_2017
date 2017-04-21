#include "vsf.h"
#include "ota.h"
#include "recovery.h"

static vsf_err_t get_info_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	
}

static vsf_err_t set_fw_key_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	
}

static vsf_err_t set_server_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	
}

static vsf_err_t connect_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	
}

static vsf_err_t help_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param = pt->user_data;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	
	vsfsm_pt_begin(pt);
	
	if (param->argc == 0x1)
	{
		// TODO
	}
	else if ((param->argc == 0x2) && (memcmp("-h", param->argv[1], 2) == 0))
	{
		vsfshell_printf(outpt, "    OTA Help." VSFSHELL_LINEEND);
	}
	else
	{
		vsfshell_printf(outpt, "    invalid option" VSFSHELL_LINEEND);
	}

	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

/*
	ota_get_info
	ota_set_fw_key -t AES128 -k 0123456789abcdef0123456789abcdef
	ota_set_fw_key -t AES256 -k 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
	ota_set_fw_key -t CHACHA20 -k 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
	ota_set_server -d xxx.xxx.xxx -p 1234
	ota_connect -t ESP8266_AT -u router_uuid -p router_passwd
*/

vsf_err_t ota_init(struct vsfshell_t *shell, struct ota_param_t *param)
{
	struct vsfshell_handler_t *handlers;

	param->shell = shell;
	param->stream_rx = shell->stream_rx;
	param->stream_tx = shell->stream_tx;

	handlers = vsf_bufmgr_malloc(sizeof(struct vsfshell_handler_t) * 5);
	if (!handlers)
		return VSFERR_FAIL;
	
	memset(handlers, 0, sizeof(struct vsfshell_handler_t));
	
	handlers[0] = (struct vsfshell_handler_t){"ota_connect", connect_thread, param};
	handlers[1] = (struct vsfshell_handler_t){"ota_set_server", set_server_thread, param};
	handlers[2] = (struct vsfshell_handler_t){"ota_set_fw_key", set_fw_key_thread, param};
	handlers[3] = (struct vsfshell_handler_t){"ota_get_info", get_info_thread, param};
	handlers[4] = (struct vsfshell_handler_t){"ota_help", help_thread, param};

	vsfshell_register_handlers(shell, handlers, 5);
	return VSFERR_NONE;
}


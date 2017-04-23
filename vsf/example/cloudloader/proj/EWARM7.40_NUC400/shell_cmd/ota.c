#include "vsf.h"
#include "ota.h"
#include "recovery.h"

static vsf_err_t info_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param = pt->user_data;
	struct ota_param_t *ota_param = param->context;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	
	vsfsm_pt_begin(pt);
	
	if (param->argc == 0x1)
	{
		vsfshell_printf(outpt, "OTA Info" VSFSHELL_LINEEND);
		vsfshell_printf(outpt, "    Target Address: 0x%08x" VSFSHELL_LINEEND,
				ota_param->target_area_addr);
		vsfshell_printf(outpt, "    Target Size: 0x%08x" VSFSHELL_LINEEND,
				ota_param->target_area_size);
		vsfshell_printf(outpt, "    UID: 0x%08x%08x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.uid[2], ota_param->store_info.uid[1], 
				ota_param->store_info.uid[0]);
		if (ota_param->store_info.key == 0xffffffff)
			vsfshell_printf(outpt, "    Key: 0xffffffff" VSFSHELL_LINEEND);
		else
			vsfshell_printf(outpt, "    Key: ********" VSFSHELL_LINEEND);
	}
	else if ((param->argc == 0x2) && (memcmp("-h", param->argv[1], 2) == 0))
	{
		vsfshell_printf(outpt, "    OTA Inof List" VSFSHELL_LINEEND);
	}
	else
	{
		vsfshell_printf(outpt, "    invalid option" VSFSHELL_LINEEND);
	}

	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t new_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param = pt->user_data;
	struct ota_param_t *ota_param = param->context;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	
	vsfsm_pt_begin(pt);
	
	if (param->argc == 0x1)
	{
		uint32_t i, op;
		
		vsfshell_printf(outpt, "OTA Create New Key" VSFSHELL_LINEEND);
		vsfshell_printf(outpt, "    Earse Target Area..." VSFSHELL_LINEEND);
		vsfsm_pt_delay(pt, 10);
		op = vsfhal_flash_blocksize(ota_param->target_area_index,
				ota_param->target_area_addr, ota_param->target_area_size, 0);
		for (i = 0; i < ota_param->target_area_size; i += op)
		{
			vsfhal_flash_erase(ota_param->target_area_index,
					ota_param->target_area_addr + i);
		}
		
		vsfshell_printf(outpt, "    Earse Recovery Area..." VSFSHELL_LINEEND);
		vsfsm_pt_delay(pt, 10);
		recovery_clean(&ota_param->recovery);
		
		vsfshell_printf(outpt, "    Reset Info..." VSFSHELL_LINEEND);
		memset(&ota_param->store_info, 0, sizeof(struct ota_store_info_t));
		vsfhal_uid_get((uint8_t *)ota_param->store_info.uid, 12);
		
		vsfshell_printf(outpt, "    Create New Key..." VSFSHELL_LINEEND);
		do
		{
			ota_param->store_info.key = vsfhal_tickclk_get_us();
			ota_param->store_info.key = ~ota_param->store_info.key;
			ota_param->store_info.key ^= vsfhal_tickclk_get_ms();
			ota_param->store_info.key *= vsfhal_tickclk_get_us();
			ota_param->store_info.key *= vsfhal_tickclk_get_us();
			ota_param->store_info.key *= vsfhal_tickclk_get_us();
			ota_param->store_info.key *= vsfhal_tickclk_get_us();
			ota_param->store_info.key ^= vsfhal_tickclk_get_ms();
		} while (ota_param->store_info.key == 0xffffffff);
		recovery_store(&ota_param->recovery, (uint8_t *)&ota_param->store_info,
				sizeof(struct ota_store_info_t));
		vsfshell_printf(outpt, "    Target Address: 0x%08x" VSFSHELL_LINEEND,
				ota_param->target_area_addr);
		vsfshell_printf(outpt, "    Target Size: 0x%08x" VSFSHELL_LINEEND,
				ota_param->target_area_size);
		vsfshell_printf(outpt, "    UID: 0x%08x%08x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.uid[2], ota_param->store_info.uid[1], 
				ota_param->store_info.uid[0]);
		vsfshell_printf(outpt, "    Key: 0x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.key);
	}
	else if ((param->argc == 0x2) && (memcmp("-h", param->argv[1], 2) == 0))
	{
		vsfshell_printf(outpt, "    OTA Create New Communication ID" VSFSHELL_LINEEND);
	}
	else
	{
		vsfshell_printf(outpt, "    invalid option" VSFSHELL_LINEEND);
	}

	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t set_fw_key_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param = pt->user_data;
	struct ota_param_t *ota_param = param->context;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	
	if ((param->argc == 0x2) && (memcmp("-h", param->argv[1], 2) == 0))
	{
		vsfshell_printf(outpt, "    OTA Firmware Key Set" VSFSHELL_LINEEND);
	}
	else if ((param->argc == 0x5) && (memcmp("-t", param->argv[1], 2) == 0) &&
			(memcmp("-k", param->argv[3], 2) == 0))
	{
		uint32_t i;
		char *argv;
		
		if (ota_param->store_info.fw_encrypt_type != ENCRYPT_TYPE_UNKNOWN)
		{
			vsfshell_printf(outpt, "    Permission Denied" VSFSHELL_LINEEND);
			goto exit;
		}
		
		if (strlen(param->argv[4]) > 32)
		{
			vsfshell_printf(outpt, "    Use up to 32 characters" VSFSHELL_LINEEND);
			goto exit;
		}
		else if (strlen(param->argv[4]) < 8)
		{
			vsfshell_printf(outpt, "    Use at least 8 characters" VSFSHELL_LINEEND);
			goto exit;
		}

		if ((memcmp("NULL", param->argv[2], 4) == 0))
		{
			ota_param->store_info.fw_encrypt_type = ENCRYPT_TYPE_NULL;
			vsfshell_printf(outpt, "    New Encrypt Type: NULL" VSFSHELL_LINEEND);
			goto exit;
		}
		else if ((memcmp("AES128", param->argv[2], 6) == 0))
		{
			ota_param->store_info.fw_encrypt_type = ENCRYPT_TYPE_AES128;
			vsfshell_printf(outpt, "    New Encrypt Type: AES128" VSFSHELL_LINEEND);
		}
		else if ((memcmp("AES256", param->argv[2], 6) == 0))
		{
			ota_param->store_info.fw_encrypt_type = ENCRYPT_TYPE_AES256;
			vsfshell_printf(outpt, "    New Encrypt Type: AES256" VSFSHELL_LINEEND);
		}
		else if ((memcmp("CHACHA20", param->argv[2], 8) == 0))
		{
			ota_param->store_info.fw_encrypt_type = ENCRYPT_TYPE_CHACHA20;
			vsfshell_printf(outpt, "    New Encrypt Type: CHACHA20" VSFSHELL_LINEEND);
		}
		else
		{
			vsfshell_printf(outpt, "    invalid encrypt type" VSFSHELL_LINEEND);
			goto exit;
		}
		
		memset(ota_param->store_info.fw_encrypt_key, 0xff, 32);
		argv = param->argv[4];
		for (i = 0; i < 32; i++)
		{
			char c = *argv++;
			
			if (c != 0)			
				((char *)ota_param->store_info.fw_encrypt_key)[i] = c;
			else
				break;
		}
		
		ota_param->store_info.fw_encrypt_key[2] ^= ota_param->store_info.fw_encrypt_key[0];
		ota_param->store_info.fw_encrypt_key[3] ^= ota_param->store_info.fw_encrypt_key[1];
		ota_param->store_info.fw_encrypt_key[4] ^= ota_param->store_info.fw_encrypt_key[0] * ota_param->store_info.fw_encrypt_key[1];
		ota_param->store_info.fw_encrypt_key[5] ^= ota_param->store_info.fw_encrypt_key[1] * ota_param->store_info.fw_encrypt_key[2];
		ota_param->store_info.fw_encrypt_key[6] ^= ota_param->store_info.fw_encrypt_key[2] * ota_param->store_info.fw_encrypt_key[3];
		ota_param->store_info.fw_encrypt_key[7] ^= ota_param->store_info.fw_encrypt_key[3] * ota_param->store_info.fw_encrypt_key[0];
		ota_param->store_info.fw_encrypt_key[0] ^= ((uint32_t)42767 << 16) + 42767;
		ota_param->store_info.fw_encrypt_key[1] ^= ((uint32_t)42773 << 16) + 42773;
		ota_param->store_info.fw_encrypt_key[2] ^= ((uint32_t)42787 << 16) + 42787;
		ota_param->store_info.fw_encrypt_key[3] ^= ((uint32_t)42793 << 16) + 42793;
		ota_param->store_info.fw_encrypt_key[4] ^= ((uint32_t)42797 << 16) + 42797;
		ota_param->store_info.fw_encrypt_key[5] ^= ((uint32_t)42821 << 16) + 42821;
		ota_param->store_info.fw_encrypt_key[6] ^= ((uint32_t)42829 << 16) + 42829;
		ota_param->store_info.fw_encrypt_key[7] ^= ((uint32_t)42839 << 16) + 42839;	
		
		recovery_store(&ota_param->recovery, (uint8_t *)&ota_param->store_info,
				sizeof(struct ota_store_info_t));
		
		vsfshell_printf(outpt, "    Key[0]: 0x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.fw_encrypt_key[0]);
		vsfshell_printf(outpt, "    Key[1]: 0x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.fw_encrypt_key[1]);
		vsfshell_printf(outpt, "    Key[2]: 0x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.fw_encrypt_key[2]);
		vsfshell_printf(outpt, "    Key[3]: 0x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.fw_encrypt_key[3]);
		vsfshell_printf(outpt, "    Key[4]: 0x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.fw_encrypt_key[4]);
		vsfshell_printf(outpt, "    Key[5]: 0x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.fw_encrypt_key[5]);
		vsfshell_printf(outpt, "    Key[6]: 0x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.fw_encrypt_key[6]);
		vsfshell_printf(outpt, "    Key[7]: 0x%08x" VSFSHELL_LINEEND,
				ota_param->store_info.fw_encrypt_key[7]);
	}
	else
	{
		vsfshell_printf(outpt, "    invalid option" VSFSHELL_LINEEND);
	}

exit:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t set_server_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param = pt->user_data;
	struct ota_param_t *ota_param = param->context;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	
	if ((param->argc == 0x2) && (memcmp("-h", param->argv[1], 2) == 0))
	{
		vsfshell_printf(outpt, "    OTA Server Set" VSFSHELL_LINEEND);
	}
	else if ((param->argc == 0x5) && (memcmp("-p", param->argv[3], 2) == 0))
	{
		uint8_t i;
		char *argv;
		
		if ((strlen(param->argv[2]) > 32))
		{
			vsfshell_printf(outpt, "    Use up to 32 characters" VSFSHELL_LINEEND);
			goto exit;
		}
		
		if (memcmp("-d", param->argv[1], 2) == 0)
		{
			ota_param->store_info.server_type = SERVER_TYPE_DOMAIN;
			memcpy(ota_param->store_info.server, param->argv[2], strlen(param->argv[2]));
		}
		else if (memcmp("-ipv4", param->argv[1], 5) == 0)
		{
			ota_param->store_info.server_type = SERVER_TYPE_IPV4;
			memcpy(ota_param->store_info.server, param->argv[2], strlen(param->argv[2]));
		}
		else
		{
			vsfshell_printf(outpt, "    invalid domain or ip" VSFSHELL_LINEEND);
			goto exit;
		}
		
		ota_param->store_info.server_port = 0;
		argv = param->argv[3];
		for (i = 0; i < 5; i++)
		{
			char v = *argv++;
			if ((v >= '0') && (v <= '9'))
			{
				v -= '0';
				ota_param->store_info.server_port *= 10;
				ota_param->store_info.server_port += v;
			}
			else
				break;				
		}
		
		if (ota_param->store_info.server_port <= 65535)
		{
			recovery_store(&ota_param->recovery, (uint8_t *)&ota_param->store_info,
					sizeof(struct ota_store_info_t));
			
			if (ota_param->store_info.server_type == SERVER_TYPE_DOMAIN)
				vsfshell_printf(outpt, "    Server Domain: %s" VSFSHELL_LINEEND, ota_param->store_info.server);
			else if (ota_param->store_info.server_type == SERVER_TYPE_IPV4)
				vsfshell_printf(outpt, "    Server IPv4: %s" VSFSHELL_LINEEND, ota_param->store_info.server);
			vsfshell_printf(outpt, "    Server Port: %d" VSFSHELL_LINEEND, ota_param->store_info.server_port);
		}
		else
		{
			vsfshell_printf(outpt, "    invalid port" VSFSHELL_LINEEND);
			ota_param->store_info.server_type = SERVER_TYPE_UNKNOWN;
		}
	}
	else
	{
		vsfshell_printf(outpt, "    invalid option" VSFSHELL_LINEEND);
	}

exit:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t connect_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param = pt->user_data;
	struct ota_param_t *ota_param = param->context;
	struct vsfsm_pt_t *outpt = &param->output_pt;

	vsfsm_pt_begin(pt);
	
	if ((param->argc == 0x2) && (memcmp("-h", param->argv[1], 2) == 0))
	{
		vsfshell_printf(outpt, "    OTA Server Set" VSFSHELL_LINEEND);
	}
	// TODO
	else
	{
		vsfshell_printf(outpt, "    invalid option" VSFSHELL_LINEEND);
	}

exit:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

/*
	ota_info
	ota_new
	ota_set_fw_key -t AES128 -k 012345678
		or ota_set_fw_key -t AES256 -k 012345678
		or ota_set_fw_key -t CHACHA20 -k 012345678
	ota_set_server -d xxx.xxx.xxx -p 1234
		or ota_set_server -ipv4 x.x.x.x -p 1234
	ota_connect -t ESP8266_AT -u router_uuid -p router_passwd
*/

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

vsf_err_t ota_init(struct vsfshell_t *shell, struct ota_param_t *param)
{
	struct vsfshell_handler_t *handlers;

	param->shell = shell;
	param->stream_rx = shell->stream_rx;
	param->stream_tx = shell->stream_tx;

	handlers = vsf_bufmgr_malloc(sizeof(struct vsfshell_handler_t) * 6);
	if (!handlers)
		return VSFERR_FAIL;
	
	memset(handlers, 0, sizeof(struct vsfshell_handler_t));
	
	handlers[0] = (struct vsfshell_handler_t){"ota_connect", connect_thread, param};
	handlers[1] = (struct vsfshell_handler_t){"ota_set_server", set_server_thread, param};
	handlers[2] = (struct vsfshell_handler_t){"ota_set_fw_key", set_fw_key_thread, param};
	handlers[3] = (struct vsfshell_handler_t){"ota_info", info_thread, param};
	handlers[4] = (struct vsfshell_handler_t){"ota_new", new_thread, param};
	handlers[5] = (struct vsfshell_handler_t){"ota_help", help_thread, param};

	vsfshell_register_handlers(shell, handlers, 6);
	
	vsfhal_flash_init(param->target_area_index);
	
	recovery_init(&param->recovery);
	if (recovery_load(&param->recovery, (uint8_t *)&param->store_info,
			sizeof(struct ota_store_info_t)) != sizeof(struct ota_store_info_t))
	{
		vsfhal_uid_get((uint8_t *)param->store_info.uid, 12);
		param->store_info.key = 0xffffffff;
	}

	return VSFERR_NONE;
}


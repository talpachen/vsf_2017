#ifndef __OTA_H__
#define __OTA_H__

#include "recovery.h"

struct ota_store_info_t
{
	uint32_t uid[3];
	uint32_t key;
	
	uint32_t fw_encrypt_type;
#define ENCRYPT_TYPE_UNKNOWN			0
#define ENCRYPT_TYPE_NULL				1
#define ENCRYPT_TYPE_AES128				2
#define ENCRYPT_TYPE_AES256				3
#define ENCRYPT_TYPE_CHACHA20			4
	uint32_t fw_encrypt_key[8];
	
	uint32_t server_type;
#define SERVER_TYPE_UNKNOWN				0
#define SERVER_TYPE_DOMAIN				0
#define SERVER_TYPE_IPV4				1
#define SERVER_TYPE_IPV6				2
	char server[32 + 4];
	uint32_t server_port;
	
	char router_uuid[32 + 4];
	char router_passwd[32 + 4];
};

struct ota_param_t
{
	uint8_t index_num;
	uint8_t index;
	uint8_t mode;
	uint16_t int_priority;
	uint32_t baudrate;

	uint8_t target_area_index;
	uint32_t target_area_addr;
	uint32_t target_area_size;
	
	struct recovery_t recovery;
	// private
	struct vsfshell_t *shell;
	struct vsf_stream_t *stream_tx;
	struct vsf_stream_t *stream_rx;
	
	struct ota_store_info_t store_info;
};

vsf_err_t ota_init(struct vsfshell_t *shell, struct ota_param_t *param);

#endif // __OTA_H__


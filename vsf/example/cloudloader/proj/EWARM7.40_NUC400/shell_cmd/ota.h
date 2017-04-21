#ifndef __OTA_H__
#define __OTA_H__

#include "recovery.h"

struct ota_param_t
{
	uint8_t index_num;
	uint8_t index;
	uint8_t mode;
	uint16_t int_priority;
	uint32_t baudrate;

	struct recovery_t recovery;
	// private
	struct vsfshell_t *shell;
	struct vsf_stream_t *stream_tx;
	struct vsf_stream_t *stream_rx;
};

vsf_err_t ota_init(struct vsfshell_t *shell, struct ota_param_t *param);

#endif // __OTA_H__


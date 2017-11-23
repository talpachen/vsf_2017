#ifndef __CDCUART_H__
#define __CDCUART_H__

struct cdcuart_param_t
{
	uint8_t index;
	uint8_t mode;
	uint16_t int_priority;
	uint32_t baudrate;

	struct vsfusbd_CDCACM_param_t *param;
	
	// private
	struct vsfshell_t *shell;
	struct vsf_stream_t *stream_tx;
	struct vsf_stream_t *stream_rx;
};

vsf_err_t cdcuart_init(struct vsfshell_t *shell,
		struct cdcuart_param_t *param);

#endif // __CDCUART_H__


#include "vsf.h"
#include "cdcuart.h"

#if VSFHAL_USART_EN

#define VSFSM_EVT_CDCUART_EXIT		(VSFSM_EVT_USER_LOCAL + 1)
#define USART_BUF_SIZE				16

static void uart_rx_int(void *p, uint16_t data)
{
	uint8_t buf[USART_BUF_SIZE];
	struct vsf_buffer_t buffer;
	struct cdcuart_param_t *param = p;

	buffer.buffer = buf;
	buf[0] = data & 0xff;
	buffer.size = vsfhal_usart_rx_get_data_size(param->index);
	if (buffer.size)
	{
		buffer.size = min(buffer.size, USART_BUF_SIZE - 1);
		buffer.size = vsfhal_usart_rx_bytes(param->index, &buf[1], buffer.size);
	}
	buffer.size++;
	stream_write(param->stream_tx, &buffer);
}

static void uart_on_tx(void *p)
{
	uint8_t buf[USART_BUF_SIZE];
	struct vsf_buffer_t buffer;
	struct cdcuart_param_t *param = p;
	
	buffer.size = min(vsfhal_usart_tx_get_free_size(param->index),
			USART_BUF_SIZE);
	if (buffer.size)
	{
		buffer.buffer = buf;
		buffer.size = stream_read(param->stream_rx, &buffer);
		if (buffer.size)
		{
			vsfhal_usart_tx_bytes(param->index, buf, buffer.size);
		}
	}
}

static void uart_on_rx(void *p)
{
	uint8_t buf[USART_BUF_SIZE];
	struct vsf_buffer_t buffer;
	struct cdcuart_param_t *param = p;

	buffer.size = vsfhal_usart_rx_get_data_size(param->index);
	if (buffer.size)
	{
		buffer.buffer = buf;
		buffer.size = min(buffer.size, USART_BUF_SIZE);
		buffer.size = vsfhal_usart_rx_bytes(param->index, &buf[0], buffer.size);
		stream_write(param->stream_tx, &buffer);	
	}	
}

static vsf_err_t uart_active(struct cdcuart_param_t *param)
{
	if (param->index == VSFHAL_DUMMY_PORT)
		return VSFERR_INVALID_PARAMETER;
	
	param->stream_rx->callback_rx.param = param;
	param->stream_rx->callback_rx.on_inout = uart_on_tx;
	param->stream_rx->callback_rx.on_connect = NULL;
	param->stream_rx->callback_rx.on_disconnect = NULL;
	param->stream_tx->callback_tx.param = param;
	param->stream_tx->callback_tx.on_inout = uart_on_rx;
	param->stream_tx->callback_tx.on_connect = NULL;
	param->stream_tx->callback_tx.on_disconnect = NULL;
	
	vsfhal_usart_init(param->index);
	vsfhal_usart_config_cb(param->index, param->int_priority, param, uart_on_tx,
			uart_rx_int);
	vsfhal_usart_config(param->index, param->baudrate, param->mode);
	
	return VSFERR_NONE;
}

static vsf_err_t cdcuart_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfshell_handler_param_t *param =
			(struct vsfshell_handler_param_t *)pt->user_data;
	struct cdcuart_param_t *cdcuart_param = param->context;
	struct vsfsm_pt_t *outpt = &param->output_pt;
	uint8_t index, i;
	uint32_t baud;

	vsfsm_pt_begin(pt);

	if ((param->argc == 0x2) && (memcmp("-h", param->argv[1], 2) == 0))
	{
		vsfshell_printf(outpt, "Connect CDC to UART\n\r");
		vsfshell_printf(outpt, "    Usage:\n\r");
		vsfshell_printf(outpt, "    \"cdcuart\" Connect uart%d(default) with CDC baudrate\n\r", cdcuart_param->index_table[0]);
		cdcuart_param->index = 0;
		while (cdcuart_param->index < cdcuart_param->index_num)
		{
			vsfshell_printf(outpt, "    \"cdcuart -u %d\" Connect uart%d with CDC baudrate\n\r", cdcuart_param->index, cdcuart_param->index);
			cdcuart_param->index++;
		}
		vsfshell_printf(outpt, "    \"cdcuart -b 115200\" Connect uart%d with custom baudrate\n\r", cdcuart_param->index_table[0]);
		goto exit;
	}

	index = 0;
	baud = cdcuart_param->param->line_coding.bitrate;

	for (i = 1; i < param->argc - 1; i += 2)
	{
		if (memcmp("-u", param->argv[i], 2) == 0)
		{
			if (param->argv[i + 1][0] >= '0' && param->argv[i + 1][0] <= '9')
			{
				index = param->argv[i + 1][0] - '0';
				
				if (index < cdcuart_param->index_num)
					cdcuart_param->index = cdcuart_param->index_table[index];
				else
					goto invalid;
			}
			else
				goto invalid;
		}
		else if (memcmp("-b", param->argv[i], 2) == 0)
		{
			uint8_t j;
			baud = 0;
			for (j = 0; j < 6; j++)
			{
				if (param->argv[i + 1][j] != '\0')
				{
					if ((param->argv[i + 1][j] <= '9') &&
							(param->argv[i + 1][j] >= '0'))
						baud = baud * 10 + param->argv[i + 1][j] - '0';
					else
						goto invalid;
				}
				else
					break;
			}
		}
		else
			goto invalid;
	}
	
	cdcuart_param->index = cdcuart_param->index_table[index];
	cdcuart_param->baudrate = baud;
	
	vsfshell_printf(outpt, "Connect UART%d, Baudrate: %d\n\r", index, baud);
	
	vsfsm_pt_delay(pt, 100);
	uart_active(cdcuart_param);
	vsfsm_pt_wfe(pt, VSFSM_EVT_CDCUART_EXIT);		
	goto exit;
invalid:
	vsfshell_printf(outpt, "    invalid option" VSFSHELL_LINEEND);
exit:
	vsfshell_handler_exit(param);
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t cdcuart_init(struct vsfshell_t *shell,
		struct cdcuart_param_t *cdcuart_param)
{
	struct vsfshell_handler_t *handlers;
	
	cdcuart_param->shell = shell;
	cdcuart_param->stream_rx = shell->stream_rx;
	cdcuart_param->stream_tx = shell->stream_tx;
		
	handlers = vsf_bufmgr_malloc(sizeof(struct vsfshell_handler_t));
	if (!handlers)
		return VSFERR_FAIL;
	
	memset(handlers, 0, sizeof(struct vsfshell_handler_t));
	
	handlers[0] = (struct vsfshell_handler_t)\
			{"cdcuart", cdcuart_thread, cdcuart_param};
	vsfshell_register_handlers(shell, handlers, 1);
	
	return VSFERR_NONE;
}

#endif


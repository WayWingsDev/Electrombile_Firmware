/*
 * uart.c
 *
 *  Created on: 2015Äê7ÔÂ8ÈÕ
 *      Author: jk
 */

#include <eat_interface.h>
#include <eat_uart.h>

#include "uart.h"
#include "log.h"

void event_uart_ready_rd(EatEvent_st* event)
{
	u16 length = 0;
	EatUart_enum uart = event->data.uart.uart;
	u8 buf[1024] = {0};	//TODO: use macro

	length = eat_uart_read(uart, buf, 1024);
	if (length)
	{
		buf[length] = 0;
		LOG_DEBUG("uart recv: %s", buf);
	}
	else
	{
		return;
	}

	if (strstr(buf, "reboot"))
	{
		eat_reset_module();
		return;
	}
}

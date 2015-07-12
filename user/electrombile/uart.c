/*
 * uart.c
 *
 *  Created on: 2015Äê7ÔÂ8ÈÕ
 *      Author: jk
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <eat_interface.h>
#include <eat_uart.h>
#include <eat_modem.h>

#include "uart.h"
#include "log.h"

int event_uart_ready_rd(const EatEvent_st* event)
{
	u16 length = 0;
	EatUart_enum uart = event->data.uart.uart;
	char buf[1024] = {0};	//TODO: use macro

	length = eat_uart_read(uart, buf, 1024);
	if (length)
	{
		buf[length] = 0;
		LOG_DEBUG("uart recv: %s", buf);
	}
	else
	{
		return 0;
	}

	if (strstr(buf, "reboot"))
	{
		eat_reset_module();
		return 0;
	}

	if (strstr(buf, "halt"))
	{
		eat_power_down();
		return 0;
	}

	if (strstr(buf, "version"))
	{
		LOG_INFO("%s", eat_get_version());
		return 0;
	}

	if (strstr(buf, "build"))
	{
		LOG_INFO("build time = %s, build No. = %s", eat_get_buildtime(), eat_get_buildno());
		return 0;
	}

    if (strstr(buf, "imei"))
    {
        u8 imei[16] = {0};
        eat_get_imei(imei, 15);
        LOG_INFO("IMEI = %s", imei);
        return 0;
    }


    if (strstr(buf, "rtc"))
    {
        EatRtc_st rtc = {0};
        eat_bool result = eat_get_rtc(&rtc);
        if (result)
        {
            LOG_INFO("RTC timer: %d-%02d-%02d %02d:%02d:%02d", rtc.year, rtc.mon, rtc.day, rtc.hour, rtc.min, rtc.sec);
        }
        else
        {
            LOG_ERROR("Get rtc time failed:%d", result);
        }
        return 0;
    }

	//forward AT command to modem
	if (strstr(buf, "AT"))
	{
		eat_modem_write(buf, length);
		return 0;
	}

	return 0;
}

void print(const char* fmt, ...)
{
    char buf[1024] = {0};
    int length = 0;

    va_list arg;
    va_start (arg, fmt);
    vsnprintf(buf, 1024, fmt, arg);
    va_end (arg);

    length = strlen(buf);

    eat_uart_write(EAT_UART_1, buf, length);
}

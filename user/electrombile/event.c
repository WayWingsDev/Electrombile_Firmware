/*
 * event.c
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#include <eat_interface.h>
#include <eat_uart.h>

#include "timer.h"
#include "watchdog.h"
#include "thread_msg.h"
#include "log.h"
#include "uart.h"
#include "socket.h"
#include "setting.h"
#include "msg.h"
#include "data.h"
#include "client.h"

typedef int (*EVENT_FUNC)(const EatEvent_st* event);
typedef struct
{
	EatEvent_enum event;
	EVENT_FUNC pfn;
}EVENT_PROC;


/*
 * local event function definition
 */
int event_timer(const EatEvent_st* event);
int event_threadMsg(const EatEvent_st* event);
void event_mod_ready_rd(EatEvent_st* event);

static EVENT_PROC msgProcs[] =
{
    {EAT_EVENT_TIMER,				event_timer},
    {EAT_EVENT_MDM_READY_RD,        event_mod_ready_rd},
    {EAT_EVENT_MDM_READY_WR,        EAT_NULL},
    {EAT_EVENT_UART_READY_RD,       event_uart_ready_rd},
    {EAT_EVENT_UART_READY_WR,		EAT_NULL},
    {EAT_EVENT_UART_SEND_COMPLETE,	EAT_NULL},
    {EAT_EVENT_USER_MSG,            event_threadMsg},
};


#define DESC_DEF(x) case x:\
                            return #x

static char* getEventDescription(EatEvent_enum event)
{
    switch (event)
    {
        DESC_DEF(EAT_EVENT_TIMER);
        DESC_DEF(EAT_EVENT_KEY);
        DESC_DEF(EAT_EVENT_INT);
        DESC_DEF(EAT_EVENT_MDM_READY_RD);
        DESC_DEF(EAT_EVENT_MDM_READY_WR);
        DESC_DEF(EAT_EVENT_MDM_RI);
        DESC_DEF(EAT_EVENT_UART_READY_RD);
        DESC_DEF(EAT_EVENT_UART_READY_WR);
        DESC_DEF(EAT_EVENT_ADC);
        DESC_DEF(EAT_EVENT_UART_SEND_COMPLETE);
        DESC_DEF(EAT_EVENT_USER_MSG);
        DESC_DEF(EAT_EVENT_IME_KEY);
        default:
        {
            static char soc_event[10] = {0};
            sprintf(soc_event, "%d", event);
            return soc_event;
        }
    }
}
void event_mod_ready_rd(EatEvent_st* event)
{
	u8 buf[256] = {0};
	u16 len = 0;
	u8* buf_ptr = NULL;

	len = eat_modem_read(buf, 256);
	LOG_DEBUG("modem recv: %s", buf);

	buf_ptr = (u8*) strstr((const char *) buf, "+CGATT: 1");
	if (buf_ptr != NULL)
	{
		socket_init();
		eat_timer_stop(TIMER_AT_CMD);
	}
 
}


int event_proc(EatEvent_st* event)
{
	int i = 0;

    LOG_DEBUG("event: %s", getEventDescription(event->event));

	for (i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
	{
		if (msgProcs[i].event == event->event)
		{
			EVENT_FUNC pfn = msgProcs[i].pfn;
			if (pfn)
			{
				return pfn(event);
			}
			else
			{
		        LOG_ERROR("event(%d) not processed!", event->event);
			}
		}
	}

	return -1;
}

int event_timer(const EatEvent_st* event)
{
    switch (event->data.timer.timer_id)
    {
        case TIMER_WATCHDOG:
            LOG_INFO("TIMER_WATCHDOG expire!");
            feedWatchdog();
            eat_timer_start(event->data.timer.timer_id, 50000);
            break;

        case TIMER_AT_CMD:
            LOG_INFO("TIMER_AT_CMD expire!");
            eat_modem_write("AT+CGATT?\n", 10);
            eat_timer_start(event->data.timer.timer_id, 5000);
            break;

        case TIMER_GPS_SEND:
            LOG_INFO("TIMER_GPS_SEND expire!");
            eat_timer_start(event->data.timer.timer_id, setting.gps_timer_period);
            client_loop();
            break;

        default:
            LOG_ERROR ("timer(%d) not processed", event->data.timer.timer_id);
            break;
    }
    return 0;
}

int event_threadMsg(const EatEvent_st* event)
{
    MSG_THREAD* msg = (MSG_THREAD*) event->data.user_msg.data_p;
    u8 msgLen = event->data.user_msg.len;

    switch (msg->cmd)
    {
        case CMD_THREAD_GPS:
        {
            GPS* gps = (GPS*) msg->data;
            if (!gps)
            {
                break;
            }
            LOG_DEBUG("receive thread command CMD_GPS_UPDATE: lat(%f), lng(%f)", gps->latitude, gps->longitude);

            //update the local GPS data
            data.gps = *gps;

            freeMsg(msg);

            break;
        }

        case CMD_THREAD_SMS:
            LOG_DEBUG("receive thread command CMD_SMS");
            break;

        case CMD_THREAD_VIBRATE:
            LOG_DEBUG("receive thread command CMD_VIBRATE");
            break;
        default:
            LOG_ERROR("unknown thread command:%d", msg->cmd);
    }

}


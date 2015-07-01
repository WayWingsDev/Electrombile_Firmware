/*
 * event.c
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#include <eat_interface.h>
#include "timer.h"
#include "watchdog.h"

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


static EVENT_PROC msgProcs[] =
{
		{EAT_EVENT_TIMER,				event_timer},
		{EAT_EVENT_MDM_READY_RD,        EAT_NULL},
		{EAT_EVENT_MDM_READY_WR,        EAT_NULL},
		{EAT_EVENT_UART_READY_RD,       EAT_NULL},
		{EAT_EVENT_UART_SEND_COMPLETE,	EAT_NULL},
		{EAT_EVENT_USER_MSG,            event_threadMsg},
};



int event_proc(EatEvent_st* event)
{
	int i = 0;
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
		        eat_trace("[%s] event(%d) not processed!", __FUNCTION__, event->event);
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
			eat_trace("TIMER_WATCHDOG expire!");
			feedWatchdog();
			eat_timer_start(event->data.timer.timer_id, 50000);
			break;
		default:
			eat_trace("[%s] timer(%d) not processed", __FUNCTION__, event->data.timer.timer_id);
			break;
	}
	return 0;
}

int event_threadMsg(const EatEvent_st* event)
{

}


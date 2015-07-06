/*
 * event.c
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#include <eat_interface.h>
#include "timer.h"
#include "watchdog.h"
#include "msg.h"
#include "eat_socket.h"
#include "log.h"

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
static void socket_init(void);
void event_mod_ready_rd();
void bear_notify_cb(cbm_bearer_state_enum state,u8 ip_addr[4]);
void soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size);;

static s8 socket_id = 0;
static EVENT_PROC msgProcs[] =
{
		{EAT_EVENT_TIMER,				event_timer},
		{EAT_EVENT_MDM_READY_RD,        event_mod_ready_rd},
		{EAT_EVENT_MDM_READY_WR,        EAT_NULL},
		{EAT_EVENT_UART_READY_RD,       EAT_NULL},
		{EAT_EVENT_UART_SEND_COMPLETE,	EAT_NULL},
		{EAT_EVENT_USER_MSG,            event_threadMsg},
};


static void socket_init(void)
{
	s8 rc = eat_gprs_bearer_open("CMNET",NULL,NULL,bear_notify_cb);

	if (rc == CBM_OK)
	{
		rc = eat_gprs_bearer_hold();
		if (rc != CBM_OK)
		{
			eat_trace("[%s] hold bearer failed", __FUNCTION__);
		}
	}
	else if (rc == CBM_WOULDBLOCK)
	{
		eat_trace("[%s] opening bearer...", __FUNCTION__);
	}
	else
	{
		eat_trace("[%s] open bearer failed", __FUNCTION__);
		//TODO: reboot
	}
}
void bear_notify_cb(cbm_bearer_state_enum state,u8 ip_addr[4])
{
	s8 rc = 0;
	eat_bool val = EAT_TRUE;
	sockaddr_struct address={SOC_SOCK_STREAM};

	eat_trace("[%s] BEAR_NOTIFY: %d", __FUNCTION__, state);

	if (state & CBM_ACTIVATED)
	{
		eat_trace("[%s] ip: %d.%d.%d.%d", __FUNCTION__, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);

        eat_soc_notify_register(soc_notify_cb);
        socket_id = eat_soc_create(SOC_SOCK_STREAM,0);
        if (socket_id < 0)
        {
    		eat_trace("[%s] eat_soc_create return error :%d", __FUNCTION__, socket_id);
    		//TODO: error handle
    		return;
        }

        rc = eat_soc_setsockopt(socket_id, SOC_NBIO, &val, sizeof(val));
        if (rc != SOC_SUCCESS)
        {
        	eat_trace("[%s] eat_soc_setsockopt set SOC_NBIO failed: %d", __FUNCTION__, rc);
        	//TODO: error handle
        	return;
        }

        rc = eat_soc_setsockopt(socket_id, SOC_NODELAY, &val, sizeof(val));
        if (rc != SOC_SUCCESS)
        {
			eat_trace("[%s] eat_soc_setsockopt set SOC_NODELAY failed: %d", __FUNCTION__, rc);
        }

        val = SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT;
        rc = eat_soc_setsockopt(socket_id, SOC_ASYNC, &val, sizeof(val));
        if (rc != SOC_SUCCESS)
        {
        	eat_trace("[%s] eat_soc_setsockopt set SOC_ASYNC failed: %d", __FUNCTION__, rc);
        	//TODO: error handle
        	return;
        }

        address.sock_type = SOC_SOCK_STREAM;
        address.addr_len = 4;
        address.port = 43210;                /* TCP server port */
        address.addr[0]=120;                /* TCP server ip address */
        address.addr[1]=25;
        address.addr[2]=157;
        address.addr[3]=233;

        rc = eat_soc_connect(socket_id, &address);
        if(rc >= 0)
        {
			eat_trace("NEW Connection ID is :%d", rc);
		}
		else if (rc == SOC_WOULDBLOCK)
		{
			eat_trace("Connection is in progressing");
		}
		else
		{
			eat_trace("Connect return error:%d", rc);
		}
	}
}
void soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size)
{

}


void event_mod_ready_rd()
{
    u8 buf[2048];
    u16 len;
    u8* buf_ptr = NULL;
    /*param:%d,extern_param:%d*/
     len = eat_modem_read(buf, 2048);
     eat_trace("modem read=%s",buf);
     buf_ptr = (u8*)strstr((const char *)buf,"+CGATT: 1");
    if( buf_ptr != NULL)
    {
        socket_init();
   //     eat_timer_stop(EAT_TIMER_6);
    
    }
 
}

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
		        LOG_ERROR("event(%d) not processed!", event->event);
			}
		}
	}

	return -1;
}

int event_timer(const EatEvent_st* event)
{
s32 ret;
	switch (event->data.timer.timer_id)
	{
		case TIMER_WATCHDOG:
			LOG_INFO("TIMER_WATCHDOG expire!");
			feedWatchdog();
			eat_timer_start(event->data.timer.timer_id, 50000);
			break;
              case EAT_TIMER_6:
                 
            ret = eat_soc_send(socket_id,"eat socket test",15);
            if (ret < 0)
                eat_trace("eat_soc_send return error :%d",ret);
            else
                eat_trace("eat_soc_send success :%d",ret);
                     eat_modem_write("AT+CGATT?\n",10);
                     break;
		default:
			LOG_ERROR ("timer(%d) not processed", event->data.timer.timer_id);
			break;
	}
	return 0;
}

int event_threadMsg(const EatEvent_st* event)
{
	MSG* msg = (MSG*)event->data.user_msg.data_p;
	u8 msgLen = event->data.user_msg.len;

	switch (msg->cmd)
	{
	case CMD_GPS_UPDATE:
	{
		MSG_GPS* gps = (MSG_GPS*)msg->data;
              
		if (!gps)
		{
			break;
		}
		LOG_DEBUG("receive thread command CMD_GPS_UPDATE: lat(%f), lng(%f)", gps->latitude, gps->longitude);
		break;
	}

	case CMD_SMS:
		LOG_DEBUG("receive thread command CMD_SMS");
		break;
	default:
		LOG_ERROR("unknown thread command:%d", msg->cmd);
	}

}


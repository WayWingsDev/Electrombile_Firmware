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

u8 *SOC_EVENT[]={
    "SOC_READ",
    "SOC_WRITE",  
    "SOC_ACCEPT", 
    "SOC_CONNECT",
    "SOC_CLOSE", 
    "SOC_ACKED"
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
    u8 buffer[128] = {0};
    u8 id = 0;
    if(event&SOC_READ) {id = 0;
        socket_id = s;
    }
    else if (event&SOC_WRITE) id = 1;
    else if (event&SOC_ACCEPT) id = 2;
    else if (event&SOC_CONNECT) id = 3;
    else if (event&SOC_CLOSE){ id = 4;
        eat_soc_close(s);
    }
    else if (event&SOC_ACKED) id = 5;
    if (id == 5)
        sprintf(buffer,"SOC_NOTIFY:%d,%s,%d\r\n",s,SOC_EVENT[id],ack_size);
    else 
        sprintf(buffer,"SOC_NOTIFY:%d,%s,%d\r\n",s,SOC_EVENT[id],result);
    eat_uart_write(EAT_UART_1,buffer,strlen(buffer));

    if(SOC_ACCEPT==event){
        u8 val = 0;
        s8 ret = 0;
        sockaddr_struct clientAddr={0};
        s8 newsocket = eat_soc_accept(s,&clientAddr);
        if (newsocket < 0){
            eat_trace("eat_soc_accept return error :%d",newsocket);
        }
        else{
            sprintf(buffer,"client accept:%s,%d:%d:%d:%d\r\n",clientAddr.addr[0],clientAddr.addr[1],clientAddr.addr[2],clientAddr.addr[3]);
        }

        val = TRUE;
        ret = eat_soc_setsockopt(socket_id, SOC_NODELAY, &val, sizeof(val));
        if (ret != SOC_SUCCESS)
            eat_trace("eat_soc_setsockopt SOC_NODELAY return error :%d",ret);

    }

    eat_trace("soc_notify_cb");

}


void event_mod_ready_rd()
{
    u8 buf[2048];
    u16 len;
    u8* buf_ptr = NULL;
    /*param:%d,extern_param:%d*/
     len = eat_modem_read(buf, 2048);
     buf_ptr = (u8*)strstr((const char *)buf,"+CGATT: 1");
    if( buf_ptr != NULL)
    {
        socket_init();
        eat_timer_stop(TIMER_AT_ENVELOPE_TIMER1);
    
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
	switch (event->data.timer.timer_id)
	{
		case TIMER_WATCHDOG:
			LOG_INFO("TIMER_WATCHDOG expire!");
			feedWatchdog();
			eat_timer_start(event->data.timer.timer_id, 50000);
			break;
              case TIMER_AT_ENVELOPE_TIMER1:
                     LOG_INFO("TIMER_AT_ENVELOPE_TIMER1 expire!");
                     eat_modem_write("AT+CGATT?\n",10);
                     eat_timer_start(event->data.timer.timer_id, 5000);
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
       u8 buffer[1046] = {0};
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
       case CMD_VIBRATE:
              LOG_DEBUG("receive thread command CMD_VIBRATE");              
              break;
	default:
		LOG_ERROR("unknown thread command:%d", msg->cmd);
	}

}


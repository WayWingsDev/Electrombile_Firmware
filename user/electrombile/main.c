/*
 * main.c
 *
 *  Created on: Jul 24, 2015
 *      Author: jk
 */

#include <stdio.h>
#include <string.h>
#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "eat_timer.h" 
#include "eat_socket.h"
#include "eat_clib_define.h" //only in main.c

#include "event.h"
#include "gps.h"
#include "watchdog.h"
#include "timer.h"
#include "setting.h"
#include "log.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define EAT_UART_RX_BUF_LEN_MAX 2048
#define EAT_MEM_MAX_SIZE 100*1024


/********************************************************************
 * Types
 ********************************************************************/
typedef void (*app_user_func)(void*);

/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/
 
/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/

static u8 s_memPool[EAT_MEM_MAX_SIZE];
static s8 socket_id = 0;


/********************************************************************
 * External Functions declaration
 ********************************************************************/
extern void APP_InitRegions(void);

/********************************************************************
 * Local Function declaration
 ********************************************************************/
void app_main(void *data);
void app_func_ext1(void *data);

static void socket_init(void);
static void bear_notify_cb(cbm_bearer_state_enum state,u8 ip_addr[4]);
static void soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size);
/********************************************************************
 * Local Function
 ********************************************************************/
#pragma arm section rodata = "APP_CFG"
APP_ENTRY_FLAG 
#pragma arm section rodata

#pragma arm section rodata="APPENTRY"
	const EatEntry_st AppEntry = 
	{
		app_main,
		app_func_ext1,
		(app_user_func)app_gps_thread,//app_user1,
		(app_user_func)EAT_NULL,//app_user2,
		(app_user_func)EAT_NULL,//app_user3,
		(app_user_func)EAT_NULL,//app_user4,
		(app_user_func)EAT_NULL,//app_user5,
		(app_user_func)EAT_NULL,//app_user6,
		(app_user_func)EAT_NULL,//app_user7,
		(app_user_func)EAT_NULL,//app_user8,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL
	};
#pragma arm section rodata

void app_func_ext1(void *data)
{
	/*This function can be called before Task running ,configure the GPIO,uart and etc.
	   Only these api can be used:
		 eat_uart_set_debug: set debug port
		 eat_pin_set_mode: set GPIO mode
		 eat_uart_set_at_port: set AT port
	*/
  
    EatUartConfig_st cfg =
    {
        EAT_UART_BAUD_115200,
        EAT_UART_DATA_BITS_8,
        EAT_UART_STOP_BITS_1,
        EAT_UART_PARITY_NONE
    };
    
    eat_uart_set_at_port(EAT_UART_1);// UART1 is as AT PORT
  
	eat_uart_set_debug(EAT_UART_USB);
    eat_uart_set_debug_config(EAT_UART_DEBUG_MODE_UART, &cfg);
}


void app_main(void *data)
{
    EatEvent_st event;
    eat_bool rc;

    APP_InitRegions();//Init app RAM
    APP_init_clib(); //C library initialize, second step

    eat_trace(" app_main ENTRY");
    rc = eat_mem_init(s_memPool,EAT_MEM_MAX_SIZE);
    if (!rc)
    {
    	LOG_ERROR("eat memory initial error:%d!", rc);
    }

    SETTING_initial();

    socket_init();

    startWatchdog();
    eat_timer_start(TIMER_WATCHDOG, 50000);


    while(EAT_TRUE)
    {
    	unsigned int event_num = eat_get_event_num();
    	if (event_num != 0)
    	{
    		int i = 0;
    		for (i = 0; i < event_num; i++)
    		{
    			eat_get_event(&event);
    	        eat_trace("MSG id:%x", event.event);

    	        event_proc(&event);
    		}
    	}

    	//poll
    }
}


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
        address.port = 6789;                /* TCP server port */
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



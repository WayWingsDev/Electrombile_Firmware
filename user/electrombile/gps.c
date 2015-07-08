/*
 * gps.c
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */
#include <string.h>
#include <stdlib.h>

#include <eat_interface.h>
#include <eat_gps.h>


#include "gps.h"
#include "timer.h"
#include "thread.h"
#include "thread_msg.h"
#include "log.h"
#include "setting.h"

#define NMEA_BUFF_SIZE 1024
static char gps_info_buf[NMEA_BUFF_SIZE]="";

static void gps_timer_handler();
static eat_bool gps_sendGPS(double lat,double lng);


void app_gps_thread(void *data)
{
    EatEvent_st event;

    eat_gps_power_req(EAT_TRUE);

    LOG_INFO("gps current sleep mode %d", eat_gps_sleep_read());

    eat_timer_start(TIMER_GPS, setting.gps_timer_period);

    while(EAT_TRUE)
    {
        eat_get_event_for_user(THREAD_GPS, &event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER :
                switch ( event.data.timer.timer_id )
                {
                    case TIMER_GPS:
                    	LOG_INFO("IMER_GPS expire!");
                        gps_timer_handler();
                        eat_timer_start(event.data.timer.timer_id, setting.gps_timer_period);
                        break;


                    default:
                    	LOG_ERROR("ERR: timer[%d] expire!", event.data.timer.timer_id);

                        break;
                }
                break;

            default:
            	LOG_ERROR("event(%d) not processed", event.event);
                break;

        }
    }
}

static void gps_timer_handler()
{
	char * GPSIM_data[9];
	char *buffer;
	double lat,lon;
	char gps_fix;
	int i;
	eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPGGA, gps_info_buf,NMEA_BUFF_SIZE);
	LOG_DEBUG("EAT_NMEA_OUTPUT_SIMCOM:%s", gps_info_buf);

	buffer = gps_info_buf;
	for(i=0;i<7;i++)
	{
		GPSIM_data[i] = strtok(buffer,",");
		buffer = NULL;		
	}
	lat = atof(GPSIM_data[2]);
	lon =  atof(GPSIM_data[4]);
	gps_fix = atoi(GPSIM_data[6]);
	if(!gps_fix)
	{
		LOG_ERROR("GPS not fixed");
		return;
	}
	
	LOG_DEBUG("lat=%f,lon=%f,fix=%d",lat,lon,gps_fix);
	gps_sendGPS(lat,lon);
}

static eat_bool gps_sendMsg2Main(MSG* msg, u8 len)
{
    return sendMsg(THREAD_GPS, THREAD_MAIN, msg, len);
}

static eat_bool gps_sendGPS(double lat,double lng)
{
    u8 msgLen = sizeof(MSG) + sizeof(MSG_GPS);
    MSG* msg = allocMsg(msgLen);
    MSG_GPS* gps = (MSG_GPS*)msg->data;

    msg->cmd = CMD_GPS_UPDATE;
    msg->length = sizeof(MSG_GPS);
    
    gps->latitude = lat;
    gps->longitude = lng;

    return gps_sendMsg2Main(msg, msgLen);
}


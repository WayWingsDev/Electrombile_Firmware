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
#include "protocol.h"

#define NMEA_BUFF_SIZE 1024
static char gps_info_buf[NMEA_BUFF_SIZE]="";

static void gps_timer_handler();
static eat_bool gps_sendGPS(GPS *gps);
static void geo_fence_proc_cb(char *msg_buf, u8 len);

void app_gps_thread(void *data)
{
    EatEvent_st event;

    eat_gps_power_req(EAT_TRUE);

    LOG_INFO("gps current sleep mode %d", eat_gps_sleep_read());

    eat_gps_register_msg_proc_callback(geo_fence_proc_cb);
    eat_timer_start(TIMER_GPS, setting.gps_timer_period);
    eat_modem_write("AT+CENG=3,1\r",12);
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
        GPS *gps = eat_mem_alloc(sizeof(GPS));
        int i;
        eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPGGA, gps_info_buf,NMEA_BUFF_SIZE);
        LOG_DEBUG("EAT_NMEA_OUTPUT_SIMCOM:%s", gps_info_buf);

        buffer = gps_info_buf;
        for(i=0;i<7;i++)
        {
        	GPSIM_data[i] = strtok(buffer,",");
        	buffer = NULL;		
        }
        gps->latitude= atof(GPSIM_data[2]);
        gps->longitude=  atof(GPSIM_data[4]);
        gps->gps_fix= atoi(GPSIM_data[6]);
        if(!gps->gps_fix)
        {
            u8 buf[2048];
            u16 len;
            LOG_ERROR("GPS not fixed");
            eat_modem_write("AT+CENG?\r",9);
            len = eat_modem_read(buf,2048);
            if(len>0)
            {
                LOG_DEBUG("AT+CENG?");
            }
       }

        LOG_DEBUG("lat=%f,lon=%f,fix=%d",gps->latitude, gps->longitude,gps->gps_fix);
        gps_sendGPS(gps);
}

static eat_bool gps_sendMsg2Main(MSG_THREAD* msg, u8 len)
{
    return sendMsg(THREAD_GPS, THREAD_MAIN, msg, len);
}

static eat_bool gps_sendGPS(GPS *gps)
{
    u8 msgLen = sizeof(MSG_THREAD) + sizeof(GPS);
    GPS *gpskk;
    MSG_THREAD* msg = allocMsg(msgLen);
    memcpy(msg->data,gps,sizeof(GPS));
    eat_mem_free(gps);
    gpskk = (GPS *)msg->data;
    
    msg->cmd = CMD_THREAD_GPS;
    msg->length = sizeof(MSG_GPS);
    LOG_DEBUG("send gps: lat(%f), lng(%f)", gpskk->latitude, gpskk->longitude);
    return gps_sendMsg2Main(msg, msgLen);
}

static void geo_fence_proc_cb(char *msg_buf, u8 len)
{
    //TODO:
}


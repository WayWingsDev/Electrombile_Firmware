/*
 * gps.c
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#include <eat_interface.h>
#include <eat_gps.h>

#include "gps.h"
#include "timer.h"
#include "thread.h"

#define NMEA_BUFF_SIZE 1024
static char gps_info_buf[NMEA_BUFF_SIZE]="";

static void gps_timer_handler();

void app_gps_thread(void *data)
{
    EatEvent_st event;

    eat_gps_power_req(EAT_TRUE);

    eat_trace("gps current sleep mode %d", eat_gps_sleep_read());

    eat_timer_start(TIMER_GPS, 30000);

    while(EAT_TRUE)
    {
        eat_get_event_for_user(THREAD_GPS, &event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER :

                switch ( event.data.timer.timer_id )
                {
                    case TIMER_GPS:
                        eat_trace("INFO: TIMER_GPS expire!");
                        gps_timer_handler();
                        eat_timer_start(event.data.timer.timer_id, 30000);
                        break;


                    default:
                        eat_trace("ERR: timer[%d] expire!");

                        break;
                }
                break;

            case EAT_EVENT_MDM_READY_RD:
//                eat_modem_ready_read_handler();
                break;

            case EAT_EVENT_MDM_READY_WR:

                break;
            case EAT_EVENT_USER_MSG:

                break;
            default:
                break;

        }
    }
}

static void gps_timer_handler()
{
    eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_SIMCOM, gps_info_buf,NMEA_BUFF_SIZE);
    eat_trace("EAT_NMEA_OUTPUT_SIMCOM=%s", gps_info_buf);

    eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPGGA, gps_info_buf,NMEA_BUFF_SIZE);
    eat_trace("EAT_NMEA_OUTPUT_GPGGA=%s", gps_info_buf);

    eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPGLL, gps_info_buf,NMEA_BUFF_SIZE);
    eat_trace("EAT_NMEA_OUTPUT_GPGLL=%s", gps_info_buf);

    eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPGSA, gps_info_buf,NMEA_BUFF_SIZE);
    eat_trace("EAT_NMEA_OUTPUT_GPGSA=%s", gps_info_buf);

    eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPGSV, gps_info_buf,NMEA_BUFF_SIZE);
    eat_trace("EAT_NMEA_OUTPUT_GPGSV=%s", gps_info_buf);

    eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPRMC, gps_info_buf,NMEA_BUFF_SIZE);
    eat_trace("EAT_NMEA_OUTPUT_GPRMC=%s", gps_info_buf);

    eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPVTG, gps_info_buf,NMEA_BUFF_SIZE);
    eat_trace("EAT_NMEA_OUTPUT_GPVTG=%s", gps_info_buf);

    eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPZDA, gps_info_buf,NMEA_BUFF_SIZE);
    eat_trace("EAT_NMEA_OUTPUT_GPZDA=%s", gps_info_buf);
}

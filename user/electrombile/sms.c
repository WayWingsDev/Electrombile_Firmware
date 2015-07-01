/*
 * sms.c
 *
 *  Created on: 2015/6/24
 *      Author: jk
 */

#include "sms.h"
#include "log.h"

void app_sms_thread(void *data)
{
    EatEvent_st event;

    eat_gps_power_req(EAT_TRUE);

    LOG_DEBUG("gps current sleep mode %d", eat_gps_sleep_read());

    while(EAT_TRUE)
    {
        eat_get_event_for_user(EAT_USER_1, &event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER :

                switch ( event.data.timer.timer_id )
                {

                    default:
                    	LOG_ERROR("ERR: timer[%d] expire!");

                        break;
                }
                break;

            case EAT_EVENT_MDM_READY_RD:
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

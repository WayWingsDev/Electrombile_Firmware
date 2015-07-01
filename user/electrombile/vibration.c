//
// Created by jk on 2015/7/1.
//

#include "vibration.h"
#include "thread.h"
#include "log.h"


void app_vibration_thread(void *data)
{
    EatEvent_st event;

    LOG_DEBUG("Vibration thread start");

    while(EAT_TRUE)
    {
        eat_get_event_for_user(TRHEAD_SMS, &event);
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

/*
 * sms.c
 *
 *  Created on: 2015/6/24
 *      Author: jk
 */

#include "sms.h"
#include "log.h"
#include "thread.h"
#include <stdio.h>
#include <string.h>
#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"
#include "eat_sms.h"

static void eat_check_sms_pdu_string(u16 length, u8 *bytes, u8 *str)
{
    u16 i = 0;
    u16 j = 0;
  
    if((str == NULL) || (bytes == NULL))
    {
        eat_trace("eat_check_sms_pdu_string() failed");
        return;    
    }
    
    while (i < length)
    {
        j += sprintf((char*)str + j, "%02x", bytes[i]);
        i++;
    }
    str[j] = 0;
    //EatToUpper(str);
}

static eat_sms_flash_message_cb(EatSmsReadCnf_st smsFlashMessage)
{
    u8 format =0;

    eat_get_sms_format(&format);
    eat_trace("eat_sms_flash_message_cb, format=%d",format);
    if(1 == format)//TEXTģʽ
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsFlashMessage.data);
        eat_trace("eat_sms_read_cb, datetime=%s",smsFlashMessage.datetime);
        eat_trace("eat_sms_read_cb, name=%s",smsFlashMessage.name);
        eat_trace("eat_sms_read_cb, status=%d",smsFlashMessage.status);
        eat_trace("eat_sms_read_cb, len=%d",smsFlashMessage.len);
        eat_trace("eat_sms_read_cb, number=%s",smsFlashMessage.number);
    }
    else//PDUģʽ
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsFlashMessage.data);
        eat_trace("eat_sms_read_cb, len=%d",smsFlashMessage.len);
    }
}


static void eat_sms_read_cb(EatSmsReadCnf_st  smsReadCnfContent)
{
    u8 format =0;

    eat_get_sms_format(&format);
    eat_trace("eat_sms_read_cb, format=%d",format);
    if(1 == format)//TEXTģʽ
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsReadCnfContent.data);
        eat_trace("eat_sms_read_cb, datetime=%s",smsReadCnfContent.datetime);
        eat_trace("eat_sms_read_cb, name=%s",smsReadCnfContent.name);
        eat_trace("eat_sms_read_cb, status=%d",smsReadCnfContent.status);
        eat_trace("eat_sms_read_cb, len=%d",smsReadCnfContent.len);
        eat_trace("eat_sms_read_cb, number=%s",smsReadCnfContent.number);
    }
    else//PDUģʽ
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsReadCnfContent.data);
        eat_trace("eat_sms_read_cb, name=%s",smsReadCnfContent.name);
        eat_trace("eat_sms_read_cb, status=%d",smsReadCnfContent.status);
        eat_trace("eat_sms_read_cb, len=%d",smsReadCnfContent.len);
    }
}

static void eat_sms_delete_cb(eat_bool result)
{
    eat_trace("eat_sms_delete_cb, result=%d",result);
}

static void eat_sms_send_cb(eat_bool result)
{
    eat_trace("eat_sms_send_cb, result=%d",result);
}

static eat_sms_new_message_cb(EatSmsNewMessageInd_st smsNewMessage)
{
    eat_trace("eat_sms_new_message_cb, storage=%d,index=%d",smsNewMessage.storage,smsNewMessage.index);
}

static void eat_sms_ready_cb(eat_bool result)
{
    eat_trace("eat_sms_ready_cb, result=%d",result);
}

void app_sms_thread(void *data)
{
    EatEvent_st event;

    LOG_DEBUG("SMS thread start");
    eat_set_sms_operation_mode(EAT_TRUE);
    eat_sms_register_new_message_callback(eat_sms_new_message_cb);
    eat_sms_register_flash_message_callback(eat_sms_flash_message_cb);
    eat_sms_register_send_completed_callback(eat_sms_send_cb);
    eat_sms_register_sms_ready_callback(eat_sms_ready_cb);
    eat_set_sms_format(1);
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

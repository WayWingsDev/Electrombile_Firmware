/*
 * sms.c
 *
 *  Created on: 2015/6/24
 *      Author: jk
 */

 /*
    server,set,1,www.sky200.com,32001
    server,get


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
#include "setting.h"
#include <stdlib.h>  //strtok()必须加载这个头函数

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
    if(1 == format)//TEXT模式
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsFlashMessage.data);
        eat_trace("eat_sms_read_cb, datetime=%s",smsFlashMessage.datetime);
        eat_trace("eat_sms_read_cb, name=%s",smsFlashMessage.name);
        eat_trace("eat_sms_read_cb, status=%d",smsFlashMessage.status);
        eat_trace("eat_sms_read_cb, len=%d",smsFlashMessage.len);
        eat_trace("eat_sms_read_cb, number=%s",smsFlashMessage.number);
    }
    else//PDU模式
    {
        eat_trace("eat_sms_read_cb, msg=%s",smsFlashMessage.data);
        eat_trace("eat_sms_read_cb, len=%d",smsFlashMessage.len);
    }
}
static void recv_server_proc(EatSmsReadCnf_st  smsReadCnfContent)
{
        char *p = smsReadCnfContent.data;
        char ack_message[64]={0};
        char*  ptr;
        u8 n=0;
        u8 type;
        char *buf[5] = NULL;
        u16 port;            
        if(ptr = strstr(p,"get"))//get cmd
        {
            if (setting.addr_type == ADDR_TYPE_IP)
            {
                sprintf(ack_message,"server,%d.%d.%d.%d,%d",setting.addr.ipaddr[0],setting.addr.ipaddr[1],setting.addr.ipaddr[2],setting.addr.ipaddr[3],setting.port);
            }
            else
            {
                sprintf(ack_message,"server,%s,%d",setting.addr.domain,setting.port);
            }
            eat_send_text_sms(smsReadCnfContent.number,ack_message);
        }            
        if(ptr = strstr(p,"set"))//set cmd
        {
            
            LOG_DEBUG("%s",p);
            buf[n] = strtok(p,",");
            for(n=1;n<5;n++)
            {
                buf[n] = strtok(NULL,",");
                LOG_DEBUG("%s",buf[n]);
                if(NULL==buf[n])
                {
                    return;
                }
            }
            type = atoi(buf[2]);
            port = atoi(buf[4]);
            if(type==1)
            {
                setting.addr_type = ADDR_TYPE_DOMAIN;
                setting.port = port;
                LOG_DEBUG("buf3=%s,len=%d",buf[3],strlen(buf[3]));
                memset(setting.addr.domain,0,MAX_DOMAIN_NAME_LEN);
                memcpy(setting.addr.domain,buf[3],strlen(buf[3]));
                LOG_DEBUG("setting.addr.type=%d,domain=%s,port=%d",setting.addr_type,setting.addr.domain,setting.port);
            }
            else
            {                 
                int addr[4];                 
                for(n=0;n<4;n++)
                {
                    ptr= strtok(buf[3],".");
                    if(NULL==ptr)
                    {
                        return;
                    }
                    addr[n] = atoi(ptr);
                    buf[3] = NULL;                        
                }
                setting.addr_type = ADDR_TYPE_IP;
                setting.port = port;//.不能用                    
                LOG_DEBUG("setting.addr.type=%d,%d:%d:%d:%d,port=%d",setting.addr_type,setting.addr.ipaddr[0],setting.addr.ipaddr[1],setting.addr.ipaddr[2],setting.addr.ipaddr[3],setting.port);
            }
             if(!setting_save())
                return;
             eat_send_text_sms(smsReadCnfContent.number,"set server ok");
             
        }
                
 }
static void eat_sms_read_cb(EatSmsReadCnf_st  smsReadCnfContent)
{
    u8 format =0;
    char *p = smsReadCnfContent.data;
    char*  ptr;
    eat_get_sms_format(&format);
    if(1 == format)//TEXT模式
    {       
        if(ptr=strstr(p,"server"))//服务器命令
        {
            recv_server_proc(smsReadCnfContent);
        }
        if(ptr=strstr(p,"timer"))
        {
            //TO DO
        }
    }
    else//PDU模式
    {
        LOG_INFO("recv pdu sms");
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
     eat_read_sms(smsNewMessage.index,eat_sms_read_cb);
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

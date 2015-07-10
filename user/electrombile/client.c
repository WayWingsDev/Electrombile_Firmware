/*
 * client.c
 *
 *  Created on: 2015Äê7ÔÂ9ÈÕ
 *      Author: jk
 */
#include <stdio.h>
#include <string.h>

#include <eat_interface.h>

#include "client.h"
#include "socket.h"
#include "msg.h"
#include "log.h"
#include "uart.h"
#include "data.h"


typedef int (*MSG_PROC)(const void* msg);
typedef struct
{
    char cmd;
    MSG_PROC pfn;
}MC_MSG_PROC;


static int mc_login_rsp(const void* msg);
static int mc_ping_rsp(const void* msg);
static int mc_alarm_rsp(const void* msg);
static int mc_sms(const void* msg);


static MC_MSG_PROC msgProcs[] =
{
        {CMD_LOGIN, mc_login_rsp},
        {CMD_PING,  mc_ping_rsp},
        {CMD_ALARM, mc_alarm_rsp},
        {CMD_SMS,   mc_sms},
};


static void print_hex(const char* data, int length)
{
    int i = 0, j = 0;

    print("   ");
    for (i  = 0; i < 16; i++)
    {
        print("%X ", i);
    }
    print("    ");
    for (i = 0; i < 16; i++)
    {
        print("%X", i);
    }

    print("\r\n");

    for (i = 0; i < length; i += 16)
    {
        print("%02d  ", i % 16 + 1);
        for (j = i; j < 16 && j < length; j++)
        {
            print("%02x ", data[j] & 0xff);
        }
        print("    ");
        for (j = i; j < 16 && j < length; j++)
        {
            print("%c ", data[j] & 0xff);
        }

        print("\r\n");
    }

    print("%02d  ", length % 16 + 1);
    for (; j < length; j++)
    {
        print("%c ", data[j] & 0xff);
    }
}

int client_proc(const void* m, int msgLen)
{
    MSG_HEADER* msg = (MSG_HEADER*)m;
    size_t i = 0;

    print_hex(m, msgLen);

    if (msgLen < sizeof(MSG_HEADER))
    {
        LOG_ERROR("receive message length not enough: %zu(at least(%zu)", msgLen, sizeof(MSG_HEADER));

        return -1;
    }

    if (msg->signature != ntohs(START_FLAG))
    {
        LOG_ERROR("receive message head signature error:%d", msg->signature);
        return -1;
    }

    for (i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
    {
        if (msgProcs[i].cmd == msg->cmd)
        {
            MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                return pfn(msg);
            }
            else
            {
                LOG_ERROR("Message %d not processed", msg->cmd);
                return -1;
            }
        }
    }

    LOG_ERROR("unknown message %d", msg->cmd);
    return -1;
}

int client_loop(void)
{
    if (socket_conneted())
    {
        if (client_logined())
        {
            MSG_GPS* msg = alloc_msg(CMD_GPS, sizeof(MSG_GPS));

            if (!msg)
            {
                LOG_ERROR("alloc message failed");
            }

            msg->gps = data.gps;
            socket_sendData(msg, sizeof(MSG_GPS));
            LOG_DEBUG("send GPS message");
        }
        else
        {
            MSG_LOGIN_REQ* msg = alloc_msg(CMD_LOGIN, sizeof(MSG_LOGIN_REQ));
            u8 imei[16] = {0};

            eat_get_imei(imei, 15);

            if (!msg)
            {
                LOG_ERROR("alloc message failed");
            }

            memcpy(msg->IMEI, imei, 16);
            socket_sendData(msg, sizeof(MSG_LOGIN_REQ));
            LOG_DEBUG("send login message");
        }

    }

}

static int mc_login_rsp(const void* msg)
{
    set_client_state(EAT_TRUE);

    return 0;
}

static int mc_ping_rsp(const void* msg)
{
    return 0;
}

static int mc_alarm_rsp(const void* msg)
{
    return 0;
}

static int mc_sms(const void* msg)
{
    return 0;
}

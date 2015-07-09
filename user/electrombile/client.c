/*
 * client.c
 *
 *  Created on: 2015Äê7ÔÂ9ÈÕ
 *      Author: jk
 */
#include <stdio.h>

#include "client.h"
#include "msg.h"
#include "log.h"


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

int client_proc(const void* m, int msgLen)
{
    MSG_HEADER* msg = (MSG_HEADER*)m;
    size_t i = 0;

    if (msgLen < sizeof(MSG_HEADER))
    {
        LOG_ERROR("receive message length not enough: %zu(at least(%zu)", msgLen, sizeof(MSG_HEADER));

        return -1;
    }

    if (msg->signature != START_FLAG)
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
        }
    }

    LOG_ERROR("Message %d not processed", msg->cmd);

    return -1;
}

static int mc_login_rsp(const void* msg)
{
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

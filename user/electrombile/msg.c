/*
 * msg.c
 *
 *  Created on: 2015Äê7ÔÂ8ÈÕ
 *      Author: jk
 */
#include <eat_interface.h>
#include <eat_mem.h>

#include "msg.h"

static unsigned short seq = 0;

void* alloc_msg(char cmd, size_t length)
{
    MSG_HEADER* msg = eat_mem_alloc(length);

    if (msg)
    {
        msg->signature = htons(START_FLAG);
        msg->cmd = cmd;
        msg->seq = htons(seq++);
        msg->length = htons(length - MSG_HEADER_LEN);
    }

    return msg;
}

void* alloc_rspMsg(const MSG_HEADER* pMsg)
{
    MSG_HEADER* msg = NULL;
    size_t msgLen = 0;
    switch (pMsg->cmd)
    {
    case CMD_LOGIN:
        msgLen = htons(sizeof(MSG_LOGIN_RSP));
        break;

    case CMD_PING:
        msgLen = htons(sizeof(MSG_PING_RSP));
        break;

    default:
        return NULL;
    }

    msg = eat_mem_alloc(msgLen);
    msg->signature = htons(START_FLAG);
    msg->cmd = pMsg->cmd;
    msg->length = htons(msgLen - MSG_HEADER_LEN);
    msg->seq = htons(pMsg->seq);

    return msg;
}


void free_msg(void* msg)
{
    eat_mem_free(msg);
}

/*
 * msg.c
 *
 *  Created on: 2015��7��8��
 *      Author: jk
 */
#include <eat_interface.h>
#include <eat_mem.h>

#include "msg.h"



void* alloc_msg(char cmd, size_t length)
{
    MSG_HEADER* msg = eat_mem_alloc(length);

    if (msg)
    {
        fill_msg_header(msg);
        set_msg_cmd(msg, cmd);
        set_msg_len(msg, length - MSG_HEADER_LEN);
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
        msgLen = sizeof(MSG_LOGIN_RSP);
        break;

    case CMD_PING:
        msgLen = sizeof(MSG_PING_RSP);
        break;

    default:
        return NULL;
    }

    msg = eat_mem_alloc(msgLen);
    msg->signature = START_FLAG;
    msg->cmd = pMsg->cmd;
    msg->length = msgLen - MSG_HEADER_LEN;
    msg->seq = pMsg->seq;

    return msg;
}


void free_msg(MSG_HEADER* msg)
{
    eat_mem_free(msg);
}
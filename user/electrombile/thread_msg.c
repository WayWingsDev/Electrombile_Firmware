//
// Created by jk on 2015/7/1.
//

#include "thread_msg.h"

MSG_THREAD* allocMsg(u8 len)
{
    return eat_mem_alloc(len);
}

void freeMsg(MSG_THREAD* msg)
{
    eat_mem_free(msg);
}

eat_bool sendMsg(EatUser_enum from, EatUser_enum to, void* msg, u8 len)
{
    return eat_send_msg_to_user(from, to, EAT_TRUE, len, EAT_NULL, &msg);
}

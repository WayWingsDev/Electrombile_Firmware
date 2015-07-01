//
// Created by jk on 2015/7/1.
//

#include "msg.h"

eat_bool sendMsg(EatUser_enum from, EatUser_enum to, u8* msg, u8 len)
{
    eat_send_msg_to_user(from, to, EAT_TRUE, len, EAT_NULL, &msg);
}
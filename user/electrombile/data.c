/*
 * data.c
 *
 *  Created on: 2015Äê7ÔÂ9ÈÕ
 *      Author: jk
 */


#include "data.h"


LOCAL_DATA data =
{
        EAT_FALSE,
        EAT_FALSE,
        {0}
};


eat_bool socket_conneted(void)
{
    return data.connected;
}

void set_socket_state(eat_bool connected)
{
    data.connected = connected;
}

eat_bool client_logined(void)
{
    return data.connected;
}

void set_client_state(eat_bool logined)
{
    data.logined = logined;
}

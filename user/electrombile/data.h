/*
 * data.h
 *
 *  Created on: 2015Äê7ÔÂ9ÈÕ
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_DATA_H_
#define USER_ELECTROMBILE_DATA_H_

#include <eat_type.h>

#include "protocol.h"

typedef struct
{
    eat_bool connected;     //is the socket connected to server
    eat_bool logined;       //is the client logined to server

    eat_bool isGpsFixed;
    GPS gps;
    CGI cgi;
    char cellNum;
    CELL cells[MAX_CELL_NUM];
}LOCAL_DATA;

extern LOCAL_DATA data;


eat_bool socket_conneted(void);
eat_bool client_logined(void);

void set_socket_state(eat_bool connected);
void set_client_state(eat_bool logined);


#endif /* USER_ELECTROMBILE_DATA_H_ */

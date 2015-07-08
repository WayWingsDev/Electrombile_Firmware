/*
 * msg.h
 *
 *  Created on: 2015Äê7ÔÂ8ÈÕ
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_MSG_H_
#define USER_ELECTROMBILE_MSG_H_

#include <stdio.h>

#include "protocol.h"

void* alloc_msg(char cmd, size_t length);
void* alloc_rspMsg(const MSG_HEADER* pMsg);

void free_msg(MSG_HEADER* msg);

#endif /* USER_ELECTROMBILE_MSG_H_ */

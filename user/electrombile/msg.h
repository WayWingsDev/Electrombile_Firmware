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

#define htonl(l) ((((l >> 24) & 0x000000ff)) | \
                  (((l >>  8) & 0x0000ff00)) | \
                  (((h) & 0x0000ff00) <<  8) | \
                  (((h) & 0x000000ff) << 24))
#define ntohl(l) htonl(l)

#define htons(s) ((((s) >> 8) & 0xff) | \
                  (((s) << 8) & 0xff00))
#define ntohs(s) htons(s)

void* alloc_msg(char cmd, size_t length);
void* alloc_rspMsg(const MSG_HEADER* pMsg);

void free_msg(void* msg);

#endif /* USER_ELECTROMBILE_MSG_H_ */

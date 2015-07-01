//
// Created by jk on 2015/7/1.
//

#ifndef ELECTROMBILE_FIRMWARE_MSG_H
#define ELECTROMBILE_FIRMWARE_MSG_H

#include <eat_type.h>
#include <eat_interface.h>
enum CMD
{
    CMD_GPS_UPDATE,
    CMD_SMS,
    CMD_VIBRATE,
};

#pragma pack(push,1)

typedef struct
{
    u8 cmd;
    u8 length;
    u8 data[];
}MSG;

typedef struct
{
    eat_bool isVibrate;
}MSG_VIBRATE;

#pragma pack(pop)

MSG* allocMsg(u8 len);
void freeMsg(MSG* msg);
eat_bool sendMsg(EatUser_enum from, EatUser_enum to, void* msg, u8 len);

#endif //ELECTROMBILE_FIRMWARE_MSG_H

/*
 * protocol.h
 *
 *  Created on: 2015骞�6鏈�29鏃�
 *      Author: jk
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define START_FLAG (0xAA55)
#define IMEI_LENGTH 15

enum
{
    CMD_LOGIN   = 0x01,
    CMD_GPS     = 0x02,
    CMD_PING    = 0x03,
    CMD_ALARM   = 0x04,
    CMD_SMS     = 0x05,
};

/*
 * GPS structure
 */
typedef struct
{
    double longitude;
    double latitude;
}GPS;
#pragma pack(push, 1)

/*
 * Message header definition
 */
typedef struct
{
    short signature;
    char cmd;
    unsigned short length;
    unsigned short seq;
}__attribute__((__packed__)) MSG_HEADER;

#define MSG_HEADER_LEN sizeof(MSG_HEADER)

/*
 * Login message structure
 */
typedef struct
{
    MSG_HEADER header;
    char IMEI[IMEI_LENGTH + 1];
}MSG_LOGIN_REQ;

typedef MSG_HEADER MSG_LOGIN_RSP;

/*
 * GPS message structure
 */
typedef struct
{
    MSG_HEADER header;
    GPS gps;
}MSG_GPS;

/*
 * heartbeat message structure
 */
typedef struct
{
    MSG_HEADER header;
    short statue;   //TODO: to define the status bits
}MSG_PING_REQ;

typedef MSG_HEADER MSG_PING_RSP;

/*
 * alarm message structure
 */
typedef struct
{
    MSG_HEADER header;
    unsigned char alarmType;
    GPS gps;
}MSG_ALARM_REQ;
typedef MSG_HEADER MSG_ALARM_RSP;

/*
 * SMS message structure
 */
typedef struct
{
    MSG_HEADER header;
    char telphone[12];
    char sms[];
}MSG_SMS_REQ;

typedef MSG_SMS_REQ MSG_SMS_RSP;

#pragma pack(pop)

#endif /* _PROTOCOL_H_ */

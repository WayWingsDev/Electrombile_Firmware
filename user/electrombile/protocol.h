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
#define MAX_CELL_NUM 7

enum
{
    CMD_LOGIN   = 0x01,
    CMD_GPS     = 0x02,
    CMD_CELL    = 0x03,
    CMD_PING    = 0x04,
    CMD_ALARM   = 0x05,
    CMD_SMS     = 0x06,
};

#pragma pack(push, 1)

/*
 * GPS structure
 */
typedef struct
{
    float longitude;
    float latitude;
}GPS;

/*
 * CELL structure
 */
typedef struct
{
   short lac;       //local area code
   short cellid;    //cell id
   short rxl;       //receive level
}__attribute__((__packed__)) CELL;

typedef struct
{
    short mcc;  //mobile country code
    short mnc;  //mobile network code
    char  cellNo;// cell count
//    CELL cell[];
}__attribute__((__packed__)) CGI;       //Cell Global Identifier





/*
 * Message header definition
 */
typedef struct
{
    short signature;
    char cmd;
    unsigned short seq;
    unsigned short length;
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

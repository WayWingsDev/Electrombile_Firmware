/*
 * gps.c
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <eat_interface.h>
#include <eat_gps.h>


#include "gps.h"
#include "timer.h"
#include "thread.h"
#include "thread_msg.h"
#include "log.h"
#include "setting.h"
#include "protocol.h"

#define NMEA_BUFF_SIZE 1024
static char gps_info_buf[NMEA_BUFF_SIZE]="";

static void gps_timer_handler(void);
static eat_bool gps_sendGPS(float latitude, float longitude);
static eat_bool gps_sendCell(short mcc, short mnc, char cellNo, CELL cells[]);
static eat_bool gps_getGps(float* latitude, float* longitude);
static eat_bool gps_getCells(short* mcc, short* mnc, char* cellNo, CELL cells[]);
static void geo_fence_proc_cb(char *msg_buf, u8 len);

void app_gps_thread(void *data)
{
    EatEvent_st event;

    eat_gps_power_req(EAT_TRUE);

    LOG_INFO("gps current sleep mode %d", eat_gps_sleep_read());

    eat_gps_register_msg_proc_callback(geo_fence_proc_cb);
    eat_timer_start(TIMER_GPS, setting.gps_timer_period);
    eat_modem_write("AT+CENG=3,1\r",12);
    while(EAT_TRUE)
    {
        eat_get_event_for_user(THREAD_GPS, &event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER :
                switch ( event.data.timer.timer_id )
                {
                    case TIMER_GPS:
                    	LOG_INFO("TIMER_GPS expire!");
                        gps_timer_handler();
                        eat_timer_start(event.data.timer.timer_id, setting.gps_timer_period);
                        break;


                    default:
                    	LOG_ERROR("ERR: timer[%d] expire!", event.data.timer.timer_id);

                        break;
                }
                break;
            default:
            	LOG_ERROR("event(%d) not processed", event.event);
                break;

        }
    }
}

static void gps_timer_handler(void)
{
    float latitude = 0.0;
    float longitude = 0.0;
    eat_bool isGpsFixed = EAT_FALSE;
    eat_bool isCellGet = EAT_FALSE;

    isGpsFixed = gps_getGps(&latitude, &longitude);

    if (isGpsFixed)
    {
        LOG_DEBUG("GPS fixed:lat=%f, lng=%f", latitude, longitude);
        gps_sendGPS(latitude, longitude);
        return;
    }
    else
    {
        short mcc = 0;  //mobile country code
        short mnc = 0;  //mobile network code
        char  cellNo;// cell count
        CELL cells[7] = {0};

        isCellGet = gps_getCells(&mcc, &mnc, &cellNo, cells);
        if (isCellGet)
        {
            gps_sendCell(mcc, mnc, cellNo, cells);
        }
    }

//    gps_sendGPS(gps);
}

static eat_bool gps_getGps(float* latitude, float* longitude)
{
    eat_bool isGpsFixed = EAT_FALSE;
    int n = 0;

    /*
     * NMEA output format:
     * $GPGGA,003634.000,8960.0000,N,00000.0000,E,0,0,,137.0,M,13.0,M,,
     * header     utc    Lat       N/S     Lng  E/W (Position Fix Indicator)  (Satellites Used)
     */
    eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_GPGGA, gps_info_buf,NMEA_BUFF_SIZE);
    LOG_DEBUG("EAT_NMEA_OUTPUT_SIMCOM:%s", gps_info_buf);

    n = sscanf(gps_info_buf + sizeof("$GPGGA"), "%*f,%f,%*c,%f,%*c,%d", latitude, longitude, &isGpsFixed);
    if (n != 3)
    {
        LOG_ERROR("Parse gps info failed");
    }

    return isGpsFixed;
}

static eat_bool gps_getCells(short* mcc, short* mnc, char* cellNo, CELL cells[])
{
    s8 buf[256] = {0};  //用于读取AT指令的响应
    u16 len = 0;        //AT指令回应报文长度

    int _mcc = 460;
    int _mnc = 0;
    int lac = 0;
    int cellid = 0;
    int rxl = 0;

    u8 cellCount = 0;

    char* p = buf;

    int n = 0;  //sscanf返回的参数个数

    /*
     * the output format of AT+CENG?
     * +CENG:<mode>,<Ncell>
     * +CENG:<cell>,"<mcc>,<mnc>,<lac>,<cellid>,<bsic>,<rxl>"
     * +CENG:<cell>,...
     *
     * eg:
     * +CENG: 3,1
     *
     * +CENG: 0,"460,00,5001,96bd,23,45"
     * +CENG: 1,"460,00,5001,96dd,33,21"
     * +CENG: 2,",,0000,0000,00,00"
     * +CENG: 3,",,0000,0000,00,00"
     * +CENG: 4,",,0000,0000,00,00"
     * +CENG: 5,",,0000,0000,00,00"
     * +CENG: 6,",,0000,0000,00,00"
     *
     */
    eat_modem_write("AT+CENG?\r", 9);
    len = eat_modem_read(buf, 256);
    if (len > 0)
    {
        LOG_DEBUG("AT+CENG? return %s", buf);
    }

    do
    {
        p = strchr(p + 1, '+');
        if (p)
        {
            n = sscanf(p + sizeof("+CENG: 0,"), "%d,%d,%d,%d,%*d,%d", &_mcc, &_mnc, &lac, &cellid, &rxl);
            if (n != 5)
            {
                break;
            }

            if (*mcc == 0)
            {
                *mcc =_mcc;
                *mnc = _mnc;
            }
            cells[cellCount].lac = lac;
            cells[cellCount].cellid = cellid;
            cells[cellCount].rxl = rxl;

            cellCount++;
        }

    } while (p);


    if (cellCount)
    {
        *cellNo = cellCount;
        return EAT_TRUE;
    }
    else
    {
        return EAT_FALSE;
    }
}

static eat_bool gps_sendMsg2Main(MSG_THREAD* msg, u8 len)
{
    return sendMsg(THREAD_GPS, THREAD_MAIN, msg, len);
}

static eat_bool gps_sendGPS(float latitude, float longitude)
{
    u8 msgLen = sizeof(MSG_THREAD) + sizeof(LOCAL_GPS);
    MSG_THREAD* msg = allocMsg(msgLen);
    LOCAL_GPS* gps = 0;


    if (!msg)
    {
        LOG_ERROR("alloc msg failed");
        return EAT_FALSE;
    }
    msg->cmd = CMD_THREAD_GPS;
    msg->length = sizeof(LOCAL_GPS);

    gps = (LOCAL_GPS*)msg->data;
    gps->isGpsFixed = EAT_TRUE;
    gps->gps.latitude = latitude;
    gps->gps.longitude = longitude;


    LOG_DEBUG("send gps: lat(%f), lng(%f)", latitude, longitude);
    return gps_sendMsg2Main(msg, msgLen);
}

static eat_bool gps_sendCell(short mcc, short mnc, char cellNo, CELL cells[])
{
    u8 msgLen = sizeof(MSG_THREAD) + sizeof(LOCAL_GPS);
    MSG_THREAD* msg = allocMsg(msgLen);
    LOCAL_GPS* gps = 0;

    int i = 0;

    if (!msg)
    {
        LOG_ERROR("alloc msg failed");
        return EAT_FALSE;
    }
    msg->cmd = CMD_THREAD_GPS;
    msg->length = sizeof(LOCAL_GPS);

    gps = (LOCAL_GPS*)msg->data;
    gps->isGpsFixed = EAT_FALSE;

    gps->cellInfo.mcc = mcc;
    gps->cellInfo.mnc = mnc;

    gps->cellInfo.cellNo = cellNo;
    for (i = 0; i < cellNo; i++)
    {
        gps->cellInfo.cell[i].lac = cells[i].lac;
        gps->cellInfo.cell[i].cellid = cells[i].cellid;
        gps->cellInfo.cell[i].rxl = cells[i].rxl;
    }
    LOG_DEBUG("send gps: mcc(%d), mnc(%d), cellNo(%d)", mcc, mnc, cellNo);

    return gps_sendMsg2Main(msg, msgLen);
}

static void geo_fence_proc_cb(char *msg_buf, u8 len)
{
    //TODO:
}


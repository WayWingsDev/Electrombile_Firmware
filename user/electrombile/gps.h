/*
 * gps.h
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_GPS_H_
#define USER_ELECTROMBILE_GPS_H_


void app_gps_thread(void *data);
static eat_bool gps_sendGPS(double lat,double lon);


#endif /* USER_ELECTROMBILE_GPS_H_ */

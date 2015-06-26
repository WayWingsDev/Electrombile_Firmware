/*
 * log.h
 *
 *  Created on: 2015Äê6ÔÂ25ÈÕ
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_LOG_H_
#define USER_ELECTROMBILE_LOG_H_

#define _DEBUG
#define _ERROR

#ifdef _DEBUG
#define LOG_DEBUG(fmt, ...) eat_trace("[DBG][s]"fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

#ifdef _ERROR
#define LOG_ERROR(fmt, ...) eat_trace("[ERR][s]"fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif



#endif /* USER_ELECTROMBILE_LOG_H_ */

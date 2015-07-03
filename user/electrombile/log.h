/*
 * log.h
 *
 *  Created on: 2015/6/25
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_LOG_H_
#define USER_ELECTROMBILE_LOG_H_

#define _DEBUG
#define _ERROR

#ifdef LOG_DEBUG_FLAG
#define LOG_DEBUG(fmt, ...) eat_trace("[DBG][%s]"fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

#ifdef LOG_INFO_FLAG
#define LOG_INFO(fmt, ...) eat_trace("[INF][%s]"fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif

#ifdef LOG_ERROR_FLAG
#define LOG_ERROR(fmt, ...) eat_trace("[ERR][%s]"fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif



#endif /* USER_ELECTROMBILE_LOG_H_ */

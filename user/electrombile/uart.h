/*
 * uart.h
 *
 *  Created on: 2015Äê7ÔÂ8ÈÕ
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_UART_H_
#define USER_ELECTROMBILE_UART_H_

#include <eat_interface.h>

int event_uart_ready_rd(const EatEvent_st* event);

void print(const char* fmt, ...);

#endif /* USER_ELECTROMBILE_UART_H_ */

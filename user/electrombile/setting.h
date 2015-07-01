/*
 * setting.h
 *
 *  Created on: 2015/6/24
 *      Author: jk
 */

#ifndef USER_ELECTROMBILE_SETTING_H_
#define USER_ELECTROMBILE_SETTING_H_

#include <eat_type.h>

#define MAX_DOMAIN_NAME_LEN 32
typedef enum
{
	ADDR_TYPE_IP,
	ADDR_TYPE_DOMAIN
}ADDR_TYPE;

typedef struct
{
	ADDR_TYPE addr_type;
	u8 ipaddr[4];
	u8 domain[MAX_DOMAIN_NAME_LEN];
}SETTING;

eat_bool SETTING_initial(void);
eat_bool SETTING_save(void);
#endif /* USER_ELECTROMBILE_SETTING_H_ */

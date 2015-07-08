/*
 * setting.c
 *
 *  Created on: 2015/6/24
 *      Author: jk
 */
#include "setting.h"

SETTING setting =
{
		ADDR_TYPE_DOMAIN,
		{
				"server.xiaoan110.com",
		},
		6789,
};

/*
 * read setting from flash
 */
eat_bool SETTING_initial(void)
{
	setting.addr_type = ADDR_TYPE_IP;

	setting.addr.ipaddr[0] = 120;
	setting.addr.ipaddr[1] = 25;
	setting.addr.ipaddr[2] = 157;
	setting.addr.ipaddr[3] = 233;

    setting.port = 9876;
}

/*
 * save setting to flash
 */
eat_bool SETTING_save(void)
{

}

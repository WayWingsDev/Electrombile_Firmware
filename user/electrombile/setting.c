/*
 * setting.c
 *
 *  Created on: 2015/6/24
 *      Author: jk
 */
#include "setting.h"

static SETTING setting =
{
		ADDR_TYPE_DOMAIN,
		{
				"server.xiaoan110.com",
		},

};

/*
 * read setting from flash
 */
eat_bool SETTING_initial(void)
{

}

/*
 * save setting to flash
 */
eat_bool SETTING_save(void)
{

}

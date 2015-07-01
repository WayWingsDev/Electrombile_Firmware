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
		{0},
		"server.xiaoan110.com"
};

/*
 * 从FLASH中读取设置
 */
eat_bool SETTING_initial(void)
{

}

/*
 * 保存设置到FLASH
 */
eat_bool SETTING_save(void)
{

}

#if !defined (__EAT_OTHER_H__)
#define __EAT_OTHER_H__

#include "eat_type.h"

typedef struct
{
    u8  status;
    u8  level;
    u32  volt;
}EAT_CBC_ST;

/*****************************************************************************
* Function :  eat_get_cbc
* Description: Get cbc function.
* Parameters:
*     bmt  EAT_CBC_ST [IN] The struct of cbc.
* Returns:
*     EAT_TRUE  The operation Success.
*     EAT_FALSE The operation False.
* NOTE:
*     Nothing
*****************************************************************************/
extern eat_bool (*const eat_get_cbc)(EAT_CBC_ST *bmt);

/*****************************************************************************
* Function : eat_get_imei
* Description: Get IMEI string.
* Parameters:
*     imei[]  [OUT] store IMEI string
*     len     [IN] length of imei[]
* Returns:
*     if succeed    return the pointer of IMEI string.
*     otherwise     return NULL.
* NOTE:
*    len: len >=15
*****************************************************************************/
extern u8 *(*const eat_get_imei)(u8* imei, u16 len );
#endif

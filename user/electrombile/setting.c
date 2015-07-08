/*
 * setting.c
 *
 *  Created on: 2015/6/24
 *      Author: jk
 */
#include "setting.h"
#include "eat_fs_type.h" 
#include "eat_fs.h"
#include "eat_fs_errcode.h"
#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_uart.h"

#define SETITINGFILE_NAME  L"C:\\setting.txt"

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
    int testFileHandle,seekRet;
    static eat_fs_error_enum fs_Op_ret;
    UINT readLen;
    testFileHandle = eat_fs_Open(SETITINGFILE_NAME,FS_READ_ONLY);
    if(testFileHandle<EAT_FS_NO_ERROR)
    {
        eat_trace("eat_fs_Open():Create File Fail,and Return Error is %x",testFileHandle);
        //TODO SETTING INIT
        return ;
    }
    else
    {
        seekRet = eat_fs_Seek(testFileHandle,0,EAT_FS_FILE_BEGIN);
	if(0>seekRet)
	{
		eat_trace("eat_fs_Seek():Seek File Pointer Fail");
		eat_fs_Close(testFileHandle);
		return;
	}
	else
	{
        	eat_trace("eat_fs_Seek():Seek File Pointer Success");
	}
        eat_trace("eat_fs_Open():Create File Success,and FileHandle is %x",testFileHandle);
        fs_Op_ret = (eat_fs_error_enum)eat_fs_Read(testFileHandle,&setting,2500,&readLen);
	if(EAT_FS_NO_ERROR != fs_Op_ret )
	{	
		eat_trace("eat_fs_Read():Read File Fail,and Return Error is %d,Readlen is %d",fs_Op_ret,readLen);
		eat_fs_Close(testFileHandle);
		return;
	}
	else
	{
		eat_trace("eat_fs_Read()=%s:Read File Success",&setting);
              eat_fs_Close(testFileHandle);
	}
    }
    
    
}

/*
 * save setting to flash
 */
eat_bool SETTING_save(void)
{
    int testFileHandle;
    static eat_fs_error_enum fs_Op_ret;
    static char writeBuf[2048]="";
    UINT writedLen;
    eat_trace("setting_save");
    testFileHandle = eat_fs_Open(SETITINGFILE_NAME,FS_CREATE_ALWAYS|FS_READ_WRITE);
    if(testFileHandle<EAT_FS_NO_ERROR)
    {
        eat_trace("eat_fs_Open():Create File Fail,and Return Error is %x",testFileHandle);
        //TODO SETTING INIT
        return ;
    }
    else
    {
        eat_trace("eat_fs_Open():Create File Success,and FileHandle is %x",testFileHandle);
        fs_Op_ret = (eat_fs_error_enum)eat_fs_Write(testFileHandle,&setting,2048,&writedLen);
	if(EAT_FS_NO_ERROR != fs_Op_ret )
	{	
		eat_trace("eat_fs_Write():eat_fs_Write File Fail,and Return Error is %d,Readlen is %d",fs_Op_ret,writedLen);
		eat_fs_Close(testFileHandle);
		return;
	}
	else
	{
		eat_trace("eat_fs_Write:eat_fs_Write File Success");
              eat_fs_Close(testFileHandle);
	}
    }
}

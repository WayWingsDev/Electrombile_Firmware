//
// Created by jk on 2015/7/1.
//

#include "vibration.h"
#include "thread.h"
#include "log.h"
#include"timer.h"
#include "thread_msg.h"

static unsigned int vibration_timer_period = 100;
static short datax[10],datay[10],dataz[10];
static long data_x2y2z2[10];
static int vibration_data_i=0;
#define VIBRATION_TRESHOLD 100000000

static eat_bool vibration_sendMsg2Main(MSG* msg, u8 len);
static eat_bool vibration_sendAlarm();
static void vibration_timer_handler();

float  fangcha(long *array,int len)
{
   int i;
   float sum=0;
   float ave;
   for(i=0;i<len;i++)
      sum+=array[i];
   ave=sum/len;
   sum=0;
   for(i=0;i<len;i++)
      sum+=(array[i]-ave)*(array[i]-ave);
   ave=sum/len;
   return ave;
}

void app_vibration_thread(void *data)
{
	EatEvent_st event;
	s32 ret;
	u8 write_buffer[10] = {0};

	LOG_INFO("vibration thread start");

	ret=eat_i2c_open(EAT_I2C_OWNER_0,0x1D,100);
	if(ret!=0)
	{
	    LOG_ERROR("i2c test eat_i2c_open fail :ret=%d",ret);
	}
	else
	{
	    LOG_INFO("i2c test eat_i2c_open success");
	}

	write_buffer[0]=MMA8X5X_CTRL_REG1;
	write_buffer[1]=0x1;
	ret=eat_i2c_write(EAT_I2C_OWNER_0,write_buffer,2);
	if(ret!=0)
	{
		LOG_ERROR("start sample fail :ret=%d",ret);
	}
	else
	{
		LOG_INFO("start sample  success");
	}

	eat_timer_start(TIMER_VIBRATION, vibration_timer_period);

	while(EAT_TRUE)
	{
        eat_get_event_for_user(THREAD_VIBRATION, &event);
        switch(event.event)
        {
            case EAT_EVENT_TIMER :

			switch (event.data.timer.timer_id)
			{

			case TIMER_VIBRATION:
				vibration_timer_handler();
				eat_timer_start(event.data.timer.timer_id, vibration_timer_period);
				break;

			default:
				LOG_ERROR("ERR: timer[%d] expire!");

				break;
			}
                break;

            case EAT_EVENT_MDM_READY_RD:
                break;

            case EAT_EVENT_MDM_READY_WR:

                break;
            case EAT_EVENT_USER_MSG:

                break;
            default:
                break;

        }
    }
}

static void vibration_timer_handler()
{
	u8 read_buffer[10]=0;
	s32 ret;
	u8 write_buffer[10]=0;
//	float x2y2z2_fangcha;
	long delta;
	write_buffer[0]=MMA8X5X_OUT_X_MSB;
	ret = eat_i2c_read(EAT_I2C_OWNER_0, &write_buffer[0],1,read_buffer,
							MMA8X5X_BUF_SIZE);

	if(ret!=0)
	{
//		eat_trace("i2c test eat_i2c_read 0AH fail :ret=%d",ret);
	}
	else
	{
		datax[vibration_data_i]  = ((read_buffer[0] << 8) & 0xff00) | read_buffer[1];
		datay[vibration_data_i]  = ((read_buffer[2] << 8) & 0xff00) | read_buffer[3];
		dataz[vibration_data_i]  = ((read_buffer[4] << 8) & 0xff00) | read_buffer[5];
		data_x2y2z2[vibration_data_i] = datax[vibration_data_i]*datax[vibration_data_i]+datay[vibration_data_i]*datay[vibration_data_i]+dataz[vibration_data_i]*dataz[vibration_data_i];
		delta =abs( data_x2y2z2[vibration_data_i]- 282305280);
//		eat_trace("delta=%d, data_x2y2z2[vibration_data_i] = %d",delta, data_x2y2z2[vibration_data_i]);
		if(delta > VIBRATION_TRESHOLD)
			vibration_sendAlarm();
		vibration_data_i++;
		if(vibration_data_i == 10)
			vibration_data_i = 0 ;	
//		x2y2z2_fangcha = fangcha(data_x2y2z2, 10);
		
		
	}
	
}
static eat_bool vibration_sendMsg2Main(MSG* msg, u8 len)
{
    return sendMsg(THREAD_VIBRATION, THREAD_MAIN, msg, len);
}

static eat_bool vibration_sendAlarm()
{
    u8 msgLen = sizeof(MSG) + sizeof(MSG_VIBRATE);
    MSG* msg = allocMsg(msgLen);
    MSG_VIBRATE* vibrate = (MSG_VIBRATE*)msg->data;

    msg->cmd = CMD_VIBRATE;
    msg->length = sizeof(MSG_VIBRATE);

    vibrate->isVibrate = EAT_TRUE;
    eat_trace("isVibrate=%d",vibrate->isVibrate);

    return vibration_sendMsg2Main(msg, msgLen);
}

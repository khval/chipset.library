
// copyright 2015, Kjetil Hvalstrand, MIT License

#define __USE_INLINE__

#include <proto/exec.h>
#include <proto/dos.h>

extern struct MsgPort			*TimerMP;
extern struct TimeRequest		*TimerIO;
extern int 					timer_device_open;


void init_timer_device()
{
	TimerMP = CreateMsgPort();
	if (!TimerMP) 
{ 
#ifdef debug
		printf("failed to create msg port\n");
#endif
		return;
	}

	TimerIO = (struct TimeRequest *) CreateIORequest(TimerMP,sizeof(struct TimeRequest));
	if (!TimerIO)
	{
		DeleteMsgPort(TimerMP);
#ifdef debug
		printf("Failed to create io req\n");
#endif
		return;
	}

	if (OpenDevice("timer.device",UNIT_MICROHZ,(struct IORequest *) TimerIO,0L)==0)
	{
		timer_device_open=TRUE;
	}
}


void close_timer_device()
{
	if (timer_device_open==TRUE)
	{
		CloseDevice((struct IORequest *) TimerIO);
		timer_device_open=FALSE;
	}

	if (TimerIO)
	{
		DeleteIORequest((struct IORequest *) TimerIO);
		TimerIO = NULL;
	}

	if (TimerMP) 
	{
		DeleteMsgPort(TimerMP);
		TimerMP = NULL;
	}

}


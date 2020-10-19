
// copyright 2015, Kjetil Hvalstrand, MIT License

#define __USE_INLINE__

#include <proto/exec.h>
#include <proto/dos.h>

extern struct MsgPort			*TimerMP;
extern struct TimeRequest		*TimerIO;
extern int 					timer_device_open;


void init_timer_device()
{
//	TimerMP = CreateMsgPort();

	TimerMP = AllocSysObjectTags(ASOT_PORT, TAG_DONE);

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
		FreeSysObject(ASOT_PORT, (APTR) TimerMP);
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
		FreeSysObject(ASOT_PORT, (APTR) TimerMP);
		TimerMP = NULL;
	}

}


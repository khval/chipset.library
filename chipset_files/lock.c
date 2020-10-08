
#include "proto/exec.h"
#include "proto/dos.h"

APTR cia_mx = NULL;
APTR event_mx = NULL;

#define debug_cia_lock_no
#define debug_event_lock_no

#ifdef debug_cia_lock_yes
int cia_lock_count = 0;
struct Task *cia_locked_task = NULL;
#endif 

void cia_lock()
{
#ifdef debug_cia_lock_yes
	IDOS->Printf("CIA lock\n");

	if (cia_lock_count)
	{
		IDOS->Printf("is already locked\n");

		if ( cia_locked_task == IExec->FindTask(NULL) )
		{
			IDOS->Printf("locked by same process\n");
			getchar();
		}
		else
		{
			IDOS->Printf("locked by different process\n");
		}
	}
#endif
	IExec -> MutexObtain(cia_mx);
#ifdef debug_cia_lock_yes
	cia_lock_count++;
	cia_locked_task = IExec->FindTask(NULL);
#endif
}

void cia_unlock()
{
#ifdef debug_cia_lock_yes

	if (cia_lock_count == 0)
	{
		IDOS->Printf("cia is not locked\n");
		getchar();
	}

	if ( cia_locked_task != IExec->FindTask(NULL) )
	{
		IDOS->Printf("not locked by same task\n");
		getchar();
	}

	IDOS->Printf("CIA unlock\n");
	cia_lock_count --;

	if (cia_lock_count == 0)
	{
		cia_locked_task = NULL;
	}

#endif

	IExec->MutexRelease(cia_mx);
}


#ifdef debug_event_lock_yes
int event_lock_count = 0;
struct Task *event_locked_task = NULL;
#endif 

void event_lock()
{
#ifdef debug_event_lock_yes
	IDOS->Printf("Event lock\n");

	if (event_lock_count)
	{
		IDOS->Printf("is already locked\n");

		if ( event_locked_task == IExec->FindTask(NULL) )
		{
			IDOS->Printf("locked by same process\n");
			getchar();
		}
		else
		{
			IDOS->Printf("locked by different process\n");
		}
	}
#endif
	IExec -> MutexObtain(event_mx);
#ifdef debug_event_lock_yes
	event_lock_count++;
	event_locked_task = IExec->FindTask(NULL);
#endif
}

void event_unlock()
{
#ifdef debug_event_lock_yes

	if (event_lock_count == 0)
	{
		IDOS->Printf("event is not locked\n");
		getchar();
	}

	if ( event_locked_task != IExec->FindTask(NULL) )
	{
		IDOS->Printf("not locked by same task\n");
		getchar();
	}

	IDOS->Printf("event unlock\n");
	event_lock_count --;

	if (event_lock_count == 0)
	{
		event_locked_task = NULL;
	}

#endif

	IExec->MutexRelease(event_mx);
}

BOOL lock_init()
{
	cia_mx = (APTR) IExec -> AllocSysObjectTags(ASOT_MUTEX, TAG_DONE);
	if ( ! cia_mx) return FALSE;

	event_mx = (APTR) IExec -> AllocSysObjectTags(ASOT_MUTEX, TAG_DONE);
	if ( ! event_mx) return FALSE;

	return TRUE;
}

#define LIBNAME "test"

void lock_cleanup()
{
	if (cia_mx) 
	{
		IExec -> FreeSysObject(ASOT_MUTEX, cia_mx); 
		cia_mx = NULL;
	}
	else if (IDOS) IDOS->Printf("%s: Failed to create cia Mutex\n",LIBNAME);

	if (event_mx) 
	{
		IExec -> FreeSysObject(ASOT_MUTEX, event_mx); 
		event_mx = NULL;
	}
	else if (IDOS) IDOS->Printf("%s: Failed to create event Mutex\n",LIBNAME);
}

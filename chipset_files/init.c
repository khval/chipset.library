/* :ts=4
 *  $VER: init.c $Revision$ (03-Mar-2020)
 *
 *  This file is part of chipset.
 *
 *  Copyright (c) 2020 LiveForIt Software.
 *  LGPL License.
 *
 * $Id$
 *
 * $Log$
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <exec/types.h>
#include <libraries/chipset.h>
#include <proto/chipset.h>
#include "spawn.h"

#include <stdarg.h>

#define LIBNAME "chipset.library"

/* Version Tag */
#include "chipset.library_rev.h"
STATIC CONST UBYTE USED verstag[] = VERSTAG;

struct _Library
{
    struct Library libNode;
    BPTR segList;
    /* If you need more data fields, add them here */
};

struct ExecIFace *IExec UNUSED = NULL;

struct NewlibIFace * INewlib = NULL;
struct DOSIFace *IDOS = NULL;
struct UtilityIFace *IUtility = NULL;

struct Library *NewLibBase = NULL;
struct Library *DOSBase = NULL;
struct Library *UtilityBase = NULL;

extern struct AHIIFace *IAHI;

extern void read_ahi_prefs();
extern uint32_t	ahi_mode_id;
extern uint32_t	ahi_frequency;

struct Task *main_task = NULL;	// main_task is whatever when running from a library
struct Process *cia_process = NULL;

APTR cia_mx = NULL;
APTR event_mx = NULL;


int spawn_count;

bool expunge_tasks = false;

/*
 * The system (and compiler) rely on a symbol named _start which marks
 * the beginning of execution of an ELF file. To prevent others from 
 * executing this library, and to keep the compiler/linker happy, we
 * define an empty _start symbol here.
 *
 * On the classic system (pre-AmigaOS 4.x) this was usually done by
 * moveq #0,d0
 * rts
 *
 */
int32 _start(void);

int32 _start(void)
{
    /* If you feel like it, open DOS and print something to the user */
    return RETURN_FAIL;
}

extern void cia_process_fn ();

extern BOOL OpenAHI( void );
extern void CloseAHI( void );

extern BOOL init_nallepuh(  	ULONG mode_id,ULONG frequency );
extern void cleanup_nallepuh();

#define __debug_cia__

void __init_cia_process__()
{
#ifdef __debug_cia__
	BPTR _out_ = IDOS -> FOpen("CON:",MODE_NEWFILE,0);
#else
	BPTR _out_ = 0;
#endif

	main_task = (struct Task *) IExec->FindTask(NULL);
	cia_process = spawn( cia_process_fn, "CIA process", _out_ );
}

/* Open the library */
STATIC struct Library *libOpen(struct LibraryManagerInterface *Self, ULONG version)
{
	struct _Library *libBase = (struct _Library *)Self->Data.LibBase; 

	if (version > VERSION)
	{
		return NULL;
	}

	/* Add any specific open code here  Return 0 before incrementing OpenCnt to fail opening */

	/* Add up the open count */
	libBase->libNode.lib_OpenCnt++;

	if ( libBase->libNode.lib_OpenCnt == 1)
	{
		expunge_tasks = false;
		__init_cia_process__();
	}

	return (struct Library *)libBase;
}


/* Close the library */
STATIC APTR libClose(struct LibraryManagerInterface *Self)
{
	struct Library *libBase = (struct Library *)Self->Data.LibBase;
	/* Make sure to undo what open did */

	/* Make the close count */
	((struct Library *)libBase)->lib_OpenCnt--;

	if ( ((struct Library *)libBase)->lib_OpenCnt == 0)
	{
		expunge_tasks = true;
		main_task = (struct Task *) IExec->FindTask(NULL);
		wait_spawns();
	}

	return 0;
}

#define close_lib(b,i)			\
	if (b) IExec->CloseLibrary(b);	\
	if (i) IExec->DropInterface( (struct Interface *) i );	\
	b = NULL; i = NULL;			


extern void cleanup();

void close_libs()
{
	cleanup();

	struct ExecIFace *IExec = (struct ExecIFace *)(*(struct ExecBase **)4)->MainInterface;

	if ( ! IAHI ) IDOS->Printf("%s: Failed to open AHI\n",LIBNAME); 

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

	cleanup_nallepuh();

	CloseAHI();

	close_lib( UtilityBase, IUtility );
	close_lib( DOSBase, IDOS);
	close_lib( NewLibBase, INewlib);
}


/* Expunge the library */
STATIC APTR libExpunge(struct LibraryManagerInterface *Self)
{
    /* If your library cannot be expunged, return 0 */
    IExec = (struct ExecIFace *)(*(struct ExecBase **)4)->MainInterface;
    APTR result = (APTR)0;
    struct _Library *libBase = (struct _Library *)Self->Data.LibBase;
    if (libBase->libNode.lib_OpenCnt == 0)
    {
	     result = (APTR)libBase->segList;
        /* Undo what the init code did */

	close_libs();

	IExec->Remove((struct Node *)libBase);
	IExec->DeleteLibrary((struct Library *)libBase);
    }
    else
    {
        result = (APTR)0;
        libBase->libNode.lib_Flags |= LIBF_DELEXP;
    }
    return result;
}

BOOL open_lib( const char *name, int ver , const char *iname, int iver, struct Library **base, struct Interface **interface)
{

	*interface = NULL;
	*base = IExec->OpenLibrary( name , ver);

	if (*base)
	{
		 *interface = IExec->GetInterface( *base,  iname , iver, TAG_END );
		if (!*interface) if (IDOS) IDOS -> Printf("Unable to getInterface %s for %s %d!\n",iname,name,ver);
	}
	else
	{
	   	if (IDOS) IDOS -> Printf("Unable to open the %s %ld!\n",name,ver);
	}

	return (*interface) ? TRUE : FALSE;
}


BOOL init()
{
	if ( ! open_lib( "dos.library", 53L , "main", 1, &DOSBase, (struct Interface **) &IDOS  ) ) return FALSE;
	if ( ! open_lib( "newlib.library", 53L , "main", 1, &NewLibBase, (struct Interface **) &INewlib  ) ) return FALSE;
	if ( ! open_lib( "utility.library", 53L , "main", 1, &UtilityBase, (struct Interface **) &IUtility  ) ) return FALSE;
	if ( ! OpenAHI() ) return FALSE;

	// AHI mode ID, check AHI prefs, need to improve this part.

	read_ahi_prefs();

	if ( ! init_nallepuh( ahi_mode_id,ahi_frequency ) ) return FALSE;

	cia_mx = (APTR) IExec -> AllocSysObjectTags(ASOT_MUTEX, TAG_DONE);
	if ( ! cia_mx) return FALSE;

	event_mx = (APTR) IExec -> AllocSysObjectTags(ASOT_MUTEX, TAG_DONE);
	if ( ! event_mx) return FALSE;

	if (setup_mem_banks() == FALSE) return FALSE;

	return TRUE;
}



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


struct _Library *libBase_debug = NULL;

/* The ROMTAG Init Function */
STATIC struct Library *libInit(struct Library *LibraryBase, APTR seglist, struct Interface *exec)
{
	struct _Library *libBase = (struct _Library *) LibraryBase;

	IExec = (struct ExecIFace *) exec;

	libBase->libNode.lib_Node.ln_Type = NT_LIBRARY;
	libBase->libNode.lib_Node.ln_Pri  = 0;
	libBase->libNode.lib_Node.ln_Name = LIBNAME;
	libBase->libNode.lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
	libBase->libNode.lib_Version      = VERSION;
	libBase->libNode.lib_Revision     = REVISION;
	libBase->libNode.lib_IdString     = VSTRING;

	libBase->segList = (BPTR)seglist;

	if (init() == FALSE)
	{
		close_libs();
		return NULL;
	}

	libBase_debug = libBase;

       return (struct Library *)libBase;
}

/* ------------------- Manager Interface ------------------------ */
/* These are generic. Replace if you need more fancy stuff */
STATIC uint32 _manager_Obtain(struct LibraryManagerInterface *Self)
{
	uint32 res;
	__asm__ __volatile__(
	"1:	lwarx	%0,0,%1\n"
	"addic	%0,%0,1\n"
	"stwcx.	%0,0,%1\n"
	"bne-	1b"
	: "=&r" (res)
	: "r" (&Self->Data.RefCount)
	: "cc", "memory");

	return res;
}

STATIC uint32 _manager_Release(struct LibraryManagerInterface *Self)
{
	uint32 res;
	__asm__ __volatile__(
	"1:	lwarx	%0,0,%1\n"
	"addic	%0,%0,-1\n"
	"stwcx.	%0,0,%1\n"
	"bne-	1b"
	: "=&r" (res)
	: "r" (&Self->Data.RefCount)
	: "cc", "memory");

	return res;
}

/* Manager interface vectors */
STATIC CONST APTR lib_manager_vectors[] =
{
	_manager_Obtain,
	_manager_Release,
	NULL,
	NULL,
	libOpen,
	libClose,
	libExpunge,
	NULL,
	(APTR)-1
};

/* "__library" interface tag list */
STATIC CONST struct TagItem lib_managerTags[] =
{
	{ MIT_Name,			(Tag)"__library"		},
	{ MIT_VectorTable,	(Tag)lib_manager_vectors},
	{ MIT_Version,		1						},
	{ TAG_DONE,			0						}
};

/* ------------------- Library Interface(s) ------------------------ */

#include "chipset_vectors.c"

/* Uncomment this line (and see below) if your library has a 68k jump table */

extern APTR VecTable68K[]; 

STATIC CONST struct TagItem main_v1_Tags[] =
{
	{ MIT_Name,			(Tag)"main"			},
	{ MIT_VectorTable,	(Tag)main_v1_vectors	},
	{ MIT_Version,		1					},
	{ TAG_DONE,			0					}
};

STATIC CONST CONST_APTR libInterfaces[] =
{
	lib_managerTags,
	main_v1_Tags,
	NULL
};

STATIC CONST struct TagItem libCreateTags[] =
{
	{ CLT_DataSize,		sizeof(struct Library)	},
	{ CLT_InitFunc,		(Tag)libInit			},
	{ CLT_Interfaces,	(Tag)libInterfaces		},
	/* Uncomment the following line if you have a 68k jump table */
	{ CLT_Vector68K, (Tag)VecTable68K },
	{TAG_DONE,		 0 }
};


/* ------------------- ROM Tag ------------------------ */
STATIC CONST struct Resident lib_res USED =
{
	RTC_MATCHWORD,
	(struct Resident *)&lib_res,
	(APTR)(&lib_res + 1),
	RTF_NATIVE|RTF_AUTOINIT, /* Add RTF_COLDSTART if you want to be resident */
	VERSION,
	NT_LIBRARY, /* Make this NT_DEVICE if needed */
	0, /* PRI, usually not needed unless you're resident */
	"chipset.library",
	VSTRING,
	(APTR)libCreateTags
};


/* $Id: Nalle.c,v 1.15 2001/05/04 08:43:33 lcs Exp $ */

/*
		 NallePUH -- Paula utan henne -- A minimal Paula emulator.
		 Copyright (C) 2001 Martin Blom <martin@blom.org>
		 
		 This program is free software; you can redistribute it and/or
		 modify it under the terms of the GNU General Public License
		 as published by the Free Software Foundation; either version 2
		 of the License, or (at your option) any later version.
		 
		 This program is distributed in the hope that it will be useful,
		 but WITHOUT ANY WARRANTY; without even the implied warranty of
		 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
		 GNU General Public License for more details.
		 
		 You should have received a copy of the GNU General Public License
		 along with this program; if not, write to the Free Software
		 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __amigaos4__
#include "CompilerSpecific.h"
#endif

#include <devices/ahi.h>
#include <exec/errors.h>
#include <exec/lists.h>
#include <exec/memory.h>

#ifndef NO_GUI
#include <classes/window.h>
#include <gadgets/listbrowser.h>
#include <intuition/gadgetclass.h>
#include <libraries/resource.h>
#endif

#include <clib/alib_protos.h>
#include <proto/ahi.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/expansion.h>

#ifndef NO_GUI
#include <proto/listbrowser.h>
#include <proto/resource.h>
#endif

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NallePUH.h"
#include "PUH.h"

#ifdef __amigaos4__
#define GETIFACE(x)	(I ## x = (struct x ## IFace *) GetInterface((struct Library *) x ## Base, "main", 1L, NULL))
#define DROPIFACE(x)	DropInterface((struct Interface *) I ## x);
#else
#define GETIFACE(x)
#define DROPIFACE(x)
#endif

extern UBYTE pooh11_sb[];
extern ULONG pooh11_sblen;

static BOOL
OpenLibs( void );

static void
CloseLibs( void );

static BOOL
OpenAHI( void );

static void
CloseAHI( void );

static BOOL
ShowGUI( struct PUHData* pd );

static BOOL
HandleGUI( Object* window,
					 struct Gadget** gadgets,
					 struct PUHData* pd );

/******************************************************************************
** Global variables ***********************************************************
******************************************************************************/

static struct MsgPort*			AHImp		 = NULL;
static struct AHIRequest*	 AHIio		 = NULL;
static BYTE								 AHIDevice = IOERR_OPENFAIL;

struct Library*			 AHIBase				 = NULL;
struct IntuitionBase* IntuitionBase	 = NULL;
struct Library	 *		LocaleBase			= NULL;
struct Library*			 MMUBase				 = NULL;
#ifndef NO_GUI
struct Library*			 ResourceBase		= NULL;
struct Library*			 ListBrowserBase = NULL;
#endif
struct UtilityBase *		 UtilityBase;

#ifdef __amigaos4__
struct AHIIFace *		 	IAHI;
struct IntuitionIFace *	IIntuition;
struct LocaleIFace *		ILocale;
struct UtilityIFace * 	IUtility;
#endif

/******************************************************************************
** Disable ctrl-c *************************************************************
******************************************************************************/

void __chkabort( void )
{
	// Disable automatic ctrl-c handling
}



/******************************************************************************
** main ***********************************************************************
******************************************************************************/

int
main( int	 argc,
			char* argv[] )
{
	int	 rc				= 0;
	BOOL	gui_mode	= FALSE;

	ULONG mode_id	 = 0;
	ULONG frequency = 0;
	ULONG level		 = 0;


	if( ! OpenLibs() )
	{
		return 20;
	}
#ifndef NO_GUI
	if( argc == 1 && ResourceBase != NULL )
	{
		// Gui mode

		gui_mode = TRUE;
	}
	else
#endif
	if( argc != 3 )
	{
		//Printf( "Usage: %s [0x]<AHI mode ID> <Frequency> <Level>\n", argv[ 0 ] );
		//Printf( "Level can be 0 (no patches), 1 (ROM patches) or 2 (appl. patches)\n" );
	 Printf( "Usage: %s [0x]<AHI mode ID> <Frequency>\n", argv[ 0 ] );
		return 10;
	}

	#ifdef __amigaos4__
	{
		struct Library	*ExpansionBase;
	 struct ExpansionIFace *IExpansion;
	 BOOL	 Classic = TRUE;

	 ExpansionBase = OpenLibrary( "expansion.library", 50 );
	 GETIFACE(Expansion);
	 if (IExpansion != NULL)
	 {
			STRPTR	extensions;

		GetMachineInfoTags(
			GMIT_Extensions, (ULONG) &extensions,
			TAG_DONE );

		if (!strstr(extensions, "classic.customchips"))
				Classic = FALSE;

	 	DROPIFACE(Expansion);
	 }

	 CloseLibrary(ExpansionBase);

	 if (Classic)
	 {
		 Printf( "Sorry, this program doesn't work on classic hardware\n" );
		return 10;
	 }
	}
	#endif

	if( ! gui_mode )
	{
		char* mode_ptr;
		char* freq_ptr;
		char* levl_ptr;

		mode_id	 = strtol( argv[ 1 ], &mode_ptr, 0 );
		frequency = strtol( argv[ 2 ], &freq_ptr, 0 );
		level		 = strtol( "0" /*argv[ 3 ]*/, &levl_ptr, 0 );
	 
		if( *mode_ptr != 0 || *freq_ptr != 0 || *levl_ptr != 0 )
		{
			Printf( "All arguments must be numbers.\n" );
			return 10;
		}

		if( level > 2 )
		{
			Printf( "Invalid value for Level.\n" );
			return 10;
		}
	}

	if( ! OpenAHI() )
	{
		Printf( "Unable to open ahi.device version 4.\n" );
		rc = 20;
	}

	if( rc == 0 )
	{
		struct PUHData* pd;

		pd = AllocPUH();
		
		if( pd == NULL )
		{
			rc = 20;
		}
		else
		{
			{
				ULONG flags = 0;
				
				LogPUH( pd, "Using mode ID 0x%08lx, %ld Hz.", mode_id, frequency );
				
				switch( level )
				{
					case 0:
						LogPUH( pd, "No patches." );

						flags = PUHF_NONE;
						break;

					case 1:
						LogPUH( pd, "ROM patches." );

						flags = PUHF_PATCH_ROM;
						break;
						
					case 2:
						LogPUH( pd, "ROM and application patches." );

						flags = PUHF_PATCH_ROM | PUHF_PATCH_APPS;
						break;
				}

				if( ! InstallPUH( flags,	mode_id, frequency,	pd ) )
				{
					rc = 20;
				}
				else
				{
					if( ! ActivatePUH( pd ) )
					{
						rc = 20;
					}
					else
					{
						LogPUH( pd, "Waiting for CTRL-C..." );
						Wait( SIGBREAKF_CTRL_C );
						LogPUH( pd, "Got it." );
				
						DeactivatePUH( pd );
					}
				
					UninstallPUH( pd );
				}
			}

			FreePUH( pd );
		}
	}


	CloseAHI();
	CloseLibs();

	return rc;
}

/******************************************************************************
** OpenAHI ********************************************************************
******************************************************************************/

/* Opens and initializes the device. */

static BOOL
OpenAHI( void )
{
	BOOL rc = FALSE;

	AHImp = CreateMsgPort();

	if( AHImp != NULL )
	{
		AHIio = (struct AHIRequest*) CreateIORequest( AHImp, 
																									sizeof( struct AHIRequest ) );

		if( AHIio != NULL ) 
		{
			AHIio->ahir_Version = 4;
			AHIDevice = OpenDevice( AHINAME,
															AHI_NO_UNIT,
															(struct IORequest*) AHIio,
															0UL );

			if( AHIDevice == 0 )
			{
				AHIBase = (struct Library*) AHIio->ahir_Std.io_Device;
			GETIFACE(AHI);
				rc = TRUE;
			}
		}
	}

	return rc;
}


/******************************************************************************
** CloseAHI *******************************************************************
******************************************************************************/

/* Closes the device, cleans up. */

static void
CloseAHI( void )
{
	if( AHIDevice == 0 )
	{
		 DROPIFACE(AHI);
		CloseDevice( (struct IORequest*) AHIio );
	}

	DeleteIORequest( (struct IORequest*) AHIio );
	DeleteMsgPort( AHImp );

	AHIBase	 = NULL;
	AHImp		 = NULL;
	AHIio		 = NULL;
	AHIDevice = IOERR_OPENFAIL;
}




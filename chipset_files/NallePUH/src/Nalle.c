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

#define __USE_INLINE__
#define asm
#define ASM
#define EXEC_AVL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "CompilerSpecific.h"

#include <proto/exec.h>
#include <exec/emulation.h>

#include <devices/ahi.h>

/*
#include <exec/errors.h>
#include <exec/lists.h>
#include <exec/memory.h>
*/

#include <libraries/resource.h>

//#include <clib/alib_protos.h>
#include <proto/ahi.h>
#include <proto/dos.h>

#include <proto/resource.h>
#include <proto/utility.h>

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#include "NallePUH.h"
#include "PUH.h"

extern UBYTE pooh11_sb[];
extern ULONG pooh11_sblen;

static BOOL OpenLibs( void );
static void CloseLibs( void );

static BOOL OpenAHI( void );
static void CloseAHI( void );

/******************************************************************************
** Global variables ***********************************************************
******************************************************************************/

static struct MsgPort		*AHImp= NULL;
static struct AHIRequest	*AHIio = NULL;
static BYTE			AHIDevice = IOERR_OPENFAIL;
struct Device			* AHIBase				 = NULL;


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
	
int nalle_main( int	 argc, char* argv[] )
{
	int		rc = 0;
	BOOL	gui_mode	= FALSE;

	ULONG	mode_id	= 0;
	ULONG	frequency	= 0;
	ULONG	level		= 0;


	if( ! OpenLibs() )
	{
		return 20;
	}

	if( argc == 1 && ResourceBase != NULL )
	{
		// Gui mode
		
		gui_mode = TRUE;
	}
	else if( argc != 4 )
	{
		printf( "Usage: %s [0x]<AHI mode ID> <Frequency> <Level>\n", argv[ 0 ] );
		printf( "Level can be 0 (no patches), 1 (ROM patches) or 2 (appl. patches)\n" );
		return 10;
	}

	if( ! gui_mode )
	{
		char* mode_ptr;
		char* freq_ptr;
		char* levl_ptr;

		mode_id	 = strtol( argv[ 1 ], &mode_ptr, 0 );
		frequency = strtol( argv[ 2 ], &freq_ptr, 0 );
		level		 = strtol( argv[ 3 ], &levl_ptr, 0 );

		if( *mode_ptr != 0 || *freq_ptr != 0 || *levl_ptr != 0 )
		{
			printf( "All arguments must be numbers.\n" );
			return 10;
		}

		if( level > 2 )
		{
			printf( "Invalid value for Level.\n" );
			return 10;
		}
	}

	if( ! OpenAHI() )
	{
		printf( "Unable to open ahi.device version 4.\n" );
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

static BOOLOpenAHI( void )
{
	BOOL rc = FALSE;

	AHImp = CreateMsgPort();
	if( AHImp != NULL )
	{
		AHIio = (struct AHIRequest*) CreateIORequest( AHImp, sizeof( struct AHIRequest ) );

		if( AHIio != NULL ) 
		{
			AHIio->ahir_Version = 4;
			AHIDevice = OpenDevice( AHINAME,
				AHI_NO_UNIT,
				(struct IORequest*) AHIio,
				NULL );
															
			if( AHIDevice == 0 )
			{
				AHIBase = (struct Library*) AHIio->ahir_Std.io_Device;
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

static void CloseAHI( void )
{
	if( AHIDevice == 0 )
	{
		CloseDevice( (struct IORequest*) AHIio );
	}

	DeleteIORequest( (struct IORequest*) AHIio );
	DeleteMsgPort( AHImp );

	AHIBase	 = NULL;
	AHImp		 = NULL;
	AHIio		 = NULL;
	AHIDevice = IOERR_OPENFAIL;
}




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

#include <devices/ahi.h>
#include <exec/errors.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <proto/ahi.h>
#include <proto/dos.h>
#include <proto/exec.h>
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
OpenAHI( void );

static void
CloseAHI( void );


/******************************************************************************
** Global variables ***********************************************************
******************************************************************************/

static struct MsgPort*	AHImp		 = NULL;
static struct AHIRequest*	 AHIio		 = NULL;
static BYTE			 AHIDevice = IOERR_OPENFAIL;
struct Device*			 AHIBase	 = NULL;


#ifdef __amigaos4__
struct AHIIFace *		 	IAHI;
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

		struct PUHData* pd = NULL;

int nallepuh_main(  	ULONG mode_id,ULONG frequency )
{
	int	 rc				= 0;

	if( ! OpenAHI() )
	{
		Printf( "Unable to open ahi.device version 4.\n" );
		rc = 20;
	}

	pd = AllocPUH();
		
	if( pd == NULL )
	{
		rc = 20;
	}
	else
	{
		ULONG flags = 0;
				
		flags = PUHF_NONE;

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
				Wait( SIGBREAKF_CTRL_C );
				DeactivatePUH( pd );
			}
				
			UninstallPUH( pd );
		}
		FreePUH( pd );
	}

	CloseAHI();
	return rc;
}

/******************************************************************************
** OpenAHI ********************************************************************
******************************************************************************/

/* Opens and initializes the device. */

static BOOL OpenAHI( void )
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
			AHIDevice = OpenDevice( AHINAME, AHI_NO_UNIT, (struct IORequest*) AHIio, 0UL );

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

static void CloseAHI( void )
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




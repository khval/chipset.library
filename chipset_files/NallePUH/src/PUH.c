/* $Id: PUH.c,v 1.17 2001/05/04 08:43:33 lcs Exp $ */

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
#define __NOLIBBASE__

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <proto/exec.h>

#undef __NOLIBBASE__

#include <proto/dos.h>

#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <devices/ahi.h>
#include <exec/emulation.h>
#include <graphics/gfxbase.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#include <proto/ahi.h>
#include <proto/exec.h>

#include <proto/utility.h>
#include <proto/dos.h>

#include <stdio.h>

#include "PUH.h"

#define SIZEOF_SHORT 2
#define SIZEOF_INT 4

#include "uae/sysdeps.h"
#include "uae/memory.h"


#define INTF_AUDIO	 ( INTF_AUD3 | INTF_AUD2 | INTF_AUD1 | INTF_AUD0 )

//#define DEBUG(...)	DebugPrintF(__VA_ARGS__)
#define DEBUG(...)

static bool puh_RemapMemory( struct PUHData* pd );

static BOOL RestoreMemory( struct PUHData* pd );
UWORD PUHRead( UWORD reg, BOOL *handled );
void PUHWrite( UWORD reg, UWORD value, BOOL *handled );

SAVEDS static void PUHSoundFunc( REG( a0, struct Hook *hook ),	REG( a2, struct AHIAudioCtrl *actrl ), REG( a1, struct AHISoundMessage* msg ) );
//ULONG DataFaultHandler(struct ExceptionContext *pContext, struct ExecBase *pSysBase, struct PUHData *pd);

SAVEDS static void PUHSoftInt( struct ExceptionContext *pContext, struct ExecBase *pSysBase, struct PUHData *pd );




/******************************************************************************
** The chip registers we trigger on *******************************************
******************************************************************************/

#define DMACONR 0x002
#define ADKCONR 0x010
#define INTENAR 0x01c
#define INTREQR 0x01e
#define DMACON	0x096
#define INTENA	0x09a
#define INTREQ	0x09c
#define ADKCON	0x09e

#define AUD0LCH 0x0a0
#define AUD0LCL 0x0a2
#define AUD0LEN 0x0a4
#define AUD0PER 0x0a6
#define AUD0VOL 0x0a8
#define AUD0DAT 0x0aa

#define AUD1LCH 0x0b0
#define AUD1LCL 0x0b2
#define AUD1LEN 0x0b4
#define AUD1PER 0x0b6
#define AUD1VOL 0x0b8
#define AUD1DAT 0x0ba

#define AUD2LCH 0x0c0
#define AUD2LCL 0x0c2
#define AUD2LEN 0x0c4
#define AUD2PER 0x0c6
#define AUD2VOL 0x0c8
#define AUD2DAT 0x0ca

#define AUD3LCH 0x0d0
#define AUD3LCL 0x0d2
#define AUD3LEN 0x0d4
#define AUD3PER 0x0d6
#define AUD3VOL 0x0d8
#define AUD3DAT 0x0da

/******************************************************************************
** Read and write hardware registers ******************************************
******************************************************************************/

STATIC UBYTE CustomData[512];

UWORD ReadWord( void* address )
{
	ULONG offset = (ULONG)address & 0x1ff;

	return ((UWORD *)CustomData)[offset/2];
}


void WriteWord( void* address, UWORD value )
{
	ULONG offset = (ULONG)address & 0x1ff;

	((UWORD *) address)[offset/2] = value;
}


ULONG ReadLong( void* address )
{
	ULONG offset = (ULONG)address & 0x1ff;

	return ((UWORD *)CustomData)[offset/4];
}


void WriteLong( void* address, ULONG value )
{
	ULONG offset = (ULONG)address & 0x1ff;

	((ULONG *) address)[offset/4] = value;
}



/******************************************************************************
** Initialize PUH *************************************************************
******************************************************************************/

extern struct PUHData* pd;

struct PUHData*AllocPUH( void )
{

	struct GfxBase* gfxbase = NULL;

	if( AHIBase == NULL )
	{
		Printf( "AHIBase not initialized!\n" );
		return NULL;
	}

	gfxbase = (struct GfxBase*) OpenLibrary( GRAPHICSNAME, 39 );

	{
		pd = (struct PUHData*) AllocVec( sizeof( struct PUHData ), MEMF_PUBLIC | MEMF_CLEAR );

		if( pd == NULL )
		{
			Printf( "Out of memory!\n" );
		}
		else
		{
			pd->m_Active							 = FALSE;

			pd->m_AudioMode						= AHI_INVALID_ID;

			pd->m_SoundFunc.h_Entry		= (ULONG(*)(void)) PUHSoundFunc;
			pd->m_SoundFunc.h_Data		 = pd;

			if ( gfxbase != NULL &&
				 ( gfxbase->DisplayFlags & REALLY_PAL ) == 0 )
			{
				// NTSC
				pd->m_ChipFreq = 3579545;
			}
			else
			{
				// PAL
				pd->m_ChipFreq = 3546895;
			}

			pd->m_DMACON					= DMAF_MASTER;	// 0x200 

			pd -> m_DMACON					|= DMAF_BLITTER;	// enable blitter.

			pd->m_INTENA						= INTF_INTEN;

			pd->m_Intercepted					= (void*) 0xdff000;
			pd->m_CustomDirect				= (void*) 0xdff000;
			pd->m_CustomSize					= 0x200;

			pd->m_SoftInt.is_Node.ln_Type = NT_EXTINTERRUPT;
			pd->m_SoftInt.is_Node.ln_Pri	= 32;
			pd->m_SoftInt.is_Node.ln_Name = "NallePUH Level 4 emulation";
			pd->m_SoftInt.is_Data				 = pd;
			pd->m_SoftInt.is_Code				 = (void(*)(void)) PUHSoftInt;
		}
	}

	CloseLibrary( (struct Library*) gfxbase );

	return pd;
}


/******************************************************************************
** Deallocate PUH *************************************************************
******************************************************************************/

void FreePUH( struct PUHData* pd )
{
	if( pd != NULL )
	{
		DeactivatePUH( pd );
		UninstallPUH( pd );

		FreeVec( pd );
	}
}


/******************************************************************************
** Set/change log hook ********************************************************
******************************************************************************/

void SetPUHLogger( struct Hook *hook, struct PUHData* pd )
{
	pd->m_LogHook = hook;
}


/******************************************************************************
** Send a message to the log function *****************************************
******************************************************************************/

void VARARGS68K LogPUH( struct PUHData* pd, STRPTR fmt, ... )
{
	va_list ap;

	#ifdef __amigaos4__
		va_startlinear( ap, fmt );
	#else
		va_start( ap, fmt );
	#endif

	if( pd->m_LogHook == NULL )
	{
		VPrintf( fmt, va_getlinearva(ap,void *) );
		Printf( "\n" );
	}
	else
	{
		char		buffer[ 256 ];
		 #ifdef __amigaos4__
			 VSNPrintf( buffer, sizeof( buffer ), fmt, va_getlinearva(ap, void *) );
		 #else
			vsnprintf( buffer, sizeof( buffer ), fmt, ap );
		 #endif

		CallHookPkt( pd->m_LogHook, pd, buffer );
	}

	va_end( ap );
}

/******************************************************************************
** Installl PUH ***************************************************************
******************************************************************************/

BOOL InstallPUH( ULONG flags,ULONG audio_mode,ULONG frequency,struct PUHData* pd )
{
	BOOL ahi_ok = FALSE;

	pd->m_Flags		 = flags;
	pd->m_AudioMode = audio_mode;

	// Activate AHI

	pd->m_AudioCtrl = AHI_AllocAudio( AHIA_AudioID,		audio_mode,
				AHIA_MixFreq,		frequency,
				AHIA_Channels,	 4,
				AHIA_Sounds,		 1,
				AHIA_SoundFunc,	(ULONG) &pd->m_SoundFunc,
				//AHIA_PlayerFreq, 100 << 16,
				TAG_DONE );

	if( pd->m_AudioCtrl == NULL )
	{
		printf( "Unable to allocate audio mode $%08lx.", audio_mode );
	}
	else
	{
		struct AHISampleInfo si =
		{
			AHIST_M8S,	 // An 8-bit sample
			0,			 // beginning at address 0
			0xffffffff		 // and ending at the last address.
		};

		if( AHI_LoadSound( 0, AHIST_DYNAMICSAMPLE, &si, pd->m_AudioCtrl ) != AHIE_OK )
		{
			printf( "Unable to load dynamic sample." );
		}
		else
		{
			if( AHI_ControlAudio( pd->m_AudioCtrl, AHIC_Play, TRUE, TAG_DONE ) != AHIE_OK )
			{
				printf( "Unable to start playback." );
			}
			else
			{
				ahi_ok = TRUE;
			}
		}
	}

	if( ! ahi_ok )
	{
		// Clean up

		UninstallPUH( pd );
		return FALSE;
	}

/*
	pd->m_FaultInt.is_Node.ln_Type = NT_EXTINTERRUPT;
	pd->m_FaultInt.is_Node.ln_Pri	= 0;
	pd->m_FaultInt.is_Node.ln_Name = "NallePUH data fault handler";
	pd->m_FaultInt.is_Code = (void (*)(void))DataFaultHandler;
	pd->m_FaultInt.is_Data = (APTR)pd;

	Forbid();
	pd->m_OldFaultInt = SetIntVector(TRAPNUM_DATA_SEGMENT_VIOLATION, &pd->m_FaultInt);
	Permit();

*/

	pd->m_Active = TRUE;

	if( ! pd->m_Active )
	{
		// Clean up

		UninstallPUH( pd );
	}

	return pd->m_Active;
}


/******************************************************************************
** Uninstall PUH **************************************************************
******************************************************************************/

void UninstallPUH( struct PUHData* pd )
{
	if( pd == NULL )
	{
		return;
	}

	DeactivatePUH( pd );

	if (pd->m_OldFaultInt != NULL)
	{
		 Forbid();
		SetIntVector(TRAPNUM_DATA_SEGMENT_VIOLATION, pd->m_OldFaultInt);
		 Permit();
	}

	if( pd->m_Active )
	{
		RestoreMemory( pd );
	}

	if( pd->m_AudioCtrl != NULL )
	{
		AHI_FreeAudio( pd->m_AudioCtrl );
		pd->m_AudioCtrl = NULL;
	}

	pd->m_Flags		= 0L;
	pd->m_AudioMode	= AHI_INVALID_ID;
	pd->m_Active		= FALSE;
}

/******************************************************************************
** Activate PUH ***************************************************************
******************************************************************************/

STATIC BOOL PUH_ON = FALSE;

BOOL ActivatePUH( struct PUHData* pd )
{
	PUH_ON = TRUE;
	return TRUE;
}

/******************************************************************************
** Deactivate PUH *************************************************************
******************************************************************************/

void DeactivatePUH( struct PUHData* pd )
{
	PUH_ON = FALSE;
}

/******************************************************************************
** Activate MMU ***************************************************************
******************************************************************************/

static bool puh_RemapMemory( struct PUHData* pd )
{
	return TRUE;
}

/******************************************************************************
** Restore MMU ****************************************************************
******************************************************************************/

// Note that this code will NOT restore the attributes for
// pd->m_CustomDirect, since the patched instructions in application code
// needs it even after Nalle PUH has terminated.

static BOOL RestoreMemory( struct PUHData* pd )
{
	return TRUE;
}

/******************************************************************************
** Handle reads ***************************************************************
******************************************************************************/

extern uae_u32 REGPARAM2 custom_wget (uaecptr addr );

UWORD PUHRead( UWORD reg, BOOL *handled )
{
	UWORD	result;
	UWORD* address = (UWORD*) ( (ULONG) pd->m_CustomDirect + reg );

	switch( reg )
	{
		case DMACONR:
			result	= ReadWord( address );
			result &= ~DMAF_AUDIO;
			result |= ( pd->m_DMACON & DMAF_AUDIO );

			*handled = TRUE;
			break;

		case INTENAR:
			result	= ReadWord( address );
			result &= ~INTF_AUDIO;
			result |= ( pd->m_INTENA & INTF_AUDIO );

			*handled = TRUE;
			break;

		case INTREQR:
			result	= ReadWord( address );
			result &= ~INTF_AUDIO;
			result |= ( pd->m_INTREQ & INTF_AUDIO );

			*handled = TRUE;
			break;

		default:
			// Just carry out the intercepted read operation

			result = custom_wget ( address );
			break;
	}

	return result;
}


/******************************************************************************
** Handle writes **************************************************************
******************************************************************************/
	
extern bool custom_wput ( uaecptr addr, uae_u32 value);

void PUHWrite( UWORD	reg, UWORD value, BOOL*handled )
{
	UWORD* address = (UWORD*) ( (ULONG) pd->m_CustomDirect + reg );

	switch( reg )
	{
		case DMACON:
		{
			UWORD old_dmacon;
			UWORD new_dmacon;
			UWORD xor_dmacon;

			if( pd->m_DMACON & DMAF_MASTER )
			{
				old_dmacon = pd->m_DMACON;
			}
			else
			{
				old_dmacon = 0;
			}

			if( value & DMAF_SETCLR )
			{
				pd->m_DMACON |= ( value & ~DMAF_SETCLR );
			}
			else
			{
				pd->m_DMACON &= ~( value & ~DMAF_SETCLR );
			}

			if( pd->m_DMACON & DMAF_MASTER )
			{
				new_dmacon = pd->m_DMACON;
			}
			else
			{
				new_dmacon = 0;
			}

			xor_dmacon = old_dmacon ^ new_dmacon;

			if( xor_dmacon & DMAF_AUD0 )
			{
				if( new_dmacon & DMAF_AUD0 )
				{
					pd->m_SoundOn[ 0 ] = TRUE;

					if( pd->m_SoundLength[ 0 ] == 2 )
					{
						// SoundTracker-style silece
						AHI_SetSound( 0, AHI_NOSOUND, 0, 0, pd->m_AudioCtrl, AHISF_IMM );
					}
					else
					{
						AHI_SetSound( 0, 0,
								pd->m_SoundLocation[ 0 ],
								pd->m_SoundLength[ 0 ],
								pd->m_AudioCtrl,
								AHISF_IMM );
					}
				}
				else
				{
					pd->m_SoundOn[ 0 ]	 = FALSE;

					AHI_SetSound( 0, AHI_NOSOUND, 0, 0, pd->m_AudioCtrl, AHISF_IMM );
				}
			}

			if( xor_dmacon & DMAF_AUD1 )
			{
				if( new_dmacon & DMAF_AUD1 )
				{
					pd->m_SoundOn[ 1 ] = TRUE;

					if( pd->m_SoundLength[ 1 ] == 2 )
					{
						// SoundTracker-style silece
						AHI_SetSound( 1, AHI_NOSOUND, 0, 0, pd->m_AudioCtrl, AHISF_IMM );
					}
					else
					{
						AHI_SetSound( 1, 0,
								pd->m_SoundLocation[ 1 ],
								pd->m_SoundLength[ 1 ],
								pd->m_AudioCtrl,
								AHISF_IMM );
					}
				}
				else
				{
					pd->m_SoundOn[ 1 ]	 = FALSE;

					AHI_SetSound( 1, AHI_NOSOUND, 0, 0, pd->m_AudioCtrl, AHISF_IMM );
				}
			}

			if( xor_dmacon & DMAF_AUD2 )
			{
				if( new_dmacon & DMAF_AUD2 )
				{
					pd->m_SoundOn[ 2 ] = TRUE;

					if( pd->m_SoundLength[ 2 ] == 2 )
					{
						// SoundTracker-style silece
						AHI_SetSound( 2, AHI_NOSOUND, 0, 0, pd->m_AudioCtrl, AHISF_IMM );
					}
					else
					{
						AHI_SetSound( 2, 0,
								pd->m_SoundLocation[ 2 ],
								pd->m_SoundLength[ 2 ],
								pd->m_AudioCtrl,
								AHISF_IMM );
					}
				}
				else
				{
					pd->m_SoundOn[ 2 ]	 = FALSE;

					AHI_SetSound( 2, AHI_NOSOUND, 0, 0, pd->m_AudioCtrl, AHISF_IMM );
				}
			}

			if( xor_dmacon & DMAF_AUD3 )
			{
				if( new_dmacon & DMAF_AUD3 )
				{
					pd->m_SoundOn[ 3 ] = TRUE;

					if( pd->m_SoundLength[ 3 ] == 2 )
					{
						// SoundTracker-style silece
						AHI_SetSound( 3, AHI_NOSOUND, 0, 0, pd->m_AudioCtrl, AHISF_IMM );
					}
					else
					{
						AHI_SetSound( 3, 0,
								pd->m_SoundLocation[ 3 ],
								pd->m_SoundLength[ 3 ],
								pd->m_AudioCtrl,
								AHISF_IMM );
					}
				}
				else
				{
					pd->m_SoundOn[ 3 ]	 = FALSE;

					AHI_SetSound( 3, AHI_NOSOUND, 0, 0, pd->m_AudioCtrl, AHISF_IMM );
				}
			}

			WriteWord( address, value & ~DMAF_AUDIO );

			*handled = TRUE;
			break;
		}

		case INTENA:

			if( value & INTF_SETCLR )
			{
				pd->m_INTENA |= ( value & ~INTF_SETCLR );
			}
			else
			{
				pd->m_INTENA &= ~( value & ~INTF_SETCLR );
			}

			WriteWord( address, value & ~INTF_AUDIO );

			if( ( pd->m_INTENA & pd->m_INTREQ & INTF_AUDIO ) &&
					! pd->m_CausePending )
			{
				pd->m_CausePending = TRUE;
				Cause( &pd->m_SoftInt );
			}

			if( value & INTF_AUDIO )
			{
				*handled = TRUE;
			}
			break;

		case INTREQ:

			if( value & INTF_SETCLR )
			{
				pd->m_INTREQ |= ( value & ~INTF_SETCLR );
			}
			else
			{
				pd->m_INTREQ &= ~( value & ~INTF_SETCLR );
			}

			WriteWord( address, value & ~INTF_AUDIO );

			if( ( pd->m_INTENA & pd->m_INTREQ & INTF_AUDIO ) &&	 !pd->m_CausePending )
			{
				pd->m_CausePending = TRUE;
				Cause( &pd->m_SoftInt );
			}

			if( value & INTF_AUDIO )
			{
				*handled = TRUE;
			}
			break;


		case ADKCON:
			WriteWord( address, value );
			break;

		case AUD0LCH:
		case AUD1LCH:
		case AUD2LCH:
		case AUD3LCH:
			{
				int channel		 = ( reg - AUD0LCH ) >> 4;

				pd->m_SoundLocation[ channel ] &= 0x0000ffff;
				pd->m_SoundLocation[ channel ] |= value << 16;

				*handled = TRUE;

				pd->m_GotDatHi[channel] = TRUE;

				if (pd->m_GotDatLo[channel])
				{
					pd->m_GotDatHi[channel] =
					pd->m_GotDatLo[channel] = FALSE;

					if( pd->m_SoundOn[ channel ] )
					{
						// Queue it
						if( pd->m_SoundLength[ channel ] == 2 )
						{
							// SoundTracker-style silece
							AHI_SetSound( channel, AHI_NOSOUND,	0, 0, pd->m_AudioCtrl, AHISF_NONE );
						}
						else
						{
							AHI_SetSound( channel, 0,
								pd->m_SoundLocation[ channel ],
								pd->m_SoundLength[ channel ],
								pd->m_AudioCtrl,
								AHISF_NONE );
						}
					}
				}
			}
			break;

		case AUD0LCL:
		case AUD1LCL:
		case AUD2LCL:
		case AUD3LCL:
		{
			int channel		 = ( reg - AUD0LCL ) >> 4;

			pd->m_SoundLocation[ channel ] &= 0xffff0000;
			pd->m_SoundLocation[ channel ] |= value;

			pd->m_GotDatLo[channel] = TRUE;

			if (pd->m_GotDatHi[channel])
			{
				pd->m_GotDatHi[channel] =
				pd->m_GotDatLo[channel] = FALSE;

				if( pd->m_SoundOn[ channel ] )
				{
					// Queue it
					if( pd->m_SoundLength[ channel ] == 2 )
					{
						// SoundTracker-style silece
						AHI_SetSound( channel, AHI_NOSOUND,
							0, 0, pd->m_AudioCtrl, AHISF_NONE );
					}
					else
					{
						AHI_SetSound( channel, 0,
							pd->m_SoundLocation[ channel ],
							pd->m_SoundLength[ channel ],
							pd->m_AudioCtrl,
							AHISF_NONE );
					}
				}
			}

			*handled = TRUE;
			break;
		}

		case AUD0LEN:
		case AUD1LEN:
		case AUD2LEN:
		case AUD3LEN:
		{
			int channel = ( reg - AUD0LEN ) >> 4;
			
			pd->m_SoundLength[ channel ] = value * 2;

			if( pd->m_SoundOn[ channel ] )
			{
				// Queue it
				if( pd->m_SoundLength[ channel ] == 2 )
				{
					// SoundTracker-style silece
					AHI_SetSound( channel, AHI_NOSOUND,
							0, 0, pd->m_AudioCtrl, AHISF_NONE );
				}
				else
				{
					AHI_SetSound( channel, 0,
							pd->m_SoundLocation[ channel ],
							pd->m_SoundLength[ channel ],
							pd->m_AudioCtrl,
							AHISF_NONE );
				}
			}

			*handled = TRUE;
			break;
		}

		case AUD0PER:
		case AUD1PER:
		case AUD2PER:
		case AUD3PER:
		{
			int	 channel = ( reg - AUD0PER ) >> 4;

			if( value == 0 )
			{
				// What is the correct emulation for this?

				AHI_SetFreq( channel,
							 pd->m_ChipFreq >> 16,
							 pd->m_AudioCtrl,
							 AHISF_IMM );
			}
			else
			{
				AHI_SetFreq( channel,
							 pd->m_ChipFreq / value,
							 pd->m_AudioCtrl,
							 AHISF_IMM );
			}

			*handled = TRUE;
			break;
		}

		case AUD0VOL:
			AHI_SetVol( 0,
									value << 10,
									0x10000,
									pd->m_AudioCtrl,
									AHISF_IMM );

			*handled = TRUE;
			break;

		case AUD1VOL:
			AHI_SetVol( 1,
									value << 10,
									0x0,
									pd->m_AudioCtrl,
									AHISF_IMM );

			*handled = TRUE;
			break;

		case AUD2VOL:
			AHI_SetVol( 2,
									value << 10,
									0x0,
									pd->m_AudioCtrl,
									AHISF_IMM );

			*handled = TRUE;
			break;

		case AUD3VOL:
			AHI_SetVol( 3,
									value << 10,
									0x10000,
									pd->m_AudioCtrl,
									AHISF_IMM );

			*handled = TRUE;
			break;

		case AUD0DAT:
			if( ( pd->m_DMACON & DMAF_AUD0 ) == 0 )
			{
				pd->m_INTREQ |= INTF_AUD0;

				if( ( pd->m_INTENA & INTF_AUD0 ) &&
						! pd->m_CausePending )
				{
					pd->m_CausePending = TRUE;
					Cause( &pd->m_SoftInt );
				}
			}

			*handled = TRUE;
			break;

		case AUD1DAT:
			if( ( pd->m_DMACON & DMAF_AUD1 ) == 0 )
			{
				pd->m_INTREQ |= INTF_AUD1;

				if( ( pd->m_INTENA & INTF_AUD1 ) &&
						! pd->m_CausePending )
				{
					pd->m_CausePending = TRUE;
					Cause( &pd->m_SoftInt );
				}
			}

			*handled = TRUE;
			break;

		case AUD2DAT:
			if( ( pd->m_DMACON & DMAF_AUD2 ) == 0 )
			{
				pd->m_INTREQ |= INTF_AUD2;

				if( ( pd->m_INTENA & INTF_AUD2 ) &&
						! pd->m_CausePending )
				{
					pd->m_CausePending = TRUE;
					Cause( &pd->m_SoftInt );
				}
			}

			*handled = TRUE;
			break;

		case AUD3DAT:
			if( ( pd->m_DMACON & DMAF_AUD3 ) == 0 )
			{
				pd->m_INTREQ |= INTF_AUD3;

				if( ( pd->m_INTENA & INTF_AUD3 ) &&
						! pd->m_CausePending )
				{
					pd->m_CausePending = TRUE;
					Cause( &pd->m_SoftInt );
				}
			}

			*handled = TRUE;
			break;

		default:

			// do the normal stuff.
			if (custom_wput ( address,  value) == false)
			{

				// Just carry out the intercepted write operation
				WriteWord( address, value );
			}
			break;
	}
}


/******************************************************************************
** Audio interrupt simulation *************************************************
******************************************************************************/

STATIC ULONG CallInt(UWORD irq, UWORD mask, struct ExceptionContext *pContext, struct ExecBase *SysBase)
{
	struct IntVector	*iv = &SysBase->IntVects[irq];
	struct Interrupt	*is = (struct Interrupt *) iv->iv_Node;
	ULONG	result;

	if (is == NULL)
		return 0;

	if (is->is_Node.ln_Type != NT_EXTINTERRUPT)
	{
		result = EmulateTags( is->is_Code,
			ET_SaveParamRegs, TRUE,
			ET_SuperState,	TRUE,
			ET_RegisterA0, 0xdff000,
			ET_RegisterD1, mask,
			ET_RegisterA1, is->is_Data,
			ET_RegisterA5, is->is_Code,
			ET_RegisterA6, SysBase,
			TAG_DONE );
	}
	else
	{
		result = ((ULONG (*)(struct ExceptionContext *, struct ExecBase *, APTR))is->is_Code)(pContext, SysBase, is->is_Data);
	}

	return result;
}

SAVEDS static void PUHSoftInt(struct ExceptionContext *pContext, struct ExecBase *pSysBase, struct PUHData *pd)
{
	UWORD	 mask;

	 pd->m_CausePending = FALSE;

	mask = (pd->m_INTREQ & pd->m_INTENA);

	do
	{
		if (mask & (1<<8)) // channel 1
		{
			CallInt(8, mask, pContext, pSysBase);
		}
		else if (mask & (1<<10)) // channel 3
		{
			CallInt(10, mask, pContext, pSysBase);
		}
		else if (mask & (1<<7)) // channel 0
		{
			CallInt(7, mask, pContext, pSysBase);
		}
		else if (mask & (1<<9)) // channel 2
		{
			CallInt(9, mask, pContext, pSysBase);
		}
		else break;

		mask = (pd->m_INTREQ & pd->m_INTENA) & 0x0780;
	}
	while (mask != 0);
}

SAVEDS static void PUHSoundFunc( REG( a0, struct Hook *hook ), REG( a2, struct AHIAudioCtrl *actrl ), REG( a1, struct AHISoundMessage* msg ) )
{
	struct PUHData* pd		= (struct PUHData*) hook->h_Data;

	pd->m_INTREQ |= ( 1 << ( INTB_AUD0 + msg->ahism_Channel ) );

	if( pd->m_INTENA & pd->m_INTREQ & INTF_AUDIO )
	{
		if (!pd->m_CausePending)
		{
			pd->m_CausePending = TRUE;
	 		Cause(&pd->m_SoftInt);
		}
	}
}

/* $Id: PUH.h,v 1.9 2001/04/28 17:48:33 lcs Exp $ */

/*
     NallePUH -- Paula utan henne -- A minimal Paula emulator.
     Copyright (C) 2001 Martin Blom <martin@blom.org>
     
     This program is free software; you can redistribute it and/or
     modify it under the terms of the GNU General Public License
     as published by the Free Software Foundation; either version 2
     of the License, or (at your option) any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef NallePUH_PUH_h
#define NallePUH_PUH_h

#include "CompilerSpecific.h"

#include <exec/interrupts.h>
#include <utility/hooks.h>

struct ExceptionData;
struct ExecBase;
struct Library;
struct MMUContext;

/* Debugging */

void
KPrintFArgs( UBYTE* fmt, 
             LONG*  args );


#define KPrintF( fmt, ... )        \
({                                 \
  LONG _args[] = { __VA_ARGS__ };  \
  KPrintFArgs( (fmt), _args );     \
})


/* Make nice accesses to hardware registers */

UWORD
ReadWord( void* address );


void
WriteWord( void* address, UWORD value );


ULONG
ReadLong( void* address );


void
WriteLong( void* address, ULONG value );


/* The emulator */


/* Flags for InstallPUH() and PUHData.m_Flags */

#define PUHF_NONE       0L
#define PUHF_PATCH_ROM	(1L << 0)
#define PUHF_PATCH_APPS	(1L << 1)
#define PUHF_TOGGLE_LED (1L << 2)

#define PUHB_PATCH_ROM	0
#define PUHB_PATCH_APPS	1
#define PUHB_TOGGLE_LED 2


struct PUHData
{
  BOOL                  m_Active;
  UWORD                 m_Pad;

  ULONG			m_Flags;

  struct Hook*          m_LogHook;

  ULONG                 m_AudioMode;
  struct AHIAudioCtrl*  m_AudioCtrl;

  struct Hook           m_SoundFunc;

  ULONG                 m_ChipFreq;

  UWORD                 m_DMACON;
  UWORD                 m_INTREQ;
  UWORD                 m_INTENA;
  BOOL                  m_CausePending;

  BOOL                  m_SoundOn[ 4 ];

  ULONG                 m_SoundLocation[ 4 ];
  ULONG                 m_SoundLength[ 4 ];

  void*                 m_Intercepted;
  void*                 m_CustomDirect;

  struct MMUContext*    m_UserContext;
  struct MMUContext*    m_SuperContext;
  
  struct ExceptionHook* m_UserException;
  struct ExceptionHook* m_SuperException;

  void*                 m_ROM;
  void*                 m_ROMShadowBuffer;

  ULONG	                m_ROMSize;
  ULONG	                m_CustomSize;

  struct
  {  
    ULONG               m_UserROM;
    ULONG               m_UserROMShadow;
    ULONG               m_UserCustom;
    ULONG               m_SuperROM;
    ULONG               m_SuperROMShadow;
    ULONG               m_SuperCustom;
  } m_Properties;

  struct Interrupt     m_SoftInt;

};


struct PUHData*
AllocPUH( void );


void
FreePUH( struct PUHData* pd );


void
SetPUHLogger( struct Hook*    hook,
              struct PUHData* pd );


void
LogPUH( struct PUHData* pd,
        STRPTR          fmt,
        ... );


BOOL
InstallPUH( ULONG           flags,
            ULONG           audio_mode,
            ULONG           frequency,
	    struct PUHData* pd );


void
UninstallPUH( struct PUHData* pd );


BOOL
ActivatePUH( struct PUHData* pd );


void
DeactivatePUH( struct PUHData* pd );

#endif /* NallePUH_PUH_h */

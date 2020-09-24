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
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <dos/dos.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <devices/ahi.h>
#include <graphics/gfxbase.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <mmu/context.h>
#include <mmu/descriptor.h>
#include <mmu/exceptions.h>
#include <mmu/mmubase.h>
#include <mmu/mmutags.h>

#include <proto/ahi.h>
#include <proto/exec.h>
#include <proto/mmu.h>
#include <proto/utility.h>

#include <stdio.h>

#include "PUH.h"

#define ROMEND       0x01000000L
#define MAGIC_ROMEND 0x14L

#define INTF_AUDIO   ( INTF_AUD3 | INTF_AUD2 | INTF_AUD1 | INTF_AUD0 )


static BOOL
RemapMemory( struct PUHData* pd );


static BOOL
RestoreMemory( struct PUHData* pd );


static void
PatchROMShadowBuffer( struct PUHData* pd );


ASMCALL INTERRUPT static ULONG
PUHHandler( REG( a0, struct ExceptionData* ed ),
            REG( a1, struct PUHData*       pd ),
            REG( a6, struct Library*       MMUBase ) );


static UWORD
PUHRead( UWORD            reg, 
         BOOL*            handled,
         struct PUHData*  pd,
         struct ExecBase* SysBase );


static void
PUHWrite( UWORD            reg, 
          UWORD            value,
          BOOL*            handled,
          struct PUHData*  pd,
          struct ExecBase* SysBase );


ASMCALL SAVEDS static void
PUHSoundFunc( REG( a0, struct Hook*            hook ),
              REG( a2, struct AHIAudioCtrl*    actrl ),
              REG( a1, struct AHISoundMessage* msg ) );


ASMCALL SAVEDS static void
PUHSoftInt( REG( d1, UWORD           active_ints ),
            REG( a0, struct Custom*  custom ),
            REG( a1, struct PUHData* pd ),
            REG( a5, void*           me ),
            REG( a6, struct ExecBase* SysBase ) );


/******************************************************************************
** The chip registers we trigger on *******************************************
******************************************************************************/

#define DMACONR 0x002
#define ADKCONR 0x010
#define INTENAR 0x01c
#define INTREQR 0x01e
#define DMACON  0x096
#define INTENA  0x09a
#define INTREQ  0x09c
#define ADKCON  0x09e

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
** Send debug to serial port **************************************************
******************************************************************************/

static UWORD rawputchar_m68k[] = 
{
  0x2C4B,             // MOVEA.L A3,A6
  0x4EAE, 0xFDFC,     // JSR     -$0204(A6)
  0x4E75              // RTS
};


void
KPrintFArgs( UBYTE* fmt, 
             LONG*  args )
{
  RawDoFmt( fmt, args, (void(*)(void)) rawputchar_m68k, SysBase );
}


/******************************************************************************
** Read and write hardware registers ******************************************
******************************************************************************/

UWORD
ReadWord( void* address )
{
  return *((UWORD*) address);
}


void
WriteWord( void* address, UWORD value )
{
  *((UWORD*) address) = value;
}


ULONG
ReadLong( void* address )
{
  return *((ULONG*) address);
}


void
WriteLong( void* address, ULONG value )
{
  *((ULONG*) address) = value;
}



/******************************************************************************
** Initialize PUH *************************************************************
******************************************************************************/

struct PUHData*
AllocPUH( void )
{
  struct PUHData* pd      = NULL;
  struct GfxBase* gfxbase = NULL;

  if( MMUBase == NULL )
  {
    printf( "MMUBase not initialized!\n" );
    return NULL;
  }

  if( AHIBase == NULL )
  {
    printf( "AHIBase not initialized!\n" );
    return NULL;
  }

  gfxbase = (struct GfxBase*) OpenLibrary( GRAPHICSNAME, 39 );
  
  if( GetMMUType() == MUTYPE_NONE )
  {
    printf( "This program requires an MMU.\n" );
  }
  else
  {
    struct MMUContext* uctx;
    struct MMUContext* sctx;
    struct MMUContext* ctxs[ 3 ];
    void*              location;
    ULONG              page_size;
    ULONG              size;

    // Get default and supervisor contexts

    uctx = DefaultContext();
    sctx = SuperContext( uctx );

    ctxs[ 0 ] = uctx;
    ctxs[ 1 ] = sctx;
    ctxs[ 2 ] = NULL;

    page_size = max( GetPageSize( uctx ), GetPageSize( sctx ) );

    size = ( ( 0x200 - 1 ) / page_size + 1 ) * page_size;


    // Set address for direct (non-intercepted) custom register area
    
#if 0
    // Using a page close to the original is good, since it would allow
    // patching of relative offset instructions, like $0a0(a5). However
    // I don't know enough about the memory map to do that yet.

    location = (void*) 0x80dff000;
#else
    // The area of $e00000 seems only to be used for the CDTV.
    // Let's hope we can use it.

    location = (void*) 0x00e00000;
#endif


    pd = (struct PUHData*) AllocVec( sizeof( struct PUHData ),
                                     MEMF_PUBLIC | MEMF_CLEAR );

    if( pd == NULL )
    {
      printf( "Out of memory!\n" );
    }
    else
    {
      void* rom_start    = NULL;
      ULONG rom_size     = NULL;

      // Get location of Kickstart ROM  
      rom_size  = *( (ULONG*) ( ROMEND - MAGIC_ROMEND ) );
      rom_start = ( (UBYTE*) ROMEND ) - rom_size;

      pd->m_Active               = FALSE;

      pd->m_AudioMode            = AHI_INVALID_ID;

      pd->m_SoundFunc.h_Entry    = (ULONG(*)(void)) PUHSoundFunc;
      pd->m_SoundFunc.h_Data     = pd;

      if( gfxbase != NULL && 
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
    
      pd->m_DMACON               = DMAF_MASTER;
      pd->m_INTENA               = INTF_INTEN;

      pd->m_Intercepted          = (void*) 0xdff000;
      pd->m_CustomDirect         = location;

      pd->m_UserContext          = uctx;        
      pd->m_SuperContext         = sctx;

      pd->m_UserException        = NULL;        
      pd->m_SuperException       = NULL;

      pd->m_ROM                  = rom_start;
      pd->m_ROMShadowBuffer      = NULL;

      pd->m_ROMSize              = rom_size;
      pd->m_CustomSize           = size;

      pd->m_SoftInt.is_Node.ln_Type = NT_INTERRUPT;
      pd->m_SoftInt.is_Node.ln_Pri  = 32;
      pd->m_SoftInt.is_Node.ln_Name = "NallePUH Level 4 emulation";
      pd->m_SoftInt.is_Data         = pd;
      pd->m_SoftInt.is_Code         = (void(*)(void)) PUHSoftInt;
    }
  }

  CloseLibrary( (struct Library*) gfxbase );

  return pd;
}


/******************************************************************************
** Deallocate PUH *************************************************************
******************************************************************************/

void
FreePUH( struct PUHData* pd )
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

void
SetPUHLogger( struct Hook*    hook,
              struct PUHData* pd )
{
  pd->m_LogHook = hook;
}


/******************************************************************************
** Send a message to the log function *****************************************
******************************************************************************/

void
LogPUH( struct PUHData* pd,
        STRPTR          fmt,
        ... )
{
  va_list ap;

  va_start( ap, fmt );

  if( pd->m_LogHook == NULL )
  {
    vprintf( fmt, ap );
    printf( "\n" );
  }
  else
  {
    char    buffer[ 256 ];
    vsnprintf( buffer, sizeof( buffer ), fmt, ap );
    
    CallHookPkt( pd->m_LogHook, pd, buffer );
  }

  va_end( ap );
}


/******************************************************************************
** Installl PUH ***************************************************************
******************************************************************************/

BOOL
InstallPUH( ULONG           flags,
            ULONG           audio_mode,
            ULONG           frequency,
            struct PUHData* pd )
{
  BOOL ahi_ok = FALSE;

  pd->m_Flags     = flags;
  pd->m_AudioMode = audio_mode;

  // Activate AHI

  pd->m_AudioCtrl = AHI_AllocAudio( AHIA_AudioID,    audio_mode,
                                    AHIA_MixFreq,    frequency,
                                    AHIA_Channels,   4,
                                    AHIA_Sounds,     1,
                                    AHIA_SoundFunc,  (ULONG) &pd->m_SoundFunc,
                                    AHIA_PlayerFreq, 100 << 16,
                                    TAG_DONE );

  if( pd->m_AudioCtrl == NULL )
  {
    LogPUH( pd, "Unable to allocate audio mode $%08lx.", audio_mode );
  }
  else
  {
    struct AHISampleInfo si = 
    {
      AHIST_M8S,   // An 8-bit sample
      0,           // beginning at address 0
      0xffffffff   // and ending at the last address.
    };
    
    if( AHI_LoadSound( 0, AHIST_DYNAMICSAMPLE, &si, pd->m_AudioCtrl ) != AHIE_OK )
    {
      LogPUH( pd, "Unable to load dynamic sample." );
    }
    else
    {
      if( AHI_ControlAudio( pd->m_AudioCtrl, 
          AHIC_Play, TRUE,
          TAG_DONE ) != AHIE_OK )
      {
        LogPUH( pd, "Unable to start playback." );
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


  // Activate MMU

  pd->m_UserException = 
      AddContextHook( MADTAG_CONTEXT, (ULONG) pd->m_UserContext,
                      MADTAG_TYPE,    MMUEH_SEGFAULT,
                      MADTAG_CODE,    (ULONG) PUHHandler,
                      MADTAG_DATA,    (ULONG) pd,
                      MADTAG_PRI,     127,
                      TAG_DONE );

  if( pd->m_UserException == NULL )
  {
    LogPUH( pd, "Unable to install user context hook." );
  }
  else
  {
    pd->m_SuperException =
        AddContextHook( MADTAG_CONTEXT, (ULONG) pd->m_SuperContext,
                        MADTAG_TYPE,    MMUEH_SEGFAULT,
                        MADTAG_CODE,    (ULONG) PUHHandler,
                        MADTAG_DATA,    (ULONG) pd,
                        MADTAG_PRI,     127,
                        TAG_DONE );

    if( pd->m_SuperException == NULL )
    {
      LogPUH( pd, "Unable to install supervisor context hook." );
    }
    else
    {
      ActivateException( pd->m_UserException );
      ActivateException( pd->m_SuperException );

      // Are we supposed to patch the ROM?

      if( flags & PUHF_PATCH_ROM )
      {
        pd->m_ROMShadowBuffer = AllocAligned( pd->m_ROMSize,
                                              MEMF_PUBLIC | MEMF_FAST,
                                              RemapSize( pd->m_UserContext  ) );
       
        if( pd->m_ROMShadowBuffer == NULL )
        {
          LogPUH( pd, "Out of memory!" );
        }
        else
        {
          CopyMemQuick( pd->m_ROM, pd->m_ROMShadowBuffer, pd->m_ROMSize );

          PatchROMShadowBuffer( pd );

          CacheClearU();
        }
      }

      // Re-map 

      if( ! RemapMemory( pd ) )
      {
        LogPUH( pd, "Unable to install remap." );
      }
      else
      {
        pd->m_Active = TRUE;
      }
    }
  }


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

void
UninstallPUH( struct PUHData* pd )
{
  if( pd == NULL )
  {
    return;
  }

	DeactivatePUH( pd );

  if( pd->m_Active )
  {
    RestoreMemory( pd );
  }

  if( pd->m_ROMShadowBuffer != NULL )
  {
    FreeMem( pd->m_ROMShadowBuffer, pd->m_ROMSize );
    pd->m_ROMShadowBuffer = NULL;
  }

  if( pd->m_SuperException != NULL )
  {
    DeactivateException( pd->m_SuperException );
    RemContextHook( pd->m_SuperException );
    pd->m_SuperException = NULL;
  }

  if( pd->m_UserException != NULL )
  {
    DeactivateException( pd->m_UserException );
    RemContextHook( pd->m_UserException );
    pd->m_UserException = NULL;
  }
  
  if( pd->m_AudioCtrl != NULL )
  {
    AHI_FreeAudio( pd->m_AudioCtrl );
    pd->m_AudioCtrl = NULL;
  }

  pd->m_Flags     = 0L;
  pd->m_AudioMode = AHI_INVALID_ID;
  pd->m_Active    = FALSE;
}


/******************************************************************************
** Activate PUH ***************************************************************
******************************************************************************/

BOOL
ActivatePUH( struct PUHData* pd )
{
  if( ! SetPageProperties( pd->m_UserContext,
                           MAPP_INVALID | MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                           MAPP_INVALID | MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                           (ULONG) pd->m_Intercepted, TAG_DONE ) )
  {
    return FALSE;
  }

  if( ! SetPageProperties( pd->m_SuperContext,
                           MAPP_INVALID | MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                           MAPP_INVALID | MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                           (ULONG) pd->m_Intercepted, TAG_DONE ) )
  {
    // Try to deactivate
    
    DeactivatePUH( pd );

    return FALSE;
  }
  
  return TRUE;
}


/******************************************************************************
** Deactivate PUH *************************************************************
******************************************************************************/

void
DeactivatePUH( struct PUHData* pd )
{
  SetPageProperties( pd->m_UserContext,
                     MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                     MAPP_INVALID | MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                     (ULONG) pd->m_Intercepted, TAG_DONE );

  SetPageProperties( pd->m_SuperContext,
                     MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                     MAPP_INVALID | MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                     (ULONG) pd->m_Intercepted, TAG_DONE );
}


/******************************************************************************
** Activate MMU ***************************************************************
******************************************************************************/

static BOOL
RemapMemory( struct PUHData* pd )
{
  BOOL  rc = FALSE;

  // Get properties for the areas we will modify

  pd->m_Properties.m_UserROM =
      GetProperties( pd->m_UserContext, (ULONG) pd->m_ROM, TAG_DONE );

  pd->m_Properties.m_UserROMShadow =
      GetProperties( pd->m_UserContext, (ULONG) pd->m_ROMShadowBuffer, TAG_DONE );

  pd->m_Properties.m_UserCustom = 
      GetProperties( pd->m_UserContext, (ULONG) 0xdff000, TAG_DONE );

  pd->m_Properties.m_SuperROM =
      GetProperties( pd->m_SuperContext, (ULONG) pd->m_ROM, TAG_DONE );

  pd->m_Properties.m_SuperROMShadow =
      GetProperties( pd->m_SuperContext, (ULONG) pd->m_ROMShadowBuffer, TAG_DONE );

  pd->m_Properties.m_SuperCustom = 
      GetProperties( pd->m_SuperContext, (ULONG) 0xdff000, TAG_DONE );

  if( ( pd->m_Properties.m_UserCustom  & MAPP_REPAIRABLE ) ||
      ( pd->m_Properties.m_SuperCustom & MAPP_REPAIRABLE ) )
  {
    // This is odd ...

    LogPUH( pd, "Custom chip register area marked 'repairable'!" );
  }
  else
  {
    struct MinList* user_mapping;
    struct MinList* super_mapping;

    LockContextList();
    LockMMUContext( pd->m_UserContext );
    LockMMUContext( pd->m_SuperContext );

    // Save current mapping if we have to abort!

    user_mapping  = GetMapping( pd->m_UserContext );
    super_mapping = GetMapping( pd->m_SuperContext );

    if( user_mapping == NULL || super_mapping == NULL )
    {
      LogPUH( pd, "Failed to get mappings." );
    }
    else
    {
      if( ! SetProperties( pd->m_UserContext,
                           ( pd->m_Properties.m_UserCustom | 
                             MAPP_REMAPPED ),
                           ~0UL,
                           (ULONG) pd->m_CustomDirect, 
                           pd->m_CustomSize,
                           MAPTAG_DESTINATION, 0xdff000,
                           TAG_DONE ) ||
          ! SetProperties( pd->m_SuperContext,
                           ( pd->m_Properties.m_SuperCustom | 
                             MAPP_REMAPPED ),
                           ~0UL,
                           (ULONG) pd->m_CustomDirect, 
                           pd->m_CustomSize,
                           MAPTAG_DESTINATION, 0xdff000,
                           TAG_DONE ) )
      {
        LogPUH( pd, "Failed to set properties for re-mapped area." );
      }
      else
      {
        LogPUH( pd, "Added unprotected custom mirror at 0x%08lx.",
                pd->m_CustomDirect );

        if( ! SetProperties( pd->m_UserContext,
                             ( pd->m_Properties.m_UserCustom | 
                               MAPP_SINGLEPAGE |
                               MAPP_REPAIRABLE ),
                             ~0UL,
                             0xdff000, 
                             pd->m_CustomSize,
                             TAG_DONE ) ||
            ! SetProperties( pd->m_SuperContext,
                             ( pd->m_Properties.m_SuperCustom | 
                               MAPP_SINGLEPAGE |
                               MAPP_REPAIRABLE ),
                             ~0UL,
                             0xdff000, 
                             pd->m_CustomSize,
                             TAG_DONE ) )
        {
          LogPUH( pd, "Failed to set properties for custom chip register area." );
        }
        else
        {
          BOOL activate = TRUE;

          LogPUH( pd, "Protected custom area at 0x%08lx.",
                  0xdff000 );

          // Remap Kickstart ROM, if provided
          
          if( pd->m_ROMShadowBuffer != NULL )
          {
            activate = 
            SetProperties( pd->m_UserContext,
                           ( pd->m_Properties.m_UserROM | 
                             MAPP_ROM ),
                           ~0UL,
                           (ULONG) pd->m_ROMShadowBuffer, 
                           pd->m_ROMSize,
                           TAG_DONE ) &&
            SetProperties( pd->m_SuperContext,
                           ( pd->m_Properties.m_SuperROM | 
                             MAPP_ROM ),
                             ~0UL,
                             (ULONG) pd->m_ROMShadowBuffer,
                             pd->m_ROMSize,
                             TAG_DONE ) &&
            SetProperties( pd->m_UserContext,
                           ( pd->m_Properties.m_UserROM |
                             MAPP_ROM | MAPP_REMAPPED ),
                             ~0UL,
                             (ULONG) pd->m_ROM,
                             pd->m_ROMSize,
                             MAPTAG_DESTINATION, (ULONG) pd->m_ROMShadowBuffer,
                             TAG_DONE ) &&
            SetProperties( pd->m_SuperContext,
                           ( pd->m_Properties.m_SuperROM |
                             MAPP_ROM | MAPP_REMAPPED ),
                             ~0UL,
                             (ULONG) pd->m_ROM,
                             pd->m_ROMSize,
                             MAPTAG_DESTINATION, (ULONG) pd->m_ROMShadowBuffer,
                             TAG_DONE );
            if( activate )
            {
              LogPUH( pd, "Installed patched ROM image." );
            }
            else
            {
              LogPUH( pd, "Failed to install patched ROM image." );
            }
          }
          
          if( activate )
          {
            struct MMUContext* ctxs[ 3 ] = 
            {
              pd->m_UserContext,
              pd->m_SuperContext,
              NULL
            };

            // We need to disable, since we may have patched exec.library!

            Disable();
            rc = RebuildTreesA( ctxs );
            Enable();

            if( rc )
            {
              LogPUH( pd, "MMU trees successfully modified." );
            }
            else
            {
              LogPUH( pd, "Failed to rebuild MMU trees." );
            }
          }
        }
      }

      if( ! rc )
      {
        // Roll-back!

        SetPropertyList( pd->m_UserContext, user_mapping );
        SetPropertyList( pd->m_SuperContext, super_mapping );
      }
    }

    if( user_mapping != NULL )
    {
      ReleaseMapping( pd->m_UserContext, user_mapping );
    }

    if( super_mapping != NULL )
    {
      ReleaseMapping( pd->m_SuperContext, super_mapping );
    }

    UnlockMMUContext( pd->m_SuperContext );
    UnlockMMUContext( pd->m_UserContext );
    UnlockContextList();
  }

  return rc;
}


/******************************************************************************
** Restore MMU ****************************************************************
******************************************************************************/

// Note that this code will NOT restore the attributes for
// pd->m_CustomDirect, since the patched instructions in application code
// needs it even after Nalle PUH has terminated.

static BOOL
RestoreMemory( struct PUHData* pd )
{
  BOOL            rc = FALSE;
  struct MinList* user_mapping;
  struct MinList* super_mapping;

  LockContextList();
  LockMMUContext( pd->m_UserContext );
  LockMMUContext( pd->m_SuperContext );

  // Save current mapping if we have to abort!

  user_mapping  = GetMapping( pd->m_UserContext );
  super_mapping = GetMapping( pd->m_SuperContext );

  if( user_mapping == NULL || super_mapping == NULL )
  {
    LogPUH( pd, "Failed to get mappings." );
  }
  else
  {
    if( ! SetProperties( pd->m_UserContext,
                         pd->m_Properties.m_UserCustom, ~0UL,
                         0xdff000, pd->m_CustomSize,
                         TAG_DONE ) ||
        ! SetProperties( pd->m_SuperContext,
                         pd->m_Properties.m_SuperCustom, ~0UL,
                         0xdff000, pd->m_CustomSize,
                         TAG_DONE ) )
    {
      LogPUH( pd, "Failed to set properties for custom chip register area." );
    }
    else
    {
      BOOL activate = TRUE;
      
      LogPUH( pd, "Removed protection for custom area." );

      if( pd->m_ROMShadowBuffer != NULL )
      {
        if( ! SetProperties( pd->m_UserContext,
                             pd->m_Properties.m_UserROMShadow, ~0UL,
                             (ULONG) pd->m_ROMShadowBuffer, 
                             pd->m_ROMSize,
                             TAG_DONE ) ||
            ! SetProperties( pd->m_SuperContext,
                             pd->m_Properties.m_SuperROMShadow, ~0UL,
                             (ULONG) pd->m_ROMShadowBuffer,
                             pd->m_ROMSize,
                             TAG_DONE ) )
        {
          LogPUH( pd, "Failed to set properties for for patched ROM area." );
          activate = FALSE;
        }
        else
        {
          if( ! SetProperties( pd->m_UserContext,
                               pd->m_Properties.m_UserROM, ~0UL,
                               (ULONG) pd->m_ROM, 
                               pd->m_ROMSize,
                               TAG_DONE ) ||
              ! SetProperties( pd->m_SuperContext,
                               pd->m_Properties.m_SuperROM, ~0UL,
                               (ULONG) pd->m_ROM,
                               pd->m_ROMSize,
                               TAG_DONE ) )
          {
            LogPUH( pd, "Failed to set properties for for ROM area." );
            activate = FALSE;
          }
          else
          {
            LogPUH( pd, "Restored original ROM image." );
          }
        }
      }

      if( activate )
      {
        struct MMUContext* ctxs[ 3 ] = 
        {
          pd->m_UserContext,
          pd->m_SuperContext,
          NULL
        };

        // We need to disable, since we may have patched exec.library!

        Disable();
        rc = RebuildTreesA( ctxs );
        Enable();

        if( rc )
        {
          LogPUH( pd, "MMU trees successfully modified." );
        }
        else
        {
          LogPUH( pd, "Failed to rebuild MMU trees." );
        }
      }
    }
  }

  if( ! rc )
  {
    // Roll-back!

    SetPropertyList( pd->m_UserContext, user_mapping );
    SetPropertyList( pd->m_SuperContext, super_mapping );
  }
  

  if( user_mapping != NULL )
  {
    ReleaseMapping( pd->m_UserContext, user_mapping );
  }

  if( super_mapping != NULL )
  {
    ReleaseMapping( pd->m_SuperContext, super_mapping );
  }

  UnlockMMUContext( pd->m_SuperContext );
  UnlockMMUContext( pd->m_UserContext );
  UnlockContextList();
  
  return rc;
}


/******************************************************************************
** Patch kickstart ROM ********************************************************
******************************************************************************/

static void
PatchROMShadowBuffer( struct PUHData* pd )
{
  union
  {
    void*            m_Void;
    UWORD*           m_Word;
    ULONG*           m_Long;
    struct Resident* m_Resident;
  } ptr;

  void*  end;

  ULONG  patches;

  STRPTR module_name;
  void*  module_rom_start;
  void*  module_rom_end;
  void*  module_end;


  ptr.m_Void = pd->m_ROMShadowBuffer;
  end        = (void*) ( (ULONG) pd->m_ROMShadowBuffer + pd->m_ROMSize );

  patches          = 0;
  module_name      = NULL;
  module_rom_start = NULL;
  module_rom_end   = NULL;
  module_end       = NULL;

  while( ptr.m_Void < end )
  {
    // Print summary

    if( module_end != NULL && ptr.m_Void >= module_end )
    {
      if( patches != 0 )
      {
        LogPUH( pd, "Patched %ld instructions in '%s'.",
                patches, module_name );
      }

      patches    = 0;
      module_end = NULL;
    }

    // Check for new module

    if( *ptr.m_Word == RTC_MATCHWORD )
    {
      if( ptr.m_Resident->rt_MatchTag == 
          pd->m_ROM + ( ptr.m_Void - pd->m_ROMShadowBuffer ) )
      {
        module_name      = ptr.m_Resident->rt_Name;
        module_rom_start = ptr.m_Void - pd->m_ROMShadowBuffer + pd->m_ROM;
        module_rom_end   = ptr.m_Resident->rt_EndSkip;
        module_end       = pd->m_ROMShadowBuffer + ( module_rom_end - pd->m_ROM );

        if( strcmp( module_name, "audio.device" ) == 0 )
        {
          LogPUH( pd, "Skipping '%s'.", 
                  module_name );

          module_end = NULL;
          ptr.m_Void = pd->m_ROMShadowBuffer + ( ptr.m_Resident->rt_EndSkip - pd->m_ROM );
          continue;
        }
      }
    }

    if( ( *ptr.m_Long & ~0x1ff ) == 0xdff000 )
    {
      *ptr.m_Long = (ULONG) pd->m_CustomDirect + ( *ptr.m_Long & 0xfff );

      ++patches;
    }

    // Skip to next word

    ++ptr.m_Word;
  }
}


/******************************************************************************
** MMU exception handler ******************************************************
******************************************************************************/

ASMCALL INTERRUPT static ULONG
PUHHandler( REG( a0, struct ExceptionData* ed ),
            REG( a1, struct PUHData*       pd ),
            REG( a6, struct Library*       MMUBase ) )
{
  struct ExecBase* SysBase;
  int              rc = 1;

  SysBase = ed->exd_SysBase;

  if( ( (ULONG) ed->exd_FaultAddress & ~0x1ffUL ) != (ULONG) pd->m_Intercepted )
  {
    // Not for us!
    return 1;
  }

  // Remove write-protection for us.

  if( ! SetPageProperties( pd->m_SuperContext,
                           MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                           MAPP_INVALID | MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                           (ULONG) pd->m_Intercepted, TAG_DONE ) )
  {
    // What to do?
    return 1;
  }

  if( pd->m_Flags & PUHF_TOGGLE_LED )
  {
    * ((UBYTE*) 0xbfe001) ^= 2;
  }

  if( ed->exd_Flags & ( EXDF_INSTRUCTION | 
                        EXDF_WRITEDATAUNKNOWN | 
                        EXDF_MISALIGNED ) )
  {
    rc = 1;
  }
  else
  { 
    int    size;
    ULONG  reg;
    BOOL   handled;

    size = (ULONG) ed->exd_NextFaultAddress - (ULONG) ed->exd_FaultAddress;
    reg  = (ULONG) ed->exd_FaultAddress - (ULONG) pd->m_Intercepted;

//    KPrintF( "%08lx: %04lx\n", ed->exd_ReturnPC, reg );

    // We must initialize handled, since it is only set to TRUE, never FALSE
    handled = FALSE;

    if( ed->exd_Flags & EXDF_WRITE )
    {
      ed->exd_Flags |= EXDF_WRITECOMPLETE;

      if( size == 0 )
      {
        // Fake byte accesses, even though "illegal"

        if( reg & 1 )
        {
          PUHWrite( reg & ~1UL, ed->exd_Data & 0x00ffUL, &handled, pd, SysBase );
        }
        else
        {
          PUHWrite( reg, ( ed->exd_Data << 8 ) & 0xff00UL, &handled, pd, SysBase );
        }
      }
      else if( size == 1 )
      {
        PUHWrite( reg, ed->exd_Data & 0xffffUL, &handled, pd, SysBase );
      }
      else if( size == 3 )
      {
        PUHWrite( reg, ( ed->exd_Data >> 16 ) & 0xffffUL, &handled, pd, SysBase );
        PUHWrite( reg + 2, ed->exd_Data & 0xffffUL, &handled, pd, SysBase );
      }
    }
    else
    {
      ed->exd_Flags |= EXDF_READBACK;

      if( size == 0 )
      {
        // Fake byte accesses, even though "illegal"
        
        if( reg & 1 )
        {
          ed->exd_Data = PUHRead( reg & ~1UL, &handled, pd, SysBase ) & 0xffUL;
        }
        else
        {
          ed->exd_Data = PUHRead( reg, &handled, pd, SysBase ) >> 8;
        }
      }
      else if( size == 1 )
      {
        ed->exd_Data = PUHRead( reg, &handled, pd, SysBase );
      }
      else if( size == 3 )
      {
        ed->exd_Data =  PUHRead( reg, &handled, pd, SysBase ) << 16;
        ed->exd_Data |= PUHRead( reg + 2, &handled, pd, SysBase );
      }
    }

    if( ! handled && ( pd->m_Flags & PUHF_PATCH_APPS ) )
    {
      if( ed->exd_ReturnPC < pd->m_ROM || 
          ed->exd_ReturnPC >= ( pd->m_ROM + pd->m_ROMSize ) )
      {
        UWORD op_code;
        ULONG address;

        op_code = ReadWord( ed->exd_ReturnPC );

        if( op_code == 0x33fc )
        {
          // move.w #$xxxx,$xxxxxxxx

          address = ReadLong( ed->exd_ReturnPC + 4 );

          if( ( address & ~0x1ffUL ) == (ULONG) pd->m_Intercepted )
          {
            WriteLong( ed->exd_ReturnPC + 4, 
                       address - 0xdff000 + (ULONG) pd->m_CustomDirect );
            CacheClearE( ed->exd_ReturnPC + 4, 4, CACRF_ClearI );
          }
        }
        else if( op_code == 0x23fc )
        {
          // move.l #$xxxxxxxx,$xxxxxxxx

          address = ReadLong( ed->exd_ReturnPC + 6 );

          if( ( address & ~0x1ffUL ) == (ULONG) pd->m_Intercepted )
          {
            WriteLong( ed->exd_ReturnPC + 6,
                       address - 0xdff000 + (ULONG) pd->m_CustomDirect );
            CacheClearE( ed->exd_ReturnPC + 6, 4, CACRF_ClearI );
          }
        }
        else if( ( op_code & 0xf1ff ) == 0x317c )
        {
          // move.w #$xxxx,$xxxx(ax)

          address = ReadWord( ed->exd_ReturnPC + 4 );
          
          if( address > 0 && address < 0x200 )
          {
            WriteWord( ed->exd_ReturnPC + 4,
                       address - 0xdff000 + (ULONG) pd->m_CustomDirect );
            CacheClearE( ed->exd_ReturnPC + 4, 2, CACRF_ClearI );
          }
        }
        else if( ( op_code & 0xf1ff ) == 0x217c )
        {
          // move.l #$xxxxxxxx,$xxxx(ax)

          address = ReadWord( ed->exd_ReturnPC + 6 );
          
          if( address > 0 && address < 0x200 )
          {
            WriteWord( ed->exd_ReturnPC + 6,
                       address - 0xdff000 + (ULONG) pd->m_CustomDirect );
            CacheClearE( ed->exd_ReturnPC + 6, 2, CACRF_ClearI );
          }
        }
        else if( ( ( op_code & 0xc03f ) == 0x0039 &&
                   ( op_code & 0x3000 ) != 0x0000 ) ||
                 ( op_code & 0xf1ff ) == 0xb039 )
        {
          // move.x $xxxxxxxx,????
          // cmp.x  $xxxxxxxx,dx

          address = ReadLong( ed->exd_ReturnPC + 2 );

          if( ( address & ~0x1ffUL ) == (ULONG) pd->m_Intercepted )
          {
            WriteLong( ed->exd_ReturnPC + 2,
                       address - 0xdff000 + (ULONG) pd->m_CustomDirect );
            CacheClearE( ed->exd_ReturnPC + 2, 4, CACRF_ClearI );
          }
        }
        else if( ( ( op_code & 0xc038 ) == 0x0028 &&
                   ( op_code & 0x3000 ) != 0x0000 ) ||
                 ( op_code & 0xf138 ) == 0xb028 )
        {
          // move.x $xxxx(ax),????
          // cmp.x  $xxxx(ax),dx

          address = ReadWord( ed->exd_ReturnPC + 2 );

          if( address > 0 && address < 0x200 )
          {
            WriteWord( ed->exd_ReturnPC + 2,
                       address - 0xdff000 + (ULONG) pd->m_CustomDirect );
            CacheClearE( ed->exd_ReturnPC + 2, 2, CACRF_ClearI );
          }
        }
        else
        {
//          KPrintF( "[Failed: %08lx: %04lx ]", ed->exd_ReturnPC, reg );
        }
      }
    }

    // Ok, we either emulated or simulated it;
    rc = 0;
  }

  // Restore write-protection for us

  SetPageProperties( pd->m_SuperContext,
                     MAPP_INVALID | MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                     MAPP_INVALID | MAPP_SINGLEPAGE | MAPP_REPAIRABLE,
                     (ULONG) pd->m_Intercepted, TAG_DONE );

  return rc;
}


/******************************************************************************
** Handle reads ***************************************************************
******************************************************************************/

static UWORD
PUHRead( UWORD            reg,
         BOOL*            handled,
         struct PUHData*  pd,
         struct ExecBase* SysBase )
{
  UWORD  result;
  UWORD* address = (UWORD*) ( (ULONG) pd->m_CustomDirect + reg );

  switch( reg )
  {
    case DMACONR:
      result  = ReadWord( address ); 
      result &= ~DMAF_AUDIO;
      result |= ( pd->m_DMACON & DMAF_AUDIO );

      *handled = TRUE;
      break;

    case INTENAR:
      result  = ReadWord( address );
      result &= ~INTF_AUDIO;
      result |= ( pd->m_INTENA & INTF_AUDIO );

      *handled = TRUE;
      break;

    case INTREQR:
      result  = ReadWord( address );
      result &= ~INTF_AUDIO;
      result |= ( pd->m_INTREQ & INTF_AUDIO );

      *handled = TRUE;
      break;

    default:
      // Just carry out the intercepted read operation

      result = ReadWord( address );
      break;
  }

  return result;
}


/******************************************************************************
** Handle writes **************************************************************
******************************************************************************/

static void
PUHWrite( UWORD            reg, 
          UWORD            value,
          BOOL*            handled,
          struct PUHData*  pd,
          struct ExecBase* SysBase )
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
          pd->m_SoundOn[ 0 ]   = FALSE;

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
          pd->m_SoundOn[ 1 ]   = FALSE;

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
          pd->m_SoundOn[ 2 ]   = FALSE;

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
          pd->m_SoundOn[ 3 ]   = FALSE;

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


    case ADKCON:
      WriteWord( address, value );
      break;

    case AUD0LCH:
    case AUD1LCH:
    case AUD2LCH:
    case AUD3LCH:
      pd->m_SoundLocation[ ( reg - AUD0LCH ) >> 4 ] &= 0x0000ffff;
      pd->m_SoundLocation[ ( reg - AUD0LCH ) >> 4 ] |= value << 16;

      *handled = TRUE;
      break;

    case AUD0LCL:
    case AUD1LCL:
    case AUD2LCL:
    case AUD3LCL:
    {
      int channel     = ( reg - AUD0LCL ) >> 4;

      pd->m_SoundLocation[ channel ] &= 0xffff0000;
      pd->m_SoundLocation[ channel ] |= value;

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
      int   channel = ( reg - AUD0PER ) >> 4;

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
      // Just carry out the intercepted write operation

      WriteWord( address, value );
      break;
  }
}


/******************************************************************************
** Audio interrupt simulation *************************************************
******************************************************************************/

ASMCALL SAVEDS static void
PUHSoftInt( REG( d1, UWORD           active_ints ),
            REG( a0, struct Custom*  custom ),
            REG( a1, struct PUHData* pd ),
            REG( a5, void*           me ),
            REG( a6, struct ExecBase* SysBase ) )
{
  pd->m_CausePending = FALSE;

  // This is almost the same code as exec's. ExecDis rules.
  // We access m_INTENA/m_INTREQ directly in order to avoid
  // the overhead of the exception handler.

  asm( "
    movem.l   d0-d1/a0-a1/a5-a6,-(sp) /* Make sure we don't kill the C code */

    move      #0x2400,sr        /* Fake a level 4 interrupt */

    move.l    %1,a0
    move.l    %0,a6
    move.w    (%2),d1
    and.w     (%3),d1

test_channel_1:
    btst      #8,d1             /* Audio channel 1? */
    beq.s     test_channel_3
    movem.l   180(a6),a1/a5     /* Get IntVects[8] handler data. */
    pea       int_exit          /* Use special ExitIntr() below. */
    jmp       (a5)

test_channel_3:
    btst      #10,d1            /* Audio channel 3? */
    beq.s     test_channel_0
    movem.l   204(a6),a1/a5     /* Get IntVects[10] handler data. */
    pea       int_exit          /* Use special ExitIntr() below. */
    jmp       (a5)

test_channel_0:
    btst      #7,d1             /* Audio channel 0? */
    beq.s     test_channel_2
    movem.l   168(a6),a1/a5     /* Get IntVects[7] handler data. */
    pea       int_exit          /* Use special ExitIntr() below. */
    jmp       (a5)

test_channel_2:
    btst      #9,d1             /* Audio channel 2? */
    beq.s     done
    movem.l   192(a6),a1/a5     /* Get IntVects[9] handler data. */
    pea       int_exit          /* Use special ExitIntr() below. */
    jmp       (a5)

    * This routine allows a single invocation of the level 4 interrupt
    * handler to service all level 4 interrupts which are pending or
    * become pending while one is serviced.
int_exit:
    move.l    %1,a0             /* Point at custom chip register area. */
    move.l    %0,a6             /* Get ExecBase */
    move.w    #0x0780,d1        /* Mask for all level 4 interrupt bits. */
    and.w     (%2),d1
    and.w     (%3),d1           /* Find enabled and pending level 4 ints. */
    bne       test_channel_1

done:
    movem.l   (sp)+,d0-d1/a0-a1/a5-a6
  "
  : /* no result */
  : "a" (SysBase), 
    "d" (pd->m_Intercepted),
    "a" (&pd->m_INTENA),
    "a" (&pd->m_INTREQ)
  : "cc", "memory", "d0", "d1", "a0", "a1", "a5", "a6" /* trashed registers */
  ); 
}


static void 
PUHSoftIntWrapper( UWORD           active_ints,
                   struct Custom*  custom,
                   struct PUHData* pd,
                   struct ExecBase* SysBase )
{
  asm("
    movem.l   d0-d1/a0-a1/a5-a6,-(sp)
    move.w    %0,d1
    move.l    %1,a0
    move.l    %2,a1
    lea       _PUHSoftInt,a5
    move.l    %3,a6
    jsr       (a5)
    movem.l   (sp)+,d0-d1/a0-a1/a5-a6
  "
  : /* no result */
  : "r" (active_ints), "r" (custom), "r" (pd), "r" (SysBase)
  : "d1", "a0", "a1", "a5", "a6"
  );
}


ASMCALL SAVEDS static void
PUHSoundFunc( REG( a0, struct Hook*            hook ),
              REG( a2, struct AHIAudioCtrl*    actrl ),
              REG( a1, struct AHISoundMessage* msg ) )
{
  APTR            stack = NULL;
  struct PUHData* pd    = (struct PUHData*) hook->h_Data;

  pd->m_INTREQ |= ( 1 << ( INTB_AUD0 + msg->ahism_Channel ) );

  if( pd->m_INTENA & pd->m_INTREQ & INTF_AUDIO )
  {
    stack = SuperState();

    PUHSoftIntWrapper( 0, pd->m_CustomDirect, pd, SysBase );

    if( stack != NULL )
    {
      UserState( stack );
    }
  }
}

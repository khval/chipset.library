

#include <exec/emulation.h>
#include <graphics/gfxbase.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#define __USE_INLINE__

#include <proto/exec.h>
#include <proto/dos.h>

#include <stdio.h>

#include "PUH.h"

#define SIZEOF_SHORT 2
#define SIZEOF_INT 4

#include "uade/sysdeps.h"
#include "uade/memory.h"

extern UWORD PUHRead( UWORD reg, BOOL *handled );
extern void PUHWrite( UWORD reg, UWORD value, BOOL *handled );

 uae_u32 nallePuh_lget (uaecptr) ;
 uae_u32 nallePuh_wget (uaecptr) ;
 uae_u32 nallePuh_bget (uaecptr) ;
 void nallePuh_lput (uaecptr, uae_u32) ;
 void nallePuh_wput (uaecptr, uae_u32) ;
 void nallePuh_bput (uaecptr, uae_u32) ;

addrbank nallePuh_bank = {
    nallePuh_lget, nallePuh_wget, nallePuh_bget,
    nallePuh_lput, nallePuh_wput, nallePuh_bput,
    default_xlate, default_check
};

 uae_u32 nallePuh_lget (uaecptr eff_addr) 
{
	BOOL bHandled = FALSE;
	return (PUHRead((eff_addr & 0x1ff)+2, &bHandled) << 16) | PUHRead((eff_addr & 0x1ff), &bHandled);
}

 uae_u32 nallePuh_wget (uaecptr eff_addr) 
{
	BOOL bHandled = FALSE;
	return PUHRead((eff_addr & 0x1ff), &bHandled);
}

 uae_u32 nallePuh_bget (uaecptr ptr) 
{
	Printf("your can't read/write a byte from %08x, most be words or longs\n",ptr & 0xFFFFFE);
	return 0;
}

 void nallePuh_lput (uaecptr eff_addr, uae_u32 value) 
{
	BOOL bHandled = FALSE;
	PUHWrite((eff_addr & 0x1ff),value>>16, &bHandled);
	PUHWrite((eff_addr & 0x1ff)+2,value&0xffff, &bHandled);
}

 void nallePuh_wput (uaecptr eff_addr, uae_u32 value) 
{
	BOOL bHandled = FALSE;
	PUHWrite((eff_addr & 0x1ff),value&0xffff, &bHandled);
}

 void nallePuh_bput (uaecptr ptr, uae_u32 value) 
{
	Printf("your can't read/write a byte from %08x, most be words or longs\n",ptr & 0xFFFFFE);
}


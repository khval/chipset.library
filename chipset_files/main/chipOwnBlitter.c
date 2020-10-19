/* :ts=4
 *  $VER: chipOwnBlitter.c $Revision$ (18-Oct-2020)
 *
 *  This file is part of chipset.
 *
 *  Copyright (c) 2020 LiveForIt Software.
 *  LGPL License..
 *
 * $Id$
 *
 * $Log$
 *
 *
 */

#include <stdint.h>

#include <exec/exec.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <exec/types.h>
#include <libraries/chipset.h>
#include <proto/chipset.h>
#include <stdarg.h>

/****** chipset/main/chipOwnBlitter ******************************************
*
*   NAME
*      chipOwnBlitter -- Description
*
*   SYNOPSIS
*      void chipOwnBlitter(void);
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*       This function does not return a result
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

extern APTR blitter_mx;

void _chipset_chipOwnBlitter(struct chipsetIFace *Self)
{
	MutexObtain(blitter_mx);
}


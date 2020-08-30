/* :ts=4
 *  $VER: writeChip.c $Revision$ (31-Aug-2020)
 *
 *  This file is part of chipset.
 *
 *  Copyright (c) 2020 LiveForIt Software.
 *  GPL2 License..
 *
 * $Id$
 *
 * $Log$
 *
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <exec/exec.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <exec/types.h>
#include <libraries/chipset.h>
#include <proto/chipset.h>
#include <stdarg.h>

/****** chipset/main/writeChip ******************************************
*
*   NAME
*      writeChip -- Description
*
*   SYNOPSIS
*      void writeChip(ULONG offset, ULONG value);
*
*   FUNCTION
*
*   INPUTS
*       offset - 
*       value - 
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

void _chipset_writeChip(struct chipsetIFace *Self,
       ULONG offset,
       ULONG value)
{
  writeChip(offset, value);
}


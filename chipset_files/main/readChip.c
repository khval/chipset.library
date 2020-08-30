/* :ts=4
 *  $VER: readChip.c $Revision$ (31-Aug-2020)
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

/****** chipset/main/readChip ******************************************
*
*   NAME
*      readChip -- Description
*
*   SYNOPSIS
*      ULONG readChip(ULONG offset);
*
*   FUNCTION
*
*   INPUTS
*       offset - 
*
*   RESULT
*       The result ...
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

ULONG _chipset_readChip(struct chipsetIFace *Self,
       ULONG offset)
{
  return readChip(offset);
}


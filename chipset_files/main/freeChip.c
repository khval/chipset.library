/* :ts=4
 *  $VER: freeChip.c $Revision$ (31-Aug-2020)
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

/****** chipset/main/freeChip ******************************************
*
*   NAME
*      freeChip -- Description
*
*   SYNOPSIS
*      void freeChip(void * mem);
*
*   FUNCTION
*
*   INPUTS
*       mem - 
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

void _chipset_freeChip(struct chipsetIFace *Self,
       void * mem)
{
  freeChip(mem);
}


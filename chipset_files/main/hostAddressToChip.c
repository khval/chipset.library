/* :ts=4
 *  $VER: hostAddressToChip.c $Revision$ (31-Aug-2020)
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

/****** chipset/main/hostAddressToChip ******************************************
*
*   NAME
*      hostAddressToChip -- Description
*
*   SYNOPSIS
*      void * hostAddressToChip(void * adr);
*
*   FUNCTION
*
*   INPUTS
*       adr - 
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

void * _chipset_hostAddressToChip(struct chipsetIFace *Self,
       void * adr)
{
  return hostAddressToChip(adr);
}


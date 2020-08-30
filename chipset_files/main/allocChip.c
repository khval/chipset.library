/* :ts=4
 *  $VER: allocChip.c $Revision$ (31-Aug-2020)
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

/****** chipset/main/allocChip ******************************************
*
*   NAME
*      allocChip -- Description
*
*   SYNOPSIS
*      char * allocChip(uint32_t size);
*
*   FUNCTION
*
*   INPUTS
*       size - 
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

char * _chipset_allocChip(struct chipsetIFace *Self,
       uint32_t size)
{
  return allocChip(size);
}


/* :ts=4
    Copyright (C) 2020  Kjetil Hvalstrand

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
 */


#include <stdint.h>
#include <stdarg.h>

#include <exec/exec.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <exec/types.h>
#include <libraries/chipset.h>
#include <proto/chipset.h>

#include "../uade/sysconfig.h"
#include "../uade/sysdeps.h"
#include "../uade/memory.h"

/****** chipset/main/bitSetChipWord ******************************************
*
*   NAME
*      bitSetChipWord -- Description
*
*   SYNOPSIS
*      void bitSetChipWord(char * offset, ULONG bit);
*
*   FUNCTION
*
*   INPUTS
*       offset - 
*       bit - 
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

void _chipset_bitSetChipWord(struct chipsetIFace *Self,
       char * offset,
       ULONG bit)
{
  _bitSetChipWord(offset, bit);
}


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

#include <stdlib.h>
#include <stdint.h>
#include <exec/exec.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <exec/types.h>
#include <libraries/chipset.h>
#include <stdarg.h>

/****** chipset/main/chipAddressToHost ******************************************
*
*   NAME
*      chipAddressToHost -- Description
*
*   SYNOPSIS
*      void chipAddressToHost(void * adr);
*
*   FUNCTION
*
*   INPUTS
*       adr - 
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

int chipAddressToHost(int adr);

void *_chipset_chipAddressToHost(struct chipsetIFace *Self,  void * adr)
{
	return _chipAddressToHost(adr);
}


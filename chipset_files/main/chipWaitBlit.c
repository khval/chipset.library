/* :ts=4
 *  $VER: chipWaitBlit.c $Revision$ (18-Oct-2020)
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

#include "hardware/dmabits.h"

/****** chipset/main/chipWaitBlit ******************************************
*
*   NAME
*      chipWaitBlit -- Description
*
*   SYNOPSIS
*      void chipWaitBlit(void);
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

void _chipset_chipWaitBlit(struct chipsetIFace *Self)
{
	// busy loop for hardware to be done ;-)
	while (DMACONR() & DMAB_BLTDONE) ;		// I belive done if the bit is not set.
}


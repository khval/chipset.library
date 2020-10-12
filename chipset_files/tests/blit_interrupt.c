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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "proto/exec.h"
#include "proto/dos.h"

#include "uae/cia.h"
#include "uae/events.h"
#include "NallePUH/src/PUH.h"

#define SIZEOF_SHORT 2
#define SIZEOF_INT 4

#include "uae/sysdeps.h"
#include "uae/blitter.h"
#include "uae/custom.h"

uint32_t cycles = 0;
struct ev eventtab[ev_max];
uint32_t nextevent =0 ;
int custom_bank = 0;

struct PUHData _pd;
struct PUHData *pd = &_pd;

void test_blitter_func(uaecptr p0, uaecptr p1, uaecptr p2, uaecptr p3, struct bltinfo *pi)
{
	printf("%s\n",__FUNCTION__);
}

blitter_func *const blitfunc_dofast[256] = {  test_blitter_func};
blitter_func *const blitfunc_dofast_desc[256];

extern void blitter_handler (void);

int main()
{
	int n;

	if (lock_init() == false)
	{
		lock_cleanup();
		return 0;
	}

//	blitfunc_dofast[0x00] = test_blitter_func;

	cycles = ~0 - 20;

 	eventtab[ev_blitter].handler = blitter_handler;
	eventtab[ev_blitter].active = 1;

	blitter_handler();	// should fail becouse DMA not ready?

	bltcon0 = 0x800 | 0x00;

	pd->m_DMACON = DMA_BLITTER | 0x200 ;	// enable DMA?

	blitter_handler();	// try again..

	lock_cleanup();
	return 0;
}

void unset_special()
{
	printf("%s\n",__FUNCTION__);
}

void set_special()
{
	printf("%s\n",__FUNCTION__);
}

void INTREQ(uae_u16 xx)
{
	printf("%s\n",__FUNCTION__);
}

void INTREQ_0(uint32_t xx)
{
	printf("%s\n",__FUNCTION__);
}

uae_u32 blit_func(uae_u32 srca, uae_u32 srcb, uae_u32 srcc, uae_u8 mt)
{
	printf("%s: mt: %08x\n",__FUNCTION__,mt);
	return 0;
}


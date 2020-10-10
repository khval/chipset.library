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

uint32_t cycles = 0;
struct ev eventtab[ev_max];
uint32_t nextevent =0 ;
int custom_bank = 0;

void blit_code_test()
{
	printf("%s\n",__FUNCTION__);
	printf("<< press enter >>\n",__FUNCTION__);
	getchar();
}

int main()
{
	int n;

	if (lock_init() == false)
	{
		lock_cleanup();
		return 0;
	}

	cycles = ~0 - 20;

	eventtab[ev_blitter].active = true;
	eventtab[ev_blitter].handler = blit_code_test;
	eventtab[ev_blitter].evtime = cycles + 30;

	events_schedule ();

	for (n=0;n<40;n++) 
	{
		do_cycles_slow (1);
	}


	lock_cleanup();
	return 0;
}

void INTREQ_0(uint32_t xx)
{

}


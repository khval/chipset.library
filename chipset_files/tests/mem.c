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
#include <stdbool.h>
#include <stdio.h>
#include "../micro_sys/memory.h"

#include "../uae/sysconfig.h"
#include "../uae/sysdeps.h"
#include "../uae/memory.h"


uae_u8 *chipmemory;
 uae_u32 chipmem_mask;


void dump_blocks()
{
	int n;

	for (n=0;n<512;n++)
	{
		printf("%c", used_blocks[n] ? '*' : '.' );
	}
	printf("\n");
}

int main()
{
	char *mem1;
	char *mem2;
	char *mem3;

	init_mem();

	chipmemory = malloc(512*1024);

	mem1 = _allocChip(512);
	mem2 = _allocChip(1024*2-1);
	mem3 = _allocChip(1024*2);


	printf("have mem1 %08x\n",mem1);
	printf("have mem2 %08x\n",mem2);
	printf("have mem3 %08x\n",mem3);

dump_blocks();

	printf("free chip\n");
	_freeChip(mem2);

dump_blocks();

	_freeChip(mem1);

dump_blocks();

	if (chipmemory) free(chipmemory);

	return 0;
}


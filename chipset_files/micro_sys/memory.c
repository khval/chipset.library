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
#include <stdio.h>
#include "micro_sys/memory.h"

struct mem_list mem_list;
int used_blocks[512];

void grow_mem_list( int n )
{
	struct allocted_mem	*_new_;

	int _new_allocated = mem_list.allocated + n;
	_new_ = (struct allocted_mem *) malloc( sizeof(struct allocted_mem) * _new_allocated );
	
	if (_new_)
	{
		if (mem_list.allocted_tab) free (mem_list.allocted_tab);
		mem_list.allocted_tab  = _new_;
		mem_list.allocated = _new_allocated;
	}
}

void init_mem()
{
	mem_list.allocated = 0;
	mem_list.used = 0;
	mem_list.allocted_tab = NULL;
}

int find_free_blocks(int blocks)
{
	int n,u;
	int found = 0;

	for (n=0;n<512-blocks;n++)
	{
		for (u=0;u<blocks;u++)
		{
			if (used_blocks[n+u] == 1)
			{
				n += u;	// continue from checked...
				found = 0;	// not found;
				break;
			}
			else
			{
				found ++;
				if (found == blocks) return n;	// return the start block ;-)
			}
		}
	}

	return -1;
}

char *allocChip(uint32_t size)
{
	// 8 = 256
	// 9 = 512
	// 10 = 1024

	int blocks = (size & 0x3FF ? 1 : 0) + (size >> 10) ;
	int start,n;

	if (mem_list.used+1 > mem_list.allocated)
	{
		grow_mem_list( 20 );
	}

	printf("mem_list.allocated: %d\n", mem_list.allocated);

	if (mem_list.used+1 <= mem_list.allocated)
	{
		start = find_free_blocks(blocks);

		printf("start: %d\n",start);

		if (start>-1) 
		{
			mem_list.allocted_tab[mem_list.used].start = chip_ram + (start << 10);
			mem_list.allocted_tab[mem_list.used].used = blocks;
			mem_list.used++;

			for (n=0;n<blocks;n++) used_blocks[start+n] = 1;
			return mem_list.allocted_tab[mem_list.used-1].start;
		}

		return NULL;
	}

	return NULL;
}


void freeChip(void *adr)
{
	int n;

	for (n=0;n<mem_list.used;n++)
	{
		printf("[%d]=%08x\n",n, (uint32_t) mem_list.allocted_tab[n].start);

		if (mem_list.allocted_tab[n].start == adr  )
		{
			int startb = (int) ((char *) adr - chip_ram ) >> 10;
			int endb = mem_list.allocted_tab[n].used + startb;

			printf("start: %d end: %d\n",startb,endb);

			for (n=startb;n<endb;n++) used_blocks[n] = 0;

			mem_list.used--;
			for (;n<mem_list.used;n++) mem_list.allocted_tab[n] = mem_list.allocted_tab[n+1];
		}
	}
}



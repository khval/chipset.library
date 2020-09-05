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

#define mem_block_size 1024
#define chip_ram_size (512*mem_block_size)

extern int used_blocks[512];
extern char chip_ram[chip_ram_size];

struct allocted_mem
{
	void *start;
	int used;
};

struct mem_list
{
	struct allocted_mem  *allocted_tab;
	int used;
	int allocated;
};

char *allocChip(uint32_t size);



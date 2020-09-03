
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "../micro_sys/memory.h"

char chip_ram[chip_ram_size];

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

	mem1 = allocChip(512);
	mem2 = allocChip(1024*2-1);
	mem3 = allocChip(1024*2);


	printf("have mem1 %08x\n",mem1);
	printf("have mem2 %08x\n",mem2);
	printf("have mem3 %08x\n",mem3);

dump_blocks();

	printf("free chip\n");
	freeChip(mem2);

dump_blocks();

	freeChip(mem1);

dump_blocks();

	return 0;
}


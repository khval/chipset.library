
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <proto/exec.h>
#include <proto/dos.h>

#define __USE_INLINE__
#include <proto/chipset.h>

struct chipsetIFace * Ichipset = NULL;
struct Library *chipsetBase = NULL;

int main()
{
	if (init())
	{
		char *chip = allocChip(50);
		printf("chip: %08x\n",chip);

	}
	close_libs();

	return 0;
}



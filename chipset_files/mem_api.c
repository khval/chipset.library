
#include <proto/exec.h>

#include "uade/sysconfig.h"
#include "uade/sysdeps.h"
#include "uade/memory.h"

extern char *chip_ram;

void setup_mem_banks()
{
	init_mem_banks( 0x00FF );

	map_banks(&chipmem_bank, 0x00,0xFF );
	map_banks(&cia_bank, 0xA0,32 );
	map_banks(&clock_bank, 0xDC,1 );
}

char *_hostAddressToChip(char *addr)
{
	return (char *) ((ULONG) addr- (ULONG) chip_ram);
}

char *_chipAddressToHost(char *addr)
{
	return (char *) ((ULONG) chip_ram+ (ULONG) addr);
}


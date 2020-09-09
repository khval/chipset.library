
#include <stdint.h>
#include <proto/exec.h>

#include "uade/sysconfig.h"
#include "uade/sysdeps.h"
#include "uade/memory.h"

extern uae_u8 *chipmemory ;

BOOL setup_mem_banks()
{
	init_mem_banks( 0x00FF );

	map_banks(&chipmem_bank, 0x00,0xFF );
	map_banks(&cia_bank, 0xA0,32 );
	map_banks(&clock_bank, 0xDC,1 );

	chipmemory = (uae_u8) malloc(512 * 1024);	// 0.5 mb 
	if (chipmemory == NULL) return FALSE;

	return TRUE;
}

void cleanup()
{
	if (chipmemory) free(chipmemory);
	chipmemory = NULL;
}

char *_hostAddressToChip(char *addr)
{
	return (char *) ((ULONG) addr- (ULONG) chipmemory);
}

char *_chipAddressToHost(char *addr)
{
	return (char *) ((ULONG) chipmemory+ (ULONG) addr);
}

extern uint32_t cia_latency_us;

void _setCIATimingAccuracy( int us )
{
	cia_latency_us = us;
}



#include <stdint.h>
#include <stdbool.h>
#include <proto/exec.h>

#include "uade/sysconfig.h"
#include "uade/sysdeps.h"
#include "uade/memory.h"

extern uae_u8 *chipmemory ;

extern bool init_mem();
extern void cleanup_mem();

BOOL setup_mem_banks()
{
	init_mem_banks( 0x00FF );

	map_banks(&chipmem_bank, 0x00,0xFF );
	map_banks(&cia_bank, 0xA0,32 );
	map_banks(&clock_bank, 0xDC,1 );

	if (init_mem()==FALSE) return FALSE;

	CIA_reset();

	return TRUE;
}

void cleanup()
{
	cleanup_mem();
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

//--

void _bitChgChipLong(char * offset, ULONG bit)
{
	ULONG value = longget( (ULONG) offset);
	longput( (ULONG) offset, value ^ (1L << bit));
}

void _bitChgChipWord(char * offset, ULONG bit)
{
	ULONG value = wordget( (ULONG) offset);
	wordput( (ULONG) offset, value ^ (1L << bit));
}

void _bitChgChipByte(char * offset, ULONG bit)
{
	ULONG value = byteget( (ULONG) offset);
	byteput( (ULONG) offset, value ^ (1L << bit));
}

void _bitSetChipLong(char * offset, ULONG bit)
{
	ULONG value = longget( (ULONG) offset);
	longput( (ULONG) offset, value | (1L << bit));
}

void _bitSetChipWord(char * offset, ULONG bit)
{
	ULONG value = wordget( (ULONG) offset);
	wordput( (ULONG) offset, value | (1L << bit));
}

void _bitSetChipByte(char * offset, ULONG bit)
{
	ULONG value = byteget( (ULONG) offset);
	byteput( (ULONG) offset, value | (1L << bit));
}

void _bitClrChipLong(char * offset, ULONG bit)
{
	ULONG value = longget( (ULONG) offset);
	longput( (ULONG) offset, value & ~(1L << bit));
}

void _bitClrChipWord(char * offset, ULONG bit)
{
	ULONG value = wordget( (ULONG) offset);
	wordput( (ULONG) offset, value & ~(1L << bit));
}

void _bitClrChipByte(char * offset, ULONG bit)
{
	ULONG value = byteget( (ULONG) offset);
	byteput( (ULONG) offset, value & ~(1L << bit));
}




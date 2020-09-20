
#include <exec/libraries.h>
#include <exec/emulation.h>

#include "uade/sysconfig.h"
#include "uade/sysdeps.h"
#include "uade/memory.h"

#define __USE_INLINE__

#include <stdio.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "mem_api.h"

#undef Close
#undef Open

#define call_mem_get_func(func,addr) ((*func)(addr))
#define call_mem_put_func(func,addr,v) ((*func)(addr,v))


STATIC APTR stub_Open_ppc(ULONG *regarray);
STATIC APTR stub_Close_ppc(ULONG *regarray);
STATIC APTR stub_Expunge_ppc(ULONG *regarray);
STATIC ULONG stub_Reserved_ppc(ULONG *regarray);

STATIC ULONG stub_allocChip_ppc(ULONG *regarray);
STATIC VOID stub_freeChip_ppc(ULONG *regarray);

STATIC ULONG stub_hostAddressToChip_ppc(ULONG *regarray);
STATIC ULONG stub_chipAddressToHost_ppc(ULONG *regarray);
STATIC VOID stub_writeChipLong_ppc(ULONG *regarray);
STATIC VOID stub_writeChipWord_ppc(ULONG *regarray);
STATIC VOID stub_writeChipByte_ppc(ULONG *regarray);
STATIC ULONG stub_readChipLong_ppc(ULONG *regarray);
STATIC ULONG stub_readChipWord_ppc(ULONG *regarray);
STATIC ULONG stub_readChipByte_ppc(ULONG *regarray);
STATIC VOID stub_bitChgChipLong_ppc(ULONG *regarray);
STATIC VOID stub_bitChgChipWord_ppc(ULONG *regarray);
STATIC VOID stub_bitChgChipByte_ppc(ULONG *regarray);
STATIC VOID stub_bitSetChipLong_ppc(ULONG *regarray);
STATIC VOID stub_bitSetChipWord_ppc(ULONG *regarray);
STATIC VOID stub_bitSetChipByte_ppc(ULONG *regarray);
STATIC VOID stub_bitClrChipLong_ppc(ULONG *regarray);
STATIC VOID stub_bitClrChipWord_ppc(ULONG *regarray);
STATIC VOID stub_bitClrChipByte_ppc(ULONG *regarray);
STATIC VOID stub_setCIATimingAccuracy_ppc(ULONG *regarray);


const struct EmuTrap stub_Open = { TRAPINST, TRAPTYPE, (void *) stub_Open_ppc };
const struct EmuTrap stub_Close = { TRAPINST, TRAPTYPE, (void *) stub_Close_ppc };
const struct EmuTrap stub_Expunge = { TRAPINST, TRAPTYPE, (void *) stub_Expunge_ppc };
const struct EmuTrap stub_Reserved = { TRAPINST, TRAPTYPE, (void *) stub_Reserved_ppc };

const struct EmuTrap stub_allocChip = { TRAPINST, TRAPTYPE, (void *) stub_allocChip_ppc };
const struct EmuTrap stub_freeChip = { TRAPINST, TRAPTYPE, (void *) stub_freeChip_ppc };
const struct EmuTrap stub_hostAddressToChip = { TRAPINST, TRAPTYPE, (void *) stub_hostAddressToChip_ppc };
const struct EmuTrap stub_chipAddressToHost = { TRAPINST, TRAPTYPE, (void *) stub_chipAddressToHost_ppc };
const struct EmuTrap stub_writeChipLong = { TRAPINST, TRAPTYPE, (void *) stub_writeChipLong_ppc };
const struct EmuTrap stub_writeChipWord = { TRAPINST, TRAPTYPE, (void *) stub_writeChipWord_ppc };
const struct EmuTrap stub_writeChipByte = { TRAPINST, TRAPTYPE, (void *) stub_writeChipByte_ppc };
const struct EmuTrap stub_readChipLong = { TRAPINST, TRAPTYPE, (void *) stub_readChipLong_ppc };
const struct EmuTrap stub_readChipWord = { TRAPINST, TRAPTYPE, (void *) stub_readChipWord_ppc };
const struct EmuTrap stub_readChipByte = { TRAPINST, TRAPTYPE, (void *) stub_readChipByte_ppc };
const struct EmuTrap stub_bitChgChipLong = { TRAPINST, TRAPTYPE, (void *) stub_bitChgChipLong_ppc };
const struct EmuTrap stub_bitChgChipWord = { TRAPINST, TRAPTYPE, (void *) stub_bitChgChipWord_ppc };
const struct EmuTrap stub_bitChgChipByte = { TRAPINST, TRAPTYPE, (void *) stub_bitChgChipByte_ppc };
const struct EmuTrap stub_bitSetChipLong = { TRAPINST, TRAPTYPE, (void *) stub_bitSetChipLong_ppc };
const struct EmuTrap stub_bitSetChipWord = { TRAPINST, TRAPTYPE, (void *) stub_bitSetChipWord_ppc };
const struct EmuTrap stub_bitSetChipByte = { TRAPINST, TRAPTYPE, (void *) stub_bitSetChipByte_ppc };
const struct EmuTrap stub_bitClrChipLong = { TRAPINST, TRAPTYPE, (void *) stub_bitClrChipLong_ppc };
const struct EmuTrap stub_bitClrChipWord = { TRAPINST, TRAPTYPE, (void *) stub_bitClrChipWord_ppc };
const struct EmuTrap stub_bitClrChipByte = { TRAPINST, TRAPTYPE, (void *) stub_bitClrChipByte_ppc };


const struct EmuTrap stub_setCIATimingAccuracy = { TRAPINST, TRAPTYPE, (void *) stub_setCIATimingAccuracy_ppc };


const ULONG VecTable68K[] = {
	(ULONG) &stub_Open,
	(ULONG) &stub_Close,
	(ULONG) &stub_Expunge,
	(ULONG) &stub_Reserved,
	(ULONG) &stub_allocChip,
	(ULONG) &stub_freeChip,
	(ULONG) &stub_hostAddressToChip,
	(ULONG) &stub_chipAddressToHost,
	(ULONG) &stub_writeChipLong,
	(ULONG) &stub_writeChipWord,
	(ULONG) &stub_writeChipByte,
	(ULONG) &stub_readChipLong,
	(ULONG) &stub_readChipWord,
	(ULONG) &stub_readChipByte,
	(ULONG) &stub_bitChgChipLong,
	(ULONG) &stub_bitChgChipWord,
	(ULONG) &stub_bitChgChipByte,
	(ULONG) &stub_bitSetChipLong,
	(ULONG) &stub_bitSetChipWord,
	(ULONG) &stub_bitSetChipByte,
	(ULONG) &stub_bitClrChipLong,
	(ULONG) &stub_bitClrChipWord,
	(ULONG) &stub_bitClrChipByte,
	(ULONG) &stub_setCIATimingAccuracy,
	(ULONG)-1
};

STATIC APTR stub_Open_ppc(ULONG *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((ULONG)Base + Base->lib_PosSize);
	struct LibraryManagerInterface *Self = (struct LibraryManagerInterface *) ExtLib->ILibrary;

	regarray[REG68K_D0/4] = (APTR) Self -> Open( 0);

	return (APTR) regarray[REG68K_D0/4];
}

STATIC APTR stub_Close_ppc(ULONG *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((ULONG)Base + Base->lib_PosSize);
	struct LibraryManagerInterface *Self = (struct LibraryManagerInterface *) ExtLib->ILibrary;

	regarray[REG68K_D0/4] = (APTR) Self -> Close();

	return (APTR) regarray[REG68K_D0/4];
}

STATIC APTR stub_Expunge_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	return NULL;
}

STATIC ULONG stub_Reserved_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	return 0UL;
}

STATIC ULONG stub_allocChip_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	regarray[REG68K_D0/4] = _allocChip(regarray[REG68K_D1/4]);;

	return (APTR) regarray[REG68K_D0/4];
}

STATIC VOID stub_freeChip_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	_freeChip(regarray[REG68K_A0/4]);;
}

STATIC ULONG stub_hostAddressToChip_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	return (APTR) regarray[REG68K_D0/4];
}

STATIC ULONG stub_chipAddressToHost_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	return (APTR) regarray[REG68K_D0/4];
}

STATIC VOID stub_writeChipLong_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);
	longput((char *) regarray[REG68K_A0/4],(ULONG) regarray[REG68K_D1/4]);
}

STATIC VOID stub_writeChipWord_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);
	wordput(regarray[REG68K_A0/4], regarray[REG68K_D1/4]);
}

STATIC VOID stub_writeChipByte_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);
	byteput(regarray[REG68K_A0/4], regarray[REG68K_D1/4]);
}

STATIC ULONG stub_readChipLong_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	regarray[REG68K_D0/4]=longget(regarray[REG68K_A0/4]);
	return regarray[REG68K_D0/4];
}

STATIC ULONG stub_readChipWord_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	regarray[REG68K_D0/4]=wordget(regarray[REG68K_A0/4]);
	return regarray[REG68K_D0/4];
}

STATIC ULONG stub_readChipByte_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	Printf("A0: %08lx\n", regarray[REG68K_A0/4] );

	regarray[REG68K_D0/4]=byteget(regarray[REG68K_A0/4]);

	Printf("D0: %08lx\n", regarray[REG68K_D0/4] );

	return regarray[REG68K_D0/4];
}

STATIC VOID stub_bitChgChipLong_ppc(ULONG *regarray)
{
	_bitChgChipLong( (char *) regarray[REG68K_A0/4],regarray[REG68K_D1/4]);
}

STATIC VOID stub_bitChgChipWord_ppc(ULONG *regarray)
{
	_bitChgChipWord( (char *) regarray[REG68K_A0/4],regarray[REG68K_D1/4]);
}

STATIC VOID stub_bitChgChipByte_ppc(ULONG *regarray)
{
	_bitChgChipByte( (char *) regarray[REG68K_A0/4],regarray[REG68K_D1/4]);
}

STATIC VOID stub_bitSetChipLong_ppc(ULONG *regarray)
{
	_bitSetChipLong( (char *) regarray[REG68K_A0/4],regarray[REG68K_D1/4]);
}

STATIC VOID stub_bitSetChipWord_ppc(ULONG *regarray)
{
	_bitSetChipWord( (char *) regarray[REG68K_A0/4],regarray[REG68K_D1/4]);
}

STATIC VOID stub_bitSetChipByte_ppc(ULONG *regarray)
{
	_bitSetChipByte( (char *) regarray[REG68K_A0/4],regarray[REG68K_D1/4]);
}

STATIC VOID stub_bitClrChipLong_ppc(ULONG *regarray)
{
	_bitClrChipLong( (char *) regarray[REG68K_A0/4],regarray[REG68K_D1/4]);
}

STATIC VOID stub_bitClrChipWord_ppc(ULONG *regarray)
{
	_bitClrChipWord( (char *) regarray[REG68K_A0/4],regarray[REG68K_D1/4]);
}

STATIC VOID stub_bitClrChipByte_ppc(ULONG *regarray)
{
	_bitClrChipByte( (char *) regarray[REG68K_A0/4],regarray[REG68K_D1/4]);
}

STATIC VOID stub_setCIATimingAccuracy_ppc(ULONG *regarray)
{
	Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);
}


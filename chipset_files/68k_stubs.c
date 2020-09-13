
#include <exec/libraries.h>
#include <exec/emulation.h>

#include <proto/exec.h>
#include <proto/dos.h>


STATIC APTR stub_Open_ppc(ULONG *regarray);
STATIC APTR stub_Close_ppc(ULONG *regarray);
STATIC APTR stub_Expunge_ppc(ULONG *regarray);
STATIC ULONG stub_Reserved_ppc(ULONG *regarray);

STATIC VOID stub_allocChip_ppc(ULONG *regarray);
STATIC VOID stub_freeChip_ppc(ULONG *regarray);
STATIC VOID stub_writeChipLong_ppc(ULONG *regarray);
STATIC VOID stub_writeChipWord_ppc(ULONG *regarray);
STATIC VOID stub_writeChipByte_ppc(ULONG *regarray);
STATIC VOID stub_readChipLong_ppc(ULONG *regarray);
STATIC VOID stub_readChipWord_ppc(ULONG *regarray);
STATIC VOID stub_readChipByte_ppc(ULONG *regarray);
STATIC VOID stub_setCIATimingAccuracy_ppc(ULONG *regarray);


const struct EmuTrap stub_Open = { TRAPINST, TRAPTYPE, (void *) stub_Open_ppc };
const struct EmuTrap stub_Close = { TRAPINST, TRAPTYPE, (void *) stub_Close_ppc };
const struct EmuTrap stub_Expunge = { TRAPINST, TRAPTYPE, (void *) stub_Expunge_ppc };
const struct EmuTrap stub_Reserved = { TRAPINST, TRAPTYPE, (void *) stub_Reserved_ppc };

const struct EmuTrap stub_allocChip = { TRAPINST, TRAPTYPE, (void *) stub_allocChip_ppc };
const struct EmuTrap stub_freeChip = { TRAPINST, TRAPTYPE, (void *) stub_freeChip_ppc };
const struct EmuTrap stub_writeChipLong = { TRAPINST, TRAPTYPE, (void *) stub_writeChipLong_ppc };
const struct EmuTrap stub_writeChipWord = { TRAPINST, TRAPTYPE, (void *) stub_writeChipWord_ppc };
const struct EmuTrap stub_writeChipByte = { TRAPINST, TRAPTYPE, (void *) stub_writeChipByte_ppc };
const struct EmuTrap stub_readChipLong = { TRAPINST, TRAPTYPE, (void *) stub_readChipLong_ppc };
const struct EmuTrap stub_readChipWord = { TRAPINST, TRAPTYPE, (void *) stub_readChipWord_ppc };
const struct EmuTrap stub_readChipByte = { TRAPINST, TRAPTYPE, (void *) stub_readChipByte_ppc };
const struct EmuTrap stub_setCIATimingAccuracy = { TRAPINST, TRAPTYPE, (void *) stub_setCIATimingAccuracy_ppc };


const ULONG VecTable68K[] = {
	(ULONG) &stub_Open,
	(ULONG) &stub_Close,
	(ULONG) &stub_Expunge,
	(ULONG) &stub_Reserved,
	(ULONG) &stub_allocChip,
	(ULONG) &stub_freeChip,
	(ULONG) &stub_writeChipLong,
	(ULONG) &stub_writeChipWord,
	(ULONG) &stub_writeChipByte,
	(ULONG) &stub_readChipLong,
	(ULONG) &stub_readChipWord,
	(ULONG) &stub_readChipByte,
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
	return NULL;
}

STATIC ULONG stub_Reserved_ppc(ULONG *regarray)
{
	return 0UL;
}

STATIC VOID stub_allocChip_ppc(ULONG *regarray)
{
}

STATIC VOID stub_freeChip_ppc(ULONG *regarray)
{
}

STATIC VOID stub_writeChipLong_ppc(ULONG *regarray)
{
}

STATIC VOID stub_writeChipWord_ppc(ULONG *regarray)
{
}

STATIC VOID stub_writeChipByte_ppc(ULONG *regarray)
{
}

STATIC VOID stub_readChipLong_ppc(ULONG *regarray)
{
}

STATIC VOID stub_readChipWord_ppc(ULONG *regarray)
{
}

STATIC VOID stub_readChipByte_ppc(ULONG *regarray)
{
}

STATIC VOID stub_setCIATimingAccuracy_ppc(ULONG *regarray)
{
}

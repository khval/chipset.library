
When accessing this function from a 680x0 assmbler..

	D0 is return value
	D1 is arg1 or arg2 (any input value)
	A0 is arg1 or arg2 (any address pointer)
	A6 is expected to be library base.

void bitChgChipByte(char * offset,ULONG bit);

	Toggle a bit in a hardware register at virtual address offset.

	This is similar to:

	*offset ~= 1L<<bit;

	However, actions are taken when things are written to hardware registers.
	for more information look in Amiga hardware reference manual.

	(Some hardware registers does not support Byte.)

void bitChgChipWord(char * offset,ULONG bit);

	Toggle a bit in a hardware register at virtual address offset.

	16bit alignment is expected.

void bitChgChipLong(char * offset,ULONG bit);

	Toggle a bit in hardware register at virtual address offset.

void bitClrChipByte(char * offset, ULONG bit)

	Clear a bit in a hardware register at virtual address offset.

	This is similar to:

	*offset &= ~(1L<<bit);

	However, actions are taken when things are written to hardware registers.
	for more information look in Amiga hardware reference manual.
	(Some hardware registers does not support Byte.)

void bitClrChipWord(char * offset, ULONG bit)

	Clear a bit in a hardware register at virtual address offset.

	16bit alignment is expected.

void bitClrChipLong(char * offset, ULONG bit)

	Clear a bit in a hardware register at virtual address offset.

void bitSetChipByte(char * offset, ULONG bit)

	Set a bit in a hardware register at virtual address offset.

	This is similar to:

	*offset |= 1L<<bit;

	However, actions are taken when things are written to hardware registers.
	for more information look in Amiga hardware reference manual.

	(Some hardware registers does not support Byte offsets.)

void bitSetChipWord(char * offset, ULONG bit)

	Set a bit in a hardware register at virtual address offset.

	16bit alignment is expected.

void bitSetChipLong(char * offset, ULONG bit)

	Set a bit in a hardware register at virtual address offset.

	16bit alignment is expected.

void allocChip( uint32_t size )

	Chipset.library also support allocating memory in virtual space, the idea is that chipset 
	should have access to it, if its moved to PCIe card or some other add on card, 
	its expected we need to translate address between two spaces.

void freeChip(void * mem)

	Free virtual memory on PCIe / Zorro card, or memory from emulated memory space.

	Currently it's not needed for Paula/NallePuh in chipset.library, but it can be needed in the future.

void * chipAddressToHost(void * adr)

	Convert PCIe / emulated address space to system memory address space.

	Currently its not needed for Paula/NallePuh in chipset.library, but it can be needed in the future.

void * hostAddressToChip( void * adr )

	Convert address space from system address space to a PCIe / emulated address space.

	Currently its not needed for Paula/NallePuh in chipset.library, but it can be needed in the future.		

ULONG readChipByte( char * offset )

	Reads a byte from hardware register.

	unlike memory the hardware registers, change states see the Amiga hardware reference manual.

	Not all hardware register, supports bytes.

ULONG readChipWord( char * offset )

	Reads a word (uint16) from hardware register.

ULONG readChipLong( char * offset )

	Reads a long (uint32) from hardware register.

ULONG writeChipByte( char * offset )

	Write a byte from hardware register.

	unlike memory the hardware registers, change states see the Amiga hardware reference manual.

		Not all hardware register, supports bytes.

ULONG writeChipWord( char * offset )

	Write a word (uint16) from hardware register.

ULONG writeChipLong( char * offset )

	Write a long (uint32) from hardware register.

void setCIAClockSpeed(double us)

	Change the speed of the MOS8520 CIA chip.
	us should be the micro seconds it takes to perform a cycle.

	us = 1 / hz

	It can be useful to slow the chipset down when debugging, you can’t do that with a real chip.

void setCIATimingAccuracy( int us )

	When emulating the CIA, we need to generate the clock count, we do this by calculating the time difference, 
	between etch time the MOS8520 CIA chip is waked up, divided it the CPU cycle time, 
	if the MOS8520 is waken up too often you get high CPU cycle usage, and maybe unassay high accuracy,
	if its too low it can get some lag in number cycles.
	ULONG spyCIAA( ULONG Addr )

	This one is a lot like readChipWord, but it only works on CIAA.

ULONG spyCIAB( ULONG Addr )

	This one is a lot like readChipWord, but it only works on CIAB.

void writeChipByte( char * offset, ULONG value)

	Write a byte (uint8) in hardware register, read hardware manual for more information.

void writeChipWord( char * offset, ULONG value)

	Write a word (uint16) in hardware register, read the Amiga Hardware manual for more information.

void writeChipLong( char * offset, ULONG value)

	Write a long (uint32) in hardware register, read the Amiga Hardware Manual for more information.



# ---- Dictionary ----

* Virtual space

		Virtual space is memory located at different location, it be emulated or it can be real,
		sometime it can be the same as the host address, don't assume you know where its location.

* Host address *

		Host address is in the computers main memory.

* Define __use_real_chipset__

	If your compiling for a Amiga, and not for chipset.library

	If you set this define, the macros are pointing to read hardware, 
	this can also be useful when developing on AmigaONE system to 
	check if your code is backward compatible.

	Note you should be using Exec to setup timing interrupts or use timer.device to time your code, 
	or delay in dos.library, using CIAA/CIAB directly and busy looping, is nasty.

	If you do not set this, you are compiling for chipset.library

# ---- Macros ----

	If __use_real_chipset__ is defined its the same as

chipReadByte offset(reg),dest

	chipReadByte sourceOffset,sourceReg,DestReg

	If __use_real_chipset__ is not defined its the same as:

	dest = readChipByte( offset + CpuReg )

	If __use_real_chipset__ is defined its the same as:

	move.b	offset(CpuReg),dest

chipWriteByte

	chipWriteByte sourceReg/Value,destOffset,DestReg
	chipWriteByte sourceValue,destOffset,DestReg

	If __use_real_chipset__ is not defined its the same as:

	writeChipByte( source , offset + CpuReg )

	If __use_real_chipset__ is defined its the same as:

	move.b	source,offset(CpuReg)

chipWriteWord

	If __use_real_chipset__ is not defined its the same as:

	writeChipWord( source , offset + CpuReg )

	If __use_real_chipset__ is defined its the same as:

	move.w	source,offset(CpuReg)

chipWriteLong

	If __use_real_chipset__ is not defined its the same as:

	writeChipLong( source , offset + CpuReg )

	If __use_real_chipset__ is defined its the same as:

	move.l	source,offset(CpuReg)

chipSetBitInByte

	If __use_real_chipset__ is not defined its the same as:

	bitSetChipByte( bit , offset + CpuReg )

	If __use_real_chipset__ is defined its the same as:

	bset.b	bit,offset(CpuReg)


chipChgBitInByte	macro 

	If __use_real_chipset__ is not defined its the same as:

	bitChgChipByte( bit , offset + CpuReg )

	If __use_real_chipset__ is defined its the same as:

	bchg.b	bit,offset(CpuReg)


# ---- LVO table ----

	_LVOAllocChip	EQU -30

	; Allocates chip memory in virtaual space

	; D1 size to allocate
	; D0 returns address (host space address), you need to convert if you need it as virtual space.

	_LVOFreeChip	EQU -36

	; Free chip memory in virtaual space

	; A0 address to free (host space address)

	_LVOHostAddressToChip	EQU -42

	; D0 returns CHIP address (virtual address)

	_LVOChipAddressToHost	EQU -48

	; D0 returns native address

	_LVOWriteChipLong	EQU -54

	; A0 address in chip (virtual mem)
	; D1 value to set

	_LVOWriteChipWord	EQU -60

	; A0 address in chip (virtual mem)
	; D1 value to set

	_LVOWriteChipByte	EQU -66

	; A0 address in chip (virtual mem)
	; D1 value to set

	_LVOReadChipLong	EQU -72

	; A0 address in chip (virtual mem)
	; D0 return value

	_LVOReadChipWord	EQU -78

	; A0 address in chip (virtual mem)
	; D0 return value

	_LVOReadChipByte	EQU -84

	; A0 address in chip (virtual mem)
	; D0 return value

	_LVOBitChgChipLong	EQU -90

	; A0 address in chip (virtual mem)
	; D1 bit to flip

	_LVOBitChgChipWord	EQU -96

	; A0 address in chip (virtual mem)
	; D1 bit to flip

	_LVOBitChgChipByte		EQU -102

	; A0 address in chip (virtual mem)
	; D1 bit to flip

	_LVOBitSetChipLong		EQU -108

	; A0 address in chip (virtual mem)
	; D1 bit to set

	_LVOBitSetChipWord		EQU -114

	; A0 address in chip (virtual mem)
	; D1 bit to set

	_LVOBitSetChipByte		EQU -120

	; A0 address in chip (virtual mem)
	; D1 bit to set

	_LVOBitClrChipLong		EQU -126

	; A0 address in chip (virtual mem)
	; D1 bit to clear

	_LVOBitClrChipWord		EQU -132

	; A0 address in chip (virtual mem)
	; D1 bit to clear

	_LVOBitClrChipByte		EQU -138

	; A0 address in chip (virtual mem)
	; D1 bit to clear

	_LVOSetCIATimingAccuracy	EQU -144

	; Because its not ideal to use 100% cpu time to emulate one chip
	; this timer sets how often the chip should wake up, and take care of timing (and count CPU cycles)
	; On real hardware you don't need to worry about this.

	; D1 sets mciro seconds accuracy, 

	


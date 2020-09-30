; :ts=4
;    Copyright (C) 2020  Kjetil Hvalstrand

;    This library is free software; you can redistribute it and/or
;    modify it under the terms of the GNU Lesser General Public
;    License as published by the Free Software Foundation; either
;    version 2.1 of the License, or (at your option) any later version.

;    This library is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;    Lesser General Public License for more details.

;    You should have received a copy of the GNU Lesser General Public
;    License along with this library; if not, write to the Free Software
;    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
;    USA


; ---- Dictionary ----
;
; * Virtual space
;
; Virtual space is memory located at different location, it be emulated or it can be real,
; sometime it can be the same as the host address, don't assume you know where its location.
;
; * Host address
;
; Host address is in the computers main memory.
;


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

; chipReadByte sourceOffset,sourceReg,DestReg

	IFD	__use_real_chipset__

chipReadByte		macro 
	move.b	\1(\2),\3
	endm

chipWriteByte		macro 
	move.b	\1,(\2)\3
	endm

chipWriteWord		macro 
	move.b	\1,(\2)\3
	endm

chipWriteLong		macro 
	move.l	\1,(\2)\3
	endm

chipSetBitInByte	macro 
	bset.b	\1,\2(\3)
	endm

chipChgBitInByte	macro 
	bchg.b	\1,\2(\3)
	endm

	ELSE

chipReadByte		macro 
	move.l	a0,-(sp)
	move.l	d1,-(sp)
	move.l	\2,A0
	add.l		#\1,A0
	LINKLIB	_LVOReadChipByte,chipsetBase
	IFNC	'\3','D0'
	move.l	d0,\3
	ENDC
	move.l	(sp)+,d1
	move.l	(sp)+,a0
	endm

; chipWriteByte sourceReg/Value,destOffset,DestReg
; chipWriteByte sourceValue,destOffset,DestReg

chipWriteByte		macro 
	move.l	a0,-(sp)
	move.l	d1,-(sp)
	IFNC	'\1','D1'
	move.l	\1,d1
	ENDC
	move.l	\3,A0
	add.l		#\2,A0
	LINKLIB	_LVOWriteChipByte,chipsetBase
	move.l	(sp)+,d1
	move.l	(sp)+,a0
	endm

chipWriteWord		macro 
	move.l	a0,-(sp)
	move.l	d1,-(sp)
	IFNC	'\1','D1'
	move.l	\1,d1
	ENDC
	move.l	\3,A0
	add.l		#\2,A0
	LINKLIB	_LVOWriteChipWord,chipsetBase
	move.l	(sp)+,d1
	move.l	(sp)+,a0
	endm

chipWriteLong		macro 
	move.l	a0,-(sp)
	move.l	d1,-(sp)
	IFNC	'\1','D1'
	move.l	\1,d1
	ENDC
	move.l	\3,A0
	add.l		#\2,A0
	LINKLIB	_LVOWriteChipLong,chipsetBase
	move.l	(sp)+,d1
	move.l	(sp)+,a0
	endm

chipChgBitInByte	macro
	move.l	a0,-(sp)
	move.l	d1,-(sp)
	move.l	\1,d1
	move.l	\3,A0
	add.l		#\2,A0
	LINKLIB	_LVOBitChgChipByte,chipsetBase
	move.l	(sp)+,d1
	move.l	(sp)+,a0
	endm

chipSetBitInByte	macro 
	move.l	a0,-(sp)
	move.l	d1,-(sp)
	move.l	\1,d1
	move.l	\3,A0
	add.l		#\2,A0
	LINKLIB	_LVOBitSetChipByte,chipsetBase
	move.l	(sp)+,d1
	move.l	(sp)+,a0
	endm

	ENDC

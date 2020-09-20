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

_LVOSetCIATimingAccuracy	EQU -90

	; Because its not ideal to use 100% cpu time to emulate one chip
	; this timer sets how often the chip should wake up, and take care of timing (and count CPU cycles)
	; On real hardware you don't need to worry about this.

	; D1 sets mciro seconds accuracy, 

; chipReadByte sourceOffset,sourceReg,DestReg

chipReadByte		macro 
	move.l	\2,d1
	add.l		#\1,d1
	LINKLIB	_LVOReadChipByte,chipsetBase
	move.l	d0,\3
	endm

; chipWriteByte sourceReg/Value,destOffset,DestReg
; chipWriteByte sourceValue,destOffset,DestReg

chipWriteByte		macro 
	move.l	\1,d1
	move.l	\3,d2
	add.l		#\2,d2
	LINKLIB	_LVOWriteChipByte,chipsetBase
	endm


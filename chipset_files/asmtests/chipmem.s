
	include "lvo/exec_lib.i"
	include "lvo/dos_lib.i"
	include "exec/libraries.i"

	include "chipset.i"


	SECTION main,CODE

openLib	macro 
	lea \1,A1
	move.l 4.w,a6
	moveq #0,d0
	jsr _LVOOpenLibrary(a6)
	move.l d0,\2
	tst.l	d0
	beq	closeLibs
	endm

closeLib	macro
		lea 1\(pc),a1
		jsr _closeLib
		endm

writeText	macro
		lea 1\(pc),a1
		jsr _writeText
		endm

main:
	openLib	dosName, dosBase
	openLib	chipsetName, chipsetBase

	lea	txtLibsOpen(pc),a1
	jsr	_writeText

	move.l #2000,D1
	LINKLIB _LVOAllocChip,chipsetBase
	move.l D0,allocated

	move.l #60,d1
	LINKLIB _LVODelay,dosBase

	move.l allocated,A0
	LINKLIB _LVOFreeChip,chipsetBase

	move.l #60,d1
	LINKLIB _LVODelay,dosBase

closeLibs:
	move.l dosBase(pc),a1
	jsr	closeLib
	move.l chipsetBase(pc),a1
	jsr	closeLib
	rts

closeLib
	move.l a1,d0
	tst.l d0
	beq.s	.notOpen
	move.l 4,a6
	jsr	_LVOCloseLibrary(a6)
	moveq #0,d0
.notOpen
	rts

_writeText:
	move.l (a1),d2
	add.l #4,a1
	move.l a1,d1
	move.l	dosBase(pc),A6
	jsr		_LVOWriteChars(a6)
	rts

dosBase:
	dc.l	0

chipsetBase
	dc.l	0

allocated:
	dc.l	0

txtLibsOpen:
	dc.l	14
	dc.b "Libs are open",$A,0

dosName:
	dc.b	"dos.library",0

chipsetName:
	dc.b	"chipset.library",0


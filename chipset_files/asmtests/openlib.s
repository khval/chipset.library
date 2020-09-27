
	include "lvo/exec_lib.i"
	include "lvo/dos_lib.i"


	SECTION main,CODE

OPENLIB	macro 
	lea \1Name,A1
	move.l 4.w,a6
	moveq #0,d0
	jsr _LVOOpenLibrary(a6)
	move.l d0,\1Base
	tst.l	d0
	beq	closeLibs
	endm

CLOSELIB	macro
	move.l \1Base(pc),a1
	jsr	closeLib
	endm

INFOTXT	macro
	move.l	#PRINT_FMT,D1	; FMT
	move.l	#printf_args,A1	; ARGS
	move.l	#txt\1,(A1)
	move.l	A1,D2			; D2
	move.l	dosBase,a6		; DosBase
	jsr		_LVOVPrintf(a6)	; Printf
	endm

INFOVALUE	macro
	move.l	#printf_args,A1	; ARGS
	move.l	\1,(A1)
	move.l	#PRINT_VALUE_FMT,D1	; FMT
	move.l	#printf_args,D2		; ARG ARRAY
	move.l	dosBase,a6		; DosBase
	jsr		_LVOVPrintf(a6)	; Printf
	endm

DELAY	macro
	move.l #\1,d1
	jsr delay
	endm

writeText	macro
		lea 1\(pc),a1
		jsr _writeText
		endm

main:
	OPENLIB	dos
	OPENLIB	chipset

	INFOTXT LibsOpen

	INFOVALUE D0
	DELAY 30

closeLibs:
	CLOSELIB	chipset
	CLOSELIB	dos
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

delay
	move.l dosBase(pc),a6
	jsr _LVODelay(a6)
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
	dc.l 0

printf_args:
		ds.l	20

txtLibsOpen:
	dc.b "Libs are open",$A,0

dosName:
	dc.b	"dos.library",0

chipsetName:
	dc.b	"chipset.library",0

PRINT_FMT
		dc.b	"%s",10,0

PRINT_VALUE_FMT
		dc.b	"VALUE: %ld",10,0


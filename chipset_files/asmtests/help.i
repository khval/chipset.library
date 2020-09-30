
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
	move.l	D0,-(SP)
	move.l	D1,-(SP)
	move.l	D2,-(SP)
	move.l	A1,-(SP)
	move.l	#PRINT_TXT_FMT,D1	; FMT
	move.l	#printf_args,A1	; ARGS
	move.l	#txt\1,(A1)
	move.l	A1,D2			; D2
	move.l	dosBase,a6		; DosBase
	jsr		_LVOVPrintf(a6)	; Printf
	move.l	(SP)+,A1
	move.l	(SP)+,D2
	move.l	(SP)+,D1
	move.l	(SP)+,D0
	endm

INFOVALUE	macro
	move.l	D0,-(SP)
	move.l	#printf_args,A1	; ARGS
	move.l	\1,(A1)
	move.l	D1,-(SP)
	move.l	D2,-(SP)
	move.l	A1,-(SP)
	move.l	#PRINT_VALUE_FMT,D1	; FMT
	move.l	#printf_args,D2		; ARG ARRAY
	move.l	dosBase,a6		; DosBase
	jsr		_LVOVPrintf(a6)	; Printf
	move.l	(SP)+,A1
	move.l	(SP)+,D2
	move.l	(SP)+,D1
	move.l	(SP)+,D0
	endm

INFOHEX	macro
	move.l	D0,-(SP)
	move.l	#printf_args,A1	; ARGS
	move.l	\1,(A1)
	move.l	D1,-(SP)
	move.l	D2,-(SP)
	move.l	A1,-(SP)
	move.l	#PRINT_HEX_FMT,D1	; FMT
	move.l	#printf_args,D2		; ARG ARRAY
	move.l	dosBase,a6		; DosBase
	jsr		_LVOVPrintf(a6)	; Printf
	move.l	(SP)+,A1
	move.l	(SP)+,D2
	move.l	(SP)+,D1
	move.l	(SP)+,D0
	endm

DELAY	macro
	move.l #\1,d1
	move.l dosBase(pc),a6
	jsr _LVODelay(a6)
	endm

GETCHAR	macro
	move.l dosBase(pc),a6
	jsr	_LVOInput(a6)
	move.l	d0,d1
	jsr	_LVOFGetC(a6)
	endm

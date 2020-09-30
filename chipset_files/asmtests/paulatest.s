
;__use_real_chipset__ set 1

	include "lvo/exec_lib.i"
	include "lvo/dos_lib.i"
	include "exec/libraries.i"
	
	include "hardware/custom.i"
	include "hardware/dmabits.i"
	include "chipset.i"
	include "help.i"

	SECTION main,CODE

custom	EQU	$DFF000


main:
	OPENLIB	dos

	IFND	__use_real_chipset__
	OPENLIB	chipset
	ENDC

	INFOHEX #custom+aud0

;	move.l 4.w,a6
;	jsr _LVOForbid(a6)
	
	jsr playSound

	DELAY 50

	jsr stopSound

;	move.l 4.w,a6
;	jsr _LVOPermit(a6)


closeLibs:

	IFND	__use_real_chipset__
	CLOSELIB	chipset
	ENDC

	CLOSELIB	dos
	rts

closeLib
	move.l	a1,d0
	tst.l d0
	beq.s	.notOpen
	move.l	4,a6
	jsr		_LVOCloseLibrary(a6)
	moveq	#0,d0
.notOpen
	rts

playSound:
		move.l	#custom,a0
		LEA SINEDATA,a1

WHERE0DATA:
;		MOVE.L	A1,aud0+ac_ptr(A0)
		chipWriteLong A1,aud0+ac_ptr,A0

SETAUD0LENGTH:
;		MOVE.W	#4,aud0+ac_len(A0)
		chipWriteWord #4,aud0+ac_len,A0

SETAUD0VOLUME:
;		MOVE.W	#64,aud0+ac_vol(A0)
		chipWriteWord #64,aud0+ac_vol,A0

SETAUD0PERIOD:
;		MOVE.W	#447,aud0+ac_per(A0)
		chipWriteWord	#447,aud0+ac_per,A0

BEGINCHAN0:
;		MOVE.W	#(DMAF_SETCLR!DMAF_AUD0!DMAF_MASTER),dmacon(A0)
		chipWriteWord	#(DMAF_SETCLR!DMAF_AUD0!DMAF_MASTER),dmacon,A0
		RTS
		DS.W 0	; Be sure its word-aligned

stopSound:
		move.l	#custom,a0
;		MOVE.W	#DMAF_AUD0,dmacon(A0)
		chipWriteWord	#DMAF_AUD0,dmacon,A0
		RTS

txtLibsOpen:
	dc.l	14
	dc.b "Libs are open",$A,0

dosBase:
	dc.l	0

chipsetBase
	dc.l 0

printf_args:
	ds.l	20

dosName:
	dc.b	"dos.library",0

chipsetName:
	dc.b	"chipset.library",0

PRINT_TXT_FMT
		dc.b	"TEXT: %s",10,0

PRINT_VALUE_FMT
		dc.b	"VALUE: %ld",10,0

PRINT_HEX_FMT
		dc.b	"HEX: %lx",10,0

;------------------------------------------------------------------------------------------

	SECTION ".data_chip",data

SINEDATA:
		DC.B		0,90,127,90,0,-90,-127,-90

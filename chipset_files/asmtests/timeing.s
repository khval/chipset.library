
;
; 8520_timing.asm
;
; A complete 8520 timing example.  This blinks the power light at (exactly)
; 3 milisecond intervals.  It takes over the machine, so watch out!
;
; The base Amiga crytal frequencies are:
;           NTSC    28.63636  MHz
;           PAL     28.37516  MHz
;
; The two 16 bit timers on the 8520 chips each count down at 1/10 the CPU
; clock, or 0.715909 MHz.  That works out to 1.3968255 microseconds per count.
; Under PAL the countdown is slightly slower, 0.709379 MHz.
;
; To wait 1/100 second would require waiting 10,000 microseconds.
; The timer register would be set to (10,000 / 1.3968255 = 7159).
;
; To wait 3 miliseconds would require waiting 3000 microseconds.
; The register would be set to (3000 / 1.3968255 = 2148).
;

;__use_real_chipset__ set 1

	include "lvo/exec_lib.i"
	include "lvo/dos_lib.i"
	include "exec/libraries.i"
	include "chipset.i"
	include "help.i"

	INCLUDE "hardware/cia.i"
	INCLUDE "hardware/custom.i"
;
;	XREF    _ciaa
;	XREF    _ciab
;	XREF    _custom
;
;        lea     _custom,a3          ; Base of custom chips
;        lea     _ciaa,a4                ; Get base address if CIA-A

	move.l #$BFE001,A4


main:
	OPENLIB	dos

	IFND	__use_real_chipset__
	OPENLIB	chipset
	ENDC

	lea	txtLibsOpen(pc),a1
	jsr	_writeText

	GETCHAR
	jsr ciatest

closeLibs:

	IFND	__use_real_chipset__
	CLOSELIB	chipset
	ENDC

	CLOSELIB	dos
	rts

ciatest

	lea	txtCiaTest(pc),a1
	jsr	_writeText

;        move.w  #$7fff,dmacon(a3)       ; Kill all chip interrupts

;----Setup, only do once
;----This sets all bits needed for timer A one-shot mode.

;-------------------------------------------------------------------------------
;        move.b  ciacra(a4),d0           ;Set control register A on CIAA
;-------------------------------------------------------------------------------

	chipReadByte ciacra,a4,d0

	and.b   #%11000000,d0           ;Don't trash bits we are not
	or.b    #%00001000,d0           ;using...

;-------------------------------------------------------------------------------
;        move.b  d0,ciacra(a4)
;-------------------------------------------------------------------------------

	chipWriteByte d0,ciacra,a4

;        move.b  #%01111111,ciaicr(a4)   ;Clear all 8520 interrupts

	chipWriteByte #%01111111,ciaicr,a4

;
;----Set time (low byte THEN high byte)
;----And the low order with $ff
;----Shift the high order by 8
;


TIME    equ     2148
;        move.b  #(TIME&$FF),ciatalo(a4)
	chipWriteByte #(TIME&$FF),ciatalo,a4

;        move.b  #(TIME>>8),ciatahi(a4)
	chipWriteByte #(TIME>>8),ciatahi,a4

;
;----Wait for the timer to count down

;	move.l	#0,d1
;	move.l	A4,A0
;	add.l		#ciapra,A0
;	LINKLIB	_LVOBitSetChipByte,chipsetBase

	chipSetBitInByte #0,ciapra,A4

busy_wait:

	IFND	__use_real_chipset__
	move.l  #0,A0
	chipReadByte 0,A0,D0
	tst.b	D0
	bne.s	.exit		; if something is set in ChipRam then quit...
	ENDC

;-------------------------------------------------------------------------------
;        btst.b  #0,ciaicr(a4)           ;Wait for timer expired flag
;---------------------------------------------------------------------------------

	chipReadByte ciaicr,a4,d0

;-------------------------------------------------------------------------------
	and.b	#1,d0
	tst.b		d0
	beq.s   busy_wait
;-------------------------------------------------------------------------------

	lea	txtBlinkLed(pc),a1
	jsr	_writeText

;-------------------------------------------------------------------------------
;        bchg.b  #CIAB_LED,ciapra(a4)    ;Blink light
;-------------------------------------------------------------------------------

	chipChgBitInByte #CIAB_LED,ciapra,a4

;-------------------------------------------------------------------------------
;        bset.b  #0,ciacra(a4)           ;Restart timer
;-------------------------------------------------------------------------------

	chipSetBitInByte #0,ciacra,a4

;-------------------------------------------------------------------------------
	bra.s   busy_wait
;-------------------------------------------------------------------------------

.exit
	INFOTXT	quit
	INFOVALUE d0
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

_writeText:
	move.l	(a1),d2
	add.l		#4,a1
	move.l	a1,d1
	move.l	dosBase(pc),A6
	jsr		_LVOWriteChars(a6)
	rts

dosBase:
	dc.l	0

chipsetBase
	dc.l 0

printf_args:
	ds.l	20

txtbusy:
	dc.b "busy loop start",$A,0

txtcheck:
	dc.b "check",$A,0

txtquit:
	dc.b "quit",$A,0

txtLibsOpen:
	dc.l	14
	dc.b "Libs are open",$A,0

txtBlinkLed:
	dc.l	10
	dc.b "blink led",$A,0

txtReadByteOk:
	dc.l	13
	dc.b "read byte ok",$A,0

txtWriteByteOk:
	dc.l	14
	dc.b "Write byte ok",$A,0

txtCiaTest:
	dc.l	9
	dc.b "CIA test",$A,0

dosName:
	dc.b	"dos.library",0

chipsetName:
	dc.b	"chipset.library",0

PRINT_TXT_FMT
		dc.b	"TEXT: %s",10,0

PRINT_VALUE_FMT
		dc.b	"VALUE: %ld",10,0


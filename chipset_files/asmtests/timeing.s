
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

	include "lvo/exec_lib.i"
	include "lvo/dos_lib.i"
	include "exec/libraries.i"
	include "chipset.i"

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

openLib	macro 
	lea \1,A1
	move.l 4.w,a6
	moveq #0,d0
	jsr _LVOOpenLibrary(a6)
	move.l d0,\2
	tst.l	d0
	beq	closeLibs
	endm

main:
	openLib	dosName, dosBase
	openLib	chipsetName,chipsetBase

	lea	txtLibsOpen(pc),a1
	jsr	_writeText

	jsr ciatest

closeLibs:
	move.l dosBase(pc),a1
	jsr	closeLib
	move.l chipsetBase(pc),a1
	jsr	closeLib
	rts

ciatest

	lea	txtCiaTest(pc),a1
	jsr	_writeText

;        move.w  #$7fff,dmacon(a3)       ; Kill all chip interrupts

;----Setup, only do once
;----This sets all bits needed for timer A one-shot mode.

;        move.b  ciacra(a4),d0           ;Set control register A on CIAA
	chipReadByte ciacra,a4,d0

        and.b   #%11000000,d0           ;Don't trash bits we are not
        or.b    #%00001000,d0           ;using...

	lea	txtReadByteOk(pc),a1
	jsr	_writeText

;        move.b  d0,ciacra(a4)
	chipWriteByte d0,ciacra,a4

	lea	txtWriteByteOk(pc),a1
	jsr	_writeText

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


busy_wait:
;        btst.b  #0,ciaicr(a4)           ;Wait for timer expired flag

	chipReadByte ciaicr,a4,d0
	and.b	#1,d0
	cmp.b	#1,d0

	beq.s   busy_wait

	lea	txtBlinkLed(pc),a1
	jsr	_writeText

;        bchg.b  #CIAB_LED,ciapra(a4)    ;Blink light

	chipReadByte ciapra,a4,d1
	eor.b	#CIAB_LED,d1
	chipWriteByte d0,ciapra,a4

;        bset.b  #0,ciacra(a4)           ;Restart timer

	chipReadByte ciapra,a4,d1
	or.b	#1,d1
	chipWriteByte d0,ciapra,a4

	bra.s   busy_wait

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
	dc.l 0

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

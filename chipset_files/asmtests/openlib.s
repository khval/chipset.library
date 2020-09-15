
oldOpenLibrary EQU -$0198
closeLibrary EQU -$019E


	SECTION main,CODE

openLib	macro 
	lea \1,A1
	move.l 4.w,a6
	jsr oldOpenLibrary(a6)
	beq.s	.out
	move.l \2,A0
	move.l D0,(A0)
	endm

closeLib	macro
		lea 1\,A1
		jsr _closeLib
		endm

writeText	macro
		lea 1\,A1
		jsr _writeText
		endm

main:
	openLib	dosName, dosBase
;	openLib	chipsetName, chipsetBase

;	writeText	txtLibsOpen

.out:
;	closeLib	chipsetBase
	closeLib	dosBase
	rts

_closeLib:
	move.l a1,d0
	tst.l	d0			; if D0==NULL
	beq	.notOpen
	move.l 4.w,a6
	jsr	closeLibrary(a6)
.notOpen:
	rts

_writeText:
	move.l (a1),d2
	add.l #4,a1
	move.l a1,d1
	lea dosBase,A6
	jsr -$03AE(a6)
	rts

txtLibsOpen:
	dc.l	14
	dc.b "Libs are open",$A,0

dosName:
	dc.b	"dos.library",0

dosBase:
	dc.l	0

chipsetName:
	dc.b	"chipset.library",0

chipsetBase
	dc.l 0


	;	oldOpenLib EQU -$0198
	; 	closeLib EQU -$019E


	SECTION main,CODE

openLib	macro
		lea 1\,A1
		move.l 4.w,a6
		jsr -$0198(a6)
		beq.s	.out
		LEA 2\,A0
		move.l D0,(A0)
		endm

closeLib	macro
		lea 1\,A1
		move.l 4.w,a6
		jsr	-$019E(a6)
		endm

main:
	openLib	chipsetName,chipsetBase
	closeLib	chipsetBase
.out
	rts


chipsetName:
	dc.b	"chipset.library",0

chipsetBase
	dc.l 0

VERSION		EQU	53
REVISION	EQU	1

DATE	MACRO
		dc.b '31.8.2020'
		ENDM

VERS	MACRO
		dc.b 'chipset.library 53.1'
		ENDM

VSTRING	MACRO
		dc.b 'chipset.library 53.1 (31.8.2020)',13,10,0
		ENDM

VERSTAG	MACRO
		dc.b 0,'$VER: chipset.library 53.1 (31.8.2020)',0
		ENDM

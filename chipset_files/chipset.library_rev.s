VERSION = 53
REVISION = 1

.macro DATE
.ascii "31.8.2020"
.endm

.macro VERS
.ascii "chipset.library 53.1"
.endm

.macro VSTRING
.ascii "chipset.library 53.1 (31.8.2020)"
.byte 13,10,0
.endm

.macro VERSTAG
.byte 0
.ascii "$VER: chipset.library 53.1 (31.8.2020)"
.byte 0
.endm

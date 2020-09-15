# asm one don't work on AmigaOS4, we need to use VBCC (vasm)

compiler=c/vasmm68k_mot
link=c/vlink

enable=.enable
disable=.disable

include makefile.cfg

tests = openlib.exe

all:	${tests} 

clean:
		delete obj/#? #?.exe 

# compile tests

openlib.exe: openlib.s
		$(compiler) $(test_options) -Iinclude -Fvobj -o obj/openlib.o openlib.s 
		$(link) -bamigahunk -o openlib.exe -s  obj/openlib.o

.PRECIOUS: %.hunk 


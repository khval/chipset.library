# asm one don't work on AmigaOS4, we need to use VBCC (vasm)

compiler=c/vasmm68k_mot

options=-m68020 

link=c/vlink

enable=.enable
disable=.disable

include makefile.cfg

inc=-Isdk31: -Isdk31:includes/include_i

tests = openlib.exe	

all:	${tests} 

clean:
		delete obj/#? #?.exe 

# compile tests

openlib.exe: openlib.s
		$(compiler) $(options) $(inc) -Fhunk -o obj/openlib.o openlib.s 
		$(link) -bamigahunk -o openlib.exe -s  obj/openlib.o

.PRECIOUS: %.hunk 


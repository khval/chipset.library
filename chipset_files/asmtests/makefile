# asm one don't work on AmigaOS4, we need to use VBCC (vasm)

compiler=c/vasmm68k_mot

options=-m68020 

link=c/vlink

enable=.enable
disable=.disable

include makefile.cfg

inc=-Isdk31: -Isdk31:includes/include_i

tests = openlib.exe	\
	chipmem.exe \
	timeing.exe \
	probe.exe \
	bitchg.exe \
	paulatest.exe

all:	${tests} 

clean:
		delete obj/#? #?.exe 

# compile tests

openlib.exe: openlib.s
		$(compiler) $(options) $(inc) -Fhunk -o obj/openlib.o openlib.s 
		$(link) -bamigahunk -o openlib.exe -s  obj/openlib.o

chipmem.exe: chipmem.s
		$(compiler) $(options) $(inc) -Fhunk -o obj/chipmem.o chipmem.s 
		$(link) -bamigahunk -o chipmem.exe -s  obj/chipmem.o

timeing.exe: timeing.s
		$(compiler) $(options) $(inc) -Fhunk -o obj/timeing.o timeing.s 
		$(link) -bamigahunk -o timeing.exe -s  obj/timeing.o

probe.exe: probe.s
		$(compiler) $(options) $(inc) -Fhunk -o obj/probe.o probe.s 
		$(link) -bamigahunk -o probe.exe -s  obj/probe.o

bitchg.exe: bitchg.s
		$(compiler) $(options) $(inc) -Fhunk -o obj/bitchg.o bitchg.s 
		$(link) -bamigahunk -o bitchg.exe -s  obj/bitchg.o

paulatest.exe: paulatest.s
		$(compiler) $(options) $(inc) -Fhunk -o obj/paulatest.o paulatest.s 
		$(link) -bamigahunk -o paulatest.exe -s  obj/paulatest.o

.PRECIOUS: %.hunk 



inc = -I../uade -I../ -I./ 

files = cia.exe cia2.exe mem.exe events.exe blit_interrupt.exe

deps =
deps_cia = ../lock.o ../uae/cia.o ../uae/memory.o ../uae/events.o
deps_cia2 = ../lock.o ../uae/cia.o ../uae/memory.o ../uae/events.o ../timer_device.o
deps_events = ../lock.o ../uae/cia.o ../uae/memory.o ../uae/events.o 
deps_blit_interrupt = ../lock.o ../uae/cia.o ../uae/memory.o ../uae/custom_mini.o ../uae/events.o  ../uae/blitter.o

deps_mem = ../micro_sys/memory.o

all: $(deps)  $(files)

%.exe:  %.c 
	gcc $(inc)  $(@:.exe=.c) $(deps) -o $@

cia.exe:  cia.c $(deps_cia)
	gcc $(inc)  $(@:.exe=.c) $(deps_cia) -o $@

cia2.exe:  cia2.c  $(deps_cia2)
	gcc $(inc)  $(@:.exe=.c) $(deps_cia2) -o $@

mem.exe: mem.c $(deps_mem)
	gcc $(inc)  $(@:.exe=.c) $(deps_mem) -o $@

events.exe: events.c $(deps_mem)
	gcc $(inc)  $(@:.exe=.c) $(deps_events) -o $@

blit_interrupt.exe: blit_interrupt.c $(deps_blit_interrupt)
	gcc $(inc)  $(@:.exe=.c) $(deps_blit_interrupt) -o $@

%.o: %.c
	gcc $(inc) -c $(@:.o=.c) -o $@

../uae/events.o: ../uae/events.c
	gcc -I../uae -I../ -I./ -D__USE_INLINE__ -D__test__ -c $(@:.o=.c) -o $@

../uae/cia.o: ../uae/cia.c
	gcc -I../uae -I../ -I./ -D__USE_INLINE__ -c $(@:.o=.c) -o $@

../uae/blitter.o: ../uae/blitter.c
	gcc -I../uae -I../ -I./ -D__USE_INLINE__ -c $(@:.o=.c) -o $@

../uae/custom_mini.o: ../uae/custom_mini.c
	gcc -I../uae -I../ -I./ -D__USE_INLINE__ -c $(@:.o=.c) -o $@

clean:
	delete #?.exe /uae/#?.o /#?.o





main_SRCS = accuracy.c \
			chipram.c \
			timeing.c 

deps = lib.o

# To compile you need to copy hardware "directory/" from NDK31 to OS4.1 SDK.

INC =   

files = $(main_SRCS:.c=.exe)  $(SRCS:.c=.o)

all:	$(deps) $(files)

%.exe: %.c
	$(CC) $(LINK) $(INC) $(deps)  $(@:.exe=.c) -o $@

clean:
	delete #?(.exe|.o)




main_SRCS = showled.c \
			CIAA_map.c \
			quit.c

deps = lib.o


INC =   

files = $(main_SRCS:.c=.exe)  $(SRCS:.c=.o)

all:	$(deps) $(files)

%.exe: %.c
	$(CC) $(LINK) $(INC) $(deps)  $(@:.exe=.c) -o $@

clean:
	delete #?(.exe|.o)

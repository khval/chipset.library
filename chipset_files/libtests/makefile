
main_SRCS = test.c

files = $(main_SRCS:.c=.exe)  $(SRCS:.c=.o)

all:	$(files)

%.exe: %.c
	$(CC) $(LINK) $(INC)  $(@:.exe=.c) -o $@

clean:
	delete #?.exe
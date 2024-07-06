.POSIX:
.SUFFIXES:

CC     = gcc
CFLAGS = -D_WIN32 -Wall -ggdb -O
LDFLAGS = -ggdb -O
LDLIBS = -lm

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<


all: apptpl

apptpl: main.o magt.o inputer.o
	$(CC) $(LDFLAGS) $^ -levent -lzmq -lwsock32 -o $@


clean:
	rm -f *.o
	rm -f apptpl



.POSIX:
.SUFFIXES:

CC     = gcc
CFLAGS = -D_WIN32 -Wall -ggdb -O -I../cchan-0.1
LDFLAGS = -ggdb -O
LDLIBS = -lm

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<


all: sndmsg

sndmsg: sndmsg.o
	$(CC) $(LDFLAGS) $^ -levent -lzmq -lwsock32 -o $@

cchan_pthread.o: ../cchan-0.1/cchan_pthread.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f sndmsg

install:


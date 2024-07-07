.POSIX:
.SUFFIXES:

CC     = gcc
CFLAGS = -D_WIN32 -Wall -ggdb -O -I../cchan-0.1 -I../c-logger-0.4.1
LDFLAGS = -ggdb -O
LDLIBS = -lm

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<


all: sndmsg

sndmsg: sndmsg.o logger.o
	$(CC) $(LDFLAGS) $^ -levent -lzmq -lwsock32 -o $@

logger.o: ../c-logger-0.4.1/logger.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f sndmsg

install:


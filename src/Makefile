.POSIX:
.SUFFIXES:

CC     = gcc
CFLAGS = -D_WIN32 -Wall -ggdb -O \
	-I../cchan-0.1 -I../c-logger-0.4.1 -I../inih-r58
LDFLAGS = -ggdb -O
LDLIBS = -lm

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<


all: apptpl

apptpl: main.o init_log.o magt.o inputer.o worker.o capture.o \
	cchan_pthread.o logger.o ini.o
	$(CC) $(LDFLAGS) $^ -levent -lzmq -lpcap -lwsock32 -o $@

cchan_pthread.o: ../cchan-0.1/cchan_pthread.c
	$(CC) $(CFLAGS) -c $<

logger.o: ../c-logger-0.4.1/logger.c
	$(CC) $(CFLAGS) -c $<

ini.o: ../inih-r58/ini.c
	$(CC) $(CFLAGS) -c $<


clean:
	rm -f *.o
	rm -f apptpl

install:


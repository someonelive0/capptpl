CFLAGS=-g -Wall

.PHONY: all
all: cchan_sdl.a cchan_pthread.a test1 test2_sdl test2_pthread

.PHONY: clean
clean:
	rm -f *.a
	rm -f *.o
	rm -f test1 test2_sdl test2_pthread

cchan_sdl.a: cchan_sdl.o
	$(AR) -r cchan_sdl.a cchan_sdl.o

cchan_pthread.a: cchan_pthread.o
	$(AR) -r cchan_pthread.a cchan_pthread.o

cchan_sdl.o: cchan_sdl.c cchan.c *.h
	$(CC) -c -o cchan_sdl.o cchan_sdl.c $(CFLAGS) `sdl-config --cflags`

cchan_pthread.o: cchan_pthread.c cchan.c *.h
	$(CC) -c -o cchan_pthread.o cchan_pthread.c $(CFLAGS) -pthread

test1: test1.c cchan_pthread.a
	$(CC) -g -o test1 test1.c cchan_pthread.a

test2_sdl: test2sdl.c cchan_sdl.a
	$(CC) -g -o test2_sdl test2sdl.c cchan_sdl.a `sdl-config --cflags` `sdl-config --libs`

test2_pthread: test2.c cchan_pthread.a
	$(CC) -g -o test2_pthread test2.c cchan_pthread.a -pthread

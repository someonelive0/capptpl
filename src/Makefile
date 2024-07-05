


all: apptpl

apptpl: main.c magt.c
	gcc -ggdb -D_WIN32  main.c magt.c -levent -lwsock32 -o $@


clean:
	rm -f apptpl *.o


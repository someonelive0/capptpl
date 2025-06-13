# capptpl

A C application template, with http management interface.

Use libevent for http service on port 3000.
A thread zmq to receive message on port 3001.
A thread worker to process message with channel between zmq thread and worker thread.
A thread capture to capture ethernet packet with libpcap.

Alse with arg parser and rotating logging.

## Code

C11 standard with -std=c11.
gcc > 4.8

Linux style code of C, and indent use space=4, use tool astyle to format codes.

```
astyle --style=linux --indent=spaces=4 --pad-header  main.c
```


## Depends

Include in my project with source code in subpath ./lib. 
- c-logger-0.4.1
- cchan-0.1
- inih-r58
- uthash-2.3.0
- cJSON-1.7.18

Depend OS env.
- gcc/clang with -std=c11
- libevent-2.1.12
- zeromq-4.3.5
- libpcap-1.10 or npcap-sdk-1.13
- hiredis-1.2.0

Attention: zeromq need libstdc++

## Build

Three subdir can make, they are apptpl, apptpl2, test

```
make
make DEBUG=1
make install
```

## Run

If not root user, first use setcap to enable CAP* ability to program.

```
setcap 'CAP_NET_RAW,CAP_NET_ADMIN,CAP_DAC_OVERRIDE+ep' ./apptpl
./apptpl
```

## Platform

Platform can be Windows Msys2/Mingw64, or Linux.

### Msys2/Mingw64

Install msys2, add C:\msys64\mingw64\bin and C:\msys64\usr\bin to env PATH.
Under mingw64, install gcc

```
pacman -Sy mingw-w64-x86_64-gcc
pacman -Sy mingw-w64-x86_64-gdb
pacman -Sy mingw-w64-x86_64-libevent
pacman -Sy mingw-w64-x86_64-zeromq
pacman -Sy mingw-w64-x86_64-hiredis
pacman -Sy make
```

### Linux

Install gcc or clang of llvm


## Analyse

Here have some tool to ayalyze c app to check memory leak, system calls except gdb.

### valgrind

```
valgrind --leak-check=full --show-leak-kinds=all ./apptpl
```

### strace

```
top -H -p $(pidof apptpl)
strace -p $(pidof apptpl)
```

### ltrace

```
ltrace -f ./apptpl
```

### source check

```
cppcheck --enable=information --force .
```

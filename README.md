# capptpl

A C application template, with http management interface.

Use libevent for http service on port 3000.
A thread zmq to receive message on port 3001.
A thread worker to process message with channel between zmq thread and worker thread.
A thread capture to capture ethernet packet with libpcap.

Alse with arg parser and rotating logging.

## Depends

Include in my project with source code in subpath ./lib. 
- c-logger-0.4.1
- cchan-0.1
- inih-r58
- uthash-2.3.0

Depend OS env.
- gcc/clang with -std=c11
- libevent-2.1.12
- zeromq-4.3.5
- libpcap-1.10 or npcap-sdk-1.13

Attention: zeromq need libstdc++

## Build

```
cd src
make
make DEBUG=1
```

## Run

```
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
pacman -Sy libevent zeromq
pacman -Sy make
```

### Linux

Install gcc or clang of llvm

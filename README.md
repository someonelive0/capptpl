# capptpl

A C application template, with http/https restful api management interface.

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
- c-logger-0.4.1      https://github.com/yksz/c-logger
- cchan-0.1           http://repo.hu/projects/cchan/
- inih-r61            https://github.com/benhoyt/inih  https://github.com/cofyc/argparse
- uthash-2.3.0        https://troydhanson.github.io/uthash/
- cJSON-1.7.18        https://github.com/DaveGamble/cJSON

Depend OS env.
- gcc/clang with -std=c11
- libevent-2.1.12
- zeromq-4.3.5
- libpcap-1.10 or npcap-sdk-1.13
- hiredis-1.2.0

Attention: zeromq need libstdc++



## SubModules

Use git submodules in path deps, something like

```
# git submodule add --depth 1 https://github.com/google/tcmalloc.git deps/tcmalloc
git submodule add --depth 1 https://github.com/microsoft/mimalloc deps/mimalloc
```



## Build

Three subdir can make, they are apptpl, apptpl2, test

```
make
make DEBUG=1
make STATIC=1 # link static
make install
```

## Run

If not root user, first use setcap to enable CAP* ability to program.

```
setcap 'CAP_IPC_LOCK,CAP_NET_RAW,CAP_NET_ADMIN,CAP_DAC_OVERRIDE+ep' ./apptpl
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



# Three sub programs

## 1. apptpl

App template of C. Use libevent+openssl to provide restful api.
And recieve msg from zeromq. And capture ethernet frames from NIC.

Usage:
```
# non root user to setcap to enable capture ability.
sudo setcap 'CAP_IPC_LOCK,CAP_NET_RAW,CAP_NET_ADMIN,CAP_DAC_OVERRIDE+ep' apptpl

# show version
./apptpl -v

# show help and usage
./apptpl -h
Usage: appctl [options] [[--] args]
   or: appctl [options]

A brief description of what the program does and how it works.

    -h, --help            show this help message and exit

Basic options
    -v, --version         show version
    -l, --list            list network devices
    -T, --test            test configuration file
    -D, --debug=<int>     set log level, TRACE=0, DEBUG=1, INFO=2, WARN=3, ERROR=4, FATAL=5, default is INFO
    -c, --config=<str>    set ini config filename

Additional description of the program after the description of the arguments.

# list NIC intraces
./apptpl -l

# test apptpl.ini config file
./apptpl -T

# run apptpl, set log level to TRACE
./apptpl -D0

# see threads of apptpl
top -H -p $(pidof apptpl)
```


## 2. apptpl2

Another App template of C. Use libevent+openssl to provide restful api.
And interactive with redis POP of list.


## 3. cpptpl

App template of C++. Use libevent+openssl to provide restful api.



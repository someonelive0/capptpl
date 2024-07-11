# Makefile for apptpl
# run 'make' to build release mode
# run 'make DEBUG=1' to build debug mode
#

.POSIX:
.SUFFIXES:

ifeq '$(findstring ;,$(PATH))' ';'
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

ifeq ($(DEBUG), 1)
$(warning compile mode is debug)
CFLAGS = -Wall -ggdb -std=c11
LDFLAGS = -ggdb -std=c11
else
$(warning compile mode is release)
CFLAGS = -W -Wall -O2 -std=c11
LDFLAGS = -O2 -std=c11
endif

#CC     = clang
CC     = gcc
CFLAGS += -I../cchan-0.1 -I../c-logger-0.4.1 -I../inih-r58 -I../lib
ifeq ($(detected_OS),MSYS)
CFLAGS += -D_WIN32 \
		-I/c/Users/tom/Downloads/npcap-sdk-1.13/Include
LDFLAGS += -L/c/Users/tom/Downloads/npcap-sdk-1.13/Lib/x64
LDLIBS = -levent -lzmq -lwsock32 -lws2_32 -lwpcap
else
CFLAGS += -D_BSD_SOURCE \
		-I/usr/local/libevent-2.1.12/include \
		-I/usr/local/zeromq-4.3.5/include \
		-I/usr/local/libpcap-1.10.4/include
#LDFLAGS += 
LDLIBS = /usr/local/libevent-2.1.12/lib/libevent.a \
	/usr/local/zeromq-4.3.5/lib/libzmq.a \
	/usr/local/libpcap-1.10.4/lib/libpcap.a \
	/usr/local/gcc-7.4.0/lib64/libstdc++.a \
	-lpthread -ldl -lunwind
endif

# CFLAGS += -fprofile-arcs -ftest-coverage
# LDFLAGS += -lgcov --coverage

# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
OBJDIR = obj
vpath %.c ../cchan-0.1 ../c-logger-0.4.1 ../inih-r58 ../lib
apptpl_srcs = main.c magt.c inputer.c worker.c capture.c \
	init_log.c load_config.c \
	cchan_pthread.c \
	logger.c \
	ini.c argparse.c
apptpl_objs = $(apptpl_srcs:%.c=$(OBJDIR)/%.o)


all: apptpl

# apptpl: main.o init_log.o magt.o inputer.o worker.o capture.o \
# 	cchan_pthread.o logger.o ini.o
apptpl: $(apptpl_objs)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(apptpl_objs): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(apptpl_objs)
	rm -f apptpl

install:


diag:
	@echo "OS is $(detected_OS)"
	@echo "apptpl_srcs: $(apptpl_srcs)"
	@echo "apptpl_objs: $(apptpl_objs)"

# .SUFFIXES: .c .o
# .c.o:
# 	$(CC) $(CFLAGS) -c $<

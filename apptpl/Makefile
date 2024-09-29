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
$(info compile mode is debug)
CFLAGS = -Wall -ggdb -std=c11
LDFLAGS = -ggdb -std=c11
else
$(info compile mode is release)
CFLAGS = -W -Wall -O2 -std=c11
LDFLAGS = -O2 -std=c11
endif

#CC     = clang
CC     = gcc
CFLAGS += -I../lib/cchan-0.1 -I../lib/c-logger-0.4.1 -I../lib/inih-r58 \
		-I../lib/uthash-2.3.0 -I../lib/cJSON-1.7.18 -I../lib/sds \
		-I../lib/aho-corasick -I../lib/cre2 -I../lib

ifeq ($(detected_OS),MSYS)
CFLAGS += -D_WIN32 \
		-I/c/Users/tom/Downloads/npcap-sdk-1.13/Include
LDFLAGS += -L/c/Users/tom/Downloads/npcap-sdk-1.13/Lib/x64
LDLIBS = -levent -levent_openssl -lzmq -lwsock32 -lws2_32 -lwpcap -lcrypto -lssl -lre2 -lstdc++
else
CFLAGS += -D_BSD_SOURCE \
		-I/usr/local/libevent-2.1.12/include \
		-I/usr/local/zeromq-4.3.5/include \
		-I/usr/local/libpcap-1.10.4/include \
		-I/usr/local/openssl-3.3.1/include
CXXFLAGS += -I/usr/local/re2-2024-07-02/include \
		-I/usr/local/abseil-cpp-20240116.1/include
LDFLAGS += -L/usr/local/re2-2024-07-02/lib
LDLIBS = /usr/local/libevent-2.1.12/lib/libevent.a \
	/usr/local/libevent-2.1.12/lib/libevent_openssl.a \
	/usr/local/zeromq-4.3.5/lib/libzmq.a \
	/usr/local/libpcap-1.10.4/lib/libpcap.a \
	/usr/local/gcc-7.4.0/lib64/libstdc++.a \
	/usr/local/openssl-3.3.1/lib64/libssl.a \
	/usr/local/openssl-3.3.1/lib64/libcrypto.a \
	-lpthread -ldl -lunwind -lz -lm -lre2
endif

# CFLAGS += -fprofile-arcs -ftest-coverage
# LDFLAGS += -lgcov --coverage

# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
OBJDIR = obj
vpath %.c ../lib/cchan-0.1 ../lib/c-logger-0.4.1 ../lib/inih-r58 \
	../lib/cJSON-1.7.18 ../lib/sds ../lib
apptpl_srcs = main.c apptpl_init.c magt.c api_handler.c \
	inputer.c worker.c capture.c parser.c pkt.c \
	word_policy.c re_policy.c \
	init_log.c load_config.c \
	cchan_pthread.c \
	logger.c \
	cJSON.c \
	sds.c \
	ini.c argparse.c hex.c
apptpl_objs = $(apptpl_srcs:%.c=$(OBJDIR)/%.o)


all: apptpl


# apptpl: main.o init_log.o magt.o inputer.o worker.o capture.o \
# 	cchan_pthread.o logger.o ini.o
apptpl: $(apptpl_objs) cre2.o
	$(CC) $(LDFLAGS) $^ ../lib/aho-corasick/libacism.a $(LDLIBS) -o $@

$(apptpl_objs): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

cre2.o: ../lib/cre2/cre2.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(apptpl_objs) cre2.o
	rm -f apptpl

install:
	mkdir -p ../bin
ifneq ($(DEBUG), 1)
ifeq ($(detected_OS),MSYS)
	strip apptpl.exe
else
	strip apptpl
endif
endif
	cp apptpl apptpl.ini.tpl ../bin


diag:
	@echo "OS is $(detected_OS)"
	@echo "apptpl_srcs: $(apptpl_srcs)"
	@echo "apptpl_objs: $(apptpl_objs)"

# .SUFFIXES: .c .o
# .c.o:
# 	$(CC) $(CFLAGS) -c $<

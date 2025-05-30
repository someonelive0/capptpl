# Makefile for cpptpl
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
CXXFLAGS = -Wall -ggdb -std=c++11
LDFLAGS = -ggdb -std=c11
else
$(info compile mode is release)
CFLAGS = -W -Wall -O2 -std=c11
CXXFLAGS = -W -Wall -O2 -std=c++11
LDFLAGS = -O2 -std=c11
endif

#CC     = clang
CC     = gcc
CXX    = g++
CFLAGS += -I../lib/cchan-0.1 -I../lib/c-logger-0.4.1 -I../lib/inih-r58 \
		-I../lib/uthash-2.3.0 -I../lib/cJSON-1.7.18 -I../lib/sds \
		-I../lib/aho-corasick -I../lib
CXXFLAGS += -I../lib/cchan-0.1 -I../lib/c-logger-0.4.1 -I../lib/inih-r58 \
		-I../lib/uthash-2.3.0 -I../lib/cJSON-1.7.18 -I../lib/sds \
		-I../lib/aho-corasick -I../lib

ifeq ($(detected_OS),MSYS)
CFLAGS += -D_WIN32 \
		-I/c/Users/tom/Downloads/npcap-sdk-1.13/Include
CXXFLAGS += -D_WIN32
LDFLAGS += -L/c/Users/tom/Downloads/npcap-sdk-1.13/Lib/x64
LDLIBS = -levent -levent_openssl -lzmq -lwsock32 -lws2_32 -lwpcap -lcrypto -lssl -lre2 -lstdc++
else
CFLAGS += -D_BSD_SOURCE -D_DEFAULT_SOURCE \
		-I/usr/local/libevent-2.1.12/include \
		-I/usr/local/zeromq-4.3.5/include \
		-I/usr/local/libpcap-1.10.4/include \
		-I/usr/local/openssl-3.3.1/include
CXXFLAGS += -I/usr/local/re2-2024-07-02/include \
		-I/usr/local/abseil-cpp-20240116.1/include \
		-I/usr/local/libevent-2.1.12/include \
		-I/usr/local/openssl-3.3.1/include
LDFLAGS += -L/usr/local/re2-2024-07-02/lib
LDLIBS = /usr/local/libevent-2.1.12/lib/libevent.a \
	/usr/local/libevent-2.1.12/lib/libevent_openssl.a \
	/usr/local/openssl-3.3.1/lib64/libssl.a \
	/usr/local/openssl-3.3.1/lib64/libcrypto.a \
	-lpthread -ldl -lunwind -lz -lm -lre2
endif

# CFLAGS += -fprofile-arcs -ftest-coverage
# LDFLAGS += -lgcov --coverage

# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
# c 和 cpp 文件分别编译成 .o 文件，分别使用不同的编译器 gcc 和 g++
OBJDIR = obj
vpath %.c ../lib/cchan-0.1 ../lib/c-logger-0.4.1 ../lib/inih-r58 \
	../lib/cJSON-1.7.18 ../lib/sds ../lib
cpptpl_srcs = main.cpp cpptpl_init.cpp magt.cpp api_handler.cpp
cpptpl_srcs_c = init_log.c load_config.c \
	cchan_pthread.c \
	logger.c \
	cJSON.c \
	sds.c \
	ini.c argparse.c hex.c
cpptpl_objs = $(cpptpl_srcs:%.cpp=$(OBJDIR)/%.o)
cpptpl_objs_c = $(cpptpl_srcs_c:%.c=$(OBJDIR)/%.o)
cpptpl_libs = $(LDLIBS)


all: cpptpl


# cpptpl: main.o init_log.o magt.o inputer.o worker.o capture.o \
# 	cchan_pthread.o logger.o ini.o
cpptpl: $(cpptpl_objs) $(cpptpl_objs_c)
	$(CXX) $(LDFLAGS) $^ $(cpptpl_libs) -o $@

$(cpptpl_objs): $(OBJDIR)/%.o : %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(cpptpl_objs_c): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(cpptpl_objs) $(cpptpl_objs_c)
	rm -f cpptpl

install:
	mkdir -p ../bin
ifneq ($(DEBUG), 1)
ifeq ($(detected_OS),MSYS)
	strip cpptpl.exe
else
	strip cpptpl
endif
endif
	cp cpptpl cpptpl.ini.tpl ../bin


diag:
	@echo "OS is $(detected_OS)"
	@echo "cpptpl_srcs: $(cpptpl_srcs)"
	@echo "cpptpl_objs: $(cpptpl_objs)"

# .SUFFIXES: .c .o
# .c.o:
# 	$(CC) $(CFLAGS) -c $<

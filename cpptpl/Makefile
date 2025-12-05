# Makefile for cpptpl
# run 'make' to build release mode
# run 'make DEBUG=1' to build debug mode
# run 'make STATIC=1' to link static mode
# run 'make DEBUG=1 STATIC=1' to build debug mode and link static mode
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
$(info detected_OS is $(detected_OS))

# make DEBUG=1
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
CFLAGS += -I../lib/cchan-0.1 -I../lib/c-logger-0.4.1 -I../lib/inih-r61 \
		-I../lib/uthash-2.3.0 -I../lib/cJSON-1.7.19 -I../lib/sds \
		-I../lib/aho-corasick -I../lib
CXXFLAGS += -I../lib/cchan-0.1 -I../lib/c-logger-0.4.1 -I../lib/inih-r61 \
		-I../lib/uthash-2.3.0 -I../lib/cJSON-1.7.19 -I../lib/sds \
		-I../lib/aho-corasick -I../lib

ifeq ($(detected_OS),MSYS)
	CFLAGS += -D_WIN32 \
			-I/c/Users/tom/Downloads/npcap-sdk-1.13/Include
	CXXFLAGS += -D_WIN32
	LDFLAGS += -L/c/Users/tom/Downloads/npcap-sdk-1.13/Lib/x64
	LDLIBS = -levent -levent_openssl -lzmq -lwsock32 -lws2_32 -lwpcap -lcrypto -lssl -lstdc++

else # detected_OS = Linux
	CFLAGS += -D_BSD_SOURCE -D_GNU_SOURCE -D_DEFAULT_SOURCE 

# make STATIC=1
ifeq ($(STATIC), 1) # static link
$(info link mode is static)
	CFLAGS += -I/usr/local/libevent-2.1.12/include 
	CXXFLAGS += -I/usr/local/libevent-2.1.12/include 
	LDFLAGS += -static \
			-L/usr/local/libevent-2.1.12/lib 
	LDLIBS = -Wl,-Bstatic -lssl -lcrypto -levent -levent_openssl \
			-Wl,-Bstatic -lstdc++ -lz \
			-Wl,-Bstatic -lpthread -ldl 

else # dynamic link
$(info link mode is dynamic)
	CFLAGS += -I/usr/local/libevent-2.1.12/include 
	CXXFLAGS += -I/usr/local/libevent-2.1.12/include 
	LDFLAGS += -Wl,-rpath=/usr/local/lib64
	LDLIBS = -Wl,-Bdynamic -lssl -lcrypto -levent -levent_openssl \
			-lstdc++ -lz \
			-lpthread -ldl

endif # endof dynamic link

endif # endof detected_OS = Linux



# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
# c 和 cpp 文件分别编译成 .o 文件，分别使用不同的编译器 gcc 和 g++
OBJDIR = obj
vpath %.c ../lib/cchan-0.1 ../lib/c-logger-0.4.1 ../lib/inih-r61 \
	../lib/cJSON-1.7.19 ../lib/sds ../lib
cpptpl_srcs = main.cpp cpptpl_init.cpp magt.cpp api_handler.cpp
cpptpl_srcs_c = init_log.c load_config.c \
	cchan_pthread.c \
	logger.c \
	cJSON.c \
	sds.c \
	ini.c argparse.c hex.c backtrace.c 
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

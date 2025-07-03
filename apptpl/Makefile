# Makefile for apptpl
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

else # detected_OS = Linux
CFLAGS += -D_BSD_SOURCE -D_GNU_SOURCE -D_DEFAULT_SOURCE  

# make STATIC=1
ifeq ($(STATIC), 1) # static link
$(info link mode is static)
CFLAGS += -I/usr/local/libevent-2.1.12/include \
		-I/usr/local/zeromq-4.3.5/include \
		-I/usr/local/re2-2019-12-01/include \
		-I/usr/local/libpcap-1.10.4/include
LDFLAGS += -static \
		-L/usr/local/libevent-2.1.12/lib \
		-L/usr/local/zeromq-4.3.5/lib \
		-L/usr/local/re2-2019-12-01/lib \
		-L/usr/local/libpcap-1.10.4/lib
LDLIBS = -Wl,-Bstatic -lssl -lcrypto -levent -levent_openssl -lzmq -lre2 -lpcap \
		-Wl,-Bstatic -lstdc++ -lpthread -ldl -lz 

else # dynamic link
$(info link mode is dynamic)
CFLAGS += 
LDFLAGS += -Wl,-rpath=/usr/local/lib64
LDLIBS = -lssl -lcrypto -levent -levent_openssl -lzmq -lre2 -lpcap \
		-lstdc++ -lpthread -ldl -lz 

endif # endof dynamic link

endif # endof detected_OS = Linux



# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
OBJDIR = obj
vpath %.c ../lib/cchan-0.1 ../lib/c-logger-0.4.1 ../lib/inih-r58 \
	../lib/cJSON-1.7.18 ../lib/sds ../lib
apptpl_srcs = main.c apptpl_init.c magt.c api_handler.c \
	inputer.c worker.c capture.c myconfig.c parser.c pkt.c \
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

# Makefile for apptpl2
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
CFLAGS += -I../lib/cchan-0.1 -I../lib/c-logger-0.4.1 -I../lib/inih-r58 -I../lib
ifeq ($(detected_OS),MSYS)
CFLAGS += -D_WIN32 
#LDFLAGS += 
LDLIBS = -levent -lhiredis -lwsock32
else
CFLAGS += -D_BSD_SOURCE \
		-I/usr/local/libevent-2.1.12/include \
		-I/usr/local/hiredis-1.2.0/include
#LDFLAGS += 
LDLIBS = /usr/local/libevent-2.1.12/lib/libevent.a \
	/usr/local/hiredis-1.2.0/lib64/libhiredis.a \
	-lpthread -ldl -lunwind
endif

# CFLAGS += -fprofile-arcs -ftest-coverage
# LDFLAGS += -lgcov --coverage

# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
OBJDIR = obj
vpath %.c ../lib/cchan-0.1 ../lib/c-logger-0.4.1 ../lib/inih-r58 ../lib
apptpl2_srcs = main.c apptpl2_init.c magt.c doredis.c \
	init_log.c load_config.c \
	cchan_pthread.c \
	logger.c \
	ini.c argparse.c
apptpl2_objs = $(apptpl2_srcs:%.c=$(OBJDIR)/%.o)


all: apptpl2


apptpl2: $(apptpl2_objs)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(apptpl2_objs): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(apptpl2_objs)
	rm -f apptpl2

install:
	mkdir -p ../bin
ifneq ($(DEBUG), 1)
ifeq ($(detected_OS),MSYS)
	strip apptpl2.exe
else
	strip apptpl2
endif
endif
	cp apptpl2 apptpl2.ini.tpl ../bin


diag:
	@echo "OS is $(detected_OS)"
	@echo "apptpl2_srcs: $(apptpl2_srcs)"
	@echo "apptpl2_objs: $(apptpl2_objs)"

# .SUFFIXES: .c .o
# .c.o:
# 	$(CC) $(CFLAGS) -c $<

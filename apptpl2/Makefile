# Makefile for apptpl2
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
CFLAGS += -I../lib/cchan-0.1 -I../lib/c-logger-0.4.1 \
		-I../lib/inih-r58 -I../lib/uthash-2.3.0 -I../lib/cJSON-1.7.18 \
		-I../lib

ifeq ($(detected_OS),MSYS)
CFLAGS += -D_WIN32 
#LDFLAGS += 
LDLIBS = -levent -levent_openssl -lhiredis -lwsock32 -lcrypto -lssl

else # detected_OS = Linux

# make STATIC=1
ifeq ($(STATIC), 1) # static link
$(info link mode is static)
CFLAGS += -D_BSD_SOURCE -D_DEFAULT_SOURCE 
LDFLAGS += -static \
		-L/usr/local/libevent-2.1.12/lib 
LDLIBS = -Wl,-Bstatic -lssl -lcrypto -levent -levent_openssl -lz \
		-Wl,-Bdynamic -lhiredis -lpthread -ldl 

else # dynamic link
$(info link mode is dynamic)
CFLAGS += -D_BSD_SOURCE -D_DEFAULT_SOURCE 
LDFLAGS += -Wl,-rpath=/usr/local/lib64
LDLIBS = -Wl,-Bdynamic -lssl -lcrypto -levent -levent_openssl \
		-Wl,-Bdynamic -lhiredis 

		#-lpthread -ldl -lunwind -lz

endif # endof dynamic link

endif # endof detected_OS = Linux



# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
OBJDIR = obj
vpath %.c ../lib/cchan-0.1 ../lib/c-logger-0.4.1 ../lib/inih-r58 ../lib/cJSON-1.7.18 ../lib
apptpl2_srcs = main.c apptpl2_init.c magt.c api_handler.c doredis.c \
	init_log.c load_config.c \
	cchan_pthread.c \
	logger.c \
	ini.c argparse.c \
	cJSON.c
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

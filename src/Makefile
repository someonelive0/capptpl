.POSIX:
.SUFFIXES:

CC     = gcc
CFLAGS = -D_WIN32 -Wall -ggdb -O \
	-I../cchan-0.1 -I../c-logger-0.4.1 -I../inih-r58 \
	-I/c/Users/tom/Downloads/npcap-sdk-1.13/Include
LDFLAGS = -ggdb -O \
	-L/c/Users/tom/Downloads/npcap-sdk-1.13/Lib/x64
LDLIBS = -lm

# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
OBJDIR = obj
vpath %.c ../cchan-0.1 ../c-logger-0.4.1 ../inih-r58
APPTPL_SRCS = main.c init_log.c magt.c inputer.c worker.c capture.c \
	cchan_pthread.c \
	logger.c \
	ini.c
APPTPL_OBJS = $(APPTPL_SRCS:%.c=$(OBJDIR)/%.o)


all: apptpl

# apptpl: main.o init_log.o magt.o inputer.o worker.o capture.o \
# 	cchan_pthread.o logger.o ini.o
apptpl: $(APPTPL_OBJS)
	$(CC) $(LDFLAGS) $^ -levent -lzmq -lwsock32 -lwpcap -o $@

$(APPTPL_OBJS): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(APPTPL_OBJS)
	rm -f apptpl

install:


# .SUFFIXES: .c .o
# .c.o:
# 	$(CC) $(CFLAGS) -c $<

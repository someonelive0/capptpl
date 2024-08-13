# Makefile for aho-corasick
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

# default use DEBUG, or make DEBUG=0 to disable it
# MSYS2 must be -O0, so MSYS should compile with DEBUG
DEBUG = 0
ifeq ($(DEBUG), 1)
$(info compile mode is debug)
CFLAGS = -Wall -ggdb -std=c11
else
$(info compile mode is release)
CFLAGS = -W -Wall -O2 -std=c11
endif

#CC     = clang
CC     = gcc
CFLAGS += -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE
ifeq ($(detected_OS),MSYS)
CFLAGS += -D_MSC_VER
else
CFLAGS += -DXXX
endif


# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
OBJDIR = obj
vpath %.c

srcs = acism.c \
	acism_create.c \
	acism_dump.c \
	acism_file.c \
	msutil.c
objs = $(srcs:%.c=$(OBJDIR)/%.o)


all: libacism.a

libacism.a:	$(objs)
	ar crs $@ $^

$(objs): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(objs) libacism.a
	rm -rf $(OBJDIR)

diag:
	@echo "OS is $(detected_OS)"
	@echo "srcs: $(srcs)"
	@echo "objs: $(objs)"


# cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o acism.o acism.c
# cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o acism_create.o acism_create.c
# cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o acism_dump.o acism_dump.c
# cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o acism_file.o acism_file.c
# cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o msutil.o msutil.c
# ar crs libacism.a acism.o acism_create.o acism_dump.o acism_file.o msutil.o

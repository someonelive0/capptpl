# Makefile for test
# run 'make' to build release mode
# run 'make DEBUG=1' to build debug mode
#

.POSIX:
.SUFFIXES:

# include depends libs
ifneq ($(wildcard ../apptpl/Makefile.deps),)
    include ../apptpl/Makefile.deps
endif

# detect OS, now support MSYS/MINGW, and linux
ifeq '$(findstring ;,$(PATH))' ';'
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif
$(info detected_OS is $(detected_OS))

# default use DEBUG, or make DEBUG=0 to disable it
DEBUG = 1
ifeq ($(DEBUG), 1)
$(info compile mode is debug)
	CFLAGS = -Wall -ggdb -std=c11
	CXXFLAGS = -Wall -ggdb
	LDFLAGS = -ggdb -std=c11 
else
$(info compile mode is release)
	CFLAGS = -W -Wall -O2 -std=c11
	CXXFLAGS = -W -Wall -O2
	LDFLAGS = -O2 -std=c11 -static 
endif

#CC     = clang
#CC     = /usr/local/musl-1.2.5/bin/musl-gcc
CC     = gcc
CFLAGS += -I../lib/cchan-0.1 -I../lib/c-logger-0.4.1 -I../lib/inih-r62 \
		-I../lib/uthash-2.3.0 -I../lib/sds -I../lib

ifeq ($(detected_OS),MSYS)
	CFLAGS += -D_WIN32 
	#LDFLAGS += 
	LDLIBS = -lzmq

else # detected_OS = Linux
	CFLAGS += -D_BSD_SOURCE -D_DEFAULT_SOURCE 
	CFLAGS += -I../deps/ck/include

	# add depends libs
	ifneq ($(LIBZMQ_INCLUDE),)
		CFLAGS += -I$(LIBZMQ_INCLUDE)
	endif
	ifneq ($(LIBZMQ_LIB),)
		LDFLAGS += -L$(LIBZMQ_LIB)
	endif

	#LDFLAGS += 
	LDLIBS = -lpthread -ldl -lzmq 
	# CFLAGS += -fprofile-arcs -ftest-coverage
	# LDFLAGS += -lgcov --coverage
endif # endof detected_OS = Linux


# obj 是输出obj目录，vpath 告之如果c不足当前文件夹，就在vpath指定的其他文件夹下找
OBJDIR = obj
vpath %.c ../lib/cchan-0.1 ../lib/c-logger-0.4.1 ../lib/inih-r62 \
	../lib/sds ../lib
sndmsg_srcs = sndmsg.c \
	logger.c \
	ini.c argparse.c
sndmsg_objs = $(sndmsg_srcs:%.c=$(OBJDIR)/%.o)

teststr_srcs = teststr.c \
	hex.c \
	sds.c
teststr_objs = $(teststr_srcs:%.c=$(OBJDIR)/%.o)

testpidstat_srcs = testpidstat.c \
	pidstat.c
testpidstat_objs = $(testpidstat_srcs:%.c=$(OBJDIR)/%.o)


all: sndmsg teststr testut testklib testpool testcre2 \
	testint testck testlockfreequeue


sndmsg: $(sndmsg_objs)
	$(CC) $(LDFLAGS) $^ \
		-L/usr/local/zeromq-4.3.5/lib \
		$(LDLIBS) -lstdc++ -o $@

$(sndmsg_objs): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@


teststr: $(teststr_objs)
	$(CC) $(LDFLAGS) $^ -o $@

$(teststr_objs): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

testut: obj/testut.o
	$(CC) $(LDFLAGS) $^ -o $@

obj/testut.o: testut.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

testklib: obj/testklib.o
	$(CC) $(LDFLAGS) $^ -o $@

obj/testklib.o: testklib.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

testpool: testpool.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

testpidstat: $(testpidstat_objs)
	$(CC) $(LDFLAGS) $^ -o $@

$(testpidstat_objs): $(OBJDIR)/%.o : %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

procstat: procstat.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

testcre2: testcre2.c ../lib/cre2/cre2.cpp
	g++ $(CXXFLAGS) -I../lib/cre2 -I/usr/local/re2-2019-12-01/include -I/usr/local/abseil-cpp-20240116.1/include ../lib/cre2/cre2.cpp -c
	$(CC) $(LDFLAGS) -I../lib/cre2 cre2.o testcre2.c -L/usr/local/re2-2019-12-01/lib -lre2 -lstdc++ -lpthread -o testcre2

testint: testint.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

testck: testck.c
	$(CC) $(CFLAGS) $(LDFLAGS) -lpthread $^ -o $@

testlockfreequeue: testlockfreequeue.c ../lib/lfqueue-1.2.2/lfqueue.c
	$(CC) $(CFLAGS) -I../lib/lfqueue-1.2.2 $(LDFLAGS) -lpthread $^ -o $@


clean:
	rm -f $(sndmsg_objs) $(teststr_objs) obj/testut.o obj/testklib.o \
		$(testpidstat_objs)
	rm -f sndmsg teststr testut testklib testpool testpidstat procstat
	rm -f testcre2 cre2.o testint testck testlockfreequeue


install:
	mkdir -p ../bin
ifneq ($(DEBUG), 1)
ifeq ($(detected_OS),MSYS)
	strip sndmsg.exe
else
	strip sndmsg
endif
endif
	cp sndmsg sndmsg.ini.tpl ../bin


diag:
	@echo "OS is $(detected_OS)"
	@echo "sndmsg_srcs: $(sndmsg_srcs)"
	@echo "sndmsg_objs: $(sndmsg_objs)"
	@echo "teststr_srcs: $(teststr_srcs)"

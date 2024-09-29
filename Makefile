# Makefile to call subdir Makefile
# make [all] [DEBUG=1]
# make clean
# make install
#

all:

%:
	make -C apptpl $@
	make -C apptpl2 $@
	make -C cpptpl $@
	make -C test $@

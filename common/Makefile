# Common code for all fem modules (except "fem" itself ;-)

CC=gcc
#CC=xlc_r -q64

ifeq ($(SYS_TYPE),IRIX)
CC=/usr/nekoware/gcc-4.7/bin/gcc
else
CC=gcc
endif


INCLUDES=-I../fem
DEBUG=-g -ansi -pedantic -Wall -O0
DEBUG=-ansi -pedantic -Wall -O2 -DRUN_VERBOSE
#DEBUG=
#CFLAGS=$(INCLUDES) -Wall -pedantic -ansi -DRUN_VERBOSE $(DEBUG)
CFLAGS=$(INCLUDES) -DRUN_VERBOSE $(DEBUG)
LIBS=
AR=ar r
#AR=ar -X64 r
RANLIB=-ranlib

# Win32 crosscompiler:
#CC=i386-mingw32-gcc -DWIN32
#AR=i386-mingw32-ar r
#RANLIB=-i386-mingw32-ranlib

all: libfem_comm.a

libfem_comm.a: fem_comm.o
	$(AR) $(@) fem_comm.o
	$(RANLIB) $(@)

fem_comm.o: fem_comm.c fem_comm.h ../fem/fem.h
	$(CC) -c $(CFLAGS) fem_comm.c

clean: 
	rm -f *.o *.a

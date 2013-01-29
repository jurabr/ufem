# Very very very simple command interpreter

CC=gcc
#CC=xlc_r -q64
ifeq ($(SYS_TYPE),IRIX)
CC=/usr/nekoware/gcc-4.7/bin/gcc
else
CC=gcc
endif

DEBUG=-Wall -ansi -pedantic -g -O0
DEBUG=-Wall -ansi -pedantic -O2 -DRUN_VERBOSE
#DEBUG=

FEATFLAGS=
FEATFLAGS=-I../meval -D_USE_LIB_EVAL_

FEATLIBS=
FEATLIBS=-L../meval -lmeval

INCLUDES=-I../common -I../fem
CFLAGS=$(INCLUDES) $(DEBUG) $(FEATFLAGS)
LIBS=$(FEATLIBS) -L../common -lfem_comm
AR=ar r
#AR=ar -X64 r
RANLIB=-ranlib

# Win32 crosscompiler:
#CC=i386-mingw32-gcc -DWIN32
#AR=i386-mingw32-ar r
#RANLIB=-i386-mingw32-ranlib

all: libcint.a

libcint.a: cint.o civar.o cscript.o clh.o ../common/libfem_comm.a
	$(AR) $(@) cint.o civar.o cscript.o clh.o
	$(RANLIB) $(@)

libfem_comm.a:
	(cd ../common ; make all)

test: cint.o civar.o cscript.o test.o
	$(CC) -o $(@) cint.o civar.o test.o $(CFLAGS) $(LIBS)

test.o: test.c cint.h
	$(CC) -c test.c $(CFLAGS)

cint.o: cint.c cint.h
	$(CC) -c cint.c $(CFLAGS)

civar.o: civar.c cint.h
	$(CC) -c civar.c $(CFLAGS)

cscript.o: cscript.c cint.h
	$(CC) -c cscript.c $(CFLAGS)

clh.o: clh.c cint.h
	$(CC) -c clh.c $(CFLAGS)


clean:
	rm -f *.o *.a test

all: libmeval.a m_test

CC=gcc
ifeq ($(SYS_TYPE),IRIX)
CC=/usr/nekoware/gcc-4.7/bin/gcc
else
CC=gcc
endif

AR=ar -r
RANLIB=-ranlib

# Win32 crosscompiler:
#CC=i386-mingw32-gcc -DWIN32
#AR=i386-mingw32-ar r
#RANLIB=-i386-mingw32-ranlib

DEBUG=-Wall -ansi -pedantic -g -O0
DEBUG=-Wall -ansi -pedantic -O2
CFLAGS=$(DEBUG) -I../fem -I../cint -I../common
LIBS=-L../cint  -L../common -lcint -lfem_comm -lm

OBJECTS=m_eval.o

m_test: $(OBJECTS) m_test.o  libmeval.a
	$(CC) $(CFLAGS) -o $(@) $(OBJECTS) m_test.o libmeval.a $(LIBS)

libmeval.a: $(OBJECTS)
	$(AR) $(@) $(OBJECTS)
	$(RANLIB) $(@)

m_eval.o: m_eval.h m_eval.c
	$(CC) $(CFLAGS) -c m_eval.c

m_test.o: m_eval.h m_test.c
	$(CC) $(CFLAGS) -c m_test.c


clean:
	-rm -f *.o m_test libmeval.a core

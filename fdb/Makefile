# Makefile for FEM database library
#
# (C) 2003 Jiri Brozovsky
#

#
# Default settings
#
#CC=tcc
#DEBUG=-g -DDEVEL_VERBOSE
#
CC=gcc
#CC=xlc_r -q64
CC=/usr/nekoware/gcc-4.3/bin/gcc

#DEBUG=-DDEVEL_VERBOSE
DEBUG=-Wall -ansi -pedantic -g -DDEVEL_VERBOSE
#DEBUG=-g -Wall -ansi -pedantic -DDEVEL_VERBOSE -O0
DEBUG=-Wall -ansi -pedantic -DRUN_VERBOSE -O2
#
INCLUDES=-I../fem -I../common -I../cint
CFLAGS=$(INCLUDES) $(DEBUG)
CFLAGS=$(INCLUDES) $(DEBUG) -D_USE_THREADS_ # threaded version
LIBS=-L../common -lfem_comm -lm
#LIBS=-L../common -lfem_comm -pthread -lm
AR=ar r
#AR=ar -X64 r
RANLIB=-ranlib
#

# Win32 crosscompiler:
#CC=i386-mingw32-gcc -DWIN32
#AR=i386-mingw32-ar r
#RANLIB=-i386-mingw32-ranlib


#
# Enable this for tcc:
#CC=tcc
#CFLAGS=$(INCLUDES)
#LIBS=
#

#
# Enable this for Minix:
#CC=cc -m -i -w
#CFLAGS=$(INCLUDES)
#LIBS=
#

OBJECTS=fdb.o fdb_io.o fdb_expt.o fdb_impt.o fdb_eans.o fdb_mem.o fdb_math.o fem_eqs.o fem_math.o fem_mem.o fdb_accs.o fdb_sele.o fdb_tdef.o fdb_deps.o fdb_n.o fdb_k.o fdb_fem.o fdb_stat.o fdb_et.o fdb_edef.o fdb_mdef.o fdb_mp.o fdb_rs.o fdb_step.o fdb_e.o fdb_fdef.o fdb_disp.o fdb_nld.o fdb_eld.o fdb_gld.o fdb_nops.o fdb_rops.o fdb_res.o fdb_cres.o fdb_rmc.o fdb_prn.o fdb_csys.o fdb_geom.o fdb_ent.o fdb_mesh.o fdb_kops.o fdb_meas.o fdb_test.o fdb_get.o fdb_rand.o fdb_grid.o fdb_mod.o fdb_exel.o

all: libfdb.a


# testing file - use "make test" to make it:
test: test.o $(OBJECTS) ../common/libfem_comm.a
	$(CC) -o$(@) $(OBJECTS) test.o $(CFLAGS) $(LIBS) 

libfem_comm.a:
	(cd ../common ; make all)

libfdb.a: $(OBJECTS)
	$(AR) $(@) $(OBJECTS)
	$(RANLIB) $(@)

test.o: test.c fdb.h
	$(CC) -c test.c $(CFLAGS)

fdb.o: fdb.c fdb.h
	$(CC) -c fdb.c $(CFLAGS)

fdb_io.o: fdb_io.c fdb.h
	$(CC) -c fdb_io.c $(CFLAGS)

fdb_expt.o: fdb_expt.c fdb.h
	$(CC) -c fdb_expt.c $(CFLAGS)

fdb_exel.o: fdb_exel.c fdb.h
	$(CC) -c fdb_exel.c $(CFLAGS)

fdb_impt.o: fdb_impt.c fdb.h
	$(CC) -c fdb_impt.c $(CFLAGS)

fdb_eans.o: fdb_eans.c fdb.h
	$(CC) -c fdb_eans.c $(CFLAGS)

fdb_mem.o: fdb_mem.c fdb.h
	$(CC) -c fdb_mem.c $(CFLAGS)

fdb_math.o: fdb_math.c fdb.h ../fem/fem_math.h
	$(CC) -c fdb_math.c $(CFLAGS)

fem_mem.o: ../fem/fem_mem.c ../fem/fem_mem.h ../fem/fem.h
	$(CC) -c ../fem/fem_mem.c $(CFLAGS)

fem_math.o: ../fem/fem_math.c ../fem/fem_math.h ../fem/fem_mem.h ../fem/fem.h
	$(CC) -c ../fem/fem_math.c $(CFLAGS)

fem_eqs.o: ../fem/fem_eqs.c ../fem/fem_math.h ../fem/fem_mem.h ../fem/fem.h
	$(CC) -c ../fem/fem_eqs.c $(CFLAGS)

fdb_accs.o: fdb_accs.c fdb.h
	$(CC) -c fdb_accs.c $(CFLAGS)

fdb_sele.o: fdb_sele.c fdb.h fdb_res.h fdb_fem.h
	$(CC) -c fdb_sele.c $(CFLAGS)

fdb_tdef.o: fdb_tdef.c fdb.h fdb_data.h
	$(CC) -c fdb_tdef.c $(CFLAGS)

fdb_deps.o: fdb_deps.c fdb.h fdb_fem.h
	$(CC) -c fdb_deps.c $(CFLAGS)


fdb_fem.o: fdb_fem.c fdb.h fdb_fem.h
	$(CC) -c fdb_fem.c $(CFLAGS)

fdb_stat.o: fdb_stat.c fdb.h fdb_fem.h
	$(CC) -c fdb_stat.c $(CFLAGS)

fdb_edef.o: fdb_edef.c fdb_edef.h fdb.h fdb_fem.h ../fem/fem_elem.h
	$(CC) -c fdb_edef.c $(CFLAGS)

fdb_mdef.o: fdb_mdef.c fdb_mdef.h fdb.h fdb_fem.h ../fem/fem_mat.h
	$(CC) -c fdb_mdef.c $(CFLAGS)

fdb_n.o: fdb_n.c fdb.h fdb_fem.h
	$(CC) -c fdb_n.c $(CFLAGS)

fdb_k.o: fdb_k.c fdb.h fdb_fem.h
	$(CC) -c fdb_k.c $(CFLAGS)

fdb_et.o: fdb_et.c fdb.h fdb_fem.h fdb_edef.o
	$(CC) -c fdb_et.c $(CFLAGS)

fdb_mp.o: fdb_mp.c fdb.h fdb_fem.h fdb_mdef.o
	$(CC) -c fdb_mp.c $(CFLAGS)

fdb_rs.o: fdb_rs.c fdb.h fdb_fem.h fdb_mdef.o
	$(CC) -c fdb_rs.c $(CFLAGS)

fdb_e.o: fdb_e.c fdb.h fdb_fem.h fdb_edef.o fdb_mdef.o fdb_edef.o
	$(CC) -c fdb_e.c $(CFLAGS)

fdb_fdef.o: fdb_fdef.c fdb.h fdb_fem.h fdb_edef.h
	$(CC) -c fdb_fdef.c $(CFLAGS)

fdb_disp.o: fdb_disp.c fdb.h fdb_fem.h
	$(CC) -c fdb_disp.c $(CFLAGS)

fdb_nld.o: fdb_nld.c fdb.h fdb_fem.h
	$(CC) -c fdb_nld.c $(CFLAGS)

fdb_eld.o: fdb_eld.c fdb.h fdb_fem.h fdb_edef.o
	$(CC) -c fdb_eld.c $(CFLAGS)

fdb_gld.o: fdb_gld.c fdb.h fdb_fem.h fdb_edef.o
	$(CC) -c fdb_gld.c $(CFLAGS)

fdb_nops.o: fdb_nops.c fdb.h fdb_fem.h
	$(CC) -c fdb_nops.c $(CFLAGS)

fdb_res.o: fdb_res.c fdb.h fdb_fem.h fdb_res.h
	$(CC) -c fdb_res.c $(CFLAGS)

fdb_rops.o: fdb_rops.c fdb.h fdb_fem.h fdb_res.h
	$(CC) -c fdb_rops.c $(CFLAGS)

fdb_cres.o: fdb_cres.c fdb.h fdb_fem.h fdb_res.h
	$(CC) -c fdb_cres.c $(CFLAGS)

fdb_rmc.o: fdb_rmc.c fdb.h fdb_fem.h fdb_res.h
	$(CC) -c fdb_rmc.c $(CFLAGS)

fdb_prn.o: fdb_prn.c fdb.h 
	$(CC) -c fdb_prn.c $(CFLAGS)

fdb_csys.o: fdb_csys.c fdb.h 
	$(CC) -c fdb_csys.c $(CFLAGS)

fdb_geom.o: fdb_geom.c fdb_geom.h fdb.h 
	$(CC) -c fdb_geom.c $(CFLAGS)

fdb_ent.o: fdb_ent.c fdb_geom.h fdb.h 
	$(CC) -c fdb_ent.c $(CFLAGS)

fdb_mesh.o: fdb_mesh.c fdb_geom.h fdb.h 
	$(CC) -c fdb_mesh.c $(CFLAGS)

fdb_grid.o: fdb_grid.c fdb_fem.h fdb.h 
	$(CC) -c fdb_grid.c $(CFLAGS)

fdb_mod.o: fdb_mod.c fdb_fem.h fdb.h 
	$(CC) -c fdb_mod.c $(CFLAGS)

fdb_kops.o: fdb_kops.c fdb_geom.h fdb_fem.h fdb.h 
	$(CC) -c fdb_kops.c $(CFLAGS)

fdb_meas.o: fdb_meas.c fdb_fem.h fdb.h 
	$(CC) -c fdb_meas.c $(CFLAGS)

fdb_step.o: fdb_step.c fdb.h fdb_fem.h fdb_eld.o fdb_nld.o fdb_disp.o
	$(CC) -c fdb_step.c $(CFLAGS)

fdb_get.o: fdb_get.c fdb.h fdb_fem.h fdb_fem.o
	$(CC) -c fdb_get.c $(CFLAGS)

fdb_rand.o: fdb_rand.c fdb.h fdb_fem.h fdb_fem.o
	$(CC) -c fdb_rand.c $(CFLAGS)


clean:
	rm -f *.o *.a test

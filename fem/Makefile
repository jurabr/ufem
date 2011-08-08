#  File name: Makefile
#  Date:      2003/04/07 09:20
#  Author:    Jiri Brozovsky
#
#  Copyright (C) 2003 Jiri Brozovsky
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  in a file called COPYING along with this program; if not, write to
#  the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
#  02139, USA.
#
# FEM Solver - Makefile for unix make
#
# $Id: Makefile,v 1.88 2005/07/11 17:56:16 jirka Exp $

#####################################################

#
# normal people can use these variables:
#
CC=gcc #-pg
#CC=gcc -pg # for GNU profiller
#CC=xlc_r -q64
#CC=cc -mips4 -n32 -O2
#CC=cc -mips4 -n32 -Ofast=ip26 -r8000 -static
#CC=/usr/nekoware/gcc-4.3/bin/gcc

ifeq ($(SYS_TYPE),IRIX)
CC=/usr/nekoware/gcc-4.3/bin/gcc
else
CC=gcc
endif



AR= ar r
RANLIB=-ranlib
LD=ld -shared


# Win32 crosscompiler:
#CC=i386-mingw32-gcc
#AR=i386-mingw32-ar r
#RANLIB=-i386-mingw32-ranlib

# Win32 Debina/Ubuntu main crosscompiler:
#CC=i586-mingw32msvc-gcc
#AR=i586-mingw32msvc-ar r
#RANLIB=-i586-mingw32msvc-ranlib

# ARM:
#CC=arm-linux-gcc
#AR=arm-linux-ar r
#RANLIB=-arm-linux-ranlib

#DEBUG= -O2 -DRUN_VERBOSE -DPSEUDO_SEC #-DDEVEL_VERBOSE #-DDEVEL
#DEBUG= -O1 -DRUN_VERBOSE -DPSEUDO_SEC -D_MATRIX_SAVING_ #-DDEVEL_VERBOSE #-DDEVEL
#DEBUG=-g -O0 -DRUN_VERBOSE -DDEVEL_VERBOSE -DDEVEL

#DEBUG=-g -O0 -DRUN_VERBOSE -DPSEUDO_SEC -D_MATRIX_SAVING_ -DDEVEL_VERBOSE -DDEVEL
#DEBUG=-O2 -DRUN_VERBOSE -D_MATRIX_SAVING_ -DDEVEL_VERBOSE -DDEVEL
#DEBUG=-g -DRUN_VERBOSE -DDEVEL_VERBOSE -DDEVEL #-DPSEUDO_SEC -D_MATRIX_SAVING_ 
#DEBUG=-g -O0 -DRUN_VERBOSE -D_MATRIX_SAVING_ 

#DEBUG=-g -O0 -DRUN_VERBOSE -DDEVEL_VERBOSE -DDEVEL #-D_MATRIX_SAVING_ #-DPSEUDO_SEC 
#DEBUG=-g -O0 -DRUN_VERBOSE -DDEVEL_VERBOSE -DDEVEL -D_MATRIX_SAVING_ #-DPSEUDO_SEC 
#DEBUG=-g -O0 -DRUN_VERBOSE -DDEVEL_VERBOSE -DDEVEL -D_MATRIX_SAVING_ #-DPSEUDO_SEC 
DEBUG=-g -O0 -DRUN_VERBOSE -DDEVEL -DDEVEL_VERBOSE -D_MATRIX_SAVING_
#DEBUG=-O2 -DRUN_VERBOSE -DDEVEL -DDEVEL_VERBOSE 
##DEBUG=-O2 -DRUN_VERBOSE -D_MATRIX_SAVING_ -D_USE_SIGNALS_ -DUSE_FAST_SAPO
DEBUG=-O3 -DRUN_VERBOSE -D_MATRIX_SAVING_ -D_USE_SIGNALS_ -DUSE_FAST_SAPO
#DEBUG=-g -O0 -DDEVEL -DDEVEL_VERBOSE -DUSE_FAST_SAPO -D_MATRIX_SAVING_ 
#DEBUG=-DRUN_VERBOSE -D_USE_SIGNALS_ -DUSE_FAST_SAPO 

# newer gcc only:
#DEBUG=-O2 -DRUN_VERBOSE -fprofile-generate
#DEBUG=-O2 -DRUN_VERBOSE -fprofile-use
#DEBUG=-O2 -DRUN_VERBOSE

FEATFLAGS=

#CFLAGS=$(FEATFLAGS) -Wall -ansi -pedantic -DUSE_NLMC $(DEBUG) -fPIC
CFLAGS=$(FEATFLAGS) -Wall -ansi -pedantic $(DEBUG) -fPIC
#CFLAGS=$(FEATFLAGS) -Wall -ansi -pedantic $(DEBUG) -D_USE_THREADS_ -DUSE_NLMC
#CFLAGS=$(FEATFLAGS) $(DEBUG) -D_USE_THREADS_ -DUSE_NLMC
#CFLAGS=$(FEATFLAGS) $(DEBUG) -Wall -ansi -pedantic -D_USE_THREADS_ -fPIC
#CFLAGS=$(FEATFLAGS) $(DEBUG) 

# traditional unix:
#CFLAGS=$(DEBUG)  -DUSE_NLMC 
#DEBUG=-O -DRUN_VERBOSE -DDEVEL

FEATLIBS=

LIBS=$(FEATLIBS) -L/usr/local/lib -lm #-lefence
#LIBS=$(FEATLIBS) -L/usr/local/lib -lm -lpthread #-lefence


PREFIX=/usr/local
INSTALL=install

MVIZLIBS=-L/usr/lib32 -L/usr/freeware/lib32 -lglut -lGL -lGLU -lX11 -lXmu -lm

#####################################################


# 
# Special settings for different purposes:
#


# enable this if you want to use "tcc" compiler:
#
#CC=tcc
#DEBUG= -g
#CFLAGS=$(DEBUG) -DNRM_SPDP -DRUN_VERBOSE -DDEVEL_VERBOSE -DDEVEL #-DPSEUDO_SEC #-g
#LIBS=-lm #-lefence
#
#end of "tcc"


# enable this if you want to use "lcc" compiler:
#
#CC=lcc
#CFLAGS= -DNRM_SPDP -DRUN_VERBOSE -DPSEUDO_SEC -D_MATRIX_SAVING_ -DDEVEL_VERBOSE -DDEVEL #-g
#LIBS=-lm #-lefence
#
#end of "lcc"


# enable this if you want MPI:
#
#CC=mpicc -pedantic -ansi 
#CFLAGS=-O0 -I/usr/include/lam -Wall -DNRM_SPDP -DUSE_MPI -DRUN_VERBOSE #-DDEVEL -DDEVEL_VERBOSE #-DBLOCK_FOR_MPI -DUSE_MPI_LOG #-DPSEUDO_SEC 
#LIBS=-lm -lmpi #-lefence #-lmpich
#
#end of "mpi"


# enable this if you want MPI (home/laptop setup with LAM-MPI):
#
#CC=/opt/lam/bin/mpicc -pedantic -ansi 
#CFLAGS=-O3 -I/opt/lam/inclide -Wall -DNRM_SPDP -DUSE_MPI -DRUN_VERBOSE #-DDEVEL -DDEVEL_VERBOSE #-DBLOCK_FOR_MPI -DUSE_MPI_LOG #-DPSEUDO_SEC 
#LIBS=-lm -lmpi #-lefence #-lmpich
#
#end of "lam-mpi"


# enable this on Minix:
#
#CC=cc -m -i -w
#CFLAGS= -DRUN_VERBOSE -DPSEUDO_SEC -DDEVEL_VERBOSE -D_SMALL_FEM_CODE_
#LIBS=
#
# end of "minix"

# enable this if you want to crosscompilation for "m$"
# works on my box _only_
#
#CC=/opt/cross/bin/ii386-mingw32msvc-gcc
#CFLAGS=-g -DRUN_VERBOSE -DPSEUDO_SEC -DDEVEL_VERBOSE -DDEVEL
#LIBS=-lm
#
#end of "m$"


#####################################################

OBJECTS=fem_sol.o fem_nrm.o fem_alm.o fem_eige.o fem_loads.o fem_mloc.o fem_io.o fem_spio.o fem_data.o fem_dama.o fem_accs.o fem_mem.o fem_math.o fem_eqs.o fem_elem.o fem_eini.o fem_e000.o fem_e001.o fem_e002.o fem_e003.o fem_e004.o fem_e005.o fem_e006.o fem_e007.o fem_e008.o fem_e009.o fem_e010.o fem_e011.o fem_e012.o fem_e013.o fem_e014.o fem_e015.o fem_e016.o fem_e017.o fem_e018.o fem_e019.o fem_e020.o fem_mat.o fem_mini.o fem_m000.o fem_m001.o fem_m002.o fem_m003.o fem_m004.o fem_m005.o fem_m006.o fem_m007.o fem_m008.o fem_m009.o fem_m010.o fem_sbet.o fem_sber.o fem_nlmc.o fem_mpi.o fem_paux.o fem_meqs.o fem_stf2.o fem_ccr2.o fem_chen.o fem_ch2d.o fem_bone.o fem_dp.o fem_vmis.o fem_pl3d.o fem_ccr3.o fem_para.o fem_spnt.o fem_pr3d.o fem_pl1d.o fem_sb1d.o fem_od3d.o fem_mrun.o fem_asse.o fem_pric.o


# targets:
all: fem

# main()
fem: fem.o $(OBJECTS) fem_mont_no.o fem_newm.o $(FEATLIBS)
	$(CC) -o $(@) $(CFLAGS) fem.o $(OBJECTS) fem_mont_no.o fem_newm.o $(LIBS)  

# library for monte():
libfem.so: fem_mont.o fem_newmmont.o $(OBJECTS)
	$(LD) fem_mont.o fem_newmmont.o $(OBJECTS) -o $(@)


# solver code ;-)
fem_sol.o: fem_sol.c fem_sol.h fem.h fem_data.o
	$(CC) -c $(CFLAGS) fem_sol.c

fem_nrm.o: fem_nrm.c fem_sol.h fem.h fem_data.o
	$(CC) -c $(CFLAGS) fem_nrm.c

fem_eige.o: fem_eige.c fem_sol.h fem.h fem_data.o
	$(CC) -c $(CFLAGS) fem_eige.c

fem_newm.o: fem_newm.c fem_sol.h fem.h fem_data.o
	$(CC) -c $(CFLAGS) fem_newm.c

fem_alm.o: fem_alm.c fem_sol.h fem.h fem_data.o
	$(CC) -c $(CFLAGS) fem_alm.c

fem_loads.o: fem_loads.c fem_sol.h fem.h fem_data.o
	$(CC) -c $(CFLAGS) fem_loads.c

fem_mloc.o: fem_mloc.c fem_sol.h fem.h fem_data.o
	$(CC) -c $(CFLAGS) fem_mloc.c

# I/O routines	
fem_io.o: fem_io.c fem.h fem_data.o
	$(CC) -c $(CFLAGS) fem_io.c

# specialI/O routines	
fem_spio.o: fem_spio.c fem.h fem_data.o
	$(CC) -c $(CFLAGS) fem_spio.c

# data access (from program)
fem_accs.o: fem_accs.c fem.h fem_dama.h fem_data.o
	$(CC) -c $(CFLAGS) fem_accs.c

# data definitions
fem_data.o: fem_data.c fem_data.h fem.h 
	$(CC) -c $(CFLAGS) fem_data.c

# data manipulations (e.g. access)
fem_dama.o: fem_dama.c fem_dama.h fem_data.h fem.h 
	$(CC) -c $(CFLAGS) fem_dama.c

# memory manipulations (alloc., free)
fem_mem.o: fem_mem.c fem.h 
	$(CC) -c $(CFLAGS) fem_mem.c

# command line parameters
fem_para.o: fem_para.c fem.h fem_para.h
	$(CC) -c $(CFLAGS) fem_para.c

# control data saving
fem_spnt.o: fem_spnt.c fem.h fem_spnt.h
	$(CC) -c $(CFLAGS) fem_spnt.c


# element library
fem_elem.o: fem_elem.c fem_elem.h fem.h fem_eres.h
	$(CC) -c $(CFLAGS) fem_elem.c

fem_eini.o: fem_eini.c fem_elem.h fem.h
	$(CC) -c $(CFLAGS) fem_eini.c

fem_e000.o: fem_e000.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e000.c

fem_e001.o: fem_e001.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e001.c

fem_e002.o: fem_e002.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e002.c

fem_e003.o: fem_e003.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e003.c

fem_e004.o: fem_e004.c fem_elem.h fem_pl3d.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e004.c

fem_e005.o: fem_e005.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e005.c

fem_e006.o: fem_e006.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e006.c

fem_e007.o: fem_e007.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e007.c

fem_e008.o: fem_e008.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e008.c

fem_e009.o: fem_e009.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e009.c

fem_e010.o: fem_e010.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e010.c

fem_e011.o: fem_e011.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e011.c

fem_e012.o: fem_e012.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e012.c

fem_e013.o: fem_e013.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e013.c

fem_e014.o: fem_e014.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e014.c

fem_e015.o: fem_e015.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e015.c

fem_e016.o: fem_e016.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e016.c

fem_e017.o: fem_e017.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e017.c

fem_e018.o: fem_e018.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e018.c

fem_e019.o: fem_e019.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e019.c

fem_e020.o: fem_e020.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_e020.c

fem_bint.o: fem_bint.c fem_elem.h fem.h fem_elem.o
	$(CC) -c $(CFLAGS) fem_bint.c


# material library
fem_mat.o: fem_mat.c fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_mat.c

fem_mini.o: fem_mini.c fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_mini.c

fem_m000.o: fem_m000.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m000.c

fem_m001.o: fem_m001.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m001.c

fem_m002.o: fem_m002.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m002.c

fem_m003.o: fem_m003.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m003.c

fem_m004.o: fem_m004.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m004.c

fem_m005.o: fem_m005.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m005.c

fem_m006.o: fem_m006.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m006.c

fem_m007.o: fem_m007.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m007.c

fem_m008.o: fem_m008.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m008.c

fem_m009.o: fem_m009.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m009.c

fem_m010.o: fem_m010.c fem_mat.h fem.h fem_mat.o
	$(CC) -c $(CFLAGS) fem_m010.c


# special (more universal) functions for elements:	
fem_stf2.o: fem_stf2.c fem_elem.h fem_mat.h
	$(CC) -c $(CFLAGS) fem_stf2.c

fem_ccr2.o: fem_ccr2.c fem_elem.h fem_mat.h
	$(CC) -c $(CFLAGS) fem_ccr2.c

fem_sbet.o: fem_sbet.c fem_elem.h fem_mat.h
	$(CC) -c $(CFLAGS) fem_sbet.c

fem_sber.o: fem_sber.c fem_elem.h fem_mat.h
	$(CC) -c $(CFLAGS) fem_sber.c

fem_sb1d.o: fem_sb1d.c fem_elem.h fem_mat.h
	$(CC) -c $(CFLAGS) fem_sb1d.c

fem_nlmc.o: fem_nlmc.c fem_elem.h fem_mat.h
	$(CC) -c $(CFLAGS) fem_nlmc.c

fem_pl1d.o: fem_pl1d.c fem_pl1d.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_pl1d.c

fem_pl3d.o: fem_pl3d.c fem_pl3d.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_pl3d.c

fem_od3d.o: fem_od3d.c fem_pl3d.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_od3d.c

fem_ccr3.o: fem_ccr3.c fem_pl3d.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_ccr3.c

fem_chen.o: fem_chen.c fem_pl3d.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_chen.c

fem_ch2d.o: fem_ch2d.c fem_elem.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_ch2d.c

fem_ch2m.o: fem_ch2m.c fem_ch2d.o fem_elem.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_ch2m.c

fem_dp.o: fem_dp.c fem_pl3d.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_dp.c

fem_vmis.o: fem_vmis.c fem_pl3d.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_vmis.c

fem_bone.o: fem_bone.c fem_pl3d.h fem_mat.h fem.h
	$(CC) -c $(CFLAGS) fem_bone.c

fem_mrun.o: fem_mrun.c fem_elem.h fem_mat.h
	$(CC) -c $(CFLAGS) fem_mrun.c



# matrix library (including equation solvers)
fem_math.o: fem_math.c fem_math.h fem.h fem_mem.o
	$(CC) -c $(CFLAGS) fem_math.c

fem_eqs.o: fem_eqs.c fem_math.h fem.h fem_mem.o
	$(CC) -c $(CFLAGS) fem_eqs.c

# message passing interface
fem_mpi.o: fem_mpi.c fem_mpi.h fem.h fem_mem.o
	$(CC) -c $(CFLAGS) fem_mpi.c

fem_paux.o: fem_paux.c fem_mpi.h fem.h fem_mem.o
	$(CC) -c $(CFLAGS) fem_paux.c

fem_meqs.o: fem_meqs.c fem_mpi.h fem.h fem_mem.o
	$(CC) -c $(CFLAGS) fem_meqs.c


# volume/price computation:
fem_pric.o: fem_pric.c fem.h fem_mont.h fem_math.h fem_mem.o
	$(CC) -c $(CFLAGS) fem_pric.c


# assesment routines:
fem_asse.o: fem_asse.c fem.h fem_mont.h fem_math.h fem_mem.o
	$(CC) -c $(CFLAGS) fem_asse.c

#
# principal stresses in 3D:
fem_pr3d.o: fem_pr3d.c fem_math.h fem.h fem_mem.o
	$(CC) -c $(CFLAGS) fem_pr3d.c

# even more random stuff (Monte cooperation):
fem_mont.o: fem_mont.c fem_mont.h fem.h fem_mem.o
	$(CC) -c $(CFLAGS) -DUSE_MONTE fem_mont.c

# Newmark for Monte cooperation:
fem_newmmont.o: fem_newm.c fem_sol.h fem.h fem_data.o
	$(CC) -c $(CFLAGS) -DUSE_MONTE fem_newm.c -o $(@)

# monte without monte support:
fem_mont_no.o: fem_mont.c fem.h fem_mem.o
	$(CC) -c $(CFLAGS) fem_mont.c -o $(@)


# testing data generator:
gen: gen.c
	$(CC) -o $(@) gen.c $(CFLAGS) $(LIBS)

# matrix vizualizer:
mviz: math_mviz.c fem_math.o fem_mem.o
	$(CC) $(CFLAGS) math_mviz.c fem_math.o fem_mem.o -o $(@) $(MVIZLIBS)

# documentation:
doc: fem.dvi fem.ps

fem.dvi: fem.1
	groff -man -Tdvi fem.1 >fem.dvi

fem.ps: fem.1
	groff -man -Tps fem.1 >fem.ps

# cleaning:
clean:
	-rm -f fem gen dul *.o *.a *.gcda *.gcno *.exe core results.txt fem.dvi fem.ps fem.pdf feminput.dvi feminput.ps feminput.pdf libfem.so mviz so_locations xkb.so xkb.c

# installation:
install: fem
	$(INSTALL) -d $(PREFIX)/bin
	$(INSTALL) -d $(PREFIX)/man/man1
	$(INSTALL) -d $(PREFIX)/man/man5
	$(INSTALL) -m 755 -s fem $(PREFIX)/bin/fem
	$(INSTALL) -m 644 fem.1 $(PREFIX)/man/man1/fem.1
	$(INSTALL) -m 644 feminput.5 $(PREFIX)/man/man5/feminput.5

# deinstallation:
uninstall:	
	-rm -f $(PREFIX)/bin/fem $(PREFIX)/man/man1/fem.1 $(PREFIX)/man/man5/feminput.5

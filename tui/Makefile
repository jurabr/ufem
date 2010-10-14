# Makefile for text user interface for FEM
#
#
CC=gcc
#CC=xlc_r -q64
#CC=/usr/nekoware/bin/gcc # for IRIX
CC=/usr/nekoware/gcc-4.3/bin/gcc

MAKE=make



DEBUG= -O0 -Wall -ansi -pedantic -g -DDEVEL_VERBOSE
#DEBUG= -O2 -Wall -ansi -pedantic -DRUN_VERBOSE
#DEBUG= -DDEVEL_VERBOSE
#
INCLUDES= -I../fem -I../fdb -I../fdb -I../cint -I../common -I../tui
CFLAGS=$(INCLUDES) -DDISABLE_UNFINISHED $(DEBUG) -DHAVE_HOME_DIR
#
##GFCFLAGS=-I../gfx -D_USE_GFX_ -D_USE_GLUT_UI_ #-DUSE_GLUT
GFCFLAGS=-I../gfx -I/usr/X11R6/include -D_USE_GFX_ -D_USE_GUI_  -DUSE_GL2PS -I../../gl2ps/ #-D_USE_LIB_TIFF_ -I/sw/include #-DUSE_FOG
#GFCFLAGS=-I../gfx -I/usr/X11R6/include -D_USE_GFX_ -D_USE_GUI_ # -DUSE_GL2PS -I../../gl2ps/ #-D_USE_LIB_TIFF_ -I/sw/include #-DUSE_FOG # no gl2ps
####GFCFLAGS=-I../gfx -I/usr/X11R6/include -D_USE_G2D_ -D_USE_GUI_  -I/sw/include #-DUSE_FOG
#GUIFLAGS=$(CFLAGS) $(GFCFLAGS) -I../gui -I../gfx `gtk-config --cflags` -D_USE_GTK_GL_AREA_
GUIFLAGS=$(CFLAGS) $(GFCFLAGS) -I../gui `pkg-config --cflags gtk+-2.0 gtkglext-1.0` -DFEM_NEW_FILE_DLG
GUIFLAGS=$(CFLAGS) $(GFCFLAGS) -I../gui `pkg-config --cflags gtk+-2.0 gtkglext-1.0` # old file dialogs here
####GUIFLAGS=$(CFLAGS) $(GFCFLAGS) -I../gui `pkg-config --cflags gtk+-2.0` 
##GUIFLAGS=$(CFLAGS) $(GFCFLAGS) -I../gui -D_USE_GLUT_UI_
#LIBS=-L/usr/local/lib -L../fdb -L../cint -L../common -lfdb -lcint -lfem_comm -lm #-lefence
LIBS=-L/usr/local/lib -L../fdb -L../cint -L../common -L../meval -lfdb -lcint -lmeval -lfem_comm -lpthread -lm #-lefence
#GUILIBS=-lGL -lGLU -lglut -lXmu -lX11 $(LIBS)
##GUILIBS=-L/usr/X11R6/lib -lGL -lGLU -lglut -lXi -lXext -lXmu -lX11 $(LIBS)
#GUILIBS=`gtk-config --libs` -L/usr/local/lib -L/usr/freeware/lib32 -lgtkgl -lGL -lGLU -lglut $(LIBS)
#GUILIBS=`gtk-config --libs` -L/usr/local/lib -L/usr/freeware/lib32 -lgtkgl -lGL -lGLU $(LIBS) -L../../gl2ps -ltiff -lgl2ps -lm
GUILIBS=-L../../gl2ps -lgl2ps `pkg-config --libs gtkglext-1.0 gtk+-2.0` -lGL -lGLU  $(LIBS) #-ltiff
#GUILIBS=`pkg-config --libs gtkglext-1.0 gtk+-2.0` -lGL -lGLU  $(LIBS) # no gl2ps
####GUILIBS=`pkg-config --libs gtk+-2.0` $(LIBS)


# Win32 crosscompiler:
#CC=i386-mingw32-gcc -DWIN32
#AR=i386-mingw32-ar r
#RANLIB=-i386-mingw32-ranlib
#CFLAGS=$(INCLUDES) -DDISABLE_UNFINISHED $(DEBUG)
#GFCFLAGS=-I../gfx -I/usr/X11R6/include -D_USE_GFX_ -D_USE_GUI_  # for win32
#GUIFLAGS=$(CFLAGS) $(GFCFLAGS) -I../gui `export PKG_CONFIG_PATH=/usr/local/i386-mingw32/lib/pkgconfig && pkg-config --cflags gtk+-2.0 ` -I/home/jirka/FEM/src/tui/gle/include/gtkglext-1.0 -I/home/jirka/FEM/src/tui/gle/lib/gtkglext-1.0/include/ -DUSE_GL2PS -I../../gl2ps/ -DFEM_NEW_FILE_DLG
#GUILIBS=-L../../gl2ps -lgl2ps-w32 `export PKG_CONFIG_PATH=/usr/local/i386-mingw32/lib/pkgconfig && pkg-config --libs gtk+-2.0` libgdkglext-win32-1.0.dll.a libgtkglext-win32-1.0.dll.a libopengl32.a libglu32.a  $(LIBS) # for windows

OBJECTS=fem_tini.o fem_tclp.o fem_tcmd.o cmd_vars.o cmd_fem.o cmd_sele.o cmd_macs.o fem_alia.o fem_conf.o fem_solv.o cmd_help.o cmd_solv.o
GFOBJECTS=fem_gfx.o fem_gfnt.o fem_gclr.o fem_gele.o gfx_e000.o gfx_e001.o gfx_e002.o gfx_e003.o gfx_e004.o gfx_e005.o gfx_e006.o gfx_load.o gfx_disp.o gfx_sele.o gfx_res.o gfx_reac.o gfx_ares.o gfx_ppm.o gfx_k.o gfx_grid.o fem_gent.o gfx_en01.o gfx_en02.o gfx_en03.o gfx_en04.o
G2DOBJECTS=
GOBJECTS=$(GFOBJECTS) $(G2DOBJECTS) fem_gui.o gui_main.o gui_gl.o gui_dlgs.o gui_canv.o

all: tfem

tfem: fem_tui.o  tcmd_plot.o $(OBJECTS) ../fdb/fdb.h ../fem/fem.h ../common/libfem_comm.a ../cint/cint.h ../fdb/libfdb.a ../cint/libcint.a  ../meval/libmeval.a
	$(CC) fem_tui.o -o $(@) tcmd_plot.o $(OBJECTS) $(CFLAGS) $(LIBS)

gfem: $(OBJECTS) $(GOBJECTS) gcmd_plot.o ../fdb/fdb.h ../fem/fem.h ../common/libfem_comm.a ../cint/cint.h ../fdb/libfdb.a ../cint/libcint.a ../meval/libmeval.a
	$(CC) -o $(@) $(OBJECTS) $(GOBJECTS) gcmd_plot.o $(GUIFLAGS) $(GUILIBS)

../common/libfem_comm.a:
	(cd ../common ; $(MAKE) all)

../cint/libcint.a:
	(cd ../cint ; $(MAKE) all)

../fdb/libfdb.a:
	(cd ../fdb ; $(MAKE) all)

../meval/libmeval.a:
	(cd ../meval ; $(MAKE) all)

fem_tui.o: fem_tui.c fem_tcmd.o fem_tui.h ../fem/fem.h ../cint/cint.h
	$(CC) -c fem_tui.c $(CFLAGS)

fem_conf.o: fem_conf.c fem_tcmd.o fem_tui.h ../fem/fem.h ../cint/cint.h
	$(CC) -c fem_conf.c $(CFLAGS)

fem_solv.o: fem_solv.c fem_tui.h ../fem/fem.h ../cint/cint.h
	$(CC) -c fem_solv.c $(CFLAGS)

fem_tclp.o: fem_tclp.c fem_tui.h ../fdb/fdb.h ../fem/fem.h
	$(CC) -c fem_tclp.c $(CFLAGS)

fem_tcmd.o: fem_tcmd.c fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c fem_tcmd.c $(CFLAGS)

fem_tini.o: fem_tini.c fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c fem_tini.c $(CFLAGS)

fem_alia.o: fem_alia.c fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c fem_alia.c $(CFLAGS)


# commands:

cmd_vars.o: cmd_vars.c fem_tui.h ../cint/cint.h
	$(CC) -c cmd_vars.c $(CFLAGS)

cmd_fem.o: cmd_fem.c fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c cmd_fem.c $(CFLAGS)

cmd_solv.o: cmd_solv.c fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c cmd_solv.c $(CFLAGS)

cmd_sele.o: cmd_sele.c fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c cmd_sele.c $(CFLAGS)

cmd_macs.o: cmd_macs.c fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c cmd_macs.c $(CFLAGS)


cmd_help.o: cmd_help.c fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c cmd_help.c $(CFLAGS)


# graphics user interface:

fem_gui.o: ../gui/fem_gui.c ../gui/fem_gui.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gui/fem_gui.c $(GUIFLAGS)

gui_main.o: ../gui/gui_main.c ../gui/fem_gui.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gui/gui_main.c $(GUIFLAGS)

gui_gl.o: ../gui/gui_gl.c ../gui/fem_gui.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gui/gui_gl.c $(GUIFLAGS)

gui_dlgs.o: ../gui/gui_dlgs.c ../gui/fem_gui.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gui/gui_dlgs.c $(GUIFLAGS)

gui_canv.o: ../gui/gui_canv.c ../gui/fem_gui.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gui/gui_canv.c $(GUIFLAGS)



# graphics:

fem_gfx.o: ../gfx/fem_gfx.c ../gfx/fem_gfx.h ../gfx/fem_gele.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/fem_gfx.c $(CFLAGS) $(GFCFLAGS)

fem_gfnt.o: ../gfx/fem_gfnt.c ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/fem_gfnt.c $(CFLAGS) $(GFCFLAGS)

fem_gclr.o: ../gfx/fem_gclr.c ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/fem_gclr.c $(CFLAGS) $(GFCFLAGS)

fem_gele.o: ../gfx/fem_gele.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/fem_gele.c $(CFLAGS) $(GFCFLAGS)

gfx_e000.o: ../gfx/gfx_e000.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_e000.c $(CFLAGS) $(GFCFLAGS)

gfx_e001.o: ../gfx/gfx_e001.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_e001.c $(CFLAGS) $(GFCFLAGS)

gfx_e002.o: ../gfx/gfx_e002.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_e002.c $(CFLAGS) $(GFCFLAGS)

gfx_e003.o: ../gfx/gfx_e003.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_e003.c $(CFLAGS) $(GFCFLAGS)

gfx_e004.o: ../gfx/gfx_e004.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_e004.c $(CFLAGS) $(GFCFLAGS)

gfx_e005.o: ../gfx/gfx_e005.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_e005.c $(CFLAGS) $(GFCFLAGS)

gfx_e006.o: ../gfx/gfx_e006.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_e006.c $(CFLAGS) $(GFCFLAGS)

gfx_load.o: ../gfx/gfx_load.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_load.c $(CFLAGS) $(GFCFLAGS)

gfx_disp.o: ../gfx/gfx_disp.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_disp.c $(CFLAGS) $(GFCFLAGS)

gfx_sele.o: ../gfx/gfx_sele.c ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_sele.c $(CFLAGS) $(GFCFLAGS)

gfx_res.o: ../gfx/gfx_res.c ../gfx/fem_gfx.h ../fdb/fdb_res.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_res.c $(CFLAGS) $(GFCFLAGS)

gfx_ares.o: ../gfx/gfx_ares.c ../gfx/fem_gfx.h ../fdb/fdb_res.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_ares.c $(CFLAGS) $(GFCFLAGS)

gfx_reac.o: ../gfx/gfx_reac.c ../gfx/fem_gfx.h ../fdb/fdb_res.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_reac.c $(CFLAGS) $(GFCFLAGS)

gfx_ppm.o: ../gfx/gfx_ppm.c ../gfx/fem_gfx.h ../fdb/fdb_res.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_ppm.c $(CFLAGS) $(GFCFLAGS)

gfx_k.o: ../gfx/gfx_k.c ../gfx/fem_gfx.h
	$(CC) -c ../gfx/gfx_k.c $(CFLAGS) $(GFCFLAGS)

fem_gent.o: ../gfx/fem_gent.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/fem_gent.c $(CFLAGS) $(GFCFLAGS)

gfx_grid.o: ../gfx/gfx_grid.c ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_grid.c $(CFLAGS) $(GFCFLAGS)

gfx_en01.o: ../gfx/gfx_en01.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_en01.c $(CFLAGS) $(GFCFLAGS)

gfx_en02.o: ../gfx/gfx_en02.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_en02.c $(CFLAGS) $(GFCFLAGS)

gfx_en03.o: ../gfx/gfx_en03.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_en03.c $(CFLAGS) $(GFCFLAGS)

gfx_en04.o: ../gfx/gfx_en04.c ../gfx/fem_gele.h ../gfx/fem_gfx.h fem_tui.h ../fdb/fdb.h ../cint/cint.h ../fem/fem.h
	$(CC) -c ../gfx/gfx_en04.c $(CFLAGS) $(GFCFLAGS)


# commands for plotting:

tcmd_plot.o: cmd_plot.c fem_tui.h
	$(CC) -c cmd_plot.c -o $(@) $(CFLAGS)

gcmd_plot.o: cmd_plot.c ../gui/fem_gui.h ../gfx/fem_gfx.h fem_tui.h
	$(CC) -c cmd_plot.c -o $(@) $(CFLAGS) $(GCFLAGS) $(GUIFLAGS)



clean:
	-rm -f tfem gfem *.o core

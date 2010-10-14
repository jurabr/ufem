/*
   File name: fem_gui.h
   Date:      2003/08/17 16:38
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.

	 FEM -  g. user interface

	 $Id: fem_gui.h,v 1.7 2004/11/11 21:41:27 jirka Exp $
*/


#ifndef __FEM_GUI_H__
#define __FEM_GUI_H__

#include <stdio.h>
#include <stdlib.h>

#ifdef _USE_GFX_
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "fem_gfx.h"
#include "fem_tui.h"
#include "cint.h"

#ifdef _USE_GUI_

#include <gtk/gtk.h>

#ifdef _USE_GFX_
#ifdef _USE_GTK_GL_AREA_ /* for Gtk+ 1.x */
#include <gtkgl/gtkglarea.h>
#else                    /* for Gtk+ 2.x */
#include <gtk/gtkgl.h>
#endif
#endif

extern GtkWidget *windowMain  ;
extern GtkWidget *cmdPrompt   ;
extern GtkWidget *cmdLine     ;
extern GtkWidget *cmdStatus   ;
extern GtkWidget *cmdTree     ;
extern GtkWidget *pickButtons ;
extern GtkWidget *glArea      ;
extern GtkWidget *glButtons   ;

extern int fem_gui_cmd( GtkWidget *widget, gpointer data );

extern GtkWidget* glArea_new(void) ;
extern int gui_winMain(int argc, char *argv[]) ;

#else /* end of _USE_GUI_ */

#ifdef _USE_GLUT_UI_
#include <GL/glut.h>

#endif
#endif

#endif

/* end of fem_gui.h */

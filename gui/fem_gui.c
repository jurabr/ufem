/*
   File name: fem_gui.c
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

   FEM - g.  user interface

   $Id: fem_gui.c,v 1.8 2005/02/16 19:41:43 jirka Exp $
*/

#include "fem_gui.h"

extern int fem_parse_params(int argc, char *argv[]);
extern int fem_get_ui_config(void);

#ifdef _USE_GUI_
/** command execution callback */
int fem_gui_cmd( GtkWidget *widget, gpointer   data )
{
  if (ciRunCmd((char *)data) == AF_OK)
  {
    gtk_label_set_text(GTK_LABEL(cmdStatus), _("O.K. ")) ;
    return(AF_OK);
  }
  else
  {
    gtk_label_set_text(GTK_LABEL(cmdStatus), _("Failed ")) ;
    return(AF_ERR);
  }
}


/** GUI initialization*/
int main(int argc, char *argv[])
{
  int rv = AF_OK ;

  msgout = stdout ; /* required here */

  if ((rv = fem_parse_params(argc, argv)) != AF_OK) {return(rv);}

  /** program core init */
  if ((rv =  fem_tui_init()) != AF_OK) { return(rv); }

  /* Gtk+ init */
  gtk_init(&argc, &argv);

#ifdef _USE_GTK_GL_EXT_
    gtk_gl_init(&argc, &argv);
#endif

  /* gfx defaults */
  femDefaultPlotProp() ;

  /* gui creation */
  if ((rv = gui_winMain(argc, argv)) != AF_OK) { return(rv); }

  fem_get_ui_config(); /* no testing needed */

  /* startup message: */
  fprintf (msgout,"\n[I] %s %s", FEM_TUI_RELEASE, _("(GUI mode)"));
  fprintf (msgout,"\n[i] (C) Jiri Brozovsky\n\n");

  /* main GUI loop*/
  gtk_main() ;

  return(rv) ;
}
#else /* end od _USE_GUI_ */

#ifdef _USE_GLUT_UI_
extern int guiMainWin(void);

/** GUI initialization*/
int main(int argc, char *argv[])
{
  int rv = AF_OK ;

  msgout = stdout ; /* required here */

  if ((rv = fem_parse_params(argc, argv)) != AF_OK) {return(rv);}

  /** program core init */
  if ((rv =  fem_tui_init()) != AF_OK) { return(rv); }

  /* GLUT init */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL|GLUT_DOUBLE);

  /* gfx defaults */
  femDefaultPlotProp() ;

  /* gui creation */
  guiMainWin();

  fem_get_ui_config(); /* no testing needed */

  glutMainLoop();

  return(rv) ;
}
#endif
#endif


/* end of fem_gui.c */

/*
   File name: gui_main.c
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

	 $Id: gui_main.c,v 1.46 2005/02/21 22:05:56 jirka Exp $
*/

#include "fem_gui.h"
#include "cint.h"
#ifdef _USE_GUI_
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

extern void gui_select_file ( GtkWidget *widget, gpointer   data );
extern GtkWidget * fem_cmd_tree(void);
extern int femCleanGUIbutt;
extern int femCleanGUItree;
#ifdef _USE_G2D_
extern GtkWidget* gArea_new(void) ;
#endif

extern int femGfxWriteConfig_data(FILE *fw); /* fem_gfx */

#ifndef _USE_GTK_GL_AREA_
void show_exit_dialog(GtkWidget *widget, gpointer data);
#endif

long cmdline_history_rot = 0 ;

/* Main window*/
GtkWidget *windowMain  = NULL; /* main window                    */
GtkWidget *cmdPrompt   = NULL; /* prompt symbol for command line */
GtkWidget *cmdLine     = NULL; /* command line                   */
GtkWidget *cmdStatus   = NULL; /* status indicator               */
GtkWidget *cmdTree     = NULL; /* command tree               */
GtkWidget *scrollWin   = NULL; /* scrolled window for cmdTree    */
GtkWidget *glArea      = NULL; /* statis indicator               */
GtkWidget *glButtons   = NULL; /* statis indicator               */
GdkCursor *cursor_std  = NULL; /* standard cursor                */
GdkCursor *cursor_clock= NULL; /* waiting cursor                 */
GdkCursor *cursor_sel  = NULL; /* selection cursor                 */
#ifndef _USE_GTK_GL_AREA_
GtkAccelGroup *accel   = NULL; /* accelerator table              */
GtkWidget *pick_run    = NULL;
GtkWidget *pick_cancel = NULL;
#endif

/* core events: */
gint delete_event( GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data )
{
    /*g_print ("delete event occurred\n");*/
#ifndef _USE_GTK_GL_AREA_
  show_exit_dialog(widget, data);
#endif
    return(TRUE);
}

void destroy( GtkWidget *widget,
              gpointer   data )
{
    /* maybe more code will be needed here */
    gtk_main_quit();
    exit(0);
}

void cmdline_activate( GtkWidget *widget,
                      gpointer   data )
{
  char cmd[FEM_STR_LEN+1];
	int i ;
  GdkRectangle update_rect;

  update_rect.x = cmdStatus->allocation.x ;
  update_rect.y = cmdStatus->allocation.y ;
  update_rect.width = cmdStatus->allocation.width ;
  update_rect.height = cmdStatus->allocation.height ;

  gdk_window_set_cursor(windowMain->window, cursor_clock);
	gtk_label_set_text(GTK_LABEL(cmdStatus), _("Wait..")) ;
  gtk_widget_draw((windowMain), &update_rect);
  gtk_widget_draw((cmdStatus), &update_rect);


	for (i=0; i<FEM_STR_LEN+1; i++) {cmd[i] = '\0';}
  strncpy(cmd, gtk_entry_get_text(GTK_ENTRY(cmdLine)), FEM_STR_LEN);

	gtk_entry_set_text(GTK_ENTRY(cmdLine), "") ;

  update_rect.x = cmdLine->allocation.x ;
  update_rect.y = cmdLine->allocation.y ;
  update_rect.width = cmdLine->allocation.width ;
  update_rect.height = cmdLine->allocation.height ;
  gtk_widget_draw((cmdLine), &update_rect);
  gtk_widget_draw((windowMain), &update_rect);


  if (ciRunCmdHs(cmd) == AF_OK)
  {
	  gtk_label_set_text(GTK_LABEL(cmdStatus), _("  Done  ")) ;
  }
  else
  {
	  gtk_label_set_text(GTK_LABEL(cmdStatus), _(" Failed ")) ;
  }

  cmdline_history_rot = 0 ;

  gdk_window_set_cursor(windowMain->window, cursor_std);
}

/* create image: */
#ifndef _USE_GTK_GL_AREA_
GdkPixbuf *create_pixbuf(void)
{
  GdkPixbuf *pixbuf;
  const char * ufem_icon_tr32_xpm[] = {
  "32 32 17 1",
  " 	c None",
  ".	c #200804",
  "+	c #024812",
  "@	c #3B3202",
  "#	c #FE0202",
  "$	c #5A5702",
  "%	c #9C8F02",
  "&	c #02023F",
  "*	c #F7D502",
  "=	c #026A02",
  "-	c #D20202",
  ";	c #027202",
  ">	c #FEFE02",
  ",	c #02026F",
  "'	c #7F0202",
  ")	c #0202ED",
  "!	c #02F102",
  "    @@@@@@@@@@@@@@.&            ",
  "    %************%>**%$$@.&     ",
  "   .*>***>******%*>>>>>>>>*%%%. ",
  "   $********>***$>>>>>>>>>>>>>$ ",
  "   %***********>%>>>>>>>>>>>>>% ",
  "   *************%>>>>>>>>>>>>>% ",
  "  @************@@>>>>>>>>>>>>>% ",
  "  %**********%.  @*>>>>>>>>>>>* ",
  "  %***>*****%.    .*>>>>>>>>>>* ",
  " @*********@       .%>>>>>>>>>> ",
  " $*******%@          %>>>>>>>>>@",
  " %******%.            $>>>>>>>>$",
  " ******@               @>>>>>>>$",
  "@%***%.                 .******$",
  ".'''''                   ,,,,,,&",
  " ####-                   )))))) ",
  " '####.                 ,))))), ",
  " '####'                 ,)))))& ",
  " .####'                 ))))))  ",
  "  -####                &))))),  ",
  "  '####'               ,)))))&  ",
  "  .####'               ))))),   ",
  "   -###-              &))))),   ",
  "   '####.             ,)))))&   ",
  "   '####'             ,)))),    ",
  "    ####-&           &)))))&    ",
  "    '####.!!!!!!!!!!;&)))))     ",
  "    '###-'!!!!!!!!!!!,)))),     ",
  "    .####';!!!!!!!!!=)))))&     ",
  "     -####=!!!!!!!!!+)))))      ",
  "     '####@!!!!!!!!!,))))&      ",
  "     .''''.;;;;;;;==&,,,,&      "};
   /* ---------------------------------------------------------- */

  pixbuf = gdk_pixbuf_new_from_xpm_data(ufem_icon_tr32_xpm);
  return(pixbuf);
}
#endif

#ifndef _USE_GTK_GL_AREA_
gboolean key_press_cmd (GtkWidget * widget, GdkEventKey * event, GtkWindow * window)
{
  switch (event->keyval)
  {
    case GDK_Up:
      if (ciHistory != NULL)
      {
        cmdline_history_rot++ ;
        if (cmdline_history_rot >= (ciHistorySize-1)) {cmdline_history_rot = ciHistorySize-1 ;}
        gtk_entry_set_text(GTK_ENTRY(cmdLine),ciCmdHsGet(cmdline_history_rot-1));
      }
      return TRUE;
      break;

    case GDK_Down:
      if (ciHistory != NULL)
      {
        cmdline_history_rot--;
        if (cmdline_history_rot < 1) {cmdline_history_rot = 1 ;}
        gtk_entry_set_text(GTK_ENTRY(cmdLine),ciCmdHsGet(cmdline_history_rot-0));
      }
      return TRUE;
      break;

    default: return(FALSE);
  }

}
#endif

#ifndef _USE_GTK_GL_AREA_
void show_about_dialog(GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new(
      GTK_WINDOW(windowMain),
      GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_INFO,
      GTK_BUTTONS_OK,
      _("%s\n\nStructural Analysis Tool\n\n(C) Jiri Brozovsky\n\nReleased under GNU General Public Licence"), 
      FEM_TUI_RELEASE
      ); 

  g_signal_connect_swapped (GTK_OBJECT (dialog),
      "response",
      G_CALLBACK (gtk_widget_destroy),
      GTK_OBJECT (dialog)); 

  gtk_widget_show_all (dialog);
}

void show_exit_dialog(GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new(
      GTK_WINDOW(windowMain),
      GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_WARNING,
      GTK_BUTTONS_OK,
      _("Please try to quit me through Menu or by use of \'quit\' command.")
      ); 

  g_signal_connect_swapped (GTK_OBJECT (dialog),
      "response",
      G_CALLBACK (gtk_widget_destroy),
      GTK_OBJECT (dialog)); 

  gtk_widget_show_all (dialog);
}

#endif

void gui_main_menu_file(GtkWidget *menubar)
{
	GtkWidget *menu_file ;
	GtkWidget *file_item ;
	GtkWidget *open_item ;
	GtkWidget *save_item ;
	GtkWidget *input_item ;
	GtkWidget *import_item ;
	GtkWidget *export_item ;
	GtkWidget *nproc_item ;
	GtkWidget *exit_item ;
	GtkWidget *tear_item ;
	GtkWidget *separ_item ;
#ifndef _USE_GTK_GL_AREA_
	GtkWidget *jobname_item ;
	GtkWidget *cleandata_item ;
#endif

	/* menu object */
	menu_file = gtk_menu_new() ;


	tear_item = gtk_tearoff_menu_item_new();
	gtk_menu_append (GTK_MENU (menu_file), tear_item);
	gtk_widget_show (tear_item);
	
	/* items */
	open_item = gtk_menu_item_new_with_label (_("Resume from.."));
	gtk_menu_append (GTK_MENU (menu_file), open_item);

	gtk_signal_connect(GTK_OBJECT (open_item), "activate",
			                       GTK_SIGNAL_FUNC (gui_select_file),
		                         (gpointer) "resu");
	gtk_widget_show (open_item);

	/* items */
	save_item = gtk_menu_item_new_with_label (_("Save to.."));
	gtk_menu_append (GTK_MENU (menu_file), save_item);

	gtk_signal_connect(GTK_OBJECT (save_item), "activate",
			                       GTK_SIGNAL_FUNC (gui_select_file),
		                         (gpointer) "save");
	gtk_widget_show (save_item);

  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_file), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

#ifndef _USE_GTK_GL_AREA_
	/* items */
	jobname_item = gtk_menu_item_new_with_label (_("Change JobName.."));
	gtk_menu_append (GTK_MENU (menu_file), jobname_item);
	g_signal_connect(GTK_OBJECT (jobname_item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,jobname");
	gtk_widget_show (jobname_item);
	
	cleandata_item = gtk_menu_item_new_with_label (_("Clean All Data"));
	gtk_menu_append (GTK_MENU (menu_file), cleandata_item);
	g_signal_connect(GTK_OBJECT (cleandata_item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "cleandata");
	gtk_widget_show (cleandata_item);

	separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_file), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

	/* items */
	import_item = gtk_menu_item_new_with_label (_("Import from solver.."));
	gtk_menu_append (GTK_MENU (menu_file), import_item);

	gtk_signal_connect(GTK_OBJECT (import_item), "activate",
			                       GTK_SIGNAL_FUNC (gui_select_file),
		                         (gpointer) "import,fem");

	gtk_widget_show (import_item);


	export_item = gtk_menu_item_new_with_label (_("Export.."));
	gtk_menu_append (GTK_MENU (menu_file), export_item);

	gtk_signal_connect(GTK_OBJECT (export_item), "activate",
			                       GTK_SIGNAL_FUNC (fem_gui_cmd),
		                         (gpointer) "dialog,export");
	gtk_widget_show (export_item);


  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_file), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

	/* items */
	input_item = gtk_menu_item_new_with_label (_("Read commands.."));
	gtk_menu_append (GTK_MENU (menu_file), input_item);

	gtk_signal_connect(GTK_OBJECT (input_item), "activate",
			                       GTK_SIGNAL_FUNC (gui_select_file),
		                         (gpointer) "input");
	gtk_widget_show (input_item);


  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_file), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  /* nproc*/
	nproc_item = gtk_menu_item_new_with_label (_("Number of Processes.."));
	gtk_menu_append (GTK_MENU (menu_file), nproc_item);

	gtk_signal_connect(GTK_OBJECT (nproc_item), "activate",
			                       GTK_SIGNAL_FUNC (fem_gui_cmd),
		                         (gpointer) "dialog,nproc");
	gtk_widget_show (nproc_item);

  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_file), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


#endif


	exit_item = gtk_menu_item_new_with_label (_("Exit"));
	gtk_menu_append (GTK_MENU (menu_file), exit_item);
	gtk_widget_show (exit_item);
	gtk_signal_connect_object (GTK_OBJECT (exit_item), "activate",
			                       GTK_SIGNAL_FUNC (destroy),
		                         (gpointer) "file.quit");

	/* appending of menu to menubar */
#ifndef _USE_GTK_GL_AREA_
	file_item = gtk_menu_item_new_with_mnemonic ("_File");
	gtk_widget_add_accelerator(file_item, "activate", accel, 
			GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
#else
	file_item = gtk_menu_item_new_with_label ("File");
#endif
	gtk_widget_show (file_item);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_item), menu_file);
	gtk_menu_bar_append (GTK_MENU_BAR (menubar), file_item);
}

#ifndef _USE_GTK_GL_AREA_
void gui_main_menu_help(GtkWidget *menubar)
{
	GtkWidget *menu_help ;
	GtkWidget *help_item ;
	GtkWidget *separ_item ;
#ifndef DISABLE_UNFINISHED
	GtkWidget *over_item ;
#endif
	GtkWidget *about_item ;
	GtkWidget *tear_item ;

	/* menu object */
	menu_help = gtk_menu_new() ;

	tear_item = gtk_tearoff_menu_item_new();
	gtk_menu_append (GTK_MENU (menu_help), tear_item);
	gtk_widget_show (tear_item);
	
	/* items */
#ifndef DISABLE_UNFINISHED
  over_item = gtk_menu_item_new_with_label (_("Overview"));
	gtk_menu_append (GTK_MENU (menu_help), over_item);
	gtk_widget_show (over_item);
	gtk_signal_connect(GTK_OBJECT (over_item), "activate",
													   GTK_SIGNAL_FUNC(fem_gui_cmd),
		                         (gpointer) "help,overview");
#endif
   
	about_item = gtk_menu_item_new_with_label (_("Element Types.."));
	gtk_menu_append (GTK_MENU (menu_help), about_item);
	gtk_widget_show (about_item);
	gtk_signal_connect(GTK_OBJECT (about_item), "activate",
													   GTK_SIGNAL_FUNC(fem_gui_cmd),
		                         (gpointer) "dialog,helpe");

	about_item = gtk_menu_item_new_with_label (_("Material Types.."));
	gtk_menu_append (GTK_MENU (menu_help), about_item);
	gtk_widget_show (about_item);
	gtk_signal_connect(GTK_OBJECT (about_item), "activate",
													   GTK_SIGNAL_FUNC(fem_gui_cmd),
		                         (gpointer) "dialog,helpm");

  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_help), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

	about_item = gtk_menu_item_new_with_label (_("Program Home Page (online).."));
	gtk_menu_append (GTK_MENU (menu_help), about_item);
	gtk_widget_show (about_item);
#ifndef WIN32
	gtk_signal_connect(GTK_OBJECT (about_item), "activate",
													   GTK_SIGNAL_FUNC(fem_gui_cmd),
		                         (gpointer) "system,firefox http://fast10.vsb.cz/brozovsky/articles/ufem/index.html &");
#else
	gtk_signal_connect(GTK_OBJECT (about_item), "activate",
													   GTK_SIGNAL_FUNC(fem_gui_cmd),
		                         (gpointer) "system,start http://fast10.vsb.cz/brozovsky/articles/ufem/index.html &");
#endif

  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_help), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);
 
	about_item = gtk_menu_item_new_with_label (_("About Program.."));
	gtk_menu_append (GTK_MENU (menu_help), about_item);
	gtk_widget_show (about_item);
	gtk_signal_connect(GTK_OBJECT (about_item), "activate",
													   GTK_SIGNAL_FUNC(show_about_dialog),
		                         (gpointer) NULL);




	/* appending of menu to menubar */
	help_item = gtk_menu_item_new_with_mnemonic ("_Help");
	gtk_widget_add_accelerator(help_item, "activate", accel, 
			GDK_h, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_show (help_item);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_item), menu_help);
	gtk_menu_bar_append (GTK_MENU_BAR (menubar), help_item);
}

void gui_main_menu_plot(GtkWidget *menubar)
{
	GtkWidget *menu_plot ;
	GtkWidget *plot_item ;
	GtkWidget *separ_item ;
	GtkWidget *tear_item ;
	GtkWidget *item ;

	/* menu object */
	menu_plot = gtk_menu_new() ;

	tear_item = gtk_tearoff_menu_item_new();
	gtk_menu_append (GTK_MENU (menu_plot), tear_item);
	gtk_widget_show (tear_item);
	
	/* items */
  item = gtk_menu_item_new_with_label (_("Replot"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "replot");

  item = gtk_menu_item_new_with_label (_("Nothing"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "noplot");



	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

  item = gtk_menu_item_new_with_label (_("Keypoints"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "kplot");

  item = gtk_menu_item_new_with_label (_("Geometric Entities"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "geplot");



	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Nodes"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "nplot");

  item = gtk_menu_item_new_with_label (_("Elements"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "eplot");

  item = gtk_menu_item_new_with_label (_("Displacements"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dplot");

  item = gtk_menu_item_new_with_label (_("Nodal Loads"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "fplot");

#ifndef DISABLE_UNFINISHED
  item = gtk_menu_item_new_with_label (_("Element Loads"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "elplot");
#endif

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Deformed Shape"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "pldef");

  item = gtk_menu_item_new_with_label (_("Reactions"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plrs");

  item = gtk_menu_item_new_with_label (_("Element Solution.."));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,ples");

#ifndef DISABLE_UNFINISHED
  item = gtk_menu_item_new_with_label (_("Nodal Solution.."));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,plns");
#endif

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

  item = gtk_menu_item_new_with_label (_("Picking Grid..."));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,pgrid");

  /* ################################ */
  /* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Keypoints Numbers"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,kpnumber,yes");

  item = gtk_menu_item_new_with_label (_("G.E. Numbers"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,genumber,yes");

  item = gtk_menu_item_new_with_label (_("Node Numbers"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,nodenumber,yes");

  item = gtk_menu_item_new_with_label (_("Element Numbers"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,elemnumber,yes");

  item = gtk_menu_item_new_with_label (_("Load Sizes"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,loadsize,yes");

  /* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

  item = gtk_menu_item_new_with_label (_("Solid Model"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,wireonly,no");

  item = gtk_menu_item_new_with_label (_("Wireframe Model"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,wireonly,yes");

  /* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

  item = gtk_menu_item_new_with_label (_("Automatic Replotting"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,autoreplot,yes");
  /* ################################ */

#ifndef WIN32
	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


	/* save to PPM */
  item = gtk_menu_item_new_with_label (_("Save as PPM.."));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "filedialog,gl2ppm");

	/* save to TIFF */
#ifdef _USE_LIB_TIFF_
  item = gtk_menu_item_new_with_label (_("Save as TIFF.."));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "filedialog,gl2tiff");
#endif

	/* save to EPS */
#ifdef USE_GL2PS
  item = gtk_menu_item_new_with_label (_("Save as EPS.."));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "filedialog,gl2ps");
#endif

#endif /* WIN32 */

	/* appending of menu to menubar */
	plot_item = gtk_menu_item_new_with_mnemonic ("_Plot");
	gtk_widget_add_accelerator(plot_item, "activate", accel, 
			GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_show (plot_item);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (plot_item), menu_plot);
	gtk_menu_bar_append (GTK_MENU_BAR (menubar), plot_item);
}

void gui_main_menu_hide(GtkWidget *menubar)
{
	GtkWidget *menu_plot ;
	GtkWidget *plot_item ;
	GtkWidget *separ_item ;
	GtkWidget *tear_item ;
	GtkWidget *item ;

	/* menu object */
	menu_plot = gtk_menu_new() ;

	tear_item = gtk_tearoff_menu_item_new();
	gtk_menu_append (GTK_MENU (menu_plot), tear_item);
	gtk_widget_show (tear_item);
	
	/* items */
  item = gtk_menu_item_new_with_label (_("Nodes"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "hide,n");

  item = gtk_menu_item_new_with_label (_("Element"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "hide,e");

  item = gtk_menu_item_new_with_label (_("Displacements"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "hide,d");

  item = gtk_menu_item_new_with_label (_("Loads"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "hide,f");


	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Keypoints"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "hide,k");

  item = gtk_menu_item_new_with_label (_("Geometric Entities"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "hide,ge");

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Results"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "hide,r");

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

  item = gtk_menu_item_new_with_label (_("Picking Grid"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotp,grid,no");

  /* ################################ */

  /* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Keypoints Numbers"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,kpnumber,no");

  item = gtk_menu_item_new_with_label (_("G.E. Numbers"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,genumber,no");

  item = gtk_menu_item_new_with_label (_("Node Numbers"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,nodenumber,no");

  item = gtk_menu_item_new_with_label (_("Element Numbers"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,elemnumber,no");

  item = gtk_menu_item_new_with_label (_("Load Sizes"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,loadsize,no");


  /* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_plot), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

  item = gtk_menu_item_new_with_label (_("Automatic Replotting"));
	gtk_menu_append (GTK_MENU (menu_plot), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "plotprop,autoreplot,no");


  /* ################################ */

	/* appending of menu to menubar */
	plot_item = gtk_menu_item_new_with_mnemonic ("H_ide");
	gtk_widget_add_accelerator(plot_item, "activate", accel, 
			GDK_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_show (plot_item);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (plot_item), menu_plot);
	gtk_menu_bar_append (GTK_MENU_BAR (menubar), plot_item);
}

void gui_main_menu_list(GtkWidget *menubar)
{
	GtkWidget *menu_list ;
	GtkWidget *list_item ;
	GtkWidget *separ_item ;
	GtkWidget *tear_item ;
	GtkWidget *item ;

	/* menu object */
	menu_list = gtk_menu_new() ;

	tear_item = gtk_tearoff_menu_item_new();
	gtk_menu_append (GTK_MENU (menu_list), tear_item);
	gtk_widget_show (tear_item);

  /* properties*/
  item = gtk_menu_item_new_with_label (_("Element Types"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "etlist");

  item = gtk_menu_item_new_with_label (_("Real Sets"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "rslist");

  item = gtk_menu_item_new_with_label (_("Materials"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "mplist");

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_list), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

  item = gtk_menu_item_new_with_label (_("Keypoints"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "klist");

  item = gtk_menu_item_new_with_label (_("Geometric Entities"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "gelist");


	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_list), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


	
	/* items */
  item = gtk_menu_item_new_with_label (_("Nodes"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "nlist");

  item = gtk_menu_item_new_with_label (_("Elements"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "elist");

  item = gtk_menu_item_new_with_label (_("Displacements"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dlist");

  item = gtk_menu_item_new_with_label (_("Nodal Loads"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "flist");

#ifndef DISABLE_UNFINISHED
  item = gtk_menu_item_new_with_label (_("Element Loads"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "ellist");
#endif

  item = gtk_menu_item_new_with_label (_("Accelerations"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "accellist");


	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_list), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("DOF Solution"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "prdof");

  item = gtk_menu_item_new_with_label (_("Reactions"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "prrs");

  item = gtk_menu_item_new_with_label (_("Element Solution.."));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,ples");

#ifndef DISABLE_UNFINISHED
  item = gtk_menu_item_new_with_label (_("Nodal Solution.."));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,plns");
#endif

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_list), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Summary"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "prsumm");

  item = gtk_menu_item_new_with_label (_("Coordinate System"));
	gtk_menu_append (GTK_MENU (menu_list), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "prcs");


	/* appending of menu to menubar */
	list_item = gtk_menu_item_new_with_mnemonic ("_List");
	gtk_widget_add_accelerator(list_item, "activate", accel, 
			GDK_l, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_show (list_item);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (list_item), menu_list);
	gtk_menu_bar_append (GTK_MENU_BAR (menubar), list_item);
}

void gui_main_menu_sele(GtkWidget *menubar)
{
	GtkWidget *menu_sele ;
	GtkWidget *sele_item ;
	GtkWidget *separ_item ;
	GtkWidget *tear_item ;
	GtkWidget *item ;

	/* menu object */
	menu_sele = gtk_menu_new() ;

	tear_item = gtk_tearoff_menu_item_new();
	gtk_menu_append (GTK_MENU (menu_sele), tear_item);
	gtk_widget_show (tear_item);
	
	/* items */
  item = gtk_menu_item_new_with_label (_("Keypoints.."));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,ksel");

  item = gtk_menu_item_new_with_label (_("Geometric Entities.."));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,gesel");

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_sele), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Nodes.."));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,nsel");

  item = gtk_menu_item_new_with_label (_("Element.."));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,esel");

  item = gtk_menu_item_new_with_label (_("Displacements.."));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,dsel");

  item = gtk_menu_item_new_with_label (_("Nodal Loads.."));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,fsel");

#ifndef DISABLE_UNFINISHED
  item = gtk_menu_item_new_with_label (_("Element Loads.."));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,elsel");
#endif

  item = gtk_menu_item_new_with_label (_("Accelerations.."));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,acccesel");

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_sele), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

  item = gtk_menu_item_new_with_label (_("Geometric Entities By Keypoints"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "geslk");

  item = gtk_menu_item_new_with_label (_("Keypoints By Geometric Entities "));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "kslge");


	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_sele), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Elements By Nodes"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "esln");

  item = gtk_menu_item_new_with_label (_("Displacements By Nodes"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,plns");

  item = gtk_menu_item_new_with_label (_("Nodal Loads By Nodes"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "fsln");

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_sele), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Nodes By Elements"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "nsle");

  item = gtk_menu_item_new_with_label (_("Nodes By Displacements"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "nsld");

  item = gtk_menu_item_new_with_label (_("Nodes By Nodal Loads"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "nslf");

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_sele), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);


  item = gtk_menu_item_new_with_label (_("Elements By Element Loads"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "eslel");

  item = gtk_menu_item_new_with_label (_("Element Loads By Elements"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "elsle");

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_sele), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);

  item = gtk_menu_item_new_with_label (_("Interactive.."));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "dialog,gsel");

	/* separator */
  separ_item = gtk_menu_item_new() ;
  gtk_menu_append (GTK_MENU (menu_sele), separ_item);
  gtk_widget_show (separ_item);
  gtk_widget_set_sensitive (separ_item, FALSE);



  item = gtk_menu_item_new_with_label (_("Everything"));
	gtk_menu_append (GTK_MENU (menu_sele), item);
	gtk_widget_show (item);
	g_signal_connect(GTK_OBJECT (item), "activate",
	GTK_SIGNAL_FUNC(fem_gui_cmd), (gpointer) "allsel");


	/* appending of menu to menubar */
	sele_item = gtk_menu_item_new_with_mnemonic ("_Select");
	gtk_widget_add_accelerator(sele_item, "activate", accel, 
			GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_show (sele_item);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (sele_item), menu_sele);
	gtk_menu_bar_append (GTK_MENU_BAR (menubar), sele_item);
}
#endif

void gui_gl_toolbar(GtkWidget *glButtons)
{

  gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Replot"), 
													_("Repeats last plotting"), 
													_("Repeats last plotting"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "replot" ); 

	gtk_toolbar_append_space(GTK_TOOLBAR(glButtons));


	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Front"), 
													_("Front view"), 
													_("Front view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "view,fro" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Back"), 
													_("Back view"), 
													_("Back view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "view,bac" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Left"), 
													_("Left view"), 
													_("Left view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "view,lef" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Right"), 
													_("Right view"), 
													_("Right view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "view,rig" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Top"), 
													_("Top view"), 
													_("Top view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "view,top" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Bottom"), 
													_("Bottom view"), 
													_("Bottom view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "view,bot" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Iso"), 
													_("Isometric view"), 
													_("Isometric view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "view,iso" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Iso2"), 
													_("Alternative isometric view"), 
													_("Alternative isometric view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "view,aiso" ); 



	gtk_toolbar_append_space(GTK_TOOLBAR(glButtons));


	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("+X"), 
													_("Move to the right"), 
													_("Move to the right"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "move,x,move_pos" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("-X"), 
													_("Move to the left"), 
													_("Move to the left"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "move,x,move_neg" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("+Y"), 
													_("Move to the right"), 
													_("Move to the right"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "move,y,move_pos" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("-Y"), 
													_("Move to the left"), 
													_("Move to the left"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "move,y,move_neg" ); 

	gtk_toolbar_append_space(GTK_TOOLBAR(glButtons));

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Rot +X "), 
													_("+X rotation "), 
													_("+X rotation"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "rot,x,rot_pos" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Rot -X "), 
													_("-X rotation"), 
													_("-X rotation"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "rot,x,rot_neg" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Rot +Y "), 
													_("+Y rotation "), 
													_("+Y rotation"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "rot,y,rot_pos" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Rot -Y "), 
													_("-Y rotation"), 
													_("-Y rotation"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "rot,y,rot_neg" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Rot +Z "), 
													_("+Z rotation "), 
													_("+Z rotation"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "rot,z,rot_pos" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Rot -Z "), 
													_("-Z rotation"), 
													_("-Z rotation"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "rot,z,rot_neg" ); 

	gtk_toolbar_append_space(GTK_TOOLBAR(glButtons));


	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Zoom"), 
													_("Scales up view"), 
													_("Scales up view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "zoom" ); 

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("UnZoom"), 
													_("Scales down view"), 
													_("Scales down view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "unzoom" ); 



	gtk_toolbar_append_space(GTK_TOOLBAR(glButtons));

	gtk_toolbar_append_item(GTK_TOOLBAR(glButtons), 
	                        _("Reset"), 
													_("Reset view"), 
													_("Reset view"), 
													NULL, /* icon */
													GTK_SIGNAL_FUNC(fem_gui_cmd),
													(gpointer) "reset" ); 
}


/** fills command line with something */
int femSetCmdLine(char *str)
{
  gtk_entry_set_text(GTK_ENTRY(cmdLine), str);
  return(AF_OK) ;
}


/** GUI initialization*/
int gui_winMain(int argc, char *argv[])
{
  int rv = AF_OK ;
  GtkWidget *thebox ;
  GtkWidget *hbox_top_1, *hbox_top_2 ;
#ifndef _USE_GTK_GL_AREA_
  GtkWidget *vbox_left ;
	GtkWidget *pick_frame  ;
	GtkWidget *bbox  ;
  GtkWidget *hsepar;
	GtkWidget *paned ;
#endif
  GtkWidget *main_menu;
	gboolean   homogenous = FALSE;
	gboolean   expand = FALSE;
	gint       spacing = 0 ;
	gint       padding = 0 ;

  cursor_std   = gdk_cursor_new(GDK_LEFT_PTR) ;
  cursor_clock = gdk_cursor_new(GDK_WATCH) ;
  cursor_sel   = gdk_cursor_new(GDK_CROSSHAIR) ;

	/* main window: */
	windowMain = gtk_window_new (GTK_WINDOW_TOPLEVEL);

#ifndef _USE_GTK_GL_AREA_
	accel = gtk_accel_group_new() ;
	gtk_window_add_accel_group(GTK_WINDOW(windowMain), accel) ;
#endif
      
	gtk_window_set_title(GTK_WINDOW(windowMain), FEM_TUI_RELEASE);
  gtk_container_set_border_width (GTK_CONTAINER (windowMain), 1);

	gtk_signal_connect (GTK_OBJECT (windowMain), "delete_event",
                      GTK_SIGNAL_FUNC (delete_event), NULL);
    
  gtk_signal_connect (GTK_OBJECT (windowMain), "destroy",
                      GTK_SIGNAL_FUNC (destroy), NULL);

	thebox = gtk_vbox_new(homogenous, spacing);
	gtk_container_add(GTK_CONTAINER(windowMain),GTK_WIDGET(thebox));
	gtk_widget_show(thebox);
	
	/* horizontal boxes */
	hbox_top_1 = gtk_hbox_new(homogenous, spacing);
	gtk_box_pack_start(GTK_BOX(thebox),hbox_top_1,expand, TRUE, padding);
	gtk_widget_show(hbox_top_1);
	
	hbox_top_2 = gtk_hbox_new(homogenous, spacing);
	gtk_box_pack_start(GTK_BOX(thebox),hbox_top_2,TRUE, TRUE, padding);
	gtk_widget_show(hbox_top_2);
	

	/* top line: */
	main_menu = gtk_menu_bar_new ();
	gtk_box_pack_start(GTK_BOX(hbox_top_1),main_menu,expand, TRUE, padding);
	gui_main_menu_file(main_menu);
#ifndef _USE_GTK_GL_AREA_
	gui_main_menu_plot(main_menu);
	gui_main_menu_hide(main_menu);
	gui_main_menu_list(main_menu);
	gui_main_menu_sele(main_menu);
  gui_main_menu_help(main_menu);
#endif
  gtk_widget_show (main_menu);
  

	cmdPrompt = gtk_label_new(_(" uFEM > ")) ;
	gtk_box_pack_start(GTK_BOX(hbox_top_1),cmdPrompt,expand, TRUE, padding);
	gtk_label_set_justify(GTK_LABEL(cmdPrompt), GTK_JUSTIFY_LEFT);
  gtk_widget_show (cmdPrompt);


	cmdLine = gtk_entry_new() ;
	gtk_box_pack_start(GTK_BOX(hbox_top_1),cmdLine,TRUE, TRUE, padding);
  gtk_widget_show (cmdLine);

  /* <Enter> in prompt */
  gtk_signal_connect(GTK_OBJECT(cmdLine), "activate",
                 GTK_SIGNAL_FUNC(cmdline_activate), NULL);

#ifndef _USE_GTK_GL_AREA_
  g_signal_connect(G_OBJECT(cmdLine), 
			        "key_press_event", G_CALLBACK(key_press_cmd), windowMain);
#endif


	cmdStatus = gtk_label_new(_("Ready...  ")) ;
	gtk_box_pack_start(GTK_BOX(hbox_top_1),cmdStatus,expand, TRUE, padding);
  gtk_widget_show (cmdStatus);
	

	/* bottom line: */
#ifndef _USE_GTK_GL_AREA_
	paned = gtk_hpaned_new();
	gtk_box_pack_start(GTK_BOX(hbox_top_2),paned,TRUE, TRUE, padding);
  gtk_widget_show (paned);

	vbox_left = gtk_vbox_new(homogenous, spacing) ;
	gtk_paned_pack1 (GTK_PANED (paned), vbox_left, FALSE, TRUE);
  /*gtk_widget_show (vbox_left);*/
	if (femCleanGUItree == AF_NO) { gtk_widget_show (vbox_left); }

	scrollWin = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox_left),scrollWin,TRUE, TRUE, padding);
  gtk_widget_show (scrollWin);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin),
			GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	cmdTree = fem_cmd_tree() ;
	gtk_container_add(GTK_CONTAINER(scrollWin),cmdTree);
	gtk_widget_show (cmdTree);

  hsepar = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox_left),hsepar,FALSE, TRUE, 8) ;
  gtk_widget_show(hsepar);
  
	pick_frame = gtk_frame_new(_("Mouse Selection"));
	gtk_box_pack_start(GTK_BOX(vbox_left),pick_frame,FALSE, TRUE, padding) ;

	bbox = gtk_hbutton_box_new();
	gtk_container_add(GTK_CONTAINER(pick_frame),bbox);
	gtk_widget_show (bbox);

	pick_run    = gtk_button_new_with_label(_("Done"));
	gtk_box_pack_start(GTK_BOX(bbox),pick_run,FALSE, TRUE, padding);
  g_signal_connect(G_OBJECT(pick_run), 
      "clicked",G_CALLBACK(fem_gui_cmd), (gpointer) "gfinish");
	gtk_widget_show(pick_run) ;

	pick_cancel = gtk_button_new_with_label(_("Cancel"));
	gtk_box_pack_end(GTK_BOX(bbox),pick_cancel,FALSE, FALSE, padding);
  g_signal_connect(G_OBJECT(pick_cancel), 
      "clicked",G_CALLBACK(fem_gui_cmd), (gpointer) "gcancel");
	gtk_widget_show(pick_cancel) ;

  /* buttons are disabled by default  */
	gtk_widget_set_sensitive(pick_run,    FALSE) ;
	gtk_widget_set_sensitive(pick_cancel, FALSE) ;

	gtk_widget_show (pick_frame);
#endif

#ifdef _USE_G2D_
	glArea = gArea_new() ;
#else
	glArea = glArea_new() ;
#endif

#ifdef _USE_GTK_GL_AREA_
	gtk_box_pack_start(GTK_BOX(hbox_top_2),glArea,TRUE, TRUE, padding);
#else
	gtk_paned_pack2 (GTK_PANED (paned), glArea, TRUE, TRUE);
#endif
  gtk_widget_show (glArea);

#ifndef _USE_G2D_
#ifdef _USE_GTK_GL_AREA_
	/*glButtons = gtk_toolbar_new(GTK_ORIENTATION_VERTICAL, GTK_TOOLBAR_ICONS) ;*/
	glButtons = gtk_toolbar_new(GTK_ORIENTATION_VERTICAL, GTK_TOOLBAR_TEXT) ;
#else
	glButtons = gtk_toolbar_new() ;
	gtk_toolbar_set_style(GTK_TOOLBAR(glButtons), GTK_TOOLBAR_TEXT);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(glButtons), GTK_ORIENTATION_VERTICAL);
#endif
	gtk_box_pack_end(GTK_BOX(hbox_top_2),glButtons,FALSE, TRUE, padding);
	gui_gl_toolbar(glButtons) ;

	if (femCleanGUIbutt == AF_NO) { gtk_widget_show (glButtons); }
#else
	femCleanGUIbutt = AF_NO ;
#endif /* _USE_G2D_ */

  gtk_window_set_focus(GTK_WINDOW(windowMain), GTK_WIDGET(cmdLine));
  
  gtk_quit_add_destroy (1, GTK_OBJECT(windowMain)); /* ? */

	/* minimal size */
#ifndef _USE_GTK_GL_AREA_
	gtk_widget_set_size_request (GTK_WIDGET(windowMain), 600, -1);
	/*gtk_widget_set_size_request(GTK_WIDGET(cmdTree), 200);*/
#else
	gtk_widget_set_usize(GTK_WIDGET(windowMain), 600,0);
	gtk_widget_set_usize(GTK_WIDGET(glArea), 400,200);
#endif
  /* geometry */
  if (guiProp.none != AF_YES)
  {
	  gtk_window_set_default_size (GTK_WINDOW (windowMain), guiProp.width,guiProp.height);
	  gtk_widget_set_uposition (GTK_WIDGET (windowMain), guiProp.x0,guiProp.y0);
  }

#ifndef _USE_GTK_GL_AREA_
  gtk_window_set_icon(GTK_WINDOW(windowMain), create_pixbuf()); /* app. icon */
#endif

  gtk_widget_show (windowMain);

  gdk_window_set_cursor(windowMain->window, cursor_std);

  return(rv);
}

void guiResizeMainWin(void)
{
#ifdef _USE_GTK_GL_AREA_
  gtk_window_set_default_size (GTK_WINDOW (windowMain), guiProp.width,guiProp.height);
#else
  gtk_window_resize (GTK_WINDOW (windowMain), guiProp.width,guiProp.height);
#endif
  gtk_widget_set_uposition (GTK_WIDGET (windowMain), guiProp.x0,guiProp.y0);
}

void guiShowPickFrame(void)
{
#ifndef _USE_GTK_GL_AREA_
  gdk_window_set_cursor(glArea->window, cursor_sel);
	gtk_widget_set_sensitive(pick_run,    TRUE) ;
	gtk_widget_set_sensitive(pick_cancel, TRUE) ;
#endif
}

void guiHidePickFrame(void)
{
#ifndef _USE_GTK_GL_AREA_
  gdk_window_set_cursor(glArea->window, cursor_std);
	gtk_widget_set_sensitive(pick_run,    FALSE) ;
	gtk_widget_set_sensitive(pick_cancel, FALSE) ;
#endif
}
#else /* end of _USE_GUI_ */

/* nothing to do if GUI is not used */
#endif 

/** Writes GUI-related part of data to a file
 * @part fw wile stream for writing
 * @return status
 */
int femGUIWriteConfig_data(char *fname, char *atype)
{
	int rv = AF_OK ;
#ifdef _USE_GUI_
#ifndef _USE_GLUT_UI_
#ifndef _USE_GTK_GL_AREA_
	gint x,y ,w, h ;
  FILE   *fw   = NULL ;

  errno = 0 ;

  if ((fw = fopen(fname,atype)) == NULL) { return(AF_ERR_IO); }

	fprintf(fw,"\n!* User interface properties:\n");

	gtk_window_get_position( GTK_WINDOW (windowMain), &x, &y	) ;
	gtk_window_get_size( GTK_WINDOW (windowMain), &w, &h	) ;
	fprintf(fw,"!* ggeom,%li,%li,%li,%li !* original size\n", 
			guiProp.x0, guiProp.y0, guiProp.width, guiProp.height);
	fprintf(fw,"ggeom,%i,%i,%i,%i !* actual size\n", x, y, w, h);

  if (fclose(fw) != 0) {return(AF_ERR_IO);}
#endif
#endif
#endif
	return(rv);
}

/* end of gui_main.c */

/*
   File name: gui_dlgs.c
   Date:      2004/01/09 21:21
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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
   
   FEM User Interface - dialogs

*/

#include "fem_gui.h"
#include "fdb_fem.h"
#include "fdb_edef.h"
#include "fdb_mdef.h"

#ifdef _USE_GUI_
extern int femGUIlearningTree ;

extern GdkCursor *cursor_std  ;
extern GdkCursor *cursor_clock;

#ifndef FEM_NEW_FILE_DLG
static GtkWidget *sele         = NULL ;
#endif

#ifndef _USE_GTK_GL_AREA_
GtkTreeStore      *treeStore;
static GtkWidget *dialogWin    = NULL ;
static GtkWidget *dialogWinBig = NULL ;

GtkWidget         **entry      = NULL ;
GtkWidget         **entry_big  = NULL ;
GtkWidget         **label      = NULL ;
long                entry_len  = 0 ;
long                entry_big_len  = 0 ;
long                label_len  = 0 ;

long  norep_len_big = 0 ;
long  rep_len_num   = 0 ;
long  rep_len_big   = 0 ;

GtkWidget          *propDlg    = NULL ;
GtkTreeStore       *propStore  = NULL ;


GtkWidget         *propTree = NULL ;
GtkTreeIter        propIter ;
#endif

extern long f_mat_check_stat_type(long mat_id, long type) ;
extern long f_rs_check_stat_type(long mat_id, long type) ;

void femDataPropList(char *title, char *cmd, 
									  long tab, long tab_col_id, long tab_col_typ,
                    char *vargrp);

int gui_run_cmd(char *cmd)
{
	int rv = AF_OK ;
	GdkRectangle update_rect;

  gdk_window_set_cursor(windowMain->window, cursor_clock);

  update_rect.x = cmdStatus->allocation.x ;
  update_rect.y = cmdStatus->allocation.y ;
  update_rect.width = cmdStatus->allocation.width ;
  update_rect.height = cmdStatus->allocation.height ;

	gtk_label_set_text(GTK_LABEL(cmdStatus), _("Wait..")) ;
  gtk_widget_draw((windowMain), &update_rect);
  gtk_widget_draw((cmdStatus), &update_rect);

  update_rect.x = cmdLine->allocation.x ;
  update_rect.y = cmdLine->allocation.y ;
  update_rect.width = cmdLine->allocation.width ;
  update_rect.height = cmdLine->allocation.height ;
  gtk_widget_draw((cmdLine), &update_rect);
  gtk_widget_draw((windowMain), &update_rect);

	if ((rv=ciRunCmd(cmd))  == AF_OK )
	{
	  gtk_label_set_text(GTK_LABEL(cmdStatus), _("  Done  ")) ;
	}
	else
	{
	  gtk_label_set_text(GTK_LABEL(cmdStatus), _(" Failed ")) ;
	}

  gdk_window_set_cursor(windowMain->window, cursor_std);

	return(rv);
}

/* ***************************************************** */
#ifdef FEM_NEW_FILE_DLG

void fem_dlg_file_react(char *outfile, gpointer data)
{
  if ( (outfile != NULL) && (data != NULL) )
  {
		gui_run_cmd(ciStrCat2CmdPers((char *)data, outfile)) ;
  }
  else
  {
    fprintf(msgout, "[E] %s!\n", _("Empty filename or action"));
  }
}

/** Fills string fname with valid filename
 * @param fname emty allocated string to fill
 * @param ext  file extension
 */
void fem_dlg_set_fname(char *fname, char *ext)
{
  if (femJobname == NULL)
  {
    sprintf(fname,"femfile.%s",ext);
  }
  else
  {
    if (strlen(femJobname) < 1)
    {
      sprintf(fname,"femfile.%s",ext);
    }
    else
    {
      sprintf(fname,"%s.%s",femJobname,ext);
    }
  }
}

void fem_dlg_file_open(GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog = NULL ;
  GtkFileFilter* lakmus = NULL ;
  char      *filename = NULL;
  int        open = AF_NO ;
  char       title[FEM_STR_LEN+1] ;
  char       fname[FEM_STR_LEN+1] ;
  char       cmd[FEM_STR_LEN+1] ;
  char       data0 ;
  int        i;

  for (i=0; i<=FEM_STR_LEN; i++) 
  {
    title[i]='\0';
    fname[i]='\0';
    cmd  [i]='\0';
  }

  /* TODO: determine action type and filename */

  if (data == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid command associated to dialog"));
    return ;
  }

  strncpy(cmd, (char *) data,FEM_STR_LEN);
  data0 = cmd[0] ;

  switch (data0)
  {
    case 'r':
    case 'R': /* resume */
      /*fem_dlg_set_fname(fname, "db") ;*/
      open = AF_YES ;
      lakmus = gtk_file_filter_new() ;
      gtk_file_filter_add_pattern(lakmus,"*.db") ;
      gtk_file_filter_set_name(lakmus,_("uFEM database")) ;
      break ;
    case 's':
    case 'S': /* save .. *.db */
      open = AF_NO ;
      fem_dlg_set_fname(fname, "db") ;
      lakmus = gtk_file_filter_new() ;
      gtk_file_filter_add_pattern(lakmus,"*.db") ;
      gtk_file_filter_set_name(lakmus,_("uFEM database")) ;
      break ;
    case 'g':
    case 'G': /* gl2something */
      if (strcmp(cmd,"gl2ps") == 0)
      {
        fem_dlg_set_fname(fname, "eps") ;
        lakmus = gtk_file_filter_new() ;
        gtk_file_filter_add_pattern(lakmus,"*.(ps|eps)") ;
        gtk_file_filter_set_name(lakmus,_("PostScript images")) ;
      }
      if (strcmp(cmd,"gl2ppm") == 0)
      {
        fem_dlg_set_fname(fname, "ppm") ;
        lakmus = gtk_file_filter_new() ;
        gtk_file_filter_add_pattern(lakmus,"*.ppm") ;
        gtk_file_filter_set_name(lakmus,_("Portable Pixmaps")) ;
      }
      if ((strcmp(cmd,"gl2tif") == 0)||(strcmp(cmd,"gl2tiff") == 0))
      {
        fem_dlg_set_fname(fname, "tif") ;
        lakmus = gtk_file_filter_new() ;
        gtk_file_filter_add_pattern(lakmus,"*.(tiff|tif)") ;
        gtk_file_filter_set_name(lakmus,_("Tagged Image Format files")) ;
      }

      open = AF_NO ;
      break ;
    case 'i':
    case 'I': /* IMport on INput... */
      if (strlen(data) < 2) 
      {
        open = AF_NO ; 
        break;
      }
      else
      {
        if ((cmd[1] == 'n')||(cmd[1] == 'N'))
        {
          /* input command script... */
          open = AF_YES ;
          lakmus = gtk_file_filter_new() ;
          gtk_file_filter_add_pattern(lakmus,"*.mac") ;
          gtk_file_filter_set_name(lakmus,_("uFEM command files")) ;
        }
        else
        {
          /* import data... */
          open = AF_YES ;
          lakmus = gtk_file_filter_new() ;

          gtk_file_filter_add_pattern(lakmus,"*.fem") ;
          gtk_file_filter_set_name(lakmus,_("uFEM solver input files")) ;
#if 0
          /* must NOT be used with current "import" command: */
          gtk_file_filter_add_pattern(lakmus,"*.msh") ;
          gtk_file_filter_set_name(lakmus,_("GMSH mesh files")) ;
#endif
        }
      }
      break ;
    case 'e':
    case 'E': /* export */
      open = AF_NO ;
      fem_dlg_set_fname(fname, "fem") ;
      break ;
    
    default: open = AF_NO ;
             break ;
  }

  if (open == AF_YES)
  {
    sprintf(title,"%s: ", _("Open file for the command"));
    strncat(title, (char *) data,FEM_STR_LEN);
    dialog = gtk_file_chooser_dialog_new (title,
				      (GtkWindow *)widget,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
  }
  else
  {
    sprintf(title,"%s: ", _("Save file for the command"));
    strncat(title, (char *) data,FEM_STR_LEN);
    dialog = gtk_file_chooser_dialog_new (title,
				      (GtkWindow *)widget,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);

    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
  }

  if (lakmus != NULL)
  {
    gtk_file_chooser_add_filter((GtkFileChooser *)dialog, lakmus) ;
    gtk_file_chooser_set_filter((GtkFileChooser *)dialog, lakmus) ;
  }

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), femDataDir);

  if (fname[0] != '\0') 
  { 
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), fname); 
  }

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    fem_dlg_file_react(filename, data); /* run command */

    g_free (filename);
    filename = NULL ; /* for sure */
  }

  gtk_object_destroy((GtkObject *)lakmus);

  gtk_widget_destroy (dialog);
}
#endif
/* ***************************************************** */

#ifndef FEM_NEW_FILE_DLG
/* Get the selected filename after <OK> is pressed */
void file_ok_sel( GtkWidget  *w, gpointer *command )
{
  char *outfile = NULL ;
  long  len, i ;

  gtk_widget_hide (sele);

#if 1
  /* stupid but works: */
  len = strlen(gtk_file_selection_get_filename(GTK_FILE_SELECTION(sele)));
  len++ ;

  if (len >1)
  {
    outfile = (char *)malloc(len*sizeof(char));
  }
  if (outfile != NULL)
  {
    for (i=0; i<len; i++){ outfile[i] = '\0'; }
    strncpy(outfile,gtk_file_selection_get_filename(GTK_FILE_SELECTION(sele)),len-1);
  }
#else
  /* this does NOT work for Gtk+-2.4 and newer: */
  outfile = (char *) gtk_file_selection_get_filename(GTK_FILE_SELECTION(sele));
#endif

  if ( (outfile != NULL) && (command != NULL) )
  {
		gui_run_cmd(ciStrCat2CmdPers((char *)command, outfile)) ;
  }
  else
  {
    fprintf(msgout, "[E] %s!\n", _("Empty filename or action"));
  }

  gtk_widget_destroy (sele);

  free(outfile) ;
  outfile = NULL ;
}
#endif

/** Selects file and runs command "data" on it
 * @param widget widget
 * @param data command string
 */
void gui_select_file ( GtkWidget *widget,
                       gpointer   data )
{
#ifndef FEM_NEW_FILE_DLG
  char title[FEM_STR_LEN+1] ;
  int i;

  for (i=0; i<=FEM_STR_LEN; i++) {title[i]='\0';}

  /*printf("%p = %s\n",data, (char *)data);*/

  sprintf(title,"%s: ", _("Selection of file for the command"));
  strncat(title, (char *) data,FEM_STR_LEN);

  sele = gtk_file_selection_new( title );

	/* dialog should be modal */
	gtk_window_set_modal(GTK_WINDOW(sele), GTK_WIN_POS_CENTER);

  /*gtk_file_selection_set_filename( GTK_FILE_SELECTION(sele), femDataDir) ;*/

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (sele)->ok_button),
                    "clicked", (GtkSignalFunc) file_ok_sel, 
                    data );
                
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
                (sele)->cancel_button),
                "clicked", (GtkSignalFunc) gtk_widget_destroy,
                GTK_OBJECT (sele));
                        
  gtk_widget_show(sele);
#else
  fem_dlg_file_open(widget, data) ;
#endif
}

/** File selection - wrapper for command interpreter */
void gui_file_select_command(char *command)
{
#ifndef FEM_NEW_FILE_DLG
  gui_select_file (windowMain, (gpointer)command ) ;
#else
  fem_dlg_file_open(windowMain, (gpointer) command) ;
#endif
}


/* Selection function for "tree" */
#ifndef _USE_GTK_GL_AREA_
void fem_tree_row_activated(
    GtkTreeView *treeview,
    GtkTreePath *path,
    GtkTreeViewColumn *collumn,
    gpointer user_data
    )
{
  GtkTreeIter iter;
  gchar *command = NULL;
  int rv = AF_OK ;

  if (gtk_tree_view_row_expanded( (GtkTreeView*)cmdTree, path))
  {
    gtk_tree_view_collapse_row(
        (GtkTreeView*)cmdTree, path);
  	gdk_window_set_cursor(windowMain->window, cursor_std);
    return;
  }
  else
  {
    gtk_tree_view_expand_row(
        (GtkTreeView*)cmdTree, path, FALSE);
  }

  gtk_tree_model_get_iter(GTK_TREE_MODEL(treeStore),&iter,path);
  gtk_tree_model_get(GTK_TREE_MODEL(treeStore),&iter,1,&command,-1);

  if (command != NULL)
  {

    if (strlen(command) > 0)
    {
      rv = gui_run_cmd(command) ;

      if (rv == AF_OK)
      {
	      gtk_label_set_text(GTK_LABEL(cmdStatus), _("O.K. ")) ;
      }
      else
      {
	      gtk_label_set_text(GTK_LABEL(cmdStatus), _("Failed ")) ;
      }
    }

    free(command);
    command = NULL ;

  }
}
#endif

GtkWidget *fem_cmd_tree(void)
{
	GtkWidget    *tree = NULL ;
#ifndef _USE_GTK_GL_AREA_
	int count = 0 ;
  /*GtkTreeStore      *treeStore;*/
  GtkTreeIter        treeIter ;
  GtkTreeIter        treeIterPar ;
  GtkTreeIter        treeIterPar0 ;
  GtkCellRenderer   *cellRenderer;
  GtkTreeViewColumn *treeColumn;
  GtkTreeSelection  *treeSelection;


	treeStore = gtk_tree_store_new(2,G_TYPE_STRING,G_TYPE_STRING);
	
	tree      = gtk_tree_view_new_with_model(GTK_TREE_MODEL(treeStore));

  g_signal_connect(G_OBJECT(tree), 
      "row_activated",G_CALLBACK(fem_tree_row_activated), NULL);

  /* preprocessor ------------------------- */
	gtk_tree_store_insert(treeStore,&treeIter,NULL,count++);
	gtk_tree_store_set(treeStore,&treeIter,0,_("Preprocessor"),1,"prep",-1);

    treeIterPar0 = treeIter ;
    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Element Types"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Add.."),1,"dialog,et",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,etdel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"etlist",-1);
#endif

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Real Sets"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Add.."),1,"dialog,rs",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Set Values.."),1,"dialog,r",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,rsdel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"rslist",-1);
#endif

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Materials"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Add.."),1,"dialog,mat",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Set Values.."),1,"dialog,mp",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,matdel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"mplist",-1);
#endif


    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Set/Time"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Set.."),1,"dialog,time",-1);

/* ------------------------------------------------------------------ */

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Keypoints"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create/Change.."),1,"dialog,k",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Pick By Mouse.."),1,"gcreate,k",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change By Mouse"),1,"gedit,k",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Copy.."),1,"dialog,kgen",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Move.."),1,"dialog,kmove",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Mirror.."),1,"dialog,kmirror",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,kdel",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete By Mouse"),1,"pick,k,kdel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"klist",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Plot"),1,"kplot",-1);
#endif



    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Geometric Entities"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Set default properties.."),1,"dialog,edef",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Set default division.."),1,"dialog,ddiv",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create/Change"),1,"dialog,ge",-1);
#endif

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create By Size.."),1,"dialog,gesize",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create By Mouse.."),1,"dialog,gcreate_ge",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change By Mouse"),1,"gedit,ge",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change Divisions By Mouse"),1,"gedit,gediv",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change Set/Time By Mouse.."),1,"dialog,gechset",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Copy.."),1,"dialog,gekgen",-1);

#ifndef DISABLE_UNFINISHED
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Mirror.."),1,"dialog,enmirror",-1);
#endif

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,gedel",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete By Mouse"),1,"pick,ge,gedel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"elist",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Plot"),1,"eplot",-1);
#endif


    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Meshing"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Mesh"),1,"mesh",-1);


/* ------------------------------------------------------------------ */


    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Nodes"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create/Change.."),1,"dialog,n",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Pick By Mouse.."),1,"gcreate,n",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change By Mouse"),1,"gedit,n",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Copy.."),1,"dialog,ngen",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Move.."),1,"dialog,nmove",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Mirror.."),1,"dialog,nmirror",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,ndel",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete By Mouse"),1,"pick,n,ndel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"nlist",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Plot"),1,"nplot",-1);
#endif


    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Elements"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Set default properties.."),1,"dialog,edef",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create/Change.."),1,"dialog,e",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create By Mouse.."),1,"dialog,gcreate_e",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change By Mouse"),1,"gedit,e",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change Set/Time By Mouse.."),1,"dialog,echset",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Copy.."),1,"dialog,engen",-1);

#ifndef DISABLE_UNFINISHED
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Mirror.."),1,"dialog,enmirror",-1);
#endif

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,edel",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete By Mouse"),1,"pick,e,edel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"elist",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Plot"),1,"eplot",-1);
#endif

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Displacements"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create/Change.."),1,"dialog,d",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create By Mouse.."),1,"dialog,gcreate_d",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change By Mouse"),1,"gedit,d",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change Set/Time By Mouse.."),1,"dialog,dchset",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Copy.."),1,"dialog,dgen",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Mirror.."),1,"dialog,dmirror",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,ddel",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete By Mouse"),1,"pick,d,ddel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"dlist",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Plot"),1,"dplot",-1);
#endif


    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Node Loads"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create/Change.."),1,"dialog,f",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create By Mouse.."),1,"dialog,gcreate_f",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change By Mouse"),1,"gedit,f",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change Set/Time By Mouse.."),1,"dialog,fchset",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Copy.."),1,"dialog,fgen",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Mirror.."),1,"dialog,fmirror",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,fdel",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete By Mouse"),1,"pick,f,fdel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"flist",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Plot"),1,"fplot",-1);
#endif


#ifndef DISABLE_UNFINISHED
    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Element Loads"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create/Change.."),1,"dialog,el",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create By Mouse"),1,"gcreate_el",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change By Mouse"),1,"gedit,el",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Change Set/Time By Mouse.."),1,"dialog,elchset",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Copy.."),1,"dialog,elgen",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Mirror.."),1,"dialog,elmirror",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,eldel",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete By Mouse"),1,"pick,el,eldel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"ellist",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Plot"),1,"elplot",-1);
#endif
#endif

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Acceleration"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create/Change.."),1,"dialog,accel",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete.."),1,"dialog,acceldel",-1);

#if 0
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List"),1,"accellist",-1);
#endif


    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Model Data"),1,"",-1);

      treeIterPar = treeIter;

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Resume.."),1,"filedialog,resu",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Save.."),1,"filedialog,save",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Test Data"),1,"datatest",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Export.."),1,"dialog,export",-1);
     


  /* solver       ------------------------- */
	gtk_tree_store_insert(treeStore,&treeIter,NULL,count++);
	gtk_tree_store_set(treeStore,&treeIter,0,_("Solver"),1,"",-1);

    treeIterPar0 = treeIter ;
    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Test Data"),1,"datatest",-1);

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Solve Current Data"),1,"solve",-1);

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Solve Selection"),1,"ssolve",-1);

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Non-linear Solution.. "),1,"dialog,solve",-1);


  /* postprocessor (general)  ------------- */
	gtk_tree_store_insert(treeStore,&treeIter,NULL,count++);
	gtk_tree_store_set(treeStore,&treeIter,0,_("Postprocessor"),1,"",-1);
     /* note that "fem_post_number_of_sets" variable must be defined! */

    treeIterPar0 = treeIter ;
#ifndef DISABLE_UNFINISHED
    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Start Postprocessor"),1,"gpost,fem_post_number_of_sets",-1);

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Read Results.."),1,"dialog,rread",-1);
#endif

#ifndef DISABLE_UNFINISHED
#endif
    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Load Step"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Set.."),1,"dialog,set",-1);
    

    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("List Results"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("DOF Solution"),1,"prdof",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Reactions"),1,"prrs",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Element Solution.."),1,"dialog,pres",-1);

#ifndef DISABLE_UNFINISHED
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Node Solution.."),1,"dialog,prns",-1);
#endif


    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Plot Results"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Deformed Shape"),1,"pldef",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Reactions"),1,"plrs",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Element Solution.."),1,"dialog,ples",-1);

#ifndef DISABLE_UNFINISHED
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Node Solution.."),1,"dialog,plns",-1);
#endif


    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Paths"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Create Path.."),1,"dialog,path",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List Paths"),1,"pathlist",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Delete Path.."),1,"dialog,pathdel",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Set Active Path.."),1,"dialog,actpath",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Add Node to Path.."),1,"dialog,pn",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Add Node By Mouse.."),1,"gcreate,p",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("Plot Results on Path.."),1,"dialog,prpath",-1);

      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("List Results on Path.."),1,"dialog,plpath",-1);

#ifndef WIN32
    gtk_tree_store_append(treeStore,&treeIter,&treeIterPar0);
    gtk_tree_store_set(treeStore,&treeIter,0,_("Save Graphics"),1,"",-1);

      treeIterPar = treeIter;
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("To PPM (bitmap).."),1,"filedialog,gl2ppm",-1);

#ifdef _USE_LIB_TIFF_
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("To TIFF (bitmap).."),1,"filedialog,gl2tiff",-1);
#endif

#ifdef USE_GL2PS
      gtk_tree_store_append(treeStore,&treeIter,&treeIterPar);
      gtk_tree_store_set(treeStore,&treeIter,0,_("To EPS (vector).."),1,"filedialog,gl2ps",-1);
#endif

#endif /* WIN32 */


	/* Rendering mode: */
  cellRenderer = gtk_cell_renderer_text_new();
  treeColumn = gtk_tree_view_column_new_with_attributes (_("Main Menu"),cellRenderer,"text",0,NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(tree),treeColumn);

	if (femGUIlearningTree == AF_YES)
	{
		/* Show attached commands: */
  	cellRenderer = gtk_cell_renderer_text_new();
  	treeColumn = gtk_tree_view_column_new_with_attributes (_("Command"),cellRenderer,"text",1,NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(tree),treeColumn);
	}

	/* ------------------------------------- */

  /* stripes: */
  gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree),TRUE);

  /* resizeable collumn */
  treeColumn = gtk_tree_view_get_column (GTK_TREE_VIEW(tree),0);
  gtk_tree_view_column_set_resizable(treeColumn,TRUE);

  /* selection mode */
  treeSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
  gtk_tree_selection_set_mode (treeSelection, GTK_SELECTION_SINGLE);

#endif
	return(tree);
}

/* Data input dialog (table) -------------------------------------------- */
#ifndef _USE_GTK_GL_AREA_

void close_cmd(GtkWidget *item, gpointer data)
{
	if (entry != NULL) {free(entry); entry= NULL ;}
	gtk_widget_destroy(dialogWin);
}

void close_cmd_big(GtkWidget *item, gpointer data)
{
	if (entry_big != NULL) {free(entry_big); entry_big = NULL ;}
	if (label != NULL) {free(label); label = NULL ;}
	gtk_widget_destroy(dialogWinBig);
}

void close_prop_cmd(GtkWidget *item, gpointer data)
{
	gtk_widget_destroy(propDlg);
}

void apply_cmd(GtkWidget *item, gpointer data)
{
	long i;
	char *tmp = NULL ;
	char len=0;

	if (data == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Command Undefined"));
		return;
	}
	else
	{
		len = strlen((char *)data);
	}
	
	for (i=0; i<entry_len; i++)
	{
		if ((GTK_IS_COMBO(entry[i])) == TRUE)
		{
			/* combo */
			if (
				((char *)gtk_entry_get_text(
										GTK_ENTRY(GTK_COMBO(entry[i])->entry)))
					!=NULL)
			{
				len += strlen((char *)gtk_entry_get_text(
							GTK_ENTRY(GTK_COMBO(entry[i])->entry)
							)) ;
			}
		}
		else
		{
			/* entry */
			if (
						((char *)gtk_entry_get_text(GTK_ENTRY(entry[i])))
					!=NULL)
			{
				len += strlen((char *)gtk_entry_get_text(GTK_ENTRY(entry[i]))) ;
			}
		}
	}

	len += (entry_len + 2) ;
	if ((tmp = ((char *)malloc((len+1)*sizeof(char)))) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Out of memory for command"));
	}
	for (i=0; i<=len; i++) {tmp[i]='\0';}

	strcpy(tmp,data);
	strcat(tmp,",");

	for (i=0; i<entry_len; i++)
	{
		if (GTK_IS_COMBO(entry[i]) == TRUE)
		{
			/* combo */
			if (
				((char *)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(entry[i])->entry)))
					!=NULL)
			{
				strcat(tmp,(char *)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(entry[i])->entry))) ;
			}
		}
		else
		{
			/* entry */
			if (
						((char *)gtk_entry_get_text(GTK_ENTRY(entry[i])))
					!=NULL)
			{
				strcat(tmp,(char *)gtk_entry_get_text(GTK_ENTRY(entry[i]))) ;
			}
		}
		if (i < (entry_len-1)) { strcat(tmp,","); }
	}

	gui_run_cmd(tmp) ;

	free(tmp);
	tmp = NULL ;

	/* gtk_widget_destroy(dialogWin); */
}


void ok_cmd(GtkWidget *item, gpointer data)
{
	if (dialogWin  != NULL) { gtk_widget_hide((GtkWidget *)dialogWin) ; }
	apply_cmd(item, data);
	close_cmd(item, data);
}
#endif

/** Dialog for: "command_and_params,VALUE1,VALUE2,..."*/
void femDataDialogSmall(char *title, char *cmd, 
									 long norep_len, 
									 char **norep_title,
									 char **norep_value,
									 long Apply)
{
#ifndef _USE_GTK_GL_AREA_
	long i, j ;
	GtkWidget *vbox  = NULL ;
	GtkWidget *bbox  = NULL ;
	GtkWidget *table = NULL ;
	GtkWidget *label1 = NULL ;
	GList     *items = NULL ;
	GtkWidget *ok_button = NULL ;
	GtkWidget *apply_button = NULL ;
	GtkWidget *cancel_button = NULL ;
	gint       spacing = 8 ;
	gint       padding = 8 ;

	entry = (GtkWidget **)malloc(norep_len*sizeof(*entry)) ;
	if (entry == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Out of memory for dialog"));
		entry_len = 0 ;
		return;
	}

	entry_len = norep_len ;

  dialogWin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW(dialogWin), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW(dialogWin), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(dialogWin), title);


	vbox = gtk_vbox_new(FALSE, spacing);
	gtk_container_add(GTK_CONTAINER(dialogWin),GTK_WIDGET(vbox));
	gtk_widget_show(vbox);

	/* Table */
	table = gtk_table_new(norep_len,3,AF_NO);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE, TRUE, padding);
	gtk_table_set_row_spacings(GTK_TABLE(table),4);
	gtk_table_set_col_spacings(GTK_TABLE(table),4);
	gtk_widget_show(table) ;

	for (i=0; i<norep_len; i++)
	{
	  label1 = gtk_label_new(norep_title[i]);
		gtk_label_set_justify(GTK_LABEL(label1),GTK_JUSTIFY_LEFT);
		gtk_table_attach(GTK_TABLE(table), label1, 0,1,i,i+1,
				GTK_FILL|GTK_SHRINK,GTK_FILL|GTK_SHRINK,0,0);
	  gtk_widget_show(label1);

		if (norep_value[i] == NULL)
		{
				/* Empty entry */
				entry[i] = gtk_entry_new();
	  		gtk_entry_set_editable(GTK_ENTRY(entry[i]), TRUE);
      	gtk_entry_set_text(GTK_ENTRY(entry[i]),"");
		}
		else
		{
			if (ciParNum(norep_value[i]) <= 1)
			{
				/* Entry */
				entry[i] = gtk_entry_new();
	  		gtk_entry_set_editable(GTK_ENTRY(entry[i]), TRUE);
      	gtk_entry_set_text(GTK_ENTRY(entry[i]),norep_value[i]);
			}
			else
			{
				/* Combo */
				for (j=0; j<ciParNum(norep_value[i]); j++)
				{
					items = g_list_append (items, ciGetParStrNoExpand(norep_value[i],j));
				}
				entry[i] = gtk_combo_new();
				gtk_combo_set_popdown_strings (GTK_COMBO (entry[i]), items);
				items=NULL ;
			}
		}
		 gtk_table_attach_defaults(GTK_TABLE(table), entry[i], 1,2,i,i+1);
		gtk_widget_show(entry[i]);
	}

	/* Buttons */
	bbox = gtk_hbutton_box_new();
	/*gtk_container_add(GTK_CONTAINER(vbox),GTK_WIDGET(bbox));*/
	gtk_box_pack_start(GTK_BOX(vbox),bbox,FALSE, TRUE, padding);
	gtk_widget_show(bbox);

	ok_button = gtk_button_new_with_label(_("OK"));
	gtk_box_pack_start(GTK_BOX(bbox),ok_button,FALSE, TRUE, padding);
  g_signal_connect(G_OBJECT(ok_button), 
      "clicked",G_CALLBACK(ok_cmd), (gpointer) cmd);
	gtk_widget_show(ok_button) ;

	if (Apply == AF_YES)
	{
		apply_button = gtk_button_new_with_label(_("Apply"));
		gtk_box_pack_start(GTK_BOX(bbox),apply_button,FALSE, TRUE, padding);
  	g_signal_connect(G_OBJECT(apply_button), 
      	"clicked",G_CALLBACK(apply_cmd), (gpointer) cmd);
		gtk_widget_show(apply_button) ;
	}

	cancel_button = gtk_button_new_with_label(_("Cancel"));
	gtk_box_pack_start(GTK_BOX(bbox),cancel_button,FALSE, TRUE, padding);
  g_signal_connect(G_OBJECT(cancel_button), 
      "clicked",G_CALLBACK(close_cmd), NULL);
	gtk_widget_show(cancel_button) ;


	
  g_signal_connect(G_OBJECT(dialogWin), 
      "delete_event",G_CALLBACK(close_cmd), NULL);

  gtk_widget_show (dialogWin);
#endif
}

/* ---------------------------------------------- */

#ifndef _USE_GTK_GL_AREA_
void apply_big_cmd(GtkWidget *item, gpointer data)
{
	long i,j,k,l, kk;
  char tmp[FEM_STR_LEN+1];
	char *cmd = NULL ;
	char *id = NULL ;

	if (data == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Command Undefined"));
		return;
	}

	cmd = ciGetParStr((char *)data,0) ;
	id  = ciGetParStr((char *)data,1) ;
	
	for (i=0; i<norep_len_big; i++)
	{
		if (GTK_IS_COMBO(entry_big[i]) == TRUE)
		{
			/* combo */
        gui_run_cmd(ciStrCat4CmdPers(
         cmd,
				 (char *)gtk_label_get_text(GTK_LABEL(label[i])),
				 id,
				 (char *)gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(entry_big[i])->entry))
              ));
		}
		else
		{
      /* entry  */
        gui_run_cmd(ciStrCat4CmdPers(
         cmd,
				 (char *)gtk_label_get_text(GTK_LABEL(label[i])),
				 id,
				 (char *)gtk_entry_get_text(GTK_ENTRY(entry_big[i]))
              ));
		}
	}


	for (k=1; k<=rep_len_num; k++)
  {
    if (k == 0)
    {
      kk = 0 ;
    }
    else
    {
      kk = norep_len_big ;
    }

    for (i=0; i<rep_len_big; i++)
	  {
      j = (norep_len_big + (k-1)*rep_len_big + i) ;

      for (l=0; l<=FEM_STR_LEN; l++) {tmp[l] = '\0'; }
      sprintf(tmp,"%li",k+1);

		  if (GTK_IS_COMBO(entry_big[j]) == TRUE)
		  {
			  /* combo */
          gui_run_cmd(ciStrCat5CmdPers(
          cmd,
				  (char *)gtk_label_get_text(GTK_LABEL(label[i+kk])),
					id,
				  (char *)gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(entry_big[j])->entry)),
					fdbFemStrFromInt(k)
                ));
		  }
		  else
		  {
        /* entry  */
          gui_run_cmd(ciStrCat5CmdPers(
          cmd,
				  (char *)gtk_label_get_text(GTK_LABEL(label[i+kk])),
					id,
				  (char *)gtk_entry_get_text(GTK_ENTRY(entry_big[j])),
					fdbFemStrFromInt(k)
                ));
		  }
	  }
  }
	free(cmd); cmd = NULL ;
	free(id); id = NULL ;
}

void ok_big_cmd(GtkWidget *item, gpointer data)
{
	if (dialogWinBig  != NULL){ gtk_widget_hide((GtkWidget *)dialogWinBig) ; }
	apply_big_cmd(item, data);
	close_cmd_big(item, data);
}
#endif

/** Dialog for: "command_and_params,VALUE1,VALUE2,..."*/
void femDataDialogBig(char *title, char *cmd, 
									 long norep_len, 
									 char **norep_title,
									 char **norep_value,
                   char rep_num,
									 long rep_len, 
									 char **rep_title,
									 char **rep_value,
									 long Apply)
{
#ifndef _USE_GTK_GL_AREA_
	long i, j, jj,k ;
	GtkWidget *vbox  = NULL ;
	GtkWidget *bbox  = NULL ;
	GtkWidget *table = NULL ;
	GList     *items = NULL ;
	GtkWidget *ok_button = NULL ;
	GtkWidget *apply_button = NULL ;
	GtkWidget *cancel_button = NULL ;
	gint       spacing = 8 ;
	gint       padding = 8 ;

	entry_big = (GtkWidget **)malloc((rep_len*rep_num+norep_len)*sizeof(GtkWidget *)) ;

	if (entry_big == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Out of memory for dialog"));
		entry_big_len = 0 ;
		return;
	}

	label = (GtkWidget **)malloc((rep_len+norep_len)*sizeof(GtkWidget *)) ;
  if (label == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Out of memory for dialog"));
		label_len = 0 ;
    free(entry_big);
		return;
	}

	entry_big_len = norep_len + rep_num*rep_len ;
	label_len = norep_len + rep_len ;
  norep_len_big = norep_len ;
  rep_len_num = rep_num ;
  rep_len_big = rep_len ;

  dialogWinBig = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW(dialogWinBig), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW(dialogWinBig), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(dialogWinBig), title);


	vbox = gtk_vbox_new(FALSE, spacing);
	gtk_container_add(GTK_CONTAINER(dialogWinBig),GTK_WIDGET(vbox));
	gtk_widget_show(vbox);

	/* Table */
	table = gtk_table_new(norep_len,3+2+2*rep_num,AF_NO);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE, TRUE, padding);
	gtk_table_set_row_spacings(GTK_TABLE(table),4);
	gtk_table_set_col_spacings(GTK_TABLE(table),4);
	gtk_widget_show(table) ;

	for (i=0; i<norep_len; i++)
	{
	  label[i] = gtk_label_new(norep_title[i]);
		gtk_label_set_justify(GTK_LABEL(label[i]),GTK_JUSTIFY_LEFT);
		gtk_table_attach(GTK_TABLE(table), label[i], 0,1,i,i+1,
				GTK_FILL|GTK_SHRINK,GTK_FILL|GTK_SHRINK,0,0);
	  gtk_widget_show(label[i]);

		if (norep_value[i] == NULL)
		{
				/* Empty entry_big */
				entry_big[i] = gtk_entry_new();
	  		gtk_entry_set_editable(GTK_ENTRY(entry_big[i]), TRUE);
      	gtk_entry_set_text(GTK_ENTRY(entry_big[i]),"");
		}
		else
		{
			if (ciParNum(norep_value[i]) <= 1)
			{
				/* Entry */
				entry_big[i] = gtk_entry_new();
	  		gtk_entry_set_editable(GTK_ENTRY(entry_big[i]), TRUE);
      	gtk_entry_set_text(GTK_ENTRY(entry_big[i]),norep_value[i]);
			}
			else
			{
				/* Combo */
				for (j=0; j<ciParNum(norep_value[i]); j++)
				{
					items = g_list_append (items, ciGetParStrNoExpand(norep_value[i],j));
				}
				entry_big[i] = gtk_combo_new();
				gtk_combo_set_popdown_strings (GTK_COMBO (entry_big[i]), items);
				items=NULL ;
			}
		}
		 gtk_table_attach_defaults(GTK_TABLE(table), entry_big[i], 1,2,i,i+1);
		gtk_widget_show(entry_big[i]);
	}

  for (k=0; k<rep_num; k++)
	{
  for (i=0; i<rep_len; i++)
	{

		if (k == 0)
		{
	  	label[i+norep_len_big] = gtk_label_new(rep_title[i]);
			gtk_label_set_justify(GTK_LABEL(label[i+norep_len_big]),GTK_JUSTIFY_LEFT);
			gtk_table_attach(GTK_TABLE(table), label[i+norep_len_big], 2,3,i,i+1,
					GTK_FILL|GTK_SHRINK,GTK_FILL|GTK_SHRINK,0,0);
	  	gtk_widget_show(label[i+norep_len_big]);
		}

    j = (norep_len_big + k*rep_len_big + i) ;

		if (rep_value[k*rep_len+i] == NULL)
		{
				/* Empty entry_big */
				entry_big[j] = gtk_entry_new();
	  		gtk_entry_set_editable(GTK_ENTRY(entry_big[j]), TRUE);
      	gtk_entry_set_text(GTK_ENTRY(entry_big[j])," ");
		}
		else
		{
			if (ciParNum(rep_value[k*rep_len+i]) <= 1)
			{
				/* Entry */
				entry_big[j] = gtk_entry_new();
	  		gtk_entry_set_editable(GTK_ENTRY(entry_big[j]), TRUE);
      	gtk_entry_set_text(GTK_ENTRY(entry_big[j]),rep_value[k*rep_len+i]);
			}
			else
			{
				/* Combo */
				for (jj=0; jj<ciParNum(rep_value[i]); jj++)
				{
					items = g_list_append (items, ciGetParStrNoExpand(rep_value[k*rep_len+i],jj));
				}
				entry_big[j] = gtk_combo_new();
				gtk_combo_set_popdown_strings (GTK_COMBO (entry_big[j]), items);
				items=NULL ;
			}
		}
		 gtk_table_attach_defaults(GTK_TABLE(table), entry_big[j], 3+k*2,4+k*2,i,i+1);
		gtk_widget_show(entry_big[j]);
	}
  }

	/* Buttons */
	bbox = gtk_hbutton_box_new();
	/*gtk_container_add(GTK_CONTAINER(vbox),GTK_WIDGET(bbox));*/
	gtk_box_pack_start(GTK_BOX(vbox),bbox,FALSE, TRUE, padding);
	gtk_widget_show(bbox);

	ok_button = gtk_button_new_with_label(_("OK"));
	gtk_box_pack_start(GTK_BOX(bbox),ok_button,FALSE, TRUE, padding);
  g_signal_connect(G_OBJECT(ok_button), 
      "clicked",G_CALLBACK(ok_big_cmd), (gpointer) cmd);
	gtk_widget_show(ok_button) ;

	if (Apply == AF_YES)
	{
		apply_button = gtk_button_new_with_label(_("Apply"));
		gtk_box_pack_start(GTK_BOX(bbox),apply_button,FALSE, TRUE, padding);
  	g_signal_connect(G_OBJECT(apply_button), 
      	"clicked",G_CALLBACK(apply_big_cmd), (gpointer) cmd);
		gtk_widget_show(apply_button) ;
	}

	cancel_button = gtk_button_new_with_label(_("Cancel"));
	gtk_box_pack_start(GTK_BOX(bbox),cancel_button,FALSE, TRUE, padding);
  g_signal_connect(G_OBJECT(cancel_button), 
      "clicked",G_CALLBACK(close_cmd_big), NULL);
	gtk_widget_show(cancel_button) ;


	
  g_signal_connect(G_OBJECT(dialogWinBig), 
      "delete_event",G_CALLBACK(close_cmd_big), NULL);

  gtk_widget_show (dialogWinBig);
#endif
}

/** Runs "r" or "mp" command dialog */
int femPrepDlg_R_MP(char *cmd, long id)
{
#ifndef _USE_GTK_GL_AREA_
	long i,j;
	long pos, datapos, type, len, len_rp ;
	long rep_sets;
	char *norep_title[1024];
	char *norep_value[1024];
	char *rep_title[1024];
	char *rep_value[1024];

	for (i=0; i<1024; i++)
	{
		norep_title[i] = NULL ;
		norep_value[i] = NULL ;
		rep_title[i] = NULL ;
		rep_value[i] = NULL ;
	}

	if (strcmp(cmd,"r") == 0)
	{
		if (fdbInputCountInt(RSET, RSET_ID, id, &pos) < 1)
		{ return(AF_ERR_EMP); }
		type = fdbInputGetInt(RSET, RSET_TYPE, pos);
		rep_sets = fdbInputGetInt(RSET, RSET_NVAL, pos) ;
		if (fdbInputCountInt(RVAL, RVAL_RSET, id ,&datapos) < 1) { return(AF_ERR_VAL); }

		len = fdbElementType[type].reals ;
		len_rp = fdbElementType[type].reals_rep ;

		for (i=0; i<len; i++)
		{
			norep_title[i] = 
				ciGetVarNameFromGrp(
				fdbFemStrFromInt(fdbElementType[type].r[i]),
				"real") ;
			norep_value[i] = fdbFemStrFromDbl(
					fdbInputGetDbl(RVAL,RVAL_VAL, datapos+i)
					);
		}

		if ((len_rp - len) > 0)
		{ 
			rep_sets = (long)( (rep_sets - len) / len_rp ) ;

		
		for (j=0; j<rep_sets; j++)
		{
			for (i=0; i<len_rp; i++)
			{
				if (j == 0)
				{
					rep_title[i] = 
						ciGetVarNameFromGrp(
						fdbFemStrFromInt(fdbElementType[type].r_rep[i]),
						"real") ;
				}
				rep_value[i+(j*len_rp)] = fdbFemStrFromDbl(
						fdbInputGetDbl( RVAL, RVAL_VAL, datapos+len+(i+(j*len_rp)))
						);
			}
		}
		}
		else
		{
			rep_sets = 0;
			len_rp   = 0;
		}
		femDataDialogBig(_("Real Set Data"), 
									 ciStrCat2CmdPers("r", fdbFemStrFromInt(id)),
									 len, 
									 norep_title,
									 norep_value,
                   rep_sets,
									 len_rp, 
									 rep_title,
									 rep_value,
									 AF_NO);
	}
	else
	{
		if (strcmp(cmd,"mp") != 0)
		{
			return(AF_ERR_VAL);
		}

		if (fdbInputCountInt(MAT, MAT_ID, id, &pos) < 0)
		{ return(AF_ERR_EMP); }
		type = fdbInputGetInt(MAT, MAT_TYPE, pos);
		rep_sets = fdbInputGetInt(MAT, MAT_NVAL, pos) ;
		if (fdbInputCountInt(MVAL, MVAL_MAT, id ,&datapos) < 1) { return(AF_ERR_VAL); }

		len = fdbMatType[type].vals ;
		len_rp = fdbMatType[type].vals_rp ;

		for (i=0; i<len; i++)
		{
			norep_title[i] = 
				ciGetVarNameFromGrp(
				fdbFemStrFromInt(fdbMatType[type].val[i]),
				"material") ;
			norep_value[i] = fdbFemStrFromDbl(
					fdbInputGetDbl(MVAL,MVAL_VAL, datapos+i)
					);
		}

		if ((rep_sets - len) > 0)
		{ 
			rep_sets = (long)( (rep_sets - len) / len_rp ) ; 

		/* ------------------------ */
		
		for (j=0; j<rep_sets; j++)
		{
			for (i=0; i<len_rp; i++)
			{
				if (j == 0)
				{
					rep_title[i] = 
						ciGetVarNameFromGrp(
						fdbFemStrFromInt(fdbMatType[type].val_rp[i]),
						"material") ;
				}

				rep_value[i+(j*len_rp)] = fdbFemStrFromDbl(
						fdbInputGetDbl(MVAL,MVAL_VAL, datapos+len+(i+(j*len_rp)))
						);
			}

		}
		}
		else
		{
			rep_sets = 0;
			len_rp   = 0;
		}
		/* ------------------------ */
		femDataDialogBig(_("Material Data"), 
									 ciStrCat2CmdPers("mp", fdbFemStrFromInt(id)),
									 len, 
									 norep_title,
									 norep_value,
                   rep_sets,
									 len_rp, 
									 rep_title,
									 rep_value,
									 AF_NO);

	}
	
	for (i=0; i<1024; i++)
	{
		if (norep_title[i]!=NULL) { free(norep_title[i]); norep_title[i]=NULL; }
		if (norep_value[i]!=NULL) { free(norep_value[i]); norep_value[i]=NULL; }
		if (rep_title[i]!=NULL) { free(rep_title[i]); rep_title[i]=NULL; }
		if (rep_value[i]!=NULL) { free(rep_value[i]); rep_value[i]=NULL; }
	}

#endif
	return(AF_OK) ;
}

#endif /* end of _USE_GUI_ */

/* end of gui_dlgs.c */

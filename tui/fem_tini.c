/*
   File name: fem_tini.c
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

	 FEM - text user interface

	 $Id: fem_tini.c,v 1.13 2004/11/11 21:41:36 jirka Exp $
*/

#include "fem_tui.h"
#include "cint.h"

extern tInData inputData ;
extern tTab InputTab[];
extern long InputTabLen ;
extern FILE *fdbPrnFile ;

extern int femCmdAutoResu ;

extern int fdbInputReadData(char *fname) ; /* from fdb_fem.h */

extern void fdbDefElemTypes(void);
extern void fdbDefMatTypes(void);

extern int fdbCreateInputTabHdr(void);
extern int my_main (int argc, char *argv[]) ;

extern int fem_create_cmd_aliases(void);
extern int fdbGeomEntInit(void);

long femUI_Mode = FEM_UI_MODE_PREP ; /* mode of work (default is preprocessor) */

/* Cleanup at exit */
void fem_ui_cleanup_at_exit(void)
{
	if (ciUseLog == 1)
	{
		if (fclose(ciLogFile) != 0)
	   	{ fprintf(msgout,"[E] %s!\n", _("Cannot close log file")); }
	}
}


int fem_tui_init(void)
{
  int  rv   = 0 ;
	long iLen = 0 ;
	long dLen = 0 ;

	/* Registering of cleanup function: */
	atexit(fem_ui_cleanup_at_exit);

	/* Default pointer for output file: */
	fdbPrnFile = stdout ;

	/* Definition of material and element types */
	fprintf(msgout, "[I] %s ..", _("Creating data types")) ;
	fdbDefElemTypes();
	fdbDefMatTypes();
	fdbGeomEntInit();
	fprintf(msgout, " %s\n", _("OK")) ;

	/* database start: */
	fprintf(msgout, "[I] %s ..", _("Creating database headers")) ;
	if ((rv = fdbCreateInputTabHdr()) != AF_OK)
	{
		fprintf(msgout, " %s\n", _("Failed")) ;
		fprintf(msgout,"[E] %s!\n", _("Cannot prepare database headers - EXITING"));
		return(rv) ;
	}
	fprintf(msgout, " %s\n", _("OK")) ;

	/* integer and floating-point dataspace  */
	fprintf(msgout, "[I] %s ..", _("Creating space for data")) ;
	fdbInitTableColNumbers(InputTab, InputTabLen, &iLen, &dLen) ;
	if ((rv = fdbIntInit(&inputData.intfld, iLen)) != AF_OK) 
	   { fprintf(msgout, " %s\n", _("Failed")) ; return(rv) ; }
	if ((rv = fdbDblInit(&inputData.dblfld, dLen) ) != AF_OK) 
	   { fprintf(msgout, " %s\n", _("Failed")) ; return(rv) ; }
	fprintf(msgout, " %s\n", _("OK")) ;

	/* support for "scripting": */
	fprintf(msgout, "[I] %s ..", _("Creating scripting tools")) ;
	  ciNullScriptField();
	fprintf(msgout, " %s\n", _("OK")) ;

	/* commands for interpreter: */
	fprintf(msgout, "[I] %s ..", _("Creating command interpreter")) ;
	if ((rv=fem_cmd_init()) != AF_OK) 
     {fprintf(msgout," %s!\n",_("Failed")); return(rv);};
  if ((rv=fem_create_cmd_aliases()) != AF_OK)
     {fprintf(msgout," %s!\n",_("Failed")); return(rv);};
	fprintf(msgout, " %s\n", _("OK")) ;

  /* resume database (if requested): */
  if (femCmdAutoResu == AF_YES)
  {
	  fprintf(msgout, "[I] %s:\n", _("Reading initial database")) ;
		if ((fdbInputReadData(ciSetPath(femGetDataDir(),femGetJobname(),"db"))) != AF_OK) 
    {
      fprintf(msgout,"[W] %s!\n",_("Invalid or empty file")); 
      femSetJobname("invalid") ;
    }
    else { fprintf(msgout, "[ ]  %s.\n", _("database is OK")) ; }
  }

  return(rv) ;
}

/* end of fem_tini.c */

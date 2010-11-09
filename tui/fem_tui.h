/*
   File name: fem_tui.h
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

	 $Id: fem_tui.h,v 1.39 2005/07/11 17:57:09 jirka Exp $
*/


#ifndef __FEM_TUI_H__
#define __FEM_TUI_H__

#include <stdio.h>
#include <stdlib.h>

#include "fem.h"
#include "fem_comm.h"
#include "fdb.h"

#define FEM_TUI_RELEASE "uFEM 0.2.53c"

#define FEM_TUI_PROMPT "uFEM >"

#define FEM_STR_LEN 2048

#define FEM_UI_MODE_PREP 1
#define FEM_UI_MODE_POST 2

#ifdef WIN32
#define FEM_SETUP_FILE "C:\\Program Files\\ufem\\tgfemrc.txt"
#endif

#define FEM_TEST_PREPROCESSOR \
	if (femUI_Mode != FEM_UI_MODE_PREP) \
	{ fprintf(msgout,"[w] %s: \"%s\".\n",\
		_("Switching to preprocessor mode"),cmd);\
	func_fem_prep(NULL) ; }

#define FEM_TEST_POSTPROCESSOR \
	if (femUI_Mode != FEM_UI_MODE_POST) \
  { fprintf(msgout,"[E] %s: \"%s\"!\n",\
		_("This command is available in postprocessor only"),cmd);\
	return(tuiCmdReact(cmd, AF_ERR_VAL)); }

extern long femUI_Mode ;

extern char *femDataDir   ;
extern char *femJobname   ;
extern char *femSolverPath;

extern int femCmdNumProcUI ;     /* number of processes for UI */

extern int tuiCmdReact(char *cmd, int rv);
extern int fem_cmd_init (void);
extern int fem_tui_init(void);

extern char *femGetJobname(void);
extern char *femGetDataDir(void);
extern char *femGetSolverPath(void);

extern int femSetDataDir(char *dir);
extern int femSetJobname(char *dir);
extern int femSetSolverPath(char *dir);

extern int femRunSolver(char *opts, char *infile, char *outfile);

extern int femUIWriteConfigFile(char *fname);

extern int func_fem_prep(char *cmd);

#endif

/* end of fem_tui.h */

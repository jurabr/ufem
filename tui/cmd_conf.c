/*
   File name: cmd_conf.c
   Date:      2008/07/15 19:01
   Author:    Jiri Brozovsky

   Copyright (C) 2008 Jiri Brozovsky

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

	 FEM User Interface - commands related to configuration
*/

#include "fem_tui.h"
#include "cint.h"
#include "fdb_fem.h"
#include "fdb_geom.h"
#include "fdb_edef.h"
#include "fdb_res.h"

extern int femCmdNumProcSolver ; /* number of processes for solver */
extern int femCmdNumProcUI ; /* number of processes for solver */


/* Sets number of threads for UI and solver: "nproc,N_iu,N_solver" */
int func_fem_set_num_proc(char *cmd)
{
	int  rv = AF_OK ;
	int  n_solver = 1 ;
	int  n_gui    = 1 ;

#if 0
	if (ciParNum(cmd) < 2)
	{
		fprintf(msgout,"[E] %s: %s,%s,%s!\n", 
				_("Number of threads must be specified"),
				ciGetParStr(cmd,0), _("for_ufem"), _("for_solver"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if (ciParNum(cmd) > 1)
  {
    if ((n_gui = ciGetParInt(cmd,1)) < 1)
    {
		  fprintf(msgout,"[W] %s: %i, %s: %i!\n", 
				_("Invalid number for uFEM"), n_gui,  
				_("using previous"), femCmdNumProcUI );
		  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
    }
  }
  else
  {
    femCmdNumProcUI = n_gui ;
  }


	if (ciParNum(cmd) > 2)
  {
    if ((n_gui = ciGetParInt(cmd,2)) < 1)
    {
		  fprintf(msgout,"[W] %s: %i, %s: %i!\n", 
				_("Invalid number for Solver"), n_solver,  
				_("using previous"), femCmdNumProcSolver );
		  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
    }
  }
  else
  {
    femCmdNumProcSolver = n_solver ;
  }


  fprintf(msgout,"[i] %s: %s = %i, %s = %i.\n",
      _("Number of allowed processes"),
      _("uFEM"), femCmdNumProcUI,
      _("Solver"), femCmdNumProcSolver
      );
#endif

  return ( tuiCmdReact(cmd, rv) ) ;
}


/* end of cmd_conf.c */

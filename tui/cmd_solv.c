/*
   File name: cmd_solv.c
   Date:      2008/02/26 34:27
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

	 FEM User Interface - commands related to solution and results
*/

#include "fem_tui.h"
#include "cint.h"
#include "fdb_fem.h"
#include "fdb_geom.h"
#include "fdb_edef.h"
#include "fdb_res.h"

extern FILE *fdbPrnFile ;  /* file pointer */
extern long  fdbOutputFormat ;
extern char *fdbPrnFileName ;
extern long  fdbPrnAutoName ;
extern long  fdbPrnAppendFile ;
extern char *fdbPrnViewCommand;

extern int femCmdNumProcSolver ; /* number of processes for solver */

/** Initialization of postprocessor in the simplest way: "gpost,number_of_result_sets"
 * @param cmd command
 * @return status
 */
int func_fem_post_simple(char *cmd)
{
	int  rv  = AF_OK ;
	long len = 0 ;


	femUI_Mode = FEM_UI_MODE_PREP ;
	ResActStep = 0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Number of result sets required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if (ciParNum(cmd) > 1) { len = ciGetParInt(cmd, 1); }

	if (len <= 0) 
	{
		fprintf(msgout,"[E] %s!\n", _("Number of result sets too small"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if (ResLen > 0) { fdbFreeResSpacePtrs(); }

	ResLen = len ;

	if ((rv=fdbAllocResSpacePtrs()) != AF_OK)
	{
		ResLen = 0 ;
		fprintf(msgout,"[E] %s!\n", _("Result initialisation failed"));
		return ( tuiCmdReact(cmd, rv) ) ;
	}

	femUI_Mode = FEM_UI_MODE_POST ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Sets active result set: "set,step_id"
 * @param cmd command
 * @return status
 */
int func_fem_post_set_set(char *cmd)
{
	int  rv  = AF_OK ;
	long stp = 0 ;
	long i = 0 ;
  char *s = NULL ;
	
	if ((ResLen <= 0) || (femUI_Mode != FEM_UI_MODE_POST)) 
	{
		fprintf(msgout,"[E] %s?\n", _("Cannot work with results - is postprocessor active"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Set number required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}


  stp = 0 ; /* for sure... */

	if ((s = ciGetParStr(cmd, 1)) != NULL)
  {
    if (strlen(s) >= 1)
    {
      if ((s[0]=='n')||(s[0]=='N')||(s[0]=='p')||(s[0]=='P')
          ||(s[0]=='l')||(s[0]=='L')||(s[0]=='f')||(s[0]=='F'))
      {
        switch (s[0])
        {
          case 'f':
          case 'F':
            stp = ResNode[0].set_id ; break ;
          case 'l':
          case 'L':
            stp = ResNode[ResLen-1].set_id ; break ;
          case 'n':
          case 'N':
            if ((ResActStep+1) < ResLen)
              { stp = ResNode[ResActStep+1].set_id ; }
            else
              { stp = ResNode[ResLen-1].set_id ; }
            break ;
          case 'p':
          case 'P':
            if ((ResActStep-1) >= 0)
              { stp = ResNode[ResActStep-1].set_id ; }
            else
              { stp = ResNode[0].set_id ; }
            break ;

          default:
            stp = 0 ; break ;
        }
      }
    }
  }

  if (stp <= 0)
  {
	  stp = ciGetParInt(cmd, 1);
  }

	if (stp <= 0) 
	{
		fprintf(msgout,"[E] %s!\n", _("Number is too small"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	ResActStep = -1 ;

	for (i=0; i<ResLen; i++)
	{
		if (ResNode[i].set_id == stp)
		{
			ResActStep = i ;
			break ;
		}
	}

	if (ResActStep == -1)
	{
		ResActStep = 0 ;
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s: \"%li\"!\n", _("Step not found"), stp);
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Sets active result set: "rread,step_num,filename" (yes|no means data are from MPI
 * @param cmd command
 * @return status
 */
int func_fem_post_read_set(char *cmd)
{
	int     rv  = AF_OK ;
	long    stp = 0 ;
	double  set_time = 0 ;
	long    i;
	char   *fname = NULL ;
	char    type_char = 'A' ;
	FILE   *fr = NULL ;

	/* several "nsel" and "esel" commands will be required here (load steps etc.) !!! */
	
	if ((ResLen <= 0) || (femUI_Mode != FEM_UI_MODE_POST)) 
	{
		fprintf(msgout,"[E] %s?\n", _("Cannot work with results - is the postprocessor active"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if (ciParNum(cmd) <= 2)
	{
		fprintf(msgout,"[E] %s!\n", _("Filename required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if (ciParNum(cmd) > 1) { stp   = ciGetParInt(cmd, 1); }

	if (ciParNum(cmd) > 2) { fname = ciGetParStr(cmd, 2); }

	if (strlen(fname) < 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid or empty filename"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if ((fr = fopen(fname,"r")) == NULL) 
	{
		fprintf(msgout,"[E] %s: \"%s\"!\n",_("Cannot open file"), fname);
		rv = AF_ERR_IO ;
		free(fname); fname = NULL ;
		return ( tuiCmdReact(cmd, rv) ) ;
	}
	else
	{
	  fscanf(fr,"%c", &type_char);
		fclose(fr);
	}

	if (stp <= 0) 
	{
		stp = -1 ;

		for (i=0; i<ResLen; i++)
		{
			if (ResNode[i].empty == AF_YES)
			{
				stp = i ;
				break ;
			}
		}

		if (stp == -1)
		{
			fprintf(msgout,"[E] %s!\n", _("Step data position number required"));
			return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
		}
	}

	set_time = (double) (stp + 1.0) ; /* should be changed later */

	switch (type_char)
	{
		case 'S':
		case 's': rv = fdbResReadStep(stp, set_time, fname);
							break ;
		case 'M':
		case 'm': rv = fdbResReadStepFromMPI(stp, set_time, fname);
							break ;
		default: rv = AF_ERR_VAL ; break ;
	}

	if (rv == AF_OK)
	{
		ResActStep = stp ;
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("Cannot load data"));
		ResActStep = 0 ;
	}

	free(fname); fname = NULL ;
	return ( tuiCmdReact(cmd, rv) ) ;
}

char *ret_minus_p(int t)
{
  if (t == 1) { return("-d") ; }
  else { return(" ") ; }
}

/** Executes solver on current data: "solve"
 * @param cmd command
 * @return status
 */
int func_fem_solve(char *cmd)
{
	int     rv  = AF_OK ;
  long    solver = 1 ;
  long    steps  = 1 ;
  long    iters  = 1 ;
  long    nsave  = 1 ;
  long    ntrack = 1 ;
  long    itersp = 0 ;
  char   *itestr = NULL ;
  char   *cmds   = NULL ;
  char    params [FEM_STR_LEN*2+1] ;
  long    tmp = 1 ;
  long    i, j ;
	div_t   d ;
	char    var[CI_STR_LEN+1];

	FEM_TEST_PREPROCESSOR

  /* check parameters here */
	if (ciParNum(cmd) <= 1)
  {
    /* no options - linear solver will be called */
    solver = 1 ;
  }
  else
  {
    if ((itestr=ciGetParStr(cmd,6)) != NULL) 
    { 
      ciStrCompr(itestr);
      if (strlen(itestr) >= 1)
      {
        if ((itestr[0] == 'y') || (itestr[0] == '1'))
          { itersp = 1 ; femOneDirSupp = 1 ; }
        else { itersp = 0 ; }
      }
      else { itersp = 0 ; }
			free(itestr); itestr = NULL ;
    }
    else { itersp = 0 ; }

    if ((solver=ciGetParInt(cmd, 1)) <= 1)
    {
      solver = 1 ;
    }
    else 
    {
      if ((steps=ciGetParInt(cmd,2)) <= 2) 
      { 
        if (solver > 1)
        {
          steps = 2 ; 
        }
      }
      else { steps = (long)(2*((double)steps/2.0)); }

      if ((iters=ciGetParInt(cmd,4)) <= 1) { iters = 333 ; }

      if ((nsave=ciGetParInt(cmd,4)) <= 1) { nsave = 1 ; }
      else { if (nsave > steps) {nsave=steps;}}

      if ((ntrack=ciGetParInt(cmd,5)) <= 1) { ntrack = 1 ; }
      if (ntrack > (tmp=fdbInputFindMaxInt(NODE, NODE_ID))){ntrack=tmp;}
    }
  }

  fprintf(msgout,"[I] %s.\n", 
      _("Your have called the solver. It will take some time. Be patient"));

  /** go to preprocessor: */

  ciRunCmd("hide,e"); /* gfx.. */

  ciRunCmd("prep");

  cmds = ciGetParStr(cmd,0) ;
  ciStrCompr(cmds) ;

  if (cmds[1] == 's')
  {
    /* only selected stuff is used: */
    ciRunCmd("nsle");
#if 0 /* probably not a good idea to change this: */
    ciRunCmd("dsln");
    ciRunCmd("fsln");
    ciRunCmd("elsle");
#endif
  }
  else
  {
    /* all available stuff is selected and used: */
    ciRunCmd("nsel,all");
    ciRunCmd("esel,all");
    ciRunCmd("fsel,all");
    ciRunCmd("dsel,all");
    ciRunCmd("gsel,all");
  }

  free(cmds); cmds = NULL ;

#if 0
  if ((rv=ciRunCmd("datatest")) != AF_OK)
  {
    fprintf(msgout,"[E] %s!\n", _("Data consistency check failed"));
	  return ( tuiCmdReact(cmd, rv) ) ;
  }
#endif

  /* must do first:
   * - save current data
   * - export data to fem
   * */
  rv = ciRunCmd(
      ciStrCat2CmdPers("save", ciSetPath(femGetDataDir(),femGetJobname(),"db"))
      ); /* "save,name.db" */

  if (rv == AF_OK )
  {
    rv = ciRunCmd(
      ciStrCat2CmdPers("export,fem", ciSetPath(femGetDataDir(),femGetJobname(),"fem"))
        ); /* "export,fem,name.fem" */
  }

  if (rv == AF_OK)
  {
    switch (solver)
    {
      case 0:
      case 1:
        for (i=0; i< (FEM_STR_LEN*2+1); i++) { params[i] = '\0' ; }
        sprintf(params,"-ssor -p %s -np %i ", ret_minus_p(itersp), femCmdNumProcSolver);
        rv = femRunSolver(
          params, 
          ciSetPath(femGetDataDir(), femGetJobname(), "fem"),
          ciSetPath(femGetDataDir(), femGetJobname(), "res")
          ) ;
        break;
      case 2:
      case 3:
      case 4:
      case 6:
        /* TODO: construct the parameters: */
        for (i=0; i< (FEM_STR_LEN*2+1); i++) { params[i] = '\0' ; }
        sprintf(params,"-ssor -p %s -nlnbrk -nls %li -nlstp %li -nlf %s -ose %li -osf %s -ln %li -lf %s -lrf %s -np %i",
            ret_minus_p(itersp),
            solver,
            steps,
            ciSetPath(femGetDataDir(), femGetJobname(), "norm"),
            nsave,
            ciSetPath(femGetDataDir(), femGetJobname(), "it"),
            ntrack,
            ciSetPath(femGetDataDir(), femGetJobname(), "ntrack"),
            ciSetPath(femGetDataDir(), femGetJobname(), "rtrack"),
            femCmdNumProcSolver
            );
        for (i=0; i< (FEM_STR_LEN*2+1); i++) {if(params[i]==','){params[i]='_';}}

#ifdef DEVEL_VERBOSE
        fprintf(msgout,"[d] Solver command line: \"%s\"\n",params);
#endif

        rv = femRunSolver(
          params, 
          ciSetPath(femGetDataDir(), femGetJobname(), "fem"),
          ciSetPath(femGetDataDir(), femGetJobname(), "res")
          ) ;

        break;
      default:
        fprintf(msgout,"[E] %s!\n", _("Solver failed - unsupported type of solution"));
	      return ( tuiCmdReact(cmd, AF_ERR) ) ;
        break;
    }
  }
  
  if (rv == AF_OK)
  {

    if (rv == AF_OK)
    {
			if (steps <= 1)
			{
    		rv = ciRunCmd("gpost,1") ;
  			ciAddVar("fem_post_number_of_sets","1");

      	rv = ciRunCmd(
      	ciStrCat2CmdPers("rread,", ciSetPath(femGetDataDir(),femGetJobname(),"res"))
          ) ; /* "rread,,name.res" */
			}
			else
			{
				/* we have multiple steps */
				if (nsave > 0)
				{
					for (i=0; i<CI_STR_LEN+1; i++) {var[i] = '\0';}
					sprintf(var,"gpost,%li",((long)(steps))+0);
    			rv = ciRunCmd(var) ;

					for (i=0; i<CI_STR_LEN+1; i++) {var[i] = '\0';}
					sprintf(var,"%li",(long)(steps)+0);
  				ciAddVar("fem_post_number_of_sets",var+0);

					for (i=1; i<=steps; i++)
					{
						d = div (steps, nsave) ;
						if (d.rem == 0.0)
						{
							/* TODO read result here */
							for (j=1; j<CI_STR_LEN+1; j++) {var[i] = '\0';}
							sprintf(var,"%s.it%li",femGetJobname(),i);

      				rv = ciRunCmd(
      				ciStrCat3CmdPers("rread",ciInt2Str(i-1),
								ciSetPath(femGetDataDir(), var, "res"))
          		) ; /* "rread,i,name.res" */

							fprintf(msgout,"[ ]   %s: (%4li) %s\n", _("Substep opened"),i,
								ciSetPath(femGetDataDir(), var, "res")

									);
						}
					}
				}
			}
    }
  }

  if (rv == AF_OK)
  {
    fprintf(msgout,"[I] %s.\n", _("Solver finished - enjoy the results"));
  }
  else
  {
    fprintf(msgout,"[E] %s!\n", _("Solver failed - observe output to find the problem"));
  }

  ciRunCmd("nplot"); /* gfx.. */
  ciRunCmd("eplot"); /* gfx.. */

	return ( tuiCmdReact(cmd, rv) ) ;
}


/* Starts postprocessor and tries to read load steps: "gpres,from,to "*/
int func_fem_gp_load_res(char *cmd)
{
	int   rv = AF_OK ;
	long  i, j ;
	long  steps = 1 ;
  long  nsave = 1 ;
  long  from  = 1 ;
  long  to    = 1 ;
	char  var[CI_STR_LEN+1];
	div_t d ;

	if (ciParNum(cmd) < 3)
	{
		fprintf(msgout,"[E] %s: %s,%s,%s!\n", 
				_("All parameters are required"),
				ciGetParStr(cmd,0), _("first"), _("last"));
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

  from = ciGetParInt(cmd,1) ; if (from < 1) { from = 1 ; }
  to   = ciGetParInt(cmd,2) ; if (to < from){ to = from ; }
  if (ciParNum(cmd) > 3)
  {
    nsave = ciGetParInt(cmd, 3) ; 
    if (nsave < 1) { nsave = 1 ; }
  }

  steps = to - from + 1 ;


		if (steps <= 1)
			{
    		rv = ciRunCmd("gpost,1") ;
  			ciAddVar("fem_post_number_of_sets","1");

      	rv = ciRunCmd(
      	ciStrCat2CmdPers("rread,", ciSetPath(femGetDataDir(),femGetJobname(),"res"))
          ) ; /* "rread,,name.res" */
			}
			else
			{
				/* we have multiple steps */
				if (nsave > 0)
				{
					for (i=0; i<CI_STR_LEN+1; i++) {var[i] = '\0';}
					sprintf(var,"gpost,%li",((long)(steps))+0);
    			rv = ciRunCmd(var) ;

					for (i=0; i<CI_STR_LEN+1; i++) {var[i] = '\0';}
					sprintf(var,"%li",(long)(steps)+0);
  				ciAddVar("fem_post_number_of_sets",var+0);

					for (i=from; i<=to; i++)
					{
						d = div (steps, nsave) ;
						if (d.rem == 0.0)
						{
							/* TODO read result here */
							for (j=1; j<CI_STR_LEN+1; j++) {var[i] = '\0';}
							sprintf(var,"%s.it%li",femGetJobname(),i);

      				rv = ciRunCmd(
      				ciStrCat3CmdPers("rread",ciInt2Str(i-1),
								ciSetPath(femGetDataDir(), var, "res"))
          		) ; /* "rread,i,name.res" */

							fprintf(msgout,"[ ]   %s: (%4li) %s\n", _("Substep opened"),i,
								ciSetPath(femGetDataDir(), var, "res")

									);
						}
					}
				}
			}

	return ( tuiCmdReact(cmd, rv) ) ;
}



/* Sets number of threads for UI and solver: "nproc,N_iu,N_solver" */
int func_fem_set_num_proc(char *cmd)
{
	int  rv = AF_OK ;
	int  n_solver = 1 ;
	int  n_gui    = 1 ;

#if 1
	if (ciParNum(cmd) < 2)
	{
		fprintf(msgout,"[E] %s: %s,%s,%s!\n", 
				_("Number of processes must be specified"),
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
    else
    {
      femCmdNumProcUI = n_gui ;
    }
  }


	if (ciParNum(cmd) > 2)
  {
    if ((n_solver = ciGetParInt(cmd,2)) < 1)
    {
		  fprintf(msgout,"[W] %s: %i, %s: %i!\n", 
				_("Invalid number for Solver"), n_solver,  
				_("using previous"), femCmdNumProcSolver );
		  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
    }
    else
    {
      femCmdNumProcSolver = n_solver ;
    }
  }


  fprintf(msgout,"[i] %s: %s = %i, %s = %i.\n",
      _("Number of allowed processes"),
      _("uFEM"), femCmdNumProcUI,
      _("Solver"), femCmdNumProcSolver
      );
#endif

  return ( tuiCmdReact(cmd, rv) ) ;
}




/** Exports data to solver's format: "export,format(fem),fname[,opts]"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_fem_export (char *cmd)
{
	int    rv = AF_OK ;
	char   *format  = NULL;
	char   *fname   = NULL;
	long    opts[1] ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("File type required")) ;
		return(AF_ERR_VAL);
	}

	if (ciParNum(cmd) > 1 ) {format = ciGetParStr(cmd, 1) ;}
  if (format == NULL)
  {
		fprintf(msgout,"[E] %s!\n", _("Empty format type")) ;
		return(AF_ERR_VAL);
  }
  if (strlen(format) < 1)
  {
		fprintf(msgout,"[E] %s!\n", _("Empty format type")) ;
		return(AF_ERR_VAL);
  }
  if ((format[0] == '\0')||(format[0] == ' ')||(format[0] == '\t')||(format[0] == ','))
  {
		fprintf(msgout,"[E] %s!\n", _("Invalid format type")) ;
		return(AF_ERR_VAL);
  }

	ciStrCompr(format);
  
	if (ciParNum(cmd) > 2 ) 
  {
    fname  = ciGetParStr(cmd, 2) ;

    if (fname == NULL)
    {
      fname = ciSetPath(femGetDataDir(),femGetJobname(),format) ;
    }
    else
    {
      if (strlen(fname) < 1)
      {
        fname = ciSetPath(femGetDataDir(),femGetJobname(),format) ;
      }
      else
      {

        if ((fname[0] == '\0')||(fname[0] == ' ')||(fname[0] == '\t')||(fname[0] == ','))
        {
          fname = ciSetPath(femGetDataDir(),femGetJobname(),format) ;
        }
      }
    }
  }
  else
  {
    fname = ciSetPath(femGetDataDir(),femGetJobname(),format) ;
  }

  fprintf(msgout,"[ ]  %s...\n",_("Export started - please wait"));

	if (strcmp(format,"fem") == 0)
	{
		opts[0] = AF_YES ;
		rv = fdbExport(fname, 1, opts, 1) ;
	}
	else
	{
    if (strcmp(format,"link3d") == 0)
	  {
		  opts[0] = AF_YES ;
		  rv = fdbExport(fname, 2, opts, 0) ;
	  }
	  else
	  {
			if (strcmp(format,"apdl") == 0)
	  	{
		  	opts[0] = AF_YES ;
		  	rv = fdbExport(fname, 3, opts, 0) ;
	  	}
			else
			{
				if (strcmp(format,"mac") == 0)
	  		{
		  		rv = fdbExport(fname, 4, NULL, 0) ;
	  		}
				else
				{
		  		rv = AF_ERR_VAL ;
				}
			}
    }
	}

  if (rv == AF_OK)
  {
    fprintf(msgout,"[ ]  %s: %s\n",_("Data exported to file"), fname);
  }

	free(format) ; format = NULL ;
	free(fname) ;  fname  = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}



/** Imports (some) data to solver's format: "import,format(fem),fname"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_fem_import(char *cmd)
{
	int    rv = AF_OK ;
	char   *format  = NULL;
	char   *fname   = NULL;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("File type required")) ;
		return(AF_ERR_VAL);
	}

	if (ciParNum(cmd) > 1 ) {format = ciGetParStr(cmd, 1) ;}
  if (format == NULL)
  {
		fprintf(msgout,"[E] %s!\n", _("Empty format type")) ;
		return(AF_ERR_VAL);
  }
  if (strlen(format) < 1)
  {
		fprintf(msgout,"[E] %s!\n", _("Empty format type")) ;
		return(AF_ERR_VAL);
  }
  if ((format[0] == '\0')||(format[0] == ' ')||(format[0] == '\t')||(format[0] == ','))
  {
		fprintf(msgout,"[E] %s!\n", _("Invalid format type")) ;
		return(AF_ERR_VAL);
  }

	ciStrCompr(format);
  
	if (ciParNum(cmd) > 2 ) 
  {
    fname  = ciGetParStr(cmd, 2) ;

    if (fname == NULL)
    {
      fname = ciSetPath(femGetDataDir(),femGetJobname(),format) ;
    }
    else
    {
      if (strlen(fname) < 1)
      {
        fname = ciSetPath(femGetDataDir(),femGetJobname(),format) ;
      }
      else
      {

        if ((fname[0] == '\0')||(fname[0] == ' ')||(fname[0] == '\t')||(fname[0] == ','))
        {
          fname = ciSetPath(femGetDataDir(),femGetJobname(),format) ;
        }
      }
    }
  }
  else
  {
    fname = ciSetPath(femGetDataDir(),femGetJobname(),format) ;
  }

  fprintf(msgout,"[ ]  %s...\n",_("Import started - please wait"));

	if (strcmp(format,"fem") == 0)
	{
		rv = fdbImport(fname, 1, NULL, 0) ;
	}
	else
	{
    if (strcmp(format,"gmsh") == 0)
    {
		  rv = fdbImport(fname, 2, NULL, 0) ;
    }
    else
    {
			if (strcmp(format,"netgen") == 0)
			{
		  	rv = fdbImport(fname, 3, NULL, 0) ;
			}
			else
			{
		  	fprintf(msgout,"[E] %s!\n", _("Invalid file type"));
      	rv = AF_ERR_VAL ; 
			}
    }
	}

  if (rv == AF_OK)
  {
    fprintf(msgout,"[ ]  %s: %s\n",_("Data exported to file"), fname);
  }

	free(format) ; format = NULL ;
	free(fname) ;  fname  = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/* end of cmd_fem.c */

/*
   File name: fem.c
   Date:      2003/04/07 09:01
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   in a file called COPYING along with this program; if not, write to
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
   02139, USA.

	 FEM Solver - main file

  $Id: fem.c,v 1.26 2004/12/31 23:09:25 jirka Exp $
*/

#include "fem.h"
#include "fem_mem.h"
#include "fem_dama.h"
#include "fem_sol.h"
#include "fem_para.h"
#include <string.h>

#ifndef _SMALL_FEM_CODE_
#include "fem_spnt.h"
extern long   femTestConstElem ;
extern long   femTestConstIpn  ;
extern int    femDryRun(void)  ;
extern int    femSolveDynNewmark(void);
#endif

#ifdef USE_MPI
#include "fem_mpi.h"
#endif


/** Main() function
 */
int main(int argc, char *argv[])
{
	int rv = AF_OK ;

#ifdef RUN_VERBOSE
	msgout = stdout ; /* for output from "fprintf(msgout,...)" */
#endif
	
	if ((rv = fem_parse_params(argc,argv)) != AF_OK) 
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("Command line michmach"));
#endif
		return(rv);
	}

#ifdef USE_MPI /* MPI CODE ---------------------------------- */
	
	/* Setting of data to NULL */
  if ((rv = femDataNull()) != 0) {return(rv);}
  if ((rv = femResNull()) != 0) {return(rv);}

	MPI_Init(&argc, &argv);
	femMPIwhoami(&ppRank, &ppSize);

	if (ppRank == 0)
	{
	  if ((rv = femReadInput(fem_ifile)) != AF_OK) 
		   { exit(rv); }
	}

  if ((rv = femSendInitDataPP()) != AF_OK) 
	{
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"[E] %s %i %s %i\n", _("Data sending error:"),rv,
		_("from process"),ppRank);
#endif
	   exit(rv);
	}

	if (solNoLinS > 1)
	{ 
		if ((rv = femSolveNRM_MPI(2)) != AF_OK) 
		{
#ifdef USE_MPI_LOG
		 printf("Solution returnes: %i\n",rv);
#endif
		 return(rv);
		} 
	}
	else
	{ 
		if ((rv = femSolveMPI()) != AF_OK) 
		{
#ifdef USE_MPI_LOG
		 printf("Solution returnes: %i\n",rv);
#endif
			return(rv);
		} 
	}


#ifndef _SMALL_FEM_CODE_
	femSaPoSave();
	femSaPoDestroy();
#endif

	MPI_Finalize();

#else /* end of MPI CODE ----------------------------------- */
	
	/* Setting of data to NULL */
  if ((rv = femDataNull()) != AF_OK) {return(rv);}
  if ((rv = femResNull()) != AF_OK) {return(rv);}
	if ((rv = femReadInput(fem_ifile)) != AF_OK) {return(rv);}

#ifndef _SMALL_FEM_CODE_
	femSaPoSetNode(0, nLen);
	if ((rv = femSaPoAlloc(solNoLinStepMax, 0)) != AF_OK) {return(rv);}
	femSaPoLen  = solNoLinStepMax ;
#endif


  if (femDynamics == AF_YES) /* dynamics */
  {
    /* TODO: modal analysis */
		if (femEigenModal == AF_YES)
		{
#if 1
    	rv = femSolveEigenInvIter(1500, 4) ;
#else
    	rv = femSolveEigenLanczos(1500, 4) ;
#endif
		}
		else
		{
			if (femNewmarkEL == AF_YES) /* Newmark: dynamics */
			{
				rv = femSolveDynNewmark() ;
			}
			else
			{
#ifdef DEVEL_VERBOSE
				fprintf(msgout,"[E] %s!", _("Unknown analysis type"));
#endif
				return(AF_ERR_VAL);
			}
		}
  }
  else 
  {
    if ((femTestConstIpn > -1) && (femTestConstElem > -1))
    {
      /* constitutive data testing */
      exit(femDryRun() ) ;
    }
    else /* statics */
    {
	    if (solNoLinS > 1)
	    {
        switch (solNoLinS)
        {
          case 2: if ((rv = femSolveNRM(0)) != AF_OK) {return(rv);} break;
          case 3: if ((rv = femSolveALM(2)) != AF_OK) {return(rv);} break;
          case 4: if ((rv = femSolveFullALM()) != AF_OK) {return(rv);} break;
          case 6: if ((rv = femSolveDirDisps(0)) != AF_OK) {return(rv);} break;
          default: return(AF_ERR_VAL); break ;
        }
      }
	    else
	      { if ((rv = femSolve()) != AF_OK) {return(rv);} }
    }
  }

#ifndef _SMALL_FEM_CODE_
	femSaPoSave();
	femSaPoDestroy();
#endif

#endif
	
	/* file names */
	free(fem_ifile); fem_ifile = NULL ;
	free(fem_ofile); fem_ofile = NULL ;
	return(rv);
}

/* end of fem.c */

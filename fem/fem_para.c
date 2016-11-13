/*
   File name: fem_para.c
   Date:      2003/05/26 23:42
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

	 FEM Solver - command line parameters handling

*/

#include "fem_para.h"

extern long   femTestConstElem ;
extern long   femTestConstIpn  ;
extern double femTestConstEX   ;
extern double femTestConstEY   ;
extern double femTestConstEZ   ;
extern double femTestConstEXY  ;
extern double femTestConstEYZ  ;
extern double femTestConstEZX  ;
extern double femTestConstA    ;

extern long  femFastBC ; /* "fast" work with boundary conditions */

long femPreparsedData  = AF_NO ; /* if input data are preparsed             */
long femReadStdInput   = AF_NO ; /* if input data comes through stdin       */
long femWriteStdOutput = AF_NO ; /* if output data are send to stdout       */
long femWriteStdThrOut = AF_NO ; /* if therm output vector is send to stdout*/
long femUseSaPo        = AF_NO ; /* if saves result data in selected point  */
long femReadPrevStep   = AF_NO ; /* if there are results from previous step */
long femReadPrevThr    = AF_NO ; /* if there are results from previous therm*/
long femPrevStdInput   = AF_NO ; /* if prev. step's results are from stdin  */
long femPrevThrStdIn   = AF_NO ; /* if prev. step's therm res. from stdin   */
long femExtraResOut    = AF_NO ; /* if extra LINEAR results will be written */
long femExtraResType   = 0     ; /* type of extra result data               */
long femComputePE      = AF_NO ; /* compute potential energy                */
long femTensorScale    = AF_NO ; /* prepare tensor scale data               */

long femTensorScaleDiv = 10 ;    /* prepare tensor scale division size      */

#ifdef _USE_THREADS_
long femUseThreads     = AF_NO ; /* if threads are used                 */
long femThreadNum      = 1 ;     /* number of threads                   */
long femThreadMin      = 1 ;     /* minimal size of vector/matrix       */
#endif

char *fem_ifile = NULL ; /* input file       */
char *fem_ofile = NULL ; /* results          */
char *fem_rfile = NULL ; /* previous results */

char *fem_thrfile = NULL ; /* previous thermal results */
char *fem_throfile = NULL ; /* thermal results saving */

char *fem_ssfile = NULL; /* substep result file */

char *fem_tsfile = NULL; /* tensor scale file */

#ifndef _SMALL_FEM_CODE_
char *fem_spec_out_file   = NULL ;
long  fem_spec_out_type   = 0 ;

#if _MATRIX_SAVING_
long  femSaveMatFStat     = AF_NO ;
long  femSaveSparMatFStat = AF_NO ;
long  femSaveSparMarketMatFStat = AF_NO ;
long  femSaveVecFStat     = AF_NO ;

char *femSaveMatFName     = NULL;
char *femSaveSparMatFName = NULL;
char *femSaveSparMarketMatFName = NULL;
char *femSaveVecFName     = NULL ;
#endif
#endif

long femSaveEveryNth      = 0 ;     /* save every Nth substep in NRM */

long  femBreakSolu        = AF_NO ; /* break linear solution after matrices were written - for MC etc. */

long  femDynamics         = AF_NO ; /* indicates non-static solution */
long  femEigenModal       = AF_NO ; /* modal solution */
long  femEigenNum         = 0 ;     /* number of computed mode shapess */
long  femEigenInvI        = AF_YES ; /* use inverse iterations method for 1st eigenvalue */
long  femNewmarkEL        = AF_NO ; /* newmark integration solver */
long  femComputePriceOnly = AF_NO ; /* computer price and exit    */
long  femThermTrans       = AF_NO ; /* indicates thermal transient solution */

FILE *fem_sol_norm_file   = NULL ;

/** Prints simple help to stdout
 * @param argc the same as "argc" from main
 * @param argv the same as "argv" from main
 */
void fem_help(int argc, char *argv[])
{
#ifdef RUN_VERBOSE
	fprintf(msgout,"\nuFEM %s: %s\n", _FEM_VERSION_, _("tool for finite element analysis"));
	fprintf(msgout,"(C) Jiri Brozovsky");
	fprintf(msgout,"\n\n");
	fprintf(msgout,"%s: %s [%s]\n\n %s:\n", _(" Usage"), argv[0], _("arguments"),
	  _("Arguments"));
	fprintf(msgout,"   -i  IFILE ... %s\n", _("read data from IFILE"));
	fprintf(msgout,"   -si       ... %s\n", _("read data from standard input"));
	fprintf(msgout,"   -p        ... %s\n", _("input data are preparsed (indexed)"));
	fprintf(msgout,"   -o  OFILE ... %s\n", _("write output data to OFILE"));
	fprintf(msgout,"   -so       ... %s\n", _("write output data to standard output"));
	fprintf(msgout,"   -e        ... %s\n", _("send all messages to stderr"));
	fprintf(msgout,"   -r  RFILE ... %s\n", _("read previous step results from RFILE"));
	fprintf(msgout,"   -sr       ... %s\n", _("read previous step results from standard input"));
#ifndef USE_MPI
	fprintf(msgout,"   -ose N    ... %s\n", _("save every N-th substep results (if applicable)"));
	fprintf(msgout,"   -osf NAME ... %s\n", _("template name for substeps results (if applicable)"));
#endif
#ifndef _SMALL_FEM_CODE_
	fprintf(msgout,"   -ln  NODE ... %s\n", _("set logged node to NODE (optional)"));
	fprintf(msgout,"   -lf  FILE ... %s\n", _("save data in logged node to FILE (required for -ln) "));
	fprintf(msgout,"   -lrf FILE ... %s\n", _("save sums of reactions to FILE"));
	fprintf(msgout,"   -lrso     ... %s\n", _("write sums of reactions to standard output"));
#if 0 /* it's too provocative */
	fprintf(msgout,"   -2t       ... %s\n", _("use 2nd order theory instead of 1st if possible"));
#endif
#endif
#ifdef _MATRIX_SAVING_
	fprintf(msgout,"   -mf  FILE ... %s\n", _("save global stiffness matrix to FILE (linear solution only) "));
	fprintf(msgout,"   -msf FILE ... %s\n", _("same as \"-mf\" but matrix is saved in sparse form "));
	fprintf(msgout,"   -mmf FILE ... %s\n", _("save global stiffness matrix in Matrix Market format"));
	fprintf(msgout,"   -vf  FILE ... %s\n", _("save global load vector to FILE (linear solution only) "));
#endif
#ifndef _SMALL_FEM_CODE_
	fprintf(msgout,"   -d        ... %s\n", _("iterative solution for compression/tension only b. conditions"));
#ifdef _USE_THREADS_
	fprintf(msgout,"   -t   N    ... %s\n", _("use \"N\" threads"));
	fprintf(msgout,"   -tm  N    ... %s\n", _("run threaded code only if vectors are larger than \"N\""));
#endif
#ifdef _MATRIX_SAVING_
	fprintf(msgout,"   -b        ... %s\n", _("halt program before DOF computation"));
#endif
	fprintf(msgout,"   -nls   N  ... %s\n", _("non-linear solver type (2..Newton-Raphson, 3..Arc-Length)"));
	fprintf(msgout,"   -nlstp N  ... %s\n", _("do N non-linear substeps (N have to be even)"));
	fprintf(msgout,"   -nlit  N  ... %s\n", _("do max N non-linear iteration in substep"));
	fprintf(msgout,"   -nlf FILE ... %s\n", _("save convergence norms (if any) to FILE"));
	fprintf(msgout,"   -nlnbrk   ... %s\n", _("do not break non-linear solution on unconverged steps"));
#ifndef USE_MPI
	fprintf(msgout,"   -mod N    ... %s\n", _("do modal analysis for first N eigenvalues"));
#if 0
	fprintf(msgout,"   -modl     ... %s\n", _("force Lanczos solver for rodal analysis"));
#endif

	fprintf(msgout,"   -nwm      ... %s\n", _("do dynamic analysis by Newmark integration"));
	fprintf(msgout,"   -tth      ... %s\n", _("do thermal transient analysis "));

	fprintf(msgout,"   -teste N  ... %s\n", _("run test solution only for element N"));
	fprintf(msgout,"   -testi M  ... %s\n", _("run \"-teste N\" for integration point M"));
	fprintf(msgout,"   -testx X  ... %s\n", _("given strain X for \"-teste\" "));
	fprintf(msgout,"   -testy Y  ... %s\n", _("given strain Y for \"-teste\" "));
#if 0
	fprintf(msgout,"   -testz Z  ... %s\n", _("given strain Z for \"-teste\" "));
	fprintf(msgout,"   -testyz YZ... %s\n", _("given strain YZ for \"-teste\" "));
	fprintf(msgout,"   -testzx ZX... %s\n", _("given strain ZX for \"-teste\" "));
#endif
	fprintf(msgout,"   -testxy XY... %s\n", _("given strain XY for \"-teste\" "));
	fprintf(msgout,"   -testa  A ... %s\n", _("area or volume for \"-teste\" (if needed) "));
#endif
	fprintf(msgout,"   -nes      ... %s\n", _("use equation solver for non-symetric systems"));
	fprintf(msgout,"   -ssor     ... %s\n", _("use SSOR instead of Jacobi preconditioning in CG solver"));

	fprintf(msgout,"   -fbc      ... %s\n", _("faster handling of fixed supports in statics (no reactions)"));

	fprintf(msgout,"   -ao FILE  ... %s\n", _("write alternative results of linear solution to FILE"));
	fprintf(msgout,"   -at TYPE  ... %s\n", _("type of alternative results (0 .. text, 1 .. VTK legacy)"));


	fprintf(msgout,"   -to FILE  ... %s\n", _("write thermal results to FILE"));
	fprintf(msgout,"   -tos      ... %s\n", _("write thermal results to stdout"));
	fprintf(msgout,"   -ti FILE  ... %s\n", _("read thermal results from FILE"));
	fprintf(msgout,"   -tis      ... %s\n", _("read thermal results from stdin"));
#endif /* end of _SMALL_FEM_CODE_*/
	fprintf(msgout,"   -po       ... %s\n", _("compute structure price and write it to stdout"));
	fprintf(msgout,"   -e        ... %s\n", _("compute potential energy and write it to stdout"));
	fprintf(msgout,"   -ts DIV   ... %s\n", _("prepare tensor scale data with X division DIV"));
	fprintf(msgout,"   -tso FILE ... %s\n", _("save tensor scale data to FILE"));
	fprintf(msgout,"   -h        ... %s\n", _("print this help"));
#endif
}

/** Sets input file name
 * @param source
 * @return filename (NULL on error)
 */
char *fem_set_iofile(char *str)
{
	static char *ifile = NULL ;

	if ((ifile = (char *)malloc(sizeof(char)*strlen(str)+1)) == NULL)
	{
		return(NULL);
	}
	else
	{
		strcpy(ifile,str);
		ifile[strlen(str)] = '\0' ;
		return(ifile);
	}
}

#ifndef _SMALL_FEM_CODE_
/** Sets filename for femSaPo data 
 *  @param fname name of file
 *  @return status
 */
char *femMiscFileName(char *fname, long *status_val)
{
  static char *name_string = NULL ;

	if ((name_string = (char *)malloc(sizeof(char)*(strlen(fname)+1))) == NULL)
	{
		*status_val = AF_NO ;
	}
	else
	{
		strcpy(name_string, fname);
		name_string[strlen(fname)] = '\0' ;
		*status_val = AF_YES ;
	}
	return(name_string);
}
#endif

/** Parses command line parameteres and sets some variables
 * @param argc the same as "argc" from main
 * @param argv the same as "argv" from main
 * @return state value (AF_OK)
 */
int fem_parse_params(int argc, char *argv[])
{
	int rv = AF_OK;
	int i;

#ifndef USE_MPW
	if (argc == 1)
	{
		fem_help(argc, argv);
		exit(AF_ERR_VAL);
	}
#endif

	if ((argc == 2) && (argv[1][0] != '-'))
	{
		if ((fem_ifile = fem_set_iofile(argv[1])) == NULL)
		{
			return(AF_ERR_VAL);
		}
	}

	for (i=1; i<argc; i++)
	{
		/* preparsed data */
		if (strcmp(argv[i],"-p") == 0)
		{
			femPreparsedData = AF_YES ;
		}

		/* input file  */
		if (strcmp(argv[i],"-i") == 0)
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((fem_ifile = fem_set_iofile(argv[i+1])) == NULL)
				{
					return(AF_ERR_VAL);
				}
			}
		}

		/* std input */
		if (strcmp(argv[i],"-si") == 0)
		{
			femReadStdInput = AF_YES ;
			fem_ifile = NULL ;
		}


		/* output file  */
		if (strcmp(argv[i],"-o") == 0)
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((fem_ofile = fem_set_iofile(argv[i+1])) == NULL)
				{
					return(AF_ERR_VAL);
				}
			}
		}

		/* std output  (implies msgout = stderr) */
		if (strcmp(argv[i],"-so") == 0)
		{
			femWriteStdOutput = AF_YES ;
			fem_ofile = NULL ;
#ifdef RUN_VERBOSE
			msgout = stderr ;
#endif
		}

		/* send messages to stderr */
		if (strcmp(argv[i],"-e") == 0)
		{
#ifdef RUN_VERBOSE
			msgout = stderr ;
#endif
		}

    /* previous step's file  */
		if (strcmp(argv[i],"-r") == 0)
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((fem_rfile = fem_set_iofile(argv[i+1])) == NULL)
				{
					return(AF_ERR_VAL);
				}
        femReadPrevStep = AF_YES ;
			}
		}
    
    /* std input */
		if (strcmp(argv[i],"-sr") == 0)
		{
			femPrevStdInput = AF_YES ;
      femReadPrevStep = AF_YES ;
			fem_rfile = NULL ;
		}

		/* SaPo handling */
#ifndef _SMALL_FEM_CODE_
		if (strcmp(argv[i],"-lf") == 0)
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if (femSaPoFileName(argv[i+1]) != AF_OK)
				{
					femUseSaPo = AF_NO ;
					return(AF_ERR_VAL);
				}
			}
		}

		if (strcmp(argv[i],"-ln") == 0)
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}

				femSaPoNode = atoi(argv[i+1]) ;
			}
		}

    /* file for saving of reactions: */
		if (strcmp(argv[i],"-lrf") == 0)
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if (femSumReactFileName(argv[i+1]) != AF_OK)
				{
					femSumReactName = NULL ;
					return(AF_ERR_VAL);
				}
        femSumReactStdOut = AF_NO ; /* use stdout? */
			}
		}

    /* saving of reactions to stdout: */
		if (strcmp(argv[i],"-lrso") == 0)
		{
#ifdef RUN_VERBOSE
      msgout = stderr ;
#endif
		  femSumReactName = NULL ;
      femSumReactStdOut = AF_YES ; /* use stdout? */
		}


    /* saving of stiffness matrix and/or load vector */
#ifdef _MATRIX_SAVING_
    if (strcmp(argv[i],"-mf") == 0) /* matrix */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femSaveMatFName=femMiscFileName(argv[i+1],&femSaveMatFStat)) == NULL)
				{
					return(AF_ERR_VAL);
				}
			}
		}

    if (strcmp(argv[i],"-msf") == 0) /* sparse matrix */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femSaveSparMatFName=femMiscFileName(argv[i+1],&femSaveSparMatFStat)) == NULL)
				{
					return(AF_ERR_VAL);
				}
			}
		}

    if (strcmp(argv[i],"-mmf") == 0) /* sparse matrix */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femSaveSparMarketMatFName=femMiscFileName(argv[i+1],&femSaveSparMarketMatFStat)) == NULL)
				{
					return(AF_ERR_VAL);
				}
			}
		}
    
    if (strcmp(argv[i],"-vf") == 0) /* vector */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femSaveVecFName=femMiscFileName(argv[i+1],&femSaveVecFStat)) == NULL)
				{
					return(AF_ERR_VAL);
				}
			}
		}
#endif

    /* 2nd order theory */
		if (strcmp(argv[i],"-2t") == 0)
		{
			fem2ndOrder = AF_YES ;
		}

    if (strcmp(argv[i],"-b") == 0)
		{
      femBreakSolu = AF_YES ;
		}

    /* substep's file template  */
		if (strcmp(argv[i],"-osf") == 0)
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((fem_ssfile = fem_set_iofile(argv[i+1])) == NULL)
				{
					return(AF_ERR_VAL);
				}

        if (femSaveEveryNth == 0)
        {
          femSaveEveryNth = 1 ;
        }
			}
		}

    if (strcmp(argv[i],"-ose") == 0) /* N for every Nth substep */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femSaveEveryNth = atoi(argv[i+1])) < 1) 
        {
          femSaveEveryNth = 1 ; 
					return(AF_ERR_SIZ);
        }
			}
		}


#ifdef _USE_THREADS_
    if (strcmp(argv[i],"-t") == 0) /* number of threads */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femThreadNum = atoi(argv[i+1])) < 1) 
        {
          femThreadNum = 1 ; 
          femUseThreads = AF_NO ;
					return(AF_ERR_SIZ);
        }
        else
        {
          if (femThreadNum > AF_MAX_THREADS)
          {
            femThreadNum = AF_MAX_THREADS ;
          }
          femUseThreads = AF_YES ;
          if (femThreadMin < 1)
          {
            femThreadMin = 1 ;
          }
        }
			}
		}

    if (strcmp(argv[i],"-tm") == 0) /* min. limit for threads */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femThreadMin = atoi(argv[i+1])) < 1) 
        {
          femThreadMin = 1 ; 
					return(AF_ERR_SIZ);
        }
			}
		}
#endif /* end of _USE_THREADS_ */

		/* iteration for b.c. */
		if (strcmp(argv[i],"-d") == 0)
		{
			femRunSolIterBC = AF_YES ;
		}

    if (strcmp(argv[i],"-nls") == 0) /* non-linear solver */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((solNoLinS_cmd = atoi(argv[i+1])) < 0) 
        {
          solNoLinS_cmd = 0 ; 
					return(AF_ERR_SIZ);
        }
			}
		}


    if (strcmp(argv[i],"-nlstp") == 0) /* non-linear substeps */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((solNoLinSteps_cmd = atoi(argv[i+1])) < 1) 
        {
          solNoLinSteps_cmd = 0 ; 
					return(AF_ERR_SIZ);
        }
        solNoLinSteps_cmd = (2*((long)((double)solNoLinSteps_cmd/2)));
        if (solNoLinSteps_cmd <= 0) {solNoLinSteps_cmd = 2; }
			}
		}

    if (strcmp(argv[i],"-nlit") == 0) /* non-linear iterations (might be ignored) */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((solNoLinIter = atoi(argv[i+1])) < 1) 
        {
          solNoLinIter = 0 ; 
					return(AF_ERR_SIZ);
        }
        solNoLinIter = (2*((long)((double)solNoLinIter/2)));
        if (solNoLinIter <= 0) {solNoLinIter = 333; }
			}
		}
    
    if (strcmp(argv[i],"-nlf") == 0) /* files for convergence norms */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
        femOpenSolNormFile(argv[i+1]);
			}
		}

    /* "unbreakable" non-linear solution */
		if (strcmp(argv[i],"-nlnbrk") == 0)
		{
			solNoBreakStep = AF_YES ;
		}


    if (strcmp(argv[i],"-mod") == 0) /* DYNAMICS: modal analysis (number of eigenvalues) */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femEigenNum = atoi(argv[i+1])) < 1) 
        {
          femEigenNum = 0 ;
          femEigenModal = AF_NO ;
          femNewmarkEL  = AF_NO ;
          femDynamics   = AF_NO ;
					return(AF_ERR_SIZ);
        }
        else
        {
          solNoLinS     = 1 ; /* we need data for linear solution */
          femEigenModal = AF_YES ;
          femDynamics   = AF_YES ;
          femNewmarkEL  = AF_NO ;
          femEigenInvI  = AF_YES ;
        }
			}
		}

    if (strcmp(argv[i],"-modl") == 0) /* DYNAMICS: Lanczos for modal analysis  */
		{
      solNoLinS     = 1 ; /* we need data for linear solution */
      femEigenModal = AF_YES ;
      femDynamics   = AF_YES ;
      femNewmarkEL  = AF_NO ;
      femEigenInvI  = AF_NO ;
      femEigenNum   = 1 ; /* just the first eigenvalue is computed! */
		}

    if (strcmp(argv[i],"-nwm") == 0) /* DYNAMICS: Newmark implicit integration */
		{
      solNoLinS     = 1 ; /* we need data for linear solution */
      femEigenModal = AF_NO ;
      femEigenNum   = 0 ;
      femDynamics   = AF_YES ;
      femNewmarkEL  = AF_YES ;
		}

    if (strcmp(argv[i],"-tth") == 0) /* THERMAL TRANSIENT */
		{
      solNoLinS     = 1 ; /* we need data for linear solution */
      femThermTrans = AF_YES ;
		}

    if (strcmp(argv[i],"-teste") == 0) /* TESTING: element number */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femTestConstElem = atoi(argv[i+1])) < 0) 
        {
					return(AF_ERR_VAL);
        }
			}
		}

    if (strcmp(argv[i],"-testi") == 0) /* TESTING: integration point */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femTestConstIpn = atoi(argv[i+1])) < 0) 
        {
					return(AF_ERR_VAL);
        }
			}
		}

    if (strcmp(argv[i],"-testx") == 0) /* TESTING: integration point */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				femTestConstEX = atof(argv[i+1]) ;
			}
		}
 
    if (strcmp(argv[i],"-testy") == 0) /* TESTING: integration point */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				femTestConstEY = atof(argv[i+1]) ;
			}
		}   

    if (strcmp(argv[i],"-testz") == 0) /* TESTING: integration point */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				femTestConstEZ = atof(argv[i+1]) ;
			}
		} 

    if (strcmp(argv[i],"-testxy") == 0) /* TESTING: integration point */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				femTestConstEXY = atof(argv[i+1]) ;
			}
		}

    if (strcmp(argv[i],"-testyz") == 0) /* TESTING: integration point */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				femTestConstEYZ = atof(argv[i+1]) ;
			}
		}

    if (strcmp(argv[i],"-testzx") == 0) /* TESTING: integration point */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				femTestConstEZX = atof(argv[i+1]) ;
			}
		}

    if (strcmp(argv[i],"-testa") == 0) /* TESTING: integration point */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femTestConstA = atof(argv[i+1])) <= 0.0) 
        {
					return(AF_ERR_VAL);
        }
			}
		}

    
    /* forced use of non-symetric solver */
		if (strcmp(argv[i],"-nes") == 0)
		{
			solUseBiCGs = AF_YES ;
		}

    /* forced use of SSOR preconditioner solver */
		if (strcmp(argv[i],"-ssor") == 0)
		{
			solUseCGSSOR = AF_YES ;
			solUseBiCGs  = AF_NO  ;
		}

		if (strcmp(argv[i],"-ao") == 0) /* alternative output: filename */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}

				if ((fem_spec_out_file = fem_set_iofile(argv[i+1])) == NULL)
				{
					return(AF_ERR_VAL);
				}
			}
		}

		if (strcmp(argv[i],"-at") == 0) /* alternative output: type */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}

				fem_spec_out_type = atoi(argv[i+1]) ;
				if (fem_spec_out_type < 0) {fem_spec_out_type = 0 ;}
			}
		}

    /* *** THERMAL LOADS **** */

    /* previous step's thermal data file  */
		if (strcmp(argv[i],"-ti") == 0)
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((fem_thrfile = fem_set_iofile(argv[i+1])) == NULL)
				{
					return(AF_ERR_VAL);
				}
        femReadPrevThr = AF_YES ;
			  femPrevThrStdIn = AF_NO ;
			}
		}

    /* std input for therm */
		if (strcmp(argv[i],"-tis") == 0)
		{
			femPrevThrStdIn = AF_YES ;
      femReadPrevThr = AF_YES ;
			fem_thrfile = NULL ;
		}

    /* thermal results file  */
		if (strcmp(argv[i],"-to") == 0)
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((fem_throfile = fem_set_iofile(argv[i+1])) == NULL)
				{
					return(AF_ERR_VAL);
				}
        femWriteStdThrOut = AF_NO ;
			}
		}

    /* std input for therm */
		if (strcmp(argv[i],"-tos") == 0)
		{
      femWriteStdThrOut = AF_YES ;
			fem_throfile = NULL ;
		}

    /* excules zero displacements from solution */
		if (strcmp(argv[i],"-fbc") == 0)
		{
      femFastBC = AF_YES ;
		}

    /* tensor scale file  */
		if (strcmp(argv[i],"-tso") == 0)
		{
      femTensorScale = AF_NO ;
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((fem_tsfile = fem_set_iofile(argv[i+1])) == NULL)
				{
					return(AF_ERR_VAL);
				}
        femTensorScale = AF_YES ;
			}
		}
    
    if (strcmp(argv[i],"-ts") == 0) /* set tensor scale division */
		{
			if (argc < (i+2)) 
			{
				return(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
					return(AF_ERR_VAL);
				}
				if ((femTensorScaleDiv = atoi(argv[i+1])) < 1) 
        {
          femTensorScaleDiv =  1 ;
					return(AF_ERR_SIZ);
        }
			}
		}

#endif /* end of _SMALL_FEM_CODE_ */

		if (strcmp(argv[i],"-po") == 0)
		{
			femComputePriceOnly = AF_YES ;
		}

		if (strcmp(argv[i],"-e") == 0)
		{
			femComputePE = AF_YES ;
		}



		/* help line */
		if (strcmp(argv[i],"-h") == 0)
		{
			fem_help(argc, argv);
			exit(0);
		}
	}

#ifdef USE_MPW 
  /* for Mac OS 8.x-9.x CLI apps (compiled by MPW): 
   * presets -p -is -so */
	femPreparsedData = AF_YES ;  /* -p  */
	femReadStdInput = AF_YES ;   /* -si */
	fem_ifile = NULL ;
	femWriteStdOutput = AF_YES ; /* -so */
	fem_ofile = NULL ;
#ifdef RUN_VERBOSE
	msgout = stderr ;
#endif
#endif /* end of USE_MPW */
	return(rv);
}

/** Returnes name for output file
 * @return filename
 */
char *fem_output_file(void)
{
	if (fem_ofile == NULL)
	{
		if (femWriteStdOutput == AF_YES)
		{
			return(NULL);
		}
		else
		{
			return("results.txt");
		}
	}
	else
	{
		return(fem_ofile);
	}
}

#ifndef _SMALL_FEM_CODE_
/** Returnes name of MC output file */
char *femMCGenFname(char *name, long sim, long mode)
{
  char *tmp = NULL;
  long len = 0 ;
  long i;

  len = strlen(name) + 10;

  if ((mode == 1) || (mode == 2))
  {
    len += 4 ;
  }
  else
  {
    len += 25 ;
  }
  
  len++ ;

  if ((tmp = (char *) malloc(len*sizeof(char))) == NULL)
  {
    return("");
  }

  for (i=0; i<len; i++) { tmp[i] = '\0' ; }

  if ((mode >= 1) && (mode <= 4))
  {
    sprintf(tmp,"%s.sum%1li",name, mode) ;
  }
  else
  {
    sprintf(tmp,"%s.r%li",name, sim) ;
  }

  return(tmp);
}
#endif

/** Returnes name of N-th number substep file */
char *femSubStepFname(long snumber)
{
  char *name         = NULL ;
  long  fem_ssfile_len    = 0 ;
  long  name_len     = 0 ;
  long  snumber_len  = 2 ;
  long  i ;

  if (fem_ssfile == NULL) {return(NULL);}
  if ((fem_ssfile_len=strlen(fem_ssfile)) <1) {return(NULL);}

  if (snumber >= 10e9) {return(NULL);} /* limits number of results */

  for (i=1; i<1e9; i++)
  {
    if ((snumber / pow(10,(double)i)) > 1)
    {
      snumber_len++ ;
    }
    else
    {
      break ;
    }
  }

  name_len = 5 + snumber_len + fem_ssfile_len ;

  if ((name = (char *)malloc(sizeof(char)*name_len)) == NULL)
  {
    return(NULL) ;
  }

  for (i=0; i<name_len; i++) {name[i] = '\0';}

  sprintf(name,"%s%li.res",fem_ssfile,snumber);

  for (i=0; i<name_len; i++) 
  {
    if ((name[i] == ' ')||
      (name[i] == '*')||
      (name[i] == '\\')||
      (name[i] == '\n')||
      (name[i] == '\t'))
    {
      name[i] = '0';
    }
  }

#ifdef DEVEL_VERBOSE
  fprintf(msgout,"Substep filename is \"%s\"\n",name);
#endif

  return(name);
}

/** Returnes name of N-th number substep file */
char *femSubStepMatrixFname(char *name0, long snumber)
{
  char *name         = NULL ;
  long  fem_ssfile_len    = 0 ;
  long  name_len     = 0 ;
  long  snumber_len  = 2 ;
  long  i ;

  if (name0 == NULL) {return(NULL);}
  if ((fem_ssfile_len=strlen(name0)) <1) {return(NULL);}

  if (snumber >= 10e9) {return(NULL);} /* limits number of results */

  for (i=1; i<1e9; i++)
  {
    if ((snumber / pow(10,(double)i)) > 1)
    {
      snumber_len++ ;
    }
    else
    {
      break ;
    }
  }

  name_len = 5 + snumber_len + fem_ssfile_len ;

  if ((name = (char *)malloc(sizeof(char)*name_len)) == NULL)
  {
    return(NULL) ;
  }

  for (i=0; i<name_len; i++) {name[i] = '\0';}

  sprintf(name,"%s%li.mat",name0,snumber);

  for (i=0; i<name_len; i++) 
  {
    if ((name[i] == ' ')||
      (name[i] == '*')||
      (name[i] == '\\')||
      (name[i] == '\n')||
      (name[i] == '\t'))
    {
      name[i] = '0';
    }
  }

#ifdef DEVEL_VERBOSE
  fprintf(msgout,"Substep filename is \"%s\"\n",name);
#endif

  return(name);
}

/** Tests if iteration have to be saved */
long femSubStepCheckNumber(long number)
{
  long num ;

  if (femSaveEveryNth == 1) { return(AF_YES); }
  if (femSaveEveryNth < 1) { return(AF_NO); }

  num = (long) (number / femSaveEveryNth) ;

  if ((num*femSaveEveryNth) == number)
  {
    return(AF_YES);
  }
  else
  {
    return(AF_NO);
  }
    
  return(AF_NO);
}

/* end of fem_para.c */

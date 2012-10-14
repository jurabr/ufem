/*
   File name: fem_nrm.c
   Date:      2007/02/05 15:17
   Author:    Jiri Brozovsky

   Copyright (C) 2007 Jiri Brozovsky

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

	 FEM solver: Newton-Raphson Method
*/

#include "fem_sol.h"

#define LASTNORMLEN 4

/* from fem_sol.c: */
extern void fem_sol_null(void);
extern void fem_sol_free(void);
extern int fem_sol_alloc(void);
extern int fem_sol_res_alloc(void);
extern int fem_dofs(void);
extern int fem_add_loads(long step);
extern int fem_add_disps(long disp_mode);
extern int fem_fill_K(long mode);

#ifndef _SMALL_FEM_CODE_
#ifdef _MATRIX_SAVING_
extern char *femSubStepMatrixFname(char *name0, long snumber);
#endif
#endif

extern long  nDOFAct  ; /* total number of ACTIVE DOFs in structure (== size of "K" matrix) */
extern long  nDOFlen  ; /* lenght of nDOFfld                        */
extern long *nDOFfld  ; /* description of DOFs in nodes             */
extern long *K_rows   ; /* numbers of items in "K" rows K_rows[nDOFAct] */
extern long *K_rowsAL ; /* K_rows for full ALM */

extern tMatrix K; /* structure stiffness matrix    */
extern tVector F; /* structure load vector         */
extern tVector Fr;/* unballanced forces vector     */
extern tVector u; /* structure displacement vector */
extern tVector u_tot; /* structure displacement vector - total displacements (NRM) */

extern tVector du0 ; /* substep displacement vector for ALM */
extern tVector dut ;  /* substep unballanced displacement vector for ALM */
extern tVector dur ;  /* substep unballanced displacement vector for ALM */
extern tVector du  ;  /* substep unballanced displacement vector for ALM */

extern tVector dFr  ;   /* step sum of unballanced forces for full ALM  */
extern tVector FF   ; /* whole load vector full ALM  */

extern tMatrix KF  ;  /* structure and step matrix for full ALM */
extern tVector uLa ;  /* displacement and lambda vectro for full ALM */
extern tVector Fra ;  /* load and arc lenght vector for full ALM */


/** Stop signal function for NRM 
 * @param sig_num (unused but required) signal number
 */
#ifdef _USE_SIGNALS_
void fem_nrm_signal_stop(int sig_num)
{
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i] NRM: %s.\n",_("stop signal received, saving and exiting"));
#endif

	femSaPoSave();

	if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); }
	femWriteRes( fem_output_file() );
	fem_sol_free();
	femDataFree();
	femResFree();
  femCloseSolNormFile();

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] NRM: %s.\n",_("Solution halted through signal"));
#endif
}

void fem_dds_signal_stop(int sig_num)
{
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i] DDS: %s.\n",_("stop signal received, saving and exiting"));
#endif

	femSaPoSave();

	if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); }
	femWriteRes( fem_output_file() );

	fem_sol_free();
	femDataFree();
	femResFree();
  femCloseSolNormFile();

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] DDS: %s.\n",_("Solution halted through signal"));
#endif
}
#endif

/** Computes multiplier for NRM
 * @param incr_type type of increment computation
 * @param i step number (1,2,..)
 * @param number of steps
 * @return multiplier value
 */
double femMultNRM(long incr_type, int i, int n)
{
	static double mult ;
	double x,x0;
	
	mult = 1.0 ;

  if (n == 1)
  {
    return(1.0) ;
  }

	switch (incr_type)
	{
		case 0: /* linear */
						if (femTangentMatrix == AF_YES)
							 { mult = 1.0 / ((double) n) ; }
						else
							 { mult = ((double) i) / ((double) n) ; }
						break;
		case 1:
						x  = (((double) i) / ((double) n)) * (0.5 * FEM_PI) ;

						if (femTangentMatrix == AF_YES)
						{
							x0 = (((double) (i-1)) / ((double) n)) * (0.5 * FEM_PI);
							mult = sin (x) - sin (x0);
						}
						else
						{
							mult = sin (x);
						}
		case 2:
						if (i <= (long)(0.5*n))
						{
							if (femTangentMatrix == AF_YES)
							{
								mult = 1.0 * (0.75 / (0.5 * (double)n)) ;
							}
							else
							{
								mult = (double)i * (0.75 / (0.5 * (double)n)) ;
							}
						}
						else
						{
							if (femTangentMatrix == AF_YES)
							{
								mult = 1.0 * (0.25 / (0.5 * (double)n)) ;
							}
							else
							{
								mult = (double)(i - 0.5*n) * (0.25 / (0.5 * (double)n)) ;
								mult += 0.75 ;
							}
						}
						break;
		default: /* unknown */
						mult = 1.0 ;
						break;
	}

	return(mult) ;
}

/** Non-linear analysis: Newton-Raphson Method
 * @param incr_type method for computation on load increment
 * @return state value
 */
int femSolveNRM(long incr_type)
{
	int rv = AF_OK;
	double multF = 1.0 ;   /* load multiplier      */
	double multSum = 0.0 ; /* total load multiplier*/
	double crit  = 2e-8 ;  /* convergence criteria (might be 2%) */
	double rsize = 1e20 ;  /* max residuum size    */
	double normF = 0.0 ;
	double normFr= 0.0 ;
	int steps    = 1;      /* number of steps      */
	int substeps = 334;    /* number of iterations */
	int nobreak  = AF_NO ;
	int converged = AF_NO ;
  long sum_jter = 0 ;
	int i,j;
#ifndef BLOCK_FOR_MPI
	long jb ;
#endif
	double lastnorm[LASTNORMLEN] ;
	long   lastnormpos = -1 ;
	long   lni ;

	/*crit     = solNoLinPre ;*/
	  steps    = solNoLinSteps ;
	  substeps = solNoLinIter ;
	/*rsize    = solNoLinBig ;*/
		nobreak  = solNoBreakStep ;

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] NRM %s:\n",_("Solution"));
#endif

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done _before_ adding of loads! */

	if (femTangentMatrix == AF_YES) { femVecSetZeroBig(&u_tot) ; }

  femGetSumReactInit(femSumReactName); /* sum of reactions */

	/* loop: */
	for (i=1; i<=steps; i++)
	{
		femVecSetZeroBig(&F);
		femVecSetZeroBig(&u);
		femMatSetZeroBig(&K);

		femVecSetZeroBig(&Fr);

		multF = femMultNRM(incr_type, i, steps);

		if (femTangentMatrix == AF_YES) { multSum += multF ; }
		else                            { multSum  = multF ; }

#ifdef RUN_VERBOSE
#if 0
		fprintf(msgout,"[ ] NRM %s %i (%s %i): %f\n",_("step"),i,_("from"),steps,multF);
#endif
#endif

 		if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 		if ((rv = fem_add_loads(0)) != AF_OK) { goto memFree; }
 		if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }
		femValVecMultSelf(multF, &F);

#if 0
printf("vec i [%e %e] %e\n", femVecGet(&F,9), femVecGet(&F,10), femVecNormBig(&F));
#endif

  
#ifndef _SMALL_FEM_CODE_
#ifdef _MATRIX_SAVING_
  if (femSaveSparMatFStat == AF_YES) 
  { 
    femSparseMatPrnF(femSubStepMatrixFname(femSaveSparMatFName,i), &K); 
  }
  if (femSaveVecFStat == AF_YES) 
  { 
    femVecPrnF(femSubStepMatrixFname(femSaveVecFName,i), &F); 
  }
#endif
#endif

    if (solUseBiCGs != AF_YES)
    {
			if (solUseCGSSOR != AF_YES)
			{
		  	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/100.0, nDOFAct*2)) != AF_OK) { goto memFree; }
			}
			else
			{
		  	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/100.0, nDOFAct*2)) != AF_OK) { goto memFree; }
			}
    }
    else
    {
		  if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/100.0, nDOFAct*2)) != AF_OK) { goto memFree; }
    }

#ifndef BLOCK_FOR_MPI
		if (femRunSolIterBC == AF_YES) { for (jb=0;jb<resRLen; jb++) { resRval0[jb] = resRval[jb]; } }
#endif

#if 0
printf("%i  u=%e normF=%e norm_u=%e\n",i,femVecGet(&u,3),femVecNormBig(&F),femVecNormBig(&u));
#endif

#if 1
		if (femTangentMatrix == AF_YES) { femVecAddVec(&u_tot, 1.0, &u) ; }
#endif

		normF  = femVecNormBig(&F)  ;

#ifdef _USE_SIGNALS_
		if (i == 1) { signal(SIGINT, fem_nrm_signal_stop); /* SIGNAL code */ }
#endif

		/* set last convergence norms to zero  **********************8 */
		for (lni=0; lni<LASTNORMLEN; lni++){lastnorm[lni]=0;}
		lastnormpos = -1 ;
		/* *************************************8********************8 */

		/* iteration: */
		for (j=1; j<=substeps; j++)
		{
      sum_jter++;

#ifdef RUN_VERBOSE
			fprintf(msgout,"[ ] NRM %s: %3i / %3i, %s: %3i / %3i\n",_("step"),i,steps,_("iteration"),j,substeps);
#endif

			femVecSetZeroBig(&Fr);
			femVecSetZeroBig(&F);

			femMatSetZeroBig(&K);
 			if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }

			normFr = femVecNormBig(&Fr) ;

#ifdef DEVEL_VERBOSE
      fprintf(msgout,"[ ] NRM [%i,%i]: norm(Fr) = %e\n",i,j, normFr);
#endif

      femWriteSolNorm(sum_jter, multSum, normFr, normF*crit,normF*rsize);

			if (normFr > (normF*rsize))
			{
				/* residuum too big */
				converged = AF_NO ;
#ifdef RUN_VERBOSE
				fprintf(msgout,"[E] NRM: %s (%e)!\n", _("Unconverged solution - residuum too large"),
						normFr);
#endif
				goto memFree;
			}

			/* convergence testing: */
			if ((normFr/normF) <= crit)
			{
				converged = AF_YES ;
				break ;
			}
			else
			{
				/* see last convergence norms: *****************************8 */
				if (lastnormpos < (LASTNORMLEN-1))
				{
					lastnormpos++ ;
					lastnorm[lastnormpos] = normFr ;
				}
				else
				{
					if (lastnormpos >= (LASTNORMLEN-1))
					{
						lastnormpos = (LASTNORMLEN-1) ; /* to be sure */
						for (lni=0; lni<(lastnormpos); lni++) 
						  	{ lastnorm[lni] = lastnorm[lni+1] ; }
						lastnorm[lastnormpos] = normFr ;
					}
				}

#if 0
				printf(" \n");
				for (lni=0; lni<(LASTNORMLEN); lni++) 
				{
					printf ("%e ",lastnorm[lni]) ;
				}
				printf(" \n");
#endif

				/* similar norms detection: */
				if (lastnormpos > 2)
				{
					if (
							fabs(
								(lastnorm[lastnormpos]-lastnorm[lastnormpos-2]
								)/lastnorm[lastnormpos])
							<= 0.01
							)
					{
						/* do something: */
#ifdef RUN_VERBOSE
						fprintf(msgout,"[W] %s [%i/%i] %e <> %e\n",
								_("Convergence norms are close at"),
								i,j,
								lastnorm[lastnormpos-2], lastnorm[lastnormpos]
								);
#endif
					}
				}
				/* end of  last convergence norms **************************8 */


				converged = AF_NO ;

				femMatSetZeroBig(&K);
 				if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
				
				if (femTangentMatrix == AF_YES)
				{
					femVecSetZeroBig(&F);
					femVecClone(&Fr, &F);
					femVecSetZeroBig(&Fr);

#if 0
					femValVecMultSelf(-1.0, &F) ; /* TODO WTF?! convention? */
#endif

					femVecSetZeroBig(&u);

 					if ((rv = fem_add_disps(AF_NO)) != AF_OK) { goto memFree; } /* is it OK?*/
				}
				else
				{
					femVecSetZeroBig(&Fr);
					femVecSetZeroBig(&F);

          /* Next two lines were interchanged. Is it correct? */
 					if ((rv = fem_add_loads(0)) != AF_OK) { goto memFree; }
					femValVecMultSelf(multF, &F);
 					if ((rv = fem_add_disps(AF_NO)) != AF_OK) { goto memFree; }
				}

#if 0
printf("vec j [%e %e] %e\n", femVecGet(&F,9), femVecGet(&F,10), femVecNormBig(&F));
#endif

        if (solUseBiCGs != AF_YES)
        {
					if (solUseCGSSOR != AF_YES)
					{
				  	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/100.0, nDOFAct*2)) != AF_OK) { goto memFree; }
					}
					else
					{
				  	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/100.0, nDOFAct*2)) != AF_OK) { goto memFree; }
					}
        }
        else
        {
				  if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/100.0, nDOFAct*2)) != AF_OK) { goto memFree; }
        }

#ifndef BLOCK_FOR_MPI
				if (femRunSolIterBC == AF_YES) { for (jb=0;jb<resRLen; jb++) { resRval0[jb] = resRval[jb]; } }
#endif

#if 0
printf("%i %i u=%e normF=%e norm_u=%e i\n",i,j,femVecGet(&u,3),femVecNormBig(&F),femVecNormBig(&u));
#endif
				
				if (femTangentMatrix == AF_YES) { femVecAddVec(&u_tot, 1.0, &u) ; }
			}
		}

		if (converged == AF_NO)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] NRM: %s: %i !\n", _("Unconverged step"), i);
#endif
			if (nobreak != AF_YES)
			{
				goto memFree;
			}
		}

#ifndef _SMALL_FEM_CODE_
		if (femTangentMatrix == AF_YES) 
		{
			femSaPoInput(multSum, 
				femVecGet(&u_tot,femKpos(femSaPoNode, U_X)),
				femVecGet(&u_tot,femKpos(femSaPoNode, U_Y)),
				femVecGet(&u_tot,femKpos(femSaPoNode, U_Z)),
				AF_NO,
				AF_NO
				) ;
		}
		else
		{
			femSaPoInput(multSum, 
				femVecGet(&u,femKpos(femSaPoNode, U_X)),
				femVecGet(&u,femKpos(femSaPoNode, U_Y)),
				femVecGet(&u,femKpos(femSaPoNode, U_Z)),
				AF_NO,
				AF_NO
				) ;
		}

    femGetSumReact(i, sum_jter, multSum, femSumReactName, femTangentMatrix); /* reactions */
#endif

#if 0
			fprintf(stderr,"%i %e %e %e %e\n",i, multSum, 
				femVecGet(&u_tot,femKpos(femSaPoNode, U_X)),
				femVecGet(&u_tot,femKpos(femSaPoNode, U_Y)),
				femVecGet(&u_tot,femKpos(femSaPoNode, U_Z)));
#endif

#if 1
      if (femSubStepCheckNumber(i) == AF_YES) /* substep results */
      {
	      if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); } /* really necessary? */
        solSimNum = multSum ;
	      if ((rv = femWriteRes(femSubStepFname(i))) != AF_OK)
        { 
          goto memFree; 
        }
	      if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); } /* really necessary? */
        solID++;

				if (femComputePE == AF_YES) { femCompPE(&K, &u, AF_YES ) ; }
      }
#endif
	}

	if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); }

  solSimNum = multSum ;
	if ((rv = femWriteRes( fem_output_file() )) != AF_OK) { goto memFree; }

	if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); } /* really necessary? */

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] NRM: %s.\n",_("Solution done"));
#endif

memFree:
	fem_sol_free();
	femDataFree();
	femResFree();
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[ ] NRM Solution return: %i\n",rv);
#endif
  femCloseSolNormFile();
	return(rv);
}

/** Direct displacement solver
 * @param incr_type type of load increment
 * @return solution status
 */ 
int femSolveDirDisps(long incr_type)
{
	int rv = AF_OK;
	double multF = 1.0 ;   /* load multiplier      */
	double multSum = 0.0 ; /* total load multiplier*/
	double crit  = 1e-6 ;  /* convergence criteria */
	double rsize = 1e18 ;  /* max residuum size    */
	double normF = 0.0 ;
	double normFr= 0.0 ;
	int steps    = 1;      /* number of steps      */
	int substeps = 667;    /* number of iterations */
	int nobreak  = AF_NO ;
	int converged = AF_NO ;
  long sum_jter = 0 ;
	int i ;
	int j = 0 ;

	/*crit     = solNoLinPre ;*/
	  steps    = solNoLinSteps ;
	  substeps = solNoLinIter ;
	/*rsize    = solNoLinBig ;*/
		nobreak  = solNoBreakStep ;

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] DDS %s:\n",_("Solution"));
#endif

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done _before_ adding of loads! */

	if (femTangentMatrix == AF_YES) { femVecSetZeroBig(&u_tot) ; }

  femGetSumReactInit(femSumReactName); /* sum of reactions */

	/* loop: */
	for (i=1; i<=steps; i++)
	{
		femVecSetZeroBig(&F);
		femVecSetZeroBig(&u);
		femMatSetZeroBig(&K);

		femVecSetZeroBig(&Fr);

		multF = femMultNRM(incr_type, i, steps);

		if (femTangentMatrix == AF_YES) { multSum += multF ; }
		else                            { multSum  = multF ; }

#ifdef RUN_VERBOSE
		fprintf(msgout,"[ ] DDS %s %i (%s %i): %f\n",_("step"),i,_("from"),steps,multF);
#endif

 		if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 		if ((rv = fem_add_loads(0)) != AF_OK) { goto memFree; }
 		if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }
		femValVecMultSelf(multF, &F);
  
#ifndef _SMALL_FEM_CODE_
#ifdef _MATRIX_SAVING_
  if (femSaveSparMatFStat == AF_YES) 
  { 
    femSparseMatPrnF(femSubStepMatrixFname(femSaveSparMatFName,i), &K); 
  }
  if (femSaveVecFStat == AF_YES) 
  { 
    femVecPrnF(femSubStepMatrixFname(femSaveVecFName,i), &F); 
  }
#endif
#endif

    if (solUseBiCGs != AF_YES)
    {
			if (solUseCGSSOR != AF_YES)
			{
		  	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10.0, nDOFAct*2)) != AF_OK) { goto memFree; }
			}
			else
			{
		  	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10.0, nDOFAct*2)) != AF_OK) { goto memFree; }
			}
    }
    else
    {
		  if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10.0, nDOFAct*2)) != AF_OK) { goto memFree; }
    }

#if 1
		if (femTangentMatrix == AF_YES) { femVecAddVec(&u_tot, 1.0, &u) ; }
#endif

		normF  = femVecNormBig(&F)  ;


#ifdef _USE_SIGNALS_
		if (i == 1) { signal(SIGINT, fem_dds_signal_stop); /* SIGNAL code */ }
#endif

    /* "iteration" */
    for (j=1; j<=substeps; j++)
		{
      sum_jter++;
#ifdef RUN_VERBOSE
			fprintf(msgout,"[ ] DDS step=%i iter=%i (from %i)\n",i,j,substeps);
#endif

			femMatSetZeroBig(&K);
 			if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }

			normFr = femVecNormBig(&Fr) ;

#ifdef DEVEL_VERBOSE
      fprintf(msgout,"[ ] DDS [%i,%i]: norm(Fr) = %e\n",i,j, normFr);
#endif

      femWriteSolNorm(sum_jter, multSum, normFr, normF*crit,normF*rsize);

#if 0
      if ((normFr/normF) <= crit)
			{
				converged = AF_YES ;
				break ;
			}
			else
			{
				converged = AF_NO ;

				femMatSetZeroBig(&K);
 				if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }

				femVecSetZeroBig(&F);
				femVecClone(&Fr, &F);
				femVecSetZeroBig(&Fr);

 			  if ((rv = fem_add_disps(AF_NO)) != AF_OK) { goto memFree; } /* is it OK?*/

        if (solUseBiCGs != AF_YES)
        {
					if (solUseCGSSOR != AF_YES)
					{
				  	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/100.0, nDOFAct*2)) != AF_OK) { goto memFree; }
					}
					else
					{
				  	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/100.0, nDOFAct*2)) != AF_OK) { goto memFree; }
					}
        }
        else
        {
				  if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/100.0, nDOFAct*2)) != AF_OK) { goto memFree; }
        }
      }
#else
		  converged = AF_YES ;
			break ;
#endif
		}

    
		if (converged == AF_NO)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] DDS: %s: %i !\n", _("Unconverged step"), i);
#endif
			if (nobreak != AF_YES)
			{
				goto memFree;
			}
		}

#ifndef _SMALL_FEM_CODE_
		if (femTangentMatrix == AF_YES) 
		{
			femSaPoInput(multSum, 
				femVecGet(&u_tot,femKpos(femSaPoNode, U_X)),
				femVecGet(&u_tot,femKpos(femSaPoNode, U_Y)),
				femVecGet(&u_tot,femKpos(femSaPoNode, U_Z)),
				AF_NO,
				AF_NO
				) ;
		}
		else
		{
			femSaPoInput(multSum, 
				femVecGet(&u,femKpos(femSaPoNode, U_X)),
				femVecGet(&u,femKpos(femSaPoNode, U_Y)),
				femVecGet(&u,femKpos(femSaPoNode, U_Z)),
				AF_NO,
				AF_NO
				) ;
		}

#if 1
    femGetSumReact(i, j, multSum, femSumReactName, femTangentMatrix); /* reactions */
#endif
#endif

      if (femSubStepCheckNumber(i) == AF_YES) /* substep results */
      {
	      if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); } /* really necessary? */
        solSimNum = multSum ;
	      if ((rv = femWriteRes(femSubStepFname(i))) != AF_OK)
        { 
          goto memFree; 
        }
	      if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); } /* really necessary? */
        solID++;
      }
	}


	if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); }
  solSimNum = multSum ;
	if ((rv = femWriteRes( fem_output_file() )) != AF_OK) { goto memFree; }
	if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); } /* really necessary? */

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] DDS: %s.\n",_("Solution done"));
#endif


memFree:
	fem_sol_free();
	femDataFree();
	femResFree();
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[ ] DDS Solution return: %i\n",rv);
#endif
  femCloseSolNormFile();

	return(rv);
}
/* end of fem_nrm.c */

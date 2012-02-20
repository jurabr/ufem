/*
   File name: fem_alm.c
   Date:      2007/02/05 15:24
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

	 FEM solver: Arc-Lenght Method
*/

#include "fem_sol.h"

/* from fem_sol.c: */
extern void fem_sol_null(void);
extern void fem_sol_free(void);
extern int fem_sol_alloc(void);
extern int fem_sol_res_alloc(void);
extern int fem_dofs(void);
extern int fem_add_loads(void);
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

extern tVector F_0 ;  /* backup load vector for ALM and full ALM */

extern tVector dFr  ;   /* step sum of unballanced forces for full ALM  */
extern tVector FF   ; /* whole load vector full ALM  */

extern tMatrix KF  ;  /* structure and step matrix for full ALM */
extern tVector uLa ;  /* displacement and lambda vectro for full ALM */
extern tVector Fra ;  /* load and arc lenght vector for full ALM */



/* Bergam's parameter of actuall stiffness (stiffness computation) */
double alm_bergam_k(double d_la_0, tVector *F, tVector *dut)
{
	if (femVecNormBig(dut) > 0.0)
	{
		return(d_la_0 * femVecVecMultBig(F, dut) / femVecVecMultBig(dut, dut) );
	}
	else
	{
		return(0.0);
	}
}


/** Non-linear analysis: Arc-Lenght Method (full version with non-symmetric matrix)
 * 3rd attempt to make it working
 * @param incr_type method for computation on load increment
 * @return state value
 */
int femSolveFullALM(void)
{
	int    rv         = AF_OK;
	long i, j, k;

	int steps       = 1;      /* number of steps      */
	int substeps    = 333;    /* number of iterations */
	double crit     = 1e-4 ;  /* convergence criteria */
	int nobreak  = AF_NO ;
	int converged   = AF_NO ;
  long sum_jter   = 0 ;
	long prev_steps = 5 ;
	long opt_steps  = 5 ;

  double  psi2        = 0.01 ;
  double  lambda_sum  = 0.0 ;
  double  lambda_step = 0.0 ;
  double  d_lambda    = 0.0 ;
  double  prev_lambda = 0.0 ;
  double  normF, normFr, a_o, sqvar, dL2, f_f ;

	long    need_repeat = AF_NO ;
	long    repeat_num = 0 ;
	long    max_repeat = 6 ;

#ifndef BLOCK_FOR_MPI
	long   jb ;
#endif

	/*crit     = solNoLinPre ;*/
	  steps    = solNoLinSteps ;
	  substeps = solNoLinIter ;
	/*rsize    = solNoLinBig ;*/
		nobreak  = solNoBreakStep ;

  femTangentMatrix = AF_YES ;
	
#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] ALM %s:\n",_("solution"));
#endif

	/* initialization of things: */
 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();
  femBackResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done _before_ adding of loads! */

	/* Backup result set: */
  femBackResNull();

  /* initial approximation of lambdas: */

  d_lambda    = 1.0 / ((double)steps) ;
	prev_lambda = d_lambda ;
  
  /* steps: */
  for (i=1; i<=(steps*2); i++)
  {
    converged = AF_NO ;

		if (lambda_sum > 0.3) {psi2 = 0.00000001 ;} /* for convergence in softening */

    femVecSetZeroBig(&F);
	  femVecSetZeroBig(&Fr);
	  femVecSetZeroBig(&u);
	  femVecSetZeroBig(&du0);

	  femMatSetZeroBig(&K);

#if 0
    lambda_step = ((double)opt_steps / (double)prev_steps)*(1.0 / ((double)steps)) ;
#else
#if 0
		if (prev_steps > opt_steps)
		{
			lambda_step = ((double)opt_steps / (double)prev_steps)*prev_lambda ;
		}
		else
		{
			lambda_step = prev_lambda ;
		}
#else
		lambda_step = ((double)opt_steps / (double)prev_steps)*prev_lambda ;

		if (need_repeat == AF_YES)
		{
			lambda_step = fabs(lambda_step) / (-10.0) ;
		}
#endif
#if 0
		prev_lambda = lambda_step ;
#endif
#endif

    /* initial step - sample computation: */
	  if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 	  if ((rv = fem_add_loads()) != AF_OK) { goto memFree; }
 	  if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }
    
    femValVecMultSelf(lambda_step, &F);
    femVecClone(&F, &FF);
    normF = femVecNormBig(&F);

    f_f = femVecVecMultBig(&FF, &FF) ;

    if (solUseBiCGs != AF_YES)
    {
			if (solUseCGSSOR != AF_YES)
			{
      	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
			}
			else
			{
      	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
			}
    }
    else
    {
      if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
    }

#ifndef BLOCK_FOR_MPI
		if (femRunSolIterBC == AF_YES) { for (jb=0;jb<resRLen; jb++) { resRval0[jb] = resRval[jb]; } }
#endif

	  if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; } /* Fr is needed */

    femVecAddVec(&u_tot, 1.0, &u);

    /* initial dL2 is needed: */
    sqvar = lambda_step * psi2 * f_f ;
    dL2 = femVecVecMultBig(&u, &u) + lambda_step*sqvar; /* dL2 have is constant in step */

    /* iteration: */
    for (j=1; j<=substeps; j++)
    {
      sum_jter++;

      femVecAddVec(&du0, 1.0, &u) ; /* from previous iter. (if any) */

      /* some values based on a previous step: */
		  sqvar = lambda_step * psi2 * f_f ;
      a_o = femVecVecMultBig(&du0,&du0) + lambda_step * sqvar - dL2 ;

				femVecClone(&Fr, &F); /* Fr -> F*/

				femVecSetZeroBig(&Fr); 
				femVecSetZeroBig(&u);
				femMatSetZeroBig(&K);

				if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
				if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }

      if (fabs(a_o) > 0.0)
			{
				/* filling of large matrix and vectors: */
				femVecCloneDiff(&u, &uLa) ; /* maybe redundant, but may help convergence */
				femVecCloneDiff(&F, &Fra) ; /* F, because is is actually a Fr! */

				femVecPut(&Fra, nDOFAct+1, a_o ) ;
    
				femMatSetZeroBig(&KF);
				femMatCloneDiffToEmpty(&K, &KF) ;
				femMatPut(&KF, nDOFAct+1, nDOFAct+1, -2.0*sqvar) ;

				for (k=1; k<=nDOFAct; k++) /* slow - TODO replace with something faster! */
				{
					femMatPut(&KF, nDOFAct+1, k, -2.0*femVecGet(&du0, k) ) ;

					/* F vector to KF (last column) !!*/
					femMatPut(&KF, k, nDOFAct+1, -1.0*femVecGet(&FF, k) ) ; /* TODO: slow! */
				}

				/* solve non-symetric problem: */
				if ((rv = femEqsBiCCSwJ(&KF, &Fra, &uLa, FEM_ZERO/10000.0, 2*nDOFAct+1)) != AF_OK)
				{ 
					goto memFree; 
				}

				d_lambda = femVecGet(&uLa, nDOFAct+1) ; 
				lambda_step += d_lambda ;

				femVecCloneDiff(&uLa, &u); /* TODO: check this!!! */
				femVecCloneDiff(&Fra, &F); /* TODO: useless??? */
			}
			else
			{
        if (solUseBiCGs != AF_YES)
        {
					if (solUseCGSSOR != AF_YES)
					{
				  	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) {goto memFree;}
					}
					else
					{
				  	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) {goto memFree;}
					}
        }
        else
        {
				  if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) {goto memFree;}
        }
				/* a_o=0 => d_lambda=0 */
				d_lambda = 0.0;

#ifndef BLOCK_FOR_MPI
		if (femRunSolIterBC == AF_YES) { for (jb=0;jb<resRLen; jb++) { resRval0[jb] = resRval[jb]; } }
#endif
			}

			femVecAddVec(&u_tot, 1.0, &u); /* !!! */

      femVecSetZeroBig(&Fr) ;

      /* get unballanced forces and results : */
	    if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; } /* Fr is needed */

#if 0
printf("LAMBDAS: d=%e  step=%f sum=%f, a_o=%e\n",d_lambda,lambda_step,lambda_sum,a_o);
#endif

      normFr = femVecNormBig(&Fr);

#ifdef RUN_VERBOSE
      fprintf(msgout, "ALM stp: %3li, iter: %3li, la: %f, norm: %e, dl: %e\n",
        i,j, lambda_sum+lambda_step, normFr, d_lambda);
#endif

			prev_steps = j ;

      femWriteSolNorm(sum_jter, lambda_sum+lambda_step, normFr, normF*crit,normF*1.0);

      /* convergence test: */
	    if ((normFr/normF) <= crit )
		  {
        if (j == 1) {d_lambda = lambda_step ;}
			  converged = AF_YES ;
			  break ;
		  }

    } /* end of "j" .. substeps */


		if (converged == AF_YES)
		{
			lambda_sum += lambda_step ;

      femGetSumReact(i, sum_jter, lambda_sum, femSumReactName, femTangentMatrix); /* reactions */

	    femSaPoInput(lambda_sum, 
			  	femVecGet(&u_tot,femKpos(femSaPoNode, U_X)),
				  femVecGet(&u_tot,femKpos(femSaPoNode, U_Y)),
				  femVecGet(&u_tot,femKpos(femSaPoNode, U_Z)),
				  AF_NO,
				  AF_NO
				) ;

			/* save step results: */
			if (femSubStepCheckNumber(i) == AF_YES) /* substep results */
			{
				femVecClone(&u_tot, &u); 

        solSimNum = lambda_sum ;
				if ((rv = femWriteRes(femSubStepFname(i))) != AF_OK)
				{ 
					goto memFree; 
				}
				solID++;
				femVecClone(&u_tot, &u); 
			}
    
			if ((lambda_sum >= 1.0)||(lambda_sum <= 0.0)) {break;} /* 1.0 is enough */

      femBackResGet();
			need_repeat = AF_NO ;

		}
		else /* converged == AF_NO */
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[w] ALM: %s: %li !\n", _("Unconverged step"), i);
#endif
#if 0
			goto memFree;
#else
			repeat_num++ ;

			if (repeat_num > max_repeat)
			{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] ALM: %s: %li !\n", _("Number of restarts exceeded in step"), i);
#endif
				if (nobreak != AF_YES)
				{
			  	goto memFree;
				}
				else
				{
					break ;
				}
			}
			else
			{
        femBackResPut();
			  need_repeat = AF_YES ;
			}
#endif
		}

	  if (femComputePE == AF_YES) { femCompPE(&K, &u_tot, AF_YES ) ; }
  } /* end of "i .. steps" */

	/* save final results: */
	femVecClone(&u_tot, &u); 
  solSimNum = lambda_sum ;
	if ((rv = femWriteRes( fem_output_file() )) != AF_OK) { goto memFree; }

memFree:

#ifdef RUN_VERBOSE
  if (converged == AF_YES)
  {
	  fprintf(msgout,"[I] ALM: %s.\n",_("Solution done"));
  }
  else
  {
	  fprintf(msgout,"[W] ALM: %s.\n",_("Unconverged solution"));
  }
#endif

	fem_sol_free();
	femDataFree();
	femResFree();
	femBackResFree();

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[ ] ALM Solution return: %i\n",rv);
#endif
  femCloseSolNormFile();
	return(rv);
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/** Computes lambda change for CALM (TODO: works? in what cases?) */
double alm_cyl_lambda(
    tVector *du0,
    tVector *dut,
    tVector *dur,
    tVector *du, /* empty one */
    double f_f,
    double d_la_0,
    double psi2,
    double dL2
    )
{
  static double d_la = 0.0;
  double dl_1, dl_2, disc, cos1, cos2 ;
  double a1, a2, a3, a4, a5 ;

  femVecSetZeroBig(du) ;
  femVecAddVec(du, 1.0, du0) ;
  femVecAddVec(du, 1.0, dur) ;

  a1 = femVecVecMultBig(dut, dut) + psi2 * f_f ;
  a2 = 2.0 * femVecVecMultBig(dur, du) + 2.0 * d_la_0 * psi2 * f_f;
  a3 = femVecVecMultBig(du, du) - dL2 + d_la_0*d_la_0 * psi2 * f_f;
  a4 = femVecVecMultBig(du0, dur) + femVecVecMultBig(du0,du0) ;
  a5 = femVecVecMultBig(du0,dut) ;

  disc = a2*a2 - 4.0*a1*a3 ;

  if (disc <= 0.0)
  {
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[W] %s!\n",_("load multiplier is imaginar"));
#endif
    return(0.0);
  }
  else
  {
    dl_1 = (-a2 + sqrt(disc)) / a1 ;
    dl_2 = (-a2 - sqrt(disc)) / a1 ;

    cos1 = (a4 + dl_1*a5) / dL2 ;
    cos2 = (a4 + dl_2*a5) / dL2 ;

    if (cos1 > cos2)
    {
      d_la = dl_1 ;
    }
    else
    {
      d_la = dl_2 ;
    }
  }

  return(d_la);
}

/** Stop signal function for NRM 
 * @param sig_num (unused but required) signal number
 */
#ifdef _USE_SIGNALS_
void fem_alm_signal_stop(int sig_num)
{
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i] ALM: %s.\n",_("stop signal received, saving and exiting"));
#endif

	femSaPoSave();

	if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); }
	femWriteRes( fem_output_file() );

	fem_sol_free();
	femDataFree();
	femResFree();

	femBackResFree();

  femCloseSolNormFile();

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] ALM: %s.\n",_("Solution halted through signal"));
#endif
}
#endif

/** TODO Non-linear analysis: Arc-Lenght Method
 * @param incr_type method for computation on load increment
 * @return state value
 */
int femSolveALM(long incr_type) 
{
  int rv = AF_OK ;
  long i, j ;
	int steps       = 1;      /* number of steps      */
	int substeps    = 99;    /* number of iterations */
	double crit     = 1e-5 ;  /* convergence criteria */
	int nobreak  = AF_NO ;
	int converged   = AF_NO ;
  long sum_jter   = 0 ;
  long was_steps  = 0 ;
  long opt_steps  = 10 ;
  double was_dl   = 0.0 ;
  double dL2      = 0.0 ;
  double d_la_0   = 0.0 ;
  double d_la     = 0.0 ;
  double lambda   = 0.0 ;
  double a_o      = 0.0 ;
  double psi2     = 0.0 ;
	double multR    = 0.3 ;

  long   alm_type = 0 ; /* 0=linearized, 1=cyllindric */

  double f_f      = 0.0 ;
  double normF    = 0.0 ;
  double normFr   = 0.0 ;

	long   need_repeat = AF_NO ;
	long   repeat_count = 0 ;
	long   repeat_max   = 10 ;
	long   laststp = 0 ;

#ifndef BLOCK_FOR_MPI
	long   jb ;
#endif

	/*crit     = solNoLinPre ;*/
	steps    = solNoLinSteps ;
	substeps = solNoLinIter ;
	/*rsize    = solNoLinBig ;*/
		nobreak  = solNoBreakStep ;

	multR   = 1.0 / (double)steps ; /* default data multiplier */

  steps *= 2 ;

	/* initialization of things: */
 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();
  femBackResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done _before_ adding of loads! */

  for (i=1; i<= steps*4; i++)
  {
    converged = AF_NO ;

		if (lambda > 0.1) {psi2 = 0.0 ;} /* for convergence in softening */

	  if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 	  if ((rv = fem_add_loads()) != AF_OK) { goto memFree; }
 	  if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }
		femValVecMultSelf(multR, &F) ; /* !!! */

    if (solUseBiCGs != AF_YES)
    {
			if (solUseCGSSOR != AF_YES)
			{
      	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
			}
			else
			{
      	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
			}
    }
    else
    {
      if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
    }

#ifndef BLOCK_FOR_MPI
		if (femRunSolIterBC == AF_YES) { for (jb=0;jb<resRLen; jb++) { resRval0[jb] = resRval[jb]; } }
#endif

    if (i == 1)
    {
      f_f = femVecVecMultBig(&F, &F) ;
      normF = femVecNormBig(&F);
      dL2 = pow(1.0/(double)steps,2) * psi2 * f_f + femVecVecMultBig(&u,&u) ;
    }

#if 0
    if (i > 1) /* TODO */
    {
      if (was_steps > opt_steps)
      {
        dL2 /=  (( (double)was_steps/(double)opt_steps )) ;
      }
    }
#endif

		if (need_repeat == AF_YES) {dL2 *= 0.1 ;} /* repeated try: reduce arc lenght */

    /* TODO: custom d_la_0 here... */
    d_la_0 = sqrt(dL2) / sqrt(femVecVecMultBig(&u,&u) + psi2*f_f) ;

    if (d_la_0 > 0.222) { d_la_0 = 0.222 ;}

    femVecSetZeroBig(&u);
    femVecSetZeroBig(&F);
    femVecSetZeroBig(&Fr);
    femMatSetZeroBig(&K) ;

	  if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 	  if ((rv = fem_add_loads()) != AF_OK) { goto memFree; }
 	  if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }

		femValVecMultSelf(multR, &F) ; /* !!! */
    femValVecMultSelf(d_la_0, &F) ;

    if (solUseBiCGs != AF_YES)
    {
			if (solUseCGSSOR != AF_YES)
			{
				if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
			}
			else
			{
				if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
			}
    }
    else
    {
      if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
    }

    femVecClone(&u, &du0) ; 
    femVecAddVec(&u_tot, 1.0, &u) ;

#ifndef BLOCK_FOR_MPI
		if (femRunSolIterBC == AF_YES) { for (jb=0;jb<resRLen; jb++) { resRval0[jb] = resRval[jb]; } }
#endif

	  if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }
    femVecClone(&Fr, &F_0) ; /* residual forces of 0th iter */

#ifdef _USE_SIGNALS_
		if (i == 1) { signal(SIGINT, fem_alm_signal_stop); /* SIGNAL code */ }
#endif

    for (j=1; j<=substeps; j++)
    {
      femVecSetZeroBig(&u);
      femVecSetZeroBig(&F);
      femVecSetZeroBig(&Fr);
      femMatSetZeroBig(&K) ;

	    if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 	    if ((rv = fem_add_loads()) != AF_OK) { goto memFree; }
 	    if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }
		
			femValVecMultSelf(multR, &F) ; /* !!! */

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
        	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
				}
				else
				{
        	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
				}
      }
      else
      {
        if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
      }

      femVecClone(&u, &dut) ; 

      /* unballanced forces*/
      femVecSetZeroBig(&u);
      femVecSetZeroBig(&F);
      femVecSetZeroBig(&Fr);
      femMatSetZeroBig(&K) ;

	    if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
      femVecClone(&F_0, &F);
 	    if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }

      if (solUseBiCGs != AF_YES)
      {
				if (solUseCGSSOR != AF_YES)
				{
        	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
				}
				else
				{
        	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
				}
      }
      else
      {
        if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
      }
      
      femVecClone(&u, &dur) ; 

#ifndef BLOCK_FOR_MPI
		if (femRunSolIterBC == AF_YES) { for (jb=0;jb<resRLen; jb++) { resRval0[jb] = resRval[jb]; } }
#endif

#if 1
      a_o = (psi2*d_la_0*d_la_0*f_f + femVecVecMultBig(&du0,&du0) - dL2) ;
#endif

      /* d_la computation */
      switch (alm_type)
      {
        case 1:
          d_la =
            alm_cyl_lambda(&du0,&dut,&dur,&du,f_f,d_la_0,psi2,dL2);
          break;
        case 0:
        default:
        /* linearized ALM: */
        d_la = (-1.0) * (
          ( 0.5*a_o + femVecVecMultBig(&du0,&dur) )
          /
          ( femVecVecMultBig(&du0,&dut) + d_la_0*psi2*f_f )
          );
        break;
      }

      if (d_la > 4.0) {d_la = 4.0 ;}

      femVecAddVec(&u, d_la, &dut) ;
      femVecAddVec(&u_tot, 1.0, &u) ;
      femVecAddVec(&du0, 1.0, &u) ;

	    if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }
      femVecClone(&Fr, &F_0) ; /* residual forces of 0th iter */
      normFr = femVecNormBig(&F_0);

      d_la_0 += d_la ;

      was_steps = j ;
      was_dl    = d_la_0 ;

      sum_jter++ ;

      femWriteSolNorm(sum_jter, d_la_0+lambda, normFr, normF*crit,normF*1.0);

#ifdef RUN_VERBOSE
      fprintf(msgout,"[ ] ALM stp: %3li, it: %3li, dla %e, la: %f, Fr: %e\n",
          i,j, d_la*multR, (d_la_0+lambda)*multR, normFr
          );
#endif

      /* convergence test: */
	    if ((normFr/normF) <= crit )
		  {
			  converged = AF_YES ;
			  break ;
		  }
    } /* for j */

		if (converged == AF_YES)
		{
      lambda += d_la_0 ;

      femGetSumReact(i, sum_jter, lambda*multR, femSumReactName, femTangentMatrix); /* reactions */

	    femSaPoInput(lambda*multR, 
			  	femVecGet(&u_tot,femKpos(femSaPoNode, U_X)),
				  femVecGet(&u_tot,femKpos(femSaPoNode, U_Y)),
				  femVecGet(&u_tot,femKpos(femSaPoNode, U_Z)),
				  AF_NO,
				  AF_NO
				) ;

			/* save step results: */
			if (femSubStepCheckNumber(i) == AF_YES) /* substep results */
			{
				femVecSwitch(&u_tot, &u); 
        solSimNum = lambda*multR ;
				if ((rv = femWriteRes(femSubStepFname(i))) != AF_OK)
				{ 
					goto memFree; 
				}
				solID++;
				femVecSwitch(&u_tot, &u); 
			}

			if (((lambda*multR) >= 1.01)||((lambda*multR) <= 0.004)) {break;} /* 1.0 is enough */

      femBackResGet();
			need_repeat = AF_NO ;
		}
		else /* converged == AF_NO */
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[w] ALM: %s: %li !\n", _("Unconverged step"), i);
#endif
#if 1
			if (i == 1) {goto memFree;}
			repeat_count++ ;
			need_repeat = AF_YES ;
			i-- ;
			if (repeat_count > repeat_max) 
			{
				if (nobreak != AF_YES)
				{
					goto memFree;
				}
				else
				{
					break ;
				}
			}
      multR /= 2.0 ;
      femBackResPut() ;
#else
			goto memFree;
#endif
		}

		laststp = i ;

	  if (femComputePE == AF_YES) { femCompPE(&K, &u_tot, AF_YES ) ; }
  } /* for i */

	/* save final results: */
	femVecSwitch(&u_tot, &u); 
  solSimNum = lambda*multR ;
	if ((rv = femWriteRes( fem_output_file() )) != AF_OK) { goto memFree; }

memFree:

#ifdef RUN_VERBOSE
  if ((converged == AF_YES)|| (laststp > 1))
  {
	  fprintf(msgout,"[I] ALM: %s, %s: %li, %s: %f %s: %li.\n",
				_("Solution done"), 
				_("steps"), laststp,
				_("load"),lambda*multR,
				_("restarts"),repeat_count

				);
  }
  else
  {
	  fprintf(msgout,"[W] ALM: %s.\n",_("Unconverged solution"));
  }
#endif

	fem_sol_free();
	femDataFree();
	femResFree();
	femBackResFree();

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[ ] ALM Solution return: %i\n",rv);
#endif
  femCloseSolNormFile();

  return(rv) ;
}

/* end of fem_alm.c */

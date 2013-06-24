/*
   File name: fem_tth.c
   Date:      2012/06/17 21:49
   Author:    Jiri Brozovsky

   Copyright (C) 2012 Jiri Brozovsky

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

   Transient thermal analysis
*/

#include "fem_mat.h"
#include "fem_elem.h"

/* from fem_sol.c: */
extern void fem_sol_null(void);
extern void fem_sol_free(void);
extern int fem_sol_alloc(void);
extern int fem_sol_res_alloc(void);
extern int fem_dofs(void);
extern int fem_add_loads(long step);
extern int fem_add_disps(long disp_mode, long step);
extern int fem_fill_K(long mode);
extern int fem_fill_M(void);

/* from fem_e010.c: */
extern double e011_area(long ePos);

extern long  nDOFAct  ; /* total number of ACTIVE DOFs in structure (== size of "K" matrix) */
extern long  nDOFlen  ; /* lenght of nDOFfld                        */
extern long *nDOFfld  ; /* description of DOFs in nodes             */
extern long *K_rows   ; /* numbers of items in "K" rows K_rows[nDOFAct] */

extern tMatrix K; /* structure stiffness matrix    */
extern tVector F; /* structure load vector         */
extern tVector Fr;/* unballanced forces vector     */
extern tVector u; /* structure displacement vector */

/* dynamics: */
extern tMatrix M;         /* mass matrix */
extern tMatrix C;         /* thermal capacity matrix */
extern tMatrix KK;        /* conductivity matrix     */
extern tVector F_0;       /* prev step load          */
extern tVector r0;        /* prev step temperatures  */
extern tVector rr0;       /* prev step temperatures  */
extern tVector pp;        /* combined load           */

/** Wrapper for linear system solvers
 * @param Ks stiffness matrix
 * @param Fs load vector
 * @param us displacement vector (result)
 * @return status
 */ 
extern int femLinEqSystemSolve(tMatrix *Ks, tVector *Fs, tVector *us);

/** Filling of results data field (ofld)
 * @param ofld field to put data in
 * @return status
 */
extern int monte_fill_ofld_data(double *ofld);

/** Checks default size of time steps */
int femCheckTTHstepSize(double d_t)
{
  long   ePos, eT ;
  long   kxx_num    = 0 ;
  double char_size  = 0.0  ;
  double kxx        = 0.0  ;
  double d_comp, A ;
  double kxx1       = 0.0 ;
  double char_size1 = 0.0 ;

  for (ePos=0; ePos < eLen; ePos++)
  {
		eT = femGetETypePos(ePos); /* element type */
    if (eT == 18)
    {
	    A=e011_area(ePos);
      kxx1 = femGetMPValPos(ePos, MAT_KXX, 0) ;

      if (fabs(kxx1) > FEM_ZERO) 
      {
        char_size1 = sqrt(A);
        char_size += char_size1 ;
        kxx += kxx1 ;
        kxx_num++ ;
      }
    }
  }
  char_size = char_size / ((double) kxx_num) ;
  kxx = kxx / ((double) kxx_num) ;

  if (fabs(kxx) < FEM_ZERO) {return(AF_ERR_VAL);}

  d_comp = (char_size*char_size) /  (4.0*kxx) ;

  if (d_comp <(1.5*d_t)) 
  {
    fprintf(msgout,"[w] %s (%s: %f, %s: %f)!\n",_("Step size too small"),_("ideal"),d_comp,_("actual"),d_t); 
    return(AF_ERR_VAL);
  }
  if (d_comp >(1.5*d_t)) 
  {
    fprintf(msgout,"[w] %s (%s: %f, %s: %f)!\n",_("Step size too big"),_("ideal"),d_comp,_("actual"),d_t); 
    return(AF_ERR_VAL);
  }

  fprintf(msgout,"[i] %s: %f, %s: %f\n",_("Recommended step size"),d_comp,_("actual size"),d_t);
  return(AF_OK) ;
}

/** Simple dynamics solver: Newmark time integration procedure is used
 *
 * @param ofld random output data filed (only != NULL for Monte Carlo)
 * @return statis
 */
int femSolveThermTrans(double *ofld)
{
	int    rv = AF_OK ;
  long   i ;
  long   steps = 10 ;  /* number of time steps */
  double d_t   = 1.0 ; /* time (step lenght)   */
  double tau   = 0.5 ; /* time step ratio      */
  double tot_time = 0.0 ; /* total elapsed time */
  char  *fnm = NULL ; /* file name for step */

	steps = dynNum ;
  d_t   = dynStp ;

#ifndef USE_MONTE
 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

#ifdef RUN_VERBOSE /* Tests time step size */
  femCheckTTHstepSize(d_t) ; /* return not checked so far */
#endif

  fem_sol_null();
  femResNull();

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("data checking and allocations"));
#endif
 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("data checking and allocations done"));
#endif
#endif

#ifndef USE_MONTE
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("assembling of stiffness matrix"));
#endif
#endif
 	if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
#ifndef USE_MONTE
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("assembling of stiffness matrix done"));
#endif
#endif

#ifndef USE_MONTE
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("assembling of mass/capacity matrix"));
#endif
#endif
 	if ((rv = femMatAllocCloneStruct(&K, &M)) != AF_OK) { goto memFree; }
 	if ((rv = femMatAllocCloneStruct(&K, &KK)) != AF_OK) { goto memFree; }
 	if ((rv = femMatAllocCloneStruct(&K, &C)) != AF_OK) { goto memFree; }
 	if ((rv = fem_fill_M()) != AF_OK) { goto memFree; }
#ifndef USE_MONTE
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("assembling of mass/capacity matrix done"));
#endif
#endif

#ifndef USE_MONTE
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("loads and supports"));
#endif
#endif
  fem_add_loads(1);
 	if ((rv = fem_add_disps(AF_YES,1)) != AF_OK) { goto memFree; }
#ifndef USE_MONTE
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("loads and supports done"));
#endif
#endif

#ifndef USE_MONTE
#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] %s:\n",_("Time integration started"));
#endif
#endif

  /* 0th step... */
#if 1
  femVecClone(&F, &F_0); /* clone load to old load vector */
#endif
#if 1
  femVecClone(&u, &r0); /* clone temperatures to old temperature vector */
#endif

  femVecPrn(&r0,"RR0");

  for (i=1; i<steps; i++)
  {
#ifndef USE_MONTE
#ifdef RUN_VERBOSE
		fprintf(msgout,"[I] %s %li / %li:\n", _("Transient step"), i, steps-1);
#endif
#endif

    /* substep size: */
    if (transTS > -1) { d_t = transMult[transTS][i] - transMult[transTS][i-1] ; }

    /* loads should be inside loop */
    femVecSetZeroBig(&F);
    femVecSetZeroBig(&u);

    femMatSetZeroBig(&C) ;
    femMatSetZeroBig(&KK) ;
    /* new K and M matrices (slowdown): */
    femMatSetZeroBig(&K) ;
    femMatSetZeroBig(&M) ;
 	  if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 	  if ((rv = fem_fill_M()) != AF_OK) { goto memFree; }

    fem_add_loads(i+1);
 	  if ((rv = fem_add_disps(AF_YES,i+1)) != AF_OK) { goto memFree; }

    /* right hand side vector ((F_0*1-tau) + F*tau) = pp0: */
    femVecLinComb((1.0-tau), &F_0,  tau, &F, &pp);

    /* right hand side:  pp0+(M/tau - K*(1-tau))*r0 */
    femMatLinCombClones(1.0/d_t, &M, -(1.0-tau), &K, &C);
    femMatVecMultBig(&C, &r0, &rr0) ;
    femVecAddVec(&pp, 1.0, &rr0); /* adds rr0 to pp */

    /* left hand side:  M/tau + K*(tau) */
    femMatLinCombClones(1.0/d_t, &M, (tau), &K, &KK);

    /* equation solution: */
    if (solUseCGSSOR != AF_YES)
		{
	  	if ((rv = femEqsCGwJ(&KK, &pp, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
		}
		else
		{
	  	if ((rv = femEqsCGwSSOR(&KK, &pp, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
		}

    tot_time += d_t ; /* total elapsed time */

		/* results on elements: */
 		if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }

    /* writing of results*/
    if ((fnm = femSubStepFname(i)) != NULL)
    {
			solID = i ;
      solSimNum = tot_time; /* time data */
		  if ((rv = femWriteRes(fnm)) != AF_OK) 
         { free(fnm); fnm = NULL ; goto memFree; }
      free(fnm); fnm = NULL ;
    }

#ifndef USE_MONTE
#ifdef RUN_VERBOSE
		fprintf(msgout,"[i]   %s: %s %f \n", _("Transient step done"), _("cummulative time"), tot_time);
#endif
#endif

    femVecClone(&F, &F_0); /* clone load to old load vector */
    femVecClone(&u, &r0); /* clone load to old load vector */
  }

		/* if running as Monte library then data should be checked here: */
#ifdef USE_MONTE
		if (ofld !=  NULL) { monte_fill_ofld_data(ofld) ; }
#endif

memFree:
#ifndef USE_MONTE
	fem_sol_free();
	femDataFree();
	femResFree();

#ifdef RUN_VERBOSE
	if (rv == AF_OK) { fprintf(msgout,"[I] %s.\n",_("Solution done")); }
	else { fprintf(msgout,"[E] %s!\n",_("Solution failed")); }
#endif
#endif

	return(rv);
}




/* end of fem_tth.c */

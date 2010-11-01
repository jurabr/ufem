/*
   File name: fem_newm.c
   Date:      2010/11/01 19:11
   Author:    Jiri Brozovsky

   Copyright (C) 2010 Jiri Brozovsky

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

	 Newmark implicit time integration routine for dynamics
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
extern int fem_fill_M(void);

extern long  nDOFAct  ; /* total number of ACTIVE DOFs in structure (== size of "K" matrix) */
extern long  nDOFlen  ; /* lenght of nDOFfld                        */
extern long *nDOFfld  ; /* description of DOFs in nodes             */
extern long *K_rows   ; /* numbers of items in "K" rows K_rows[nDOFAct] */

extern tMatrix K; /* structure stiffness matrix    */
extern tVector F; /* structure load vector         */
extern tVector Fr;/* unballanced forces vector     */
extern tVector u; /* structure displacement vector */

/* dynamics: */
extern tMatrix M;         /* structure mass matrix    */

/* newmark */
extern tMatrix KK;         /* combined stiffness matrix combined stiffness matrix */
extern tMatrix C;          /* damping matrix */
extern tVector pp;         /* combined load vector */
extern tVector dr;         /* displacement change   */
extern tVector ra;         /* temporary vector    */
extern tVector rb;         /* temporary vector    */
extern tVector r0;         /* previous displacement */
extern tVector rr0;        /* previous velocity     */
extern tVector rrr0;       /* previous acceleration */
extern tVector rr1;        /* current velocity     */
extern tVector rrr1;       /* current acceleration */


/** Wrapper for linear system solvers
 * @param Ks stiffness matrix
 * @param Fs load vector
 * @param us displacement vector (result)
 * @return status
 */ 
int femLinEqSystemSolve(tMatrix *Ks, tVector *Fs, tVector *us)
{
  int rv = AF_OK ;
  long steps ;
  double precision ;

  steps     = nDOFAct ;
  precision = FEM_ZERO/10000.0 ;
    
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("solution of linear equations"));
#endif
  if (solUseBiCGs != AF_YES)
  {
		if (solUseCGSSOR != AF_YES) { rv = femEqsCGwJ(Ks, Fs, us, precision, steps); }
		else { rv = femEqsCGwSSOR(Ks, Fs, us, precision, steps); }
  }
  else { rv = femEqsBiCCSwJ(Ks, Fs, us, precision, steps); }
#ifdef DEVEL_VERBOSE
  if (rv == AF_OK)
    { fprintf(msgout,"[i]   %s.\n",_("solution of linear equations done")); }
  else
    { fprintf(msgout,"[E]   %s!\n",_("solution of linear equations FAILED")); }
#endif

  return(rv);
}

/** Simple implicit dynamics solver: see Bitnar, Rericha: "Metoda
 * konecnych prvku v dynamice konstrukci", SNTL, Prague, 1981, p. 122
 * Newmark time integration procedure is used
 *
 * @param start_time time when solution starts
 * @param endif time when solution ends
 * @param steps number of steps
 * @return statis
 */
int femSolveDynNewmark(void)
{
	int    rv = AF_OK ;
	double dt = 0.0 ;
	long   steps = 0 ;
  double a[11] ;
  double r_alpha = 0.1 ;
  double r_beta  = 0.1 ;
  long   i ;

	dt      = dynStp ;
	steps   = dynNum ;
	r_alpha = dynAlpha ;
	r_beta  = dynBeta ;

	if ((dt < 0.0) || (steps < 1)) 
	{
#ifdef RUN_VERBOSE
		fprintf(msgout, "[E] %s!\n", _("No valid data for Newmark available"));
#endif
		return(AF_ERR_VAL);
	}

  /* Solution constants: */
  a[0] = 4.0 / (dt*dt) ;
  a[1] = 2.0 / dt ;
  a[2] = 2.0 * a[1] ;
  a[3] = 1.0 ;
  a[4] = 1.0 ;
  a[5] = 0.0 ;
  a[6] = a[0] ;
  a[7] = (-2.0)*a[1] ;
  a[8] = (-1.0) ;
  a[9] = 1.0/a[2] ;
  a[10] = a[9] ;

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("data checking and allocations"));
#endif
 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */

 	if ((rv = femMatAllocCloneStruct(&K, &M)) != AF_OK) { goto memFree; }
 	if ((rv = femMatAllocCloneStruct(&K, &C)) != AF_OK) { goto memFree; }
 	if ((rv = femMatAllocCloneStruct(&K, &KK)) != AF_OK) { goto memFree; }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("data checking and allocations done"));
#endif

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("assembling of stiffness matrix"));
#endif
 	if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("assembling of stiffness matrix done"));
#endif

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("assembling of mass matrix"));
#endif
 	if ((rv = fem_fill_M()) != AF_OK) { goto memFree; }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("assembling of math matrix done"));
#endif

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("assembling of Rayleigh damping matrix"));
#endif
  if ( (M.type == MAT_SPAR) && (K.type == MAT_SPAR) && (C.type == MAT_SPAR) && (KK.type == MAT_SPAR) )
  {
    /* this code assumes that structure of all matrices is identical! */
    for (i=0; i<K.len; i++)
        { C.data[i] = r_alpha*M.data[i] + r_beta*K.data[i] ; }
  }
	else
	{
  	femMatLinComb(r_alpha, &M, r_beta, &K, &C); /*slow as a hell */
	}
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("assembling of damping matrix done"));
#endif

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("loads and supports"));
#endif
	/*TODO: unit load for mass matrix: */
#if 0
 	if ((rv = fem_add_loads()) != AF_OK) { goto memFree; }
#endif
 	if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("loads and supports done"));
#endif

  /* newmark preparations (initial KK matrix): */
  if ( (M.type == MAT_SPAR) && (K.type == MAT_SPAR) && (C.type == MAT_SPAR) && (KK.type == MAT_SPAR) )
  {
    /* this assumes that structure of all matrices is identical: */
    for (i=0; i<K.len; i++)
    {
      KK.data[i] = K.data[i] + a[0]*M.data[i] + a[1]*C.data[i] ;
    }
  }
  else { rv = AF_ERR_TYP ; goto memFree ; }

  /* TODO: fill F correctly for 0th step ! */
  /* initial acceleration: */
  femLinEqSystemSolve(&M, &F, &rrr0) ;

  for (i=0; i<steps; i++) /* iteration over time steps */
  {
#ifdef RUN_VERBOSE
		fprintf(msgout,"[ ] %s %li/%li ", _("Newmark step"), i+1, steps);
#endif
		femVecSetZeroBig(&pp) ;

		femVecSetZeroBig(&ra) ;
    femVecAddVec(&ra, a[1], &r0);
    femVecAddVec(&ra, a[2], &rr0);
    femVecAddVec(&ra, a[3], &rrr0);
		femMatVecMult(&M, &ra, &rb);
    femVecAddVec(&pp, 1.0, &rb); /* 1st component of ra added */

		femVecSetZeroBig(&ra) ;
    femVecAddVec(&ra, a[1], &r0); /* TODO check this!!! */
    femVecAddVec(&ra, a[4], &rr0);
    femVecAddVec(&ra, a[5], &rrr0);
		femMatVecMult(&C, &ra, &rb);
    femVecAddVec(&pp, 1.0, &rb); /* 1st component of ra added */

    /* TODO: get current value of F! */
    femVecAddVec(&pp, 1.0, &F); /* TODO compute correct load first! */

  	femLinEqSystemSolve(&KK, &pp, &u) ;
		femVecLinCombBig(1.0, &u, -1.0, &r0, &dr); /* displacement change coputation */

		femVecSetZeroBig(&rrr1);
		femVecAddVec(&rrr1, a[6], &dr);
		femVecAddVec(&rrr1, a[7], &rr0);
		femVecAddVec(&rrr1, a[8], &rrr0);

		femVecSetZeroBig(&rr1);
		femVecAddVec(&rr1, 1.0, &rr0);
		femVecAddVec(&rr1, a[9], &rrr1);
		femVecAddVec(&rr1, -1.0*a[9], &rrr0);

		/* clone new to old for next step: */
		if (i < (steps-1))
		{
			femVecClone(&u, &r0);
			femVecClone(&rr1, &rr0);
			femVecClone(&rrr1, &rrr0);
		}

		/* results on elements: */
 		if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }
		if ((rv = femWriteRes( femSubStepFname(i))) != AF_OK) { goto memFree; }

#ifdef RUN_VERBOSE
		fprintf(msgout," ...  %s: %s %f \n", _("done"), _("cummulative time"), (i+1)*dt);
#endif
  } /*end of time steps iteration */

memFree:
	fem_sol_free();
	femDataFree();
	femResFree();

#ifdef RUN_VERBOSE
	if (rv == AF_OK) { fprintf(msgout,"[I] %s.\n",_("Solution done")); }
	else { fprintf(msgout,"[E] %s!\n",_("Solution failed")); }
#endif

	return(rv);
}

/* end of fem_newm.c */

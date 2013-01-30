/*
   File name: fem_eige.c
   Date:      2007/10/22 15:47
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

	 FEM solver: Eigenproblem solver
*/

#include "fem_sol.h"

/* from fem_sol.c: */
extern void fem_sol_null(void);
extern void fem_sol_free(void);
extern int fem_sol_alloc(void);
extern int fem_sol_res_alloc(void);
extern int fem_dofs(void);
extern int fem_add_disps(long disp_mode, long step);
extern int fem_fill_K(long mode);
extern int fem_fill_M(void);

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

/* dynamics: */
extern tMatrix M;         /* structure mass matrix    */

/* eigensolver */
extern tVector eig_omega; /* vector of eigenvalues    */
extern tVector eig_x;     /* i-th iteration vector of eigenvalues    */
extern tVector eig_xM;     /* i-th iteration vector of eigenvalues    */
extern tMatrix eig_shap;  /* matrix of eigenvectors   */
extern tMatrix eig_oMK;   /* (-omega*M+ K matrix)     */
extern tVector *eig_y ;   /* FIELD of Gram-Schmidt data */


/** Computation of eigenvalues and eigenvectors for dynamics
 * @return state value
 */
int femSolveEigenInvIter(long max_iter, double eps)
{
	int  rv = AF_ERR_VAL;
  long i, j, jj ;
  double xmult ;
  double om_top ;
  double om_bot ;
  double omega = 0.0 ;
  double omega0 = 0.0 ;
  double c ;

  femEigenNum = 1 ; /* TODO: current implementation ha so shift to higher eigenvalues! */

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] %s:\n",_("Computation of 1st eigenvalue")); 
#endif

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */

 	if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }

  for (i=1; i<=nDOFAct; i++) { femVecPut(&u, i, 1.0) ; /* initial approximation */ }

 	if ((rv = fem_fill_M()) != AF_OK) { goto memFree; } /* mass matrix*/

 	if ((rv = fem_add_disps(AF_YES,0)) != AF_OK) { goto memFree; }


  /* main loop: */
  for (j=1; j<=femEigenNum; j++)
  {
    rv = AF_ERR_VAL ; /* initial return value */
    solID = j ;

  for (i=0; i<max_iter; i++)
  {
#if 1
    femValVecMultSelf(8.8,&u) ;
#if 0
    if ((i > 0)||(j>1)) { if ((rv = fem_add_disps(AF_YES,0)) != AF_OK) { goto memFree; } }
#endif
#endif

    /* Gram-Schmidt: */
    if (j > 1)
    {
      femVecSetZeroBig(&Fr);
      femVecSetZeroBig(&F);

      for (jj=0; jj<(j-1); jj++)
      {
        femVecSetZeroBig(&eig_xM) ;
        femMatVecMultBig(&M, &u, &eig_xM);
        c = femVecVecMultBig(&eig_y[jj], &eig_xM) ;
        femVecAddVec(&Fr, c, &eig_y[jj]);
      }
      femVecLinCombBig(1.0, &u, -1.0, &Fr, &F) ;
      femVecClone(&F, &u);
    }
    /* end of Gram-Schmidt */

    femVecSetZeroBig(&eig_xM) ;
    femMatVecMultBig(&M, &u, &eig_xM);

#if 0
    if ((rv = femEqsCGwJ(&K, &eig_xM, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK)
#else
    if ((rv = femEqsBiCCSwJ(&K, &eig_xM, &u, FEM_ZERO/1000.0, 3*nDOFAct)) != AF_OK)
#endif
      { goto memFree; } 

#ifdef DEVEL_VERBOSE
    fprintf(msgout,"NORM: %e %e\n", femVecNormBig(&u), femVecNormBig(&eig_y[0]));
#endif
    
    /* normalize x: */
    femVecSetZeroBig(&eig_xM) ;
    femMatVecMultBig(&M, &u, &eig_xM);
    xmult = femVecVecMultBig(&u, &eig_xM) ;
    if (xmult != 0.0)
    {
      xmult = (1.0 / sqrt(xmult)) ;
    }
    else
    {
      rv = AF_ERR_VAL ;
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s!\n", _("Inverse iteration failed - can not normalize"));
#endif
      goto memFree;
    }

    femValVecMultSelf(xmult, &u);

    /* compute omega^2 (Rayleight): */
    femVecSetZeroBig(&eig_xM) ;
    femMatVecMultBig(&K, &u, &eig_xM);
    om_top = femVecVecMultBig(&u, &eig_xM) ;

    femVecSetZeroBig(&eig_xM) ;
    femMatVecMultBig(&M, &u, &eig_xM);
    om_bot = femVecVecMultBig(&u, &eig_xM) ;

    if (om_bot == 0.0)
    {
      rv = AF_ERR_VAL ;
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s!\n", _("Inverse iteration failed - can not compute omega"));
#endif
      goto memFree;
    }

    omega = om_top / om_bot ;

#ifdef DEVEL_VERBOSE
    fprintf(msgout,"OMEGA[%li/%li] = %f (%f/%f)\n",i,max_iter,sqrt(fabs(omega)),om_top, om_bot);    
#endif

    if (i > 0)
    {
      if ((fabs(omega - omega0)/omega) <= (pow(10,(-2.0*eps))))
      {
#ifdef RUN_VERBOSE
        fprintf(msgout,"[i] %s: %f (%s %li)\n",_("  Eigenvalue"),
            sqrt(fabs(omega))/(2.0*FEM_PI),_("in iteration"),i+1);
#endif
        /* we are converged */
        rv = AF_OK ;
        break ;
      }
    }

    omega0 = omega ;
    
  }
    if (rv != AF_OK)
    {
#ifdef RUN_VERBOSE
      fprintf(msgout, "[E] %s %li %s!\n", _("Computation of eigenvalue no."), j ,_("failed"));
      break ;
#endif
    }

    solSimNum = sqrt(fabs(omega))/(2.0*FEM_PI) ; /* frequency */
  
    if (j == 1) /* first eigenvector */
    {
	    if ((rv = femWriteRes(fem_output_file())) != AF_OK) 
      { 
        goto memFree; 
      }
    }

    /* all eigenvectors ("-osf NAME")*/
    if (femSubStepFname(j) != NULL)
    {
	    if ((rv = femWriteRes(femSubStepFname(j))) != AF_OK)
      { 
        goto memFree; 
      }
    }
    if (j == femEigenNum) {break ;} /* we are finished */

    /* TODO prepare next Gram-Schmidt HERE: */
    femVecClone(&u, &eig_y[j-1]) ;

  } /* end "j": multiple eigenvalues */

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

/* ############################################################ */
/* Lanczos method attempt */

/**
 * This is function solves eigenvalues for tridiagonal matrix.
 * Algoritm is based on LU decoposition. A = LU, A_{i+1} = UL
 * @param alpha diagonal elements (on output eigenvalues)
 * @param beta  1st U-subdigonal (will bee destroyed)
 * @param gamma 1st L-subdiagonal (will bee destroyed)
 * @return error code.   
 */
int femTriEigLU(tVector *alpha, tVector *beta, tVector *gamma)
{
  int     rv = AF_OK ;
  tVector P, R;
  long    i, j, n;
  double  norm;
  
  n = alpha->rows;
  if (n < 1) { rv = AF_ERR_EMP ; goto memFree ;}

#if 1
  for (i=1; i<=n; i++)
  {
    printf(" %2.4e   %2.4e   %2.4e\n", 
        femVecGet(beta, i),
        femVecGet(alpha, i),
        femVecGet(gamma, i)
        );
  }
#endif

  femVecNull(&P);
  femVecNull(&R);

  if ((rv=femVecFullInit(&P, n) != AF_OK)) {goto memFree;}
  if ((rv=femVecFullInit(&R, n) != AF_OK)) {goto memFree;}

  norm = femVecNormBig(beta); 

  j = 0 ; /* only for counting of steps */
 
  while (norm > 0.0001) 
  { 
    femVecPut(&P,1, femVecGet(alpha, 1)) ;

    for (i = 1; i < n ; i++) /* not to n? */
    {
      femVecPut(&R, i, femVecGet(beta, i) / femVecGet(&P, i) );
      
      femVecPut(&P, i+1, femVecGet(alpha, i+1) - 
          ( femVecGet(gamma, i) * femVecGet(&R, i) ) );

      femVecPut(alpha, i, femVecGet(&P, i) + 
          ( femVecGet(gamma, i) * femVecGet(&R, i) ) );

      femVecPut(beta, i, 
          ( femVecGet(&R, i) * femVecGet(&P, i+1) ) );
    }

    femVecPut(alpha, i, femVecGet(&P, i) );

    norm = femVecNormBig(beta); 

    j++ ;
    fprintf(msgout,"EIG-LU STEP: %li\n", j);
#ifdef DEVEL_VERBOSE
#endif
  }

  femVecPrn(alpha,"EIGENVALUES");

  if (n > 4) {n = 4 ;}
  for (i=1; i<=n; i++)
  {
    printf("a[%4li] = %e\n", i, femVecGet(alpha, i));
  }
  
memFree:
  femVecFree(&P);
  femVecFree(&R);
  
  return(rv);
}

/** Lanczos method  
 *  
 */
int femEigLanczos(tMatrix *K, tMatrix *M, long k, tVector *eigval)
{
  int rv = AF_OK ;
  tVector r, rr, rrr, q, q1, p, pp, s,  a, b, c;
  double beta, alpha, betadiv;
  long n  = 0;
  long i ;

  n = K->rows ;
  if (n < 1) { rv = AF_ERR_EMP ; goto memFree ;}

  femVecNull(&r);
  femVecNull(&rr);
  femVecNull(&rrr);
  femVecNull(&q);
  femVecNull(&q1);
  femVecNull(&p);
  femVecNull(&pp);
  femVecNull(&s);
  femVecNull(&a);
  femVecNull(&b);
  femVecNull(&c);

  femVecFullInit(&r,   n);
  femVecFullInit(&rr,  n);
  femVecFullInit(&rrr, n);
  femVecFullInit(&q,   n);
  femVecFullInit(&q1,  n);
  femVecFullInit(&p,   n);
  femVecFullInit(&pp,  n);
  femVecFullInit(&s,   n);
  femVecFullInit(&a,   n);
  femVecFullInit(&b,   n);
  femVecFullInit(&c,   n);
  

  /* Preparations: */
  for (i=1; i<=n; i++) { femVecPut(&r, i, 0.001*rand()); }
  femVecSetZeroBig(&q1);

  /* TODO: {Kg} = {K} - sigma*{M} */

  /* beta = sqrt({r}^T.[M].{r}) */
  femMatVecMultBig(M, &r, &s);
  beta = sqrt (femVecVecMult(&r, &s) );

  if (beta == 0.0)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Eigenvalue solver stopped: beta = 0 not allowed"));
#endif
    rv = AF_ERR_VAL ;
    goto memFree;
  }
 	/* {q} = {r} / beta */
  betadiv = 1.0 / beta ;
  femValVecMult(betadiv, &r, &q) ;

  /* {p} = {M}*{q} */
  femMatVecMultBig(M, &q, &p);


  /* Main loop for "k" eigenvectors: */
  for (i = 1; i <= k; i++)
  {
    /* {Kg}*{rr} = {p} */
	  femEqsCGwJ(K, &p, &rr, 1e-8, 3 * n);

    /* {rrr} = {rr} - {q1} * beta */
    femVecLinCombBig(1.0,  &rr, -1.0 * beta, &q1, &rrr);

    /* alpha = {p}^T*{rrr} */
    alpha = femVecVecMult(&p, &rrr);

    /* {r} = {rrr} - alpha*{q} */
    femVecLinCombBig(1.0,  &rrr, -1.0 * alpha, &q, &r);

    /* {pp} = {M} * {r} */
    femMatVecMultBig(M, &r, &pp);

    /* Fill tridiagonal matrix: */
    if (i == 1) /* first line */
    {
      femVecPut(&a, 1, alpha) ;
    }
    else /* normal line */
    {
      femVecPut(&a, i  , alpha) ;
      femVecPut(&c, i-1  , beta ) ;
      femVecPut(&b, i, beta ) ;
    }
printf("Lanczos [%li]: a=%e b=%e\n", i, alpha, beta);

    /* beta = sqrt({pp}^T * {r})*/
    beta = sqrt (femVecVecMult(&pp, &r) );

    if (i == k) {break;} /* check for the last step */

    /* only for non-last steps: */
    betadiv = 1.0 / beta ;

    /* {q}  = {r} / beta*/
    femValVecMult(betadiv, &r, &q) ;

    /* {p}  = {pp} / beta*/
    femValVecMult(betadiv, &pp, &p) ;

    /* q(i-1): */
    femVecClone(&q,&q1);
  }

printf("CCCCCCCCCCCCCCCCCCCCCCCCCC\n");
  rv = femTriEigLU(&a, &b, &c);
printf("CCCCCCCCCCCCCCCCCCCCCCCCCC\n");

memFree:
  femVecFree(&r);
  femVecFree(&rr);
  femVecFree(&rrr);
  femVecFree(&q);
  femVecFree(&q1);
  femVecFree(&p);
  femVecFree(&pp);
  femVecFree(&s);

  femVecFree(&a);
  femVecFree(&b);
  femVecFree(&c);
  return(rv);
}

int femSolveEigenLanczos(long max_iter, double eps)
{
	int  rv = AF_ERR_VAL;
  long i, j ;

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */

 	if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }

  for (i=1; i<=nDOFAct; i++) { femVecPut(&u, i, 1.0) ; /* initial approximation */ }

 	if ((rv = fem_add_disps(AF_YES,0)) != AF_OK) { goto memFree; }

 	if ((rv = fem_fill_M()) != AF_OK) { goto memFree; } /* mass matrix*/

  femMatPrn(&M, "MASS");

printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
  rv = femEigLanczos(&K, &M, femEigenNum, &u) ;
printf("YYYYYYYYYYYYYYYYYYYYYYYYYYYYY\n");

  for (j=1; j<=femEigenNum; j++)
  {
#if 0
    rv = AF_ERR_VAL ; /* initial return value */

      /* TODO: some code here (nothing is done in this place!) */

    if (rv != AF_OK)
    {
#ifdef RUN_VERBOSE
      fprintf(msgout, "[E] %s %li %s!\n", _("Computation of eigenvalue no."), j ,_("failed"));
      break ;
#endif
    }
  
    if (j == 1) /* first eigenvector */
    {
	    if ((rv = femWriteRes(fem_output_file())) != AF_OK) 
      { 
        goto memFree; 
      }
    }

    /* all eigenvectors ("-osf NAME")*/
    if (femSubStepFname(j) != NULL)
    {
	    if ((rv = femWriteRes(femSubStepFname(j))) != AF_OK)
      { 
        goto memFree; 
      }
    }
    if (j == femEigenNum) {break ;} /* we are finished */

    /* TODO prepare next Gram-Schmidt HERE: */
    femVecClone(&u, &eig_y[j-1]) ;

#endif
  } /* end "j": multiple eigenvalues */

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

/* end of fem_eige.c */

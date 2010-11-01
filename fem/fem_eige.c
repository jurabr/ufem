/*
   File name: fem_eige.c
   Date:      2007/10/22 15:47
   Author:    Jiri Brozovsky

   Copyright (C) 2007, 2010 Jiri Brozovsky

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
extern int fem_add_loads(void);
extern int fem_add_disps(long disp_mode);
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

 	if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }

  femMatPrn(&M, "MASS");


  for (j=1; j<=femEigenNum; j++)
  {
    rv = AF_ERR_VAL ; /* initial return value */
    solID = j ;

  for (i=0; i<max_iter; i++)
  {
#if 1
    femValVecMultSelf(8.8,&u) ;
#if 0
    if ((i > 0)||(j>1)) { if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; } }
#endif
#endif

    /* Gram-Schmidt: */
#if 1
    if (j > 1)
    {
      femVecSetZeroBig(&Fr);
      femVecSetZeroBig(&F);

      for (jj=0; jj<(j-1); jj++)
      {
        femVecSetZeroBig(&eig_xM) ;
        femMatVecMultBig(&M, &u, &eig_xM);
        c = femVecVecMultBig(&eig_y[jj], &eig_xM) ;
printf("C[%2li/%2li] = %e \n",jj,j-1, c);
        femVecAddVec(&Fr, c, &eig_y[jj]);
      }
      femVecLinCombBig(1.0, &u, -1.0, &Fr, &F) ;
      femVecClone(&F, &u);
    }
#endif
    /* end of Gram-Schmidt */

    femVecSetZeroBig(&eig_xM) ;
    femMatVecMultBig(&M, &u, &eig_xM);

#if 0
    if ((rv = femEqsCGwJ(&K, &eig_xM, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK)
#else
    if ((rv = femEqsBiCCSwJ(&K, &eig_xM, &u, FEM_ZERO/1000.0, 3*nDOFAct)) != AF_OK)
#endif
      { goto memFree; } 

    printf("NORM: %e %e\n", femVecNormBig(&u), femVecNormBig(&eig_y[0]));
    
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

printf("OMEGA[%li/%li] = %f (%f/%f)\n",i,max_iter,sqrt(fabs(omega)),om_top, om_bot);    

    if (i > 0)
    {
      if ((fabs(omega - omega0)/omega) <= (pow(10,(-2.0*eps))))
      {
#ifdef RUN_VERBOSE
        fprintf(msgout,"[i] %s: %e (%s %li)\n",_("Eigenvalue"),
            sqrt(fabs(omega)),_("in iteration"),i+1);
#endif

        /* we are converged */
	      if ((rv = femWriteRes(fem_output_file())) != AF_OK) { goto memFree; }

        femVecPrn(&u, "EIGENVECTOR");
        rv = AF_OK ;
        break ;
      }
    }

    omega0 = omega ;
    
  }
    if (rv != AF_OK)
    {
      /* TODO */
#ifdef RUN_VERBOSE
      fprintf(msgout, "[E] %s %li %s!\n", _("Computation of eigenvalue no."), j ,_("failed"));
      break ;
#endif
    }

    solSimNum = sqrt(fabs(omega)) ;
  
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
 * Algoritm is based on LU decopozition. A = LU, A_{i+1} = UL
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

#if 0
  for (i=1; i<=n; i++)
  {
    printf(" %2.4e   %2.4e   %2.4e\n", 
        femVecGet(alpha, i),
        femVecGet(beta, i),
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
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"EIG-LU STEP: %li\n", j);
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
#if 0

  tVector r, rr, q, p, s, t, a, b, c;
  double beta, alpha, betadiv;
  long n  = 0;
  long i, j;

  n = K->rows ;
  if (n < 1) { rv = AF_ERR_EMP ; goto memFree ;}

  femVecNull(&r);
  femVecNull(&rr);
  femVecNull(&q);
  femVecNull(&p);
  femVecNull(&s);
  femVecNull(&t);
  femVecNull(&a);
  femVecNull(&b);
  femVecNull(&c);

  femVecFullInit(&r, n);
  femVecFullInit(&rr, n);
  femVecFullInit(&q, n);
  femVecFullInit(&p, n);
  femVecFullInit(&s, n);
  femVecFullInit(&t, n);
  femVecFullInit(&a, n);
  femVecFullInit(&b, n);
  femVecFullInit(&c, n);
  
  femVecPut(&r, 1, 1.0);

  /* beta = sqrt({r}^T.[M].{r}) */
  femMatVecMult(M, &r, &s);
  beta = sqrt (femVecVecMult(&r, &s) );

  printf("BETA0 = %e\n", beta );

  if (beta == 0.0)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Eigenvalue solver stopped: beta = 0 not allowed"));
#endif
    rv = AF_ERR_VAL ;
    goto memFree;
  }
 
  for (i = 1; i <= k; i++)
  {
printf("==================== i = %li\n", i );
 	  /* {q} = {r} / beta */
    betadiv = 1.0 / beta ;
    femValVecMult(betadiv, &r, &q) ;

printf("1/BETA = %e (r=%e, q=%e)\n", betadiv, femVecGet(&r,1), femVecGet(&q,1)); 

    /* p = [M]^{-1}.K.q */
	  femMatVecMult(K, &q, &s);
	  femEqsCGwJ(M, &s, &p, 1e-8, 3 * n);
	
	  /* alpha = {q}[M]{p} */
	  femMatVecMult(M, &p, &s);
	  alpha = femVecVecMult(&q, &s);
    femVecPut(&a, i, alpha);

printf("ALPHA1 = %e\n", alpha );
printf("BETA1  = %e\n", beta );

	  /* {r} = {p} - alpha * {q} - beta * {t} */
    femVecLinCombBig(1.0,  &p, -1.0 * alpha, &q, &rr);
	  femVecLinCombBig(1.0, &rr, -1.0 * beta,  &t, &r);

#if 1
for (j=1; j<=4; j++) {printf("_%f", femVecGet(&r,j));}
printf("\n----------------------------\n");
#endif

	  /* beta = sqrt({r}^T.[M].{r}) */
    femMatVecMult(M, &r, &s);
    beta = femVecVecMult(&r, &s);
	  beta = sqrt(beta);
    femVecPut(&b, i, beta );
    femVecPut(&c, i, beta );

printf("ALPHA = %e\n", alpha );
printf("BETA  = %e\n", beta );

    femVecClone(&q, &t) ;
  }

printf("CCCCCCCCCCCCCCCCCCCCCCCCCC\n");
  rv = femTriEigLU(&a, &b, &c);
printf("CCCCCCCCCCCCCCCCCCCCCCCCCC\n");

memFree:
  femVecFree(&r);
  femVecFree(&rr);
  femVecFree(&q);
  femVecFree(&p);
  femVecFree(&s);
  femVecFree(&t);
  femVecFree(&a);
  femVecFree(&b);
  femVecFree(&c);
#endif
  return(rv);
}

int femSolveEigenLanczos(long max_iter, double eps)
{
	int  rv = AF_ERR_VAL;
#if 0
  long i, j, jj ;
  double xmult ;
  double om_top ;
  double om_bot ;
  double omega = 0.0 ;
  double omega0 = 0.0 ;
  double c ;

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */

 	if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }

  for (i=1; i<=nDOFAct; i++) { femVecPut(&u, i, 1.0) ; /* initial approximation */ }

 	if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }

 	if ((rv = fem_fill_M()) != AF_OK) { goto memFree; } /* mass matrix*/

  femMatPrn(&M, "MASS");

printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
  rv = femEigLanczos(&K, &M, femEigenNum, &u) ;
printf("YYYYYYYYYYYYYYYYYYYYYYYYYYYYY\n");

#if 0
  for (j=1; j<=femEigenNum; j++)
  {
    rv = AF_ERR_VAL ; /* initial return value */

    if (rv != AF_OK)
    {
      /* TODO */
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

  } /* end "j": multiple eigenvalues */
#endif

memFree:
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


/* ############################################################ */


/** Does one step of explicit solution 
 *
 * */
int fem_dyn_implicit_step(
		tMatrix *K,
		tMatrix *M,
		tMatrix *C,
		tVector *F,
		double d_t,
		tVector *u0, 
		tVector *u1, 
		tVector *u2
		)
{
	int rv = AF_OK;
	tVector Rint, MD1, CD1, MD0, CD0, expV1, expV2 ;

	/* TODO: move this to fem_sol_aloc() / fem_sol.c: */
	femVecNull(&Rint);
	femVecNull(&MD1);
	femVecNull(&CD1);
	femVecNull(&MD0);
	femVecNull(&CD0);
	femVecNull(&expV1);
	femVecNull(&expV2);

	if ((rv = femVecFullInit(&Rint, nDOFAct)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&MD1, nDOFAct)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&CD1, nDOFAct)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&MD0, nDOFAct)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&CD0, nDOFAct)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&expV1, nDOFAct)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&expV2, nDOFAct)) != AF_OK) { goto memFree; }

	/* Computation of components: */
	femMatVecMult(K, u0, &Rint);
	femMatVecMult(M, u1, &MD1); femValVecMultSelf(2.0/(d_t*d_t),&MD1);
	femMatVecMult(C, u1, &CD1); femValVecMultSelf(1.0/(d_t),&CD1);
	femMatVecMult(M, u0, &MD0); femValVecMultSelf(2.0/(d_t*d_t),&MD0);
	femMatVecMult(C, u0, &CD0); femValVecMultSelf(1.0/(d_t),&CD0);

	femVecLinCombBig(1.0, F, -1.0, &Rint, &expV1);
	femVecLinCombBig(1.0, &expV1, 1.0, &MD1, &expV2);
	femVecLinCombBig(1.0, &expV2, -1.0, &CD1, &expV1);
	femVecLinCombBig(1.0, &expV1, -1.0, &MD0, &expV2);
	femVecLinCombBig(1.0, &expV2, +1.0, &CD0, &expV1); /* all right hand side*/

	femValVecMultSelf((d_t*d_t),&expV1);

#if 0
  if ((rv = femEqsCGwJ(M, &expV1, u2, FEM_ZERO/10000.0, nDOFAct)) != AF_OK)
#else
  if ((rv = femEqsBiCCSwJ(M, &expV1, u2, FEM_ZERO/1000.0, 3*nDOFAct)) != AF_OK)
#endif
     { goto memFree; } 

	femVecPrn(u2, "U2");

memFree:

	femVecFree(&Rint);
	femVecFree(&MD1);
	femVecFree(&CD1);
	femVecFree(&MD0);
	femVecFree(&CD0);

	femVecFree(&expV1);
	femVecFree(&expV2);

	return(rv);
}


/* end of fem_eige.c */

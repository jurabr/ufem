/*
   File name: fem_eqs.c
   Date:      2003/04/13 10:38
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

	 FEM Solver - linear equation system solver(s)

	 $Id: fem_eqs.c,v 1.13 2005/07/11 17:56:16 jirka Exp $
*/

#include "fem_math.h"

/** Computes r = A.x - b
 * @param a matrix
 * @param x results
 * @param b right-side
 * @param r computed residuum vector
 * @return state value
 */
int eqsCompResid(tMatrix *a, tVector *x, tVector *b, tVector *r)
{
	long  i,j;

#ifdef DEVEL
	if ((a->cols != b->rows)||(b->rows != x->rows)||(x->rows != r->rows))
		 { return(AF_ERR_SIZ); }
	if ((b->type != VEC_FULL)||(x->type != VEC_FULL)||(r->type != VEC_FULL))
		 { return(AF_ERR_TYP); }
#endif

	if (a->type == MAT_SPAR)
	{
		for (i=0; i<a->rows; i++)
		{
			r->data[i] = 0 - b->data[i] ;
			
			for (j=a->frompos[i]; j<(a->frompos[i]+a->defpos[i]); j++)
			{
				if (a->pos[j] <= 0) {break;}
				r->data[i] += (a->data[j] * x->data[a->pos[j]-1]);
			}
		}
	}
	else
	{
		for (i=1; i<=a->rows; i++)
		{
			femVecPut(r,i,  (0 - femVecGet(b,i)));
			for (j=1; j<a->cols; j++)
			{
				femVecAdd(r,i, femMatGet(a, i,j) * femVecGet(x,j) );
			}
		}

	}

	return(AF_OK);
}



/** Conjugate gradient method with Jacobi preconditioner 
 *  (for symetric matrices only!) 
 *  @param a      matrix
 *  @param b      "load" vector
 *  @param x      results (vector - given as first iteration)
 *  @param eps    error (min.)
 *  @param maxIt  max. number of iterations
 *  @return state value
 */
int femEqsCGwJ(tMatrix *a, tVector *b, tVector *x, double eps, long maxIt)
{
	tVector  M; /* Jacobi preconditioner (diag[A] ;-) */
	tVector  r;
	tVector  z;
	tVector  p;
	tVector  q;
	double   ro, alpha, beta;
	double   roro = 0.0 ;
	long     n=0;
	long     i,j;
	int      rv = AF_OK;
	int      converged = AF_NO;
	double   normRes, normX, normA, normB;

	if ((a->cols != x->rows) || (x->rows != b->rows)) {return(AF_ERR_SIZ);}
	
	n = a->rows;

	normA = femMatNormBig(a);
	normB = femVecNormBig(b);

	if (normB <= 0.0)
	{
		femVecSetZeroBig(x);
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"[ ]  %s!\n",_("solution done without iterations because of zero load"));
#endif
		return(AF_OK);
	}

	/* vector initialization */
  femVecNull(&M);
  femVecNull(&r);
  femVecNull(&z);
  femVecNull(&p);
  femVecNull(&q);
	/* memory allocation */
  if ((rv=femVecFullInit(&M,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&r,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&z,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&p,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&q,   n))!=AF_OK) { goto memFree; }


	/* Jacobi preconditioner creation: */
	for (i=1; i<=n; i++) 
	{ 
		M.data[i-1] = femMatGet(a, i,i); 
		if (fabs(M.data[i-1]) < FEM_ZERO)
		{
			rv = AF_ERR_ZER;
#ifdef RUN_VERBOSE
 fprintf(msgout,"[ ]   %s[%li][%li] %s\n",_("matrix member"),i,i, _("has zero size"));
#endif
			goto memFree;
		}
	}

	
	/* next two lines mean: r = b - A*x  */
  femMatVecMultBig(a, x, &r);
	for (i=0; i<n; i++) { r.data[i] = b->data[i] - r.data[i] ; }


	/* main loop */
	for (i=1; i<= maxIt; i++)
	{
#ifdef DEVEL_VERBOSE
 fprintf(msgout,"[ ]   %s %li\n",_("linear step"),i);
#endif

	  /* using preconditioner: */
		for (j=0; j<n; j++) { z.data[j] = (r.data[j] / M.data[j]) ; }

		ro = femVecVecMultBig(&r,&z);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"ro = %f\n",ro);
#endif

		if (i == 1)
		{
		  for (j=0; j<n; j++) { p.data[j] = z.data[j]; }
		}
		else
		{
			beta = ro / roro ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"beta = %f\n",beta);
#endif

		  for (j=0; j<n; j++) 
			    { p.data[j] = (z.data[j] + (beta*p.data[j])) ; }

		}

		femMatVecMultBig(a,&p,&q);

		alpha = ro / femVecVecMultBig(&p,&q);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"alpha = %f\n",alpha);
#endif
		
		for (j=0; j<n; j++) 
		{ 
			x->data[j] = x->data[j] + (alpha * p.data[j])  ; 
			r.data[j] = r.data[j] - (alpha * q.data[j])  ; 
		}

		/* Convergence testing */

	  normRes = femVecNormBig(&r);
	  normX   = femVecNormBig(x);

		/* convergence test */
		/*if (fabs(norm - norm0) < eps )*/
    if (normRes  <= (eps*((normA*normX) + normB)) ) 
		{
			converged = AF_YES;
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[ ]  %s %li %s!\n",_("linear solution done in"),i,_("iterations"));
#endif
			break;
		}

#ifdef DEVEL_VERBOSE
#if 1
fprintf(msgout,"[i] Convergence test %f < %f (step %li from %li)\n",
				normRes  , (eps*((normA*normX) + normB)) , i, maxIt
				);
#endif
#endif

		roro = ro;

	} /* end of main loop */

	/*fprintf(msgout,"[I] normRes = %f\n",normRes);*/

	if (converged != AF_YES)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n",_("unconverged solution"));
#endif
		rv=AF_ERR;
	}
	
	/* freeing memory: */
memFree:
	femVecFree(&M);
	femVecFree(&r);
	femVecFree(&z);
	femVecFree(&p);
	femVecFree(&q);

	return(rv);
}

/** Bi-Conjugate Gradient Stabilized Method with Jacobi preconditioner 
 *  (for symetric and non-symetric matrices) 
 *  @param a      matrix
 *  @param b      "load" vector
 *  @param x      results (vector - given as first iteration)
 *  @param eps    error (min.)
 *  @param maxIt  max. number of iterations
 *  @return state value
 *
 *  Note: "res" is probably useless and *NormBig(res) can be replaced by *NormBig(r).
 *  Test it!!
 *  
 */
int femEqsBiCCSwJ(tMatrix *a, tVector *b, tVector *x, double eps, long maxIt)
{
  tVector    M ; /* preconditioner (diag[a])*/
  tVector    r ;
  tVector    rr ;
  tVector    p ;
  tVector    pp ;
  tVector    s ;
  tVector    ss ;
  tVector    t ;
  tVector    v ;
	double     ro, beta ;
	double     roro  = 0.0 ;
	double     alpha = 0.0 ;
	double     omega = 0.0 ;
	long       i,j;
	long       n; /* size of matrix "a"*/
	int        converged = AF_NO ;
	tVector    res ; /* residuum */
	double     normRes, normX, normA, normB; /* norms */
	int        rv = AF_OK ;

	n = a->rows ;

	normA   = femMatNormBig(a) ;

	normX   = femVecNormBig(x) ;
	normB   = femVecNormBig(b) ;

	/* vector initialization */
  femVecNull(&M);
  femVecNull(&r);
  femVecNull(&rr);
  femVecNull(&p);
  femVecNull(&pp);
  femVecNull(&s);
  femVecNull(&ss);
  femVecNull(&t);
  femVecNull(&v);
#if 1
  femVecNull(&res);
#endif
	/* memory allocation */
  if ((rv=femVecFullInit(&M,   n)) != AF_OK) {goto memFree;}
  if ((rv=femVecFullInit(&r,   n)) != AF_OK) {goto memFree;}
  if ((rv=femVecFullInit(&rr,  n)) != AF_OK) {goto memFree;}
  if ((rv=femVecFullInit(&p,   n)) != AF_OK) {goto memFree;}
  if ((rv=femVecFullInit(&pp,  n)) != AF_OK) {goto memFree;}
  if ((rv=femVecFullInit(&s,   n)) != AF_OK) {goto memFree;}
  if ((rv=femVecFullInit(&ss,  n)) != AF_OK) {goto memFree;}
  if ((rv=femVecFullInit(&t,   n)) != AF_OK) {goto memFree;}
  if ((rv=femVecFullInit(&v,   n)) != AF_OK) {goto memFree;}
#if 1
  if ((rv=femVecFullInit(&res, n)) != AF_OK) {goto memFree;}
#endif

	/* Jacobi preconditioner creation: */
	for (i=1; i<=n; i++) 
	{ 
		M.data[i-1] = femMatGet(a, i,i); 
		if (fabs(M.data[i-1]) < FEM_ZERO)
		{
			rv = AF_ERR_ZER;
#ifdef RUN_VERBOSE
 fprintf(msgout,"[ ]   %s[%li][%li] %s\n",_("matrix member"),i,i, _("has zero size"));
#endif
#if 1
			goto memFree;
#endif
		}
	}

	
	/* next two lines mean: r = b - A*x  */
  femMatVecMultBig(a, x, &r);
	for (i=0; i<n; i++) 
	{ 
		r.data[i] = b->data[i] - r.data[i] ; 
		rr.data[i] = r.data[i] ;
	}

  /* convergence test */
	if (femVecNormBig(&r) <= FEM_ZERO)
	{
    converged = AF_YES ;
    goto memFree;
	}	


  /* main loop */
	for (i = 1; i <= maxIt; i++)
	{
#ifdef DEVEL_VERBOSE
 fprintf(msgout,"[ ]   %s %li\n",_("linear step"),i);
#endif
		
		ro = femVecVecMultBig(&rr, &r);
		
		if (fabs(ro) <= 0.0) 
		{
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s!\n",_("solution interrupted on zero value")); 
#endif
      goto memFree;
    }

		if (i == 1) /* in first iteration*/
		{
			for (j=0; j<n; j++) { p.data[j] = r.data[j] ; }
		}
		else /* int all iterations except first */
		{
			beta = (ro / roro) * (alpha/omega) ;

			for (j=0; j<n; j++) 
			{ 
        p.data[j] =  r.data[j] + beta*(p.data[j] - (omega*v.data[j]) ) ; 
			}
		}

		/* using preconditioner M.pp=p -> pp */
	  for (j=0; j<n; j++) 
    {	
      pp.data[j] = p.data[j] / M.data[j] ; 
    }

		femMatVecMultBig(a, &pp, &v);

		alpha = (ro / femVecVecMultBig(&rr, &v));

    for (j=0; j<n; j++) { s.data[j] = r.data[j] - (alpha*v.data[j]) ; }


		/* test of "s" size */
		if (femVecNormBig(&s) <= FEM_ZERO)
		{
			for (j=0; j<n; j++) { x->data[j] += alpha*pp.data[j] ; }

			converged = AF_YES;
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[ ]  %s %li %s!\n",_("linear solution done in"),i,_("iterations"));
#endif
			break;
		}

		/* using preconditioner M.ss=s -> ss */
	  for (j=0; j<n; j++) {	ss.data[j] = s.data[j] / M.data[j] ; }

    femMatVecMultBig(a, &ss, &t) ;

		omega = (femVecVecMultBig(&t,&s) / femVecVecMultBig(&t,&t)) ;


		for (j = 0; j<n; j++) 
		{ 
			x->data[j] += alpha*pp.data[j] + (omega*ss.data[j]) ;  
			r.data[j] = s.data[j] - (omega*t.data[j]) ;
		}

		roro = ro;
		
    /* Convergence testing */
#if 1
    eqsCompResid(a, b, x, &res);

	  normRes = femVecNormBig(&res);
#else
	  normRes = femVecNormBig(&r);
#endif
	  normX   = femVecNormBig(x);

    if (normRes  < (eps*((normA*normX) + normB)) ) 
		{
			converged = AF_YES;
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[ ]  %s %li %s!\n",("solution done in"),i,("iterations"));
#endif
			break;
		}
		
	} /* end of main loop */


	if (converged != AF_YES)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] BiCGS: %s!\n",("unconverged solution"));
#endif
	}

	/* freeing of memory: */
memFree:
	femVecFree(&M  );
	femVecFree(&r  );
	femVecFree(&rr );
	femVecFree(&p  );
	femVecFree(&pp );
	femVecFree(&s  );
	femVecFree(&ss );
	femVecFree(&t  );
	femVecFree(&v  );
#if 1
	femVecFree(&res);
#endif
  
  return(AF_OK);
}

/** Solver that uses LU - for full matrices!
 *  @param a      matrix
 *  @param b      "load" vector
 *  @param x      results (vector - given as first iteration)
 *  @param eps    error (min.)
 *  @param maxIt  max. number of iterations
 *  @return state value
 *
 *  Note: "res" is probably useless and *NormBig(res) can be replaced by *NormBig(r).
 *  Test it!!
 *  
 */
int femEqsLU(tMatrix *a, tVector *b, tVector *x, double eps, long maxIt)
{
	int     rv = AF_OK ;
	long    n ;
	tVector indx ;

	if ((n = a->rows) <= 0) {return(AF_ERR_SIZ);}
	femVecNull(&indx);
	if ((rv=femVecFullInit(&indx, n)) != AF_OK) {goto memFree;}

  if ((rv=femLUdecomp(a, &indx)) != AF_OK)
  { 
#ifdef RUN_VERBOSE
    fprintf(msgout, "[E] %s!\n", _("LU decomposition failed"));
#endif
    goto memFree;
  }

  if ((rv=femLUback(a, &indx, b)) != AF_OK)
  { 
#ifdef RUN_VERBOSE
    fprintf(msgout, "[E] %s!\n", _("Backward run of LU failed"));
#endif
    goto memFree;
  }

memFree:
	femVecFree(&indx);
	return(rv);
}

/** Alternative version of the Conjugate Gradient Method () */
int femEqsPCGwJ(tMatrix *a, tVector *b, tVector *x, double eps, long maxIt)
{
	int      rv = AF_OK ;
#if 1
  long     converged = AF_NO ;
	double   nui, dei ;
	double   lambda ;
	double   alpha ;
	double   normRes, normX, normA, normB; /* norms */
	tVector  p  ;
	tVector  r  ;
	tVector  d  ;
	tVector  M  ; /* Jacobi preconditioner */
	tVector  ap ; /* a*p result vector  */
	long     n ;  /* number of rows */
	long     i, j ;

	if ((a->rows != x->rows) || (x->rows != b->rows)) {return(AF_ERR_SIZ);}

	n = a->rows;

	normA = femMatNormBig(a);
	normB = femVecNormBig(b);
	normX = femVecNormBig(x);

	if (normB <= 0.0)
	{
		femVecSetZeroBig(x);
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"[ ]  %s!\n",_("solution done without iterations because of zero load"));
#endif
		return(AF_OK);
	}

	/* vector initializations */
	femVecNull(&p);
	femVecNull(&r);
	femVecNull(&d);
	femVecNull(&M);
	femVecNull(&ap);

	/* memory allocation */
  if ((rv=femVecFullInit(&p,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&r,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&d,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&M,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&ap,  n))!=AF_OK) { goto memFree; }

	/* Jacobi preconditioner */
	for (i=1; i<=n; i++) 
	{ 
		M.data[i-1] = femMatGet(a, i, i ); 
		if (fabs(M.data[i-1]) < FEM_ZERO)
		{
			rv = AF_ERR_ZER;
#ifdef DEVEL_VERBOSE
 fprintf(msgout,"[ ]   %s[%li][%li] %s\n",_("matrix member"),i,i, _("has zero size"));
#endif
			goto memFree;
		}
	}

	/* next several lines mean: r = b - A*x  */
  femMatVecMultBig(a, x, &r);
	for (i=0; i<n; i++) { r.data[i] = b->data[i] - r.data[i] ; }

	/* using preconditioner: */
	for (j=0; j<n; j++) 
	{ 
		d.data[j] = (r.data[j] / M.data[j]) ; 
		p.data[j] = d.data[j]  ;
	}
	
	for (i=1; i<= maxIt; i++)
	{
#ifdef DEVEL_VERBOSE
 		fprintf(msgout,"[ ]   %s %li (%s %li)\n",_("linear step"),i,_("from"),maxIt);
#endif

 		/* untested code follows... */

		femMatVecMultBig(a, &p, &ap) ;
		
		nui = femVecVecMultBig(&r, &d) ;
		dei = femVecVecMultBig(&p, &ap) ;

		lambda = nui / dei ;

#ifdef DEVEL_VERBOSE
		printf("NUI = %f DEI = %f LAMBDA = %f\n", nui, dei, lambda);
#endif

		for (j=0; j<n; j++) 
		{ 
			x->data[j] += (lambda * p.data[j]) ; 
			
			r.data[j] = (r.data[j] - (lambda * ap.data[j])) ; 
			d.data[j] = (r.data[j] / M.data[j]) ; 
		}

	  normRes = femVecNormBig(&r);
	  normX   = femVecNormBig(x);
		
		nui = femVecVecMultBig(&r, &d) ;
		dei = femVecVecMultBig(&p, &ap) ;

#ifdef DEVEL_VERBOSE
		printf("NORMS: A=%f X=%f B=%f <> R=%f\n",
				normA, normX, normB, normRes
				);
#endif

		/* convergence test */
    if (normRes  < (eps*((normA*normX) + normB)) ) 
		{
			converged = AF_YES;
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[ ]  %s %li %s!\n",_("solution done in"),i,_("iterations"));
#endif
			break;
		}

		alpha = nui / dei ;
		for (j=0; j<n; j++) 
		{ 
			p.data[j] = (d.data[j] + alpha*p.data[j]) ; 
		}
		
	} /* end of "for i" */

		femVecPrn(x, "X");

memFree:
	femVecFree(&p);
	femVecFree(&r);
	femVecFree(&d);
	femVecFree(&M);
	femVecFree(&ap);
#endif
  return(rv);
}

/** Choleski decomposition - forward run only! 
 * @param a matrix (must  be a MAT_FULL)
 * @return status
 */
int femMatCholFact(tMatrix *a, tVector *C)
{
  int rv = AF_OK ;
	double  sum ;
	long    n ;
	long    i,j,k;
  long    have_C = AF_NO ;

	n = a->rows ;

  if (C != NULL)
  {
    if (C->rows != a->rows)
    {
      return(AF_ERR_VAL) ;
    }
    else
    {
      have_C = AF_YES ;
    }
  }

  if (have_C == AF_NO)
  {
    femVecNull(C);
    if (femVecFullInit(C,   n) != AF_OK) {goto memFree;}
  }

  for (i=1; i<=n; i++)
  {
    for (j=i; j<=n; j++)
    {
      sum = femMatGet(a, i,j);
      for (k=i-1; k>=1; k--)
      {
        sum -= femMatGet(a, i,k) * femMatGet(a, j, k) ;
      }
      if (i == j)
      {
        if (sum <= 0.0)
        {
          rv = AF_ERR_VAL ;
#ifdef RUN_VERBOSE
					fprintf(msgout, "[E] %s!\n", _("Given matrix is singular"));
#endif
          goto memFree ;
        }
        femVecPut(C,i, sqrt(sum));
      }
      else
      {
        femMatPut(a, j, i, sum / femVecGet(C,i));
      }
    }
  }

  for (i=1; i<=n; i++)
  {
    for (j=i; j<=n; j++)
    {
      if (i != j)
      {
        femMatPut(a,i,j, femMatGet(a,j,i));
        femMatPut(a,j,i, 0);
      }
      else
      {
        femMatPut(a,j,i, femVecGet(C,i));
      }
    }
  }

  femVecPrn(C, "C");

#if 0
  for (i=1; i<=n; i++)
  {
    for (j=1; j<=n; j++)
    {
      printf("_%e", femMatGet(a, i,j));
    }
    printf("\n ");
  }
#endif

	/* freeing of memory: */
memFree:
  if (have_C == AF_NO)
  {
	  femVecFree(C  );
  }
  
  return(rv);
}

/** Choleski decomposition - complete
 * @param a matrix (must  be a MAT_FULL)
 * @return status
 */
int femEqsChol(tMatrix *a, tVector *b, tVector *x)
{
  int rv = AF_OK ;
	double  sum ;
	long    n ;
	long    i,j,k;
  tVector C;

	n = a->rows ;

  femVecNull(&C);
  if (femVecFullInit(&C,   n) != AF_OK) {goto memFree;}

  for (i=1; i<=n; i++)
  {
    for (j=i; j<=n; j++)
    {
      sum = femMatGet(a, i,j);
      for (k=i-1; k>=1; k--)
      {
        sum -= femMatGet(a, i,k) * femMatGet(a, j, k) ;
      }
      if (i == j)
      {
        if (sum <= 0.0)
        {
          rv = AF_ERR_VAL ;
#ifdef RUN_VERBOSE
					fprintf(msgout, "[E] %s!\n", _("Given matrix is singular"));
#endif
          goto memFree ;
        }
        femVecPut(&C,i, sqrt(sum));
      }
      else
      {
        femMatPut(a, j, i, sum / femVecGet(&C,i));
      }
    }
  }

  /* backward run: */
  for (i=1; i<=n; i++)
  {
    sum = femVecGet(b,i);

    for (k=(i-1);k>=1; k--)
    {
      sum -= (femMatGet(a,i,k)*femVecGet(x,k)) ;
    }

    femVecPut(x,i, sum/femVecGet(&C,i));
  }

  for (i=n; i>=1; i--)
  {
    sum = femVecGet(x, i);
    for (k=(i+1);k<=n; k++)
    {
      sum -= femMatGet(a,k,i)*femVecGet(x, k);
    }
    femVecPut(x,i, sum/femVecGet(&C,i));
  }

	/* freeing of memory: */
memFree:
	femVecFree(&C);
  
  return(rv);
}


/** rotation for Jacobi computation of eigenvalues */
void femMatJacRotate (tMatrix *a, int i, int j, int k, int l, 
    double g, double h, double s, double tau)
{
  g = femMatGet(a, i,j);
  h = femMatGet(a, k,l);

  femMatPut(a, i,j, g - s*(h+g*tau) );
  femMatPut(a, k,l, h + s*(g-h*tau) );
}

/** Compute eigen numbers and vectors (Jacobi method)
 * @param a matrix to be analysed
 * @param d vector to store eigenvalues
 * @param v matrix to store eigenvectors
 * @return status
 */
int femMatEigenJacobi(tMatrix *a, tVector *d, tMatrix *v, long *nrot)
{
  long    iters = 100 ;
  int     i,iq, ip, j, n ;
  double  sm ;
  double  tresh, g, h, t, c, theta, s, tau ;
  double  checkp, checkq, checkh ;
  tVector b; 
  tVector z;

  *nrot = 0 ;

  n = a->rows ;

  femVecNull(&b);
  femVecNull(&z);

  femVecFullInit(&b, n);
  femVecFullInit(&z, n);

  for (i=1; i<=n; i++)
  {
    femVecPut( &b, i, femMatGet(a, i, i) );
    femVecPut(  d, i, femMatGet(a, i, i) );
    femVecPut( &z, i, 0.0 );

    femMatPut( v, i, i, 1.0 );
  }

  for (i=1; i<=iters; i++)
  {
    sm = 0.0 ;

    for (ip=0; ip<=(n-1); ip++)
    {
      for (iq=(ip+1); iq<=n; iq++)
      {
        sm += fabs( femMatGet(a, ip, iq) );
      }
    }

    if (sm <= FEM_ZERO) /* sum <= 0 so we are finished */
    {
      /*printf("iterations: %li\n", *nrot);*/
      femVecFree(&b);
      femVecFree(&z);
      return(0);
    }

    if (i < 4)
    {
      tresh = (0.2 * sm) / ((double)(n*n)) ;
    }
    else
    {
      tresh = 0.0 ;
    }

    for (ip=1; ip<=(n-1); ip++)
    {
      for (iq=(ip+1); iq<=n; iq++)
      {
        g = 100.0 * fabs(femMatGet(a,ip,iq));

        checkp = fabs(g*fabs(femVecGet(d, ip)) - fabs(femVecGet(d,ip)));
        checkq = fabs(g*fabs(femVecGet(d, iq)) - fabs(femVecGet(d,iq)));
        if ((i > 4) && (checkp <= FEM_ZERO) && (checkq <= FEM_ZERO))
        {
          /* off-diagonal elements are small */
          femMatPut (a, ip,iq, 0.0) ;
        }
        else
        {
          /* still are big.. */
          h = femVecGet(d, iq) - femVecGet(d, ip) ;

          checkh = fabs( fabs(h)+g - fabs(h) );

          if (checkh < FEM_ZERO)
          {
            if (h != 0.0) { t = femMatGet(a, ip, iq) / h ; }
            else          { t = 0 ; }
          }
          else
          {
            theta = (0.5*h) / femMatGet(a, ip, iq) ;
            t = 1.0 / ( fabs (theta) + sqrt(1.0 + pow(theta, 2)));
            if (theta < 0.0) { t = (-1.0)*t ; }
          }

          c = 1.0 / sqrt(1.0 + pow(t, 2) );
          s = t * c ;
          tau = s / (1.0 + c);
          h = t* femMatGet(a, ip, iq);

          femVecPut(&z, ip, femVecGet(&z, ip) - h );
          femVecPut(&z, iq, femVecGet(&z, iq) + h );

          femVecPut(d, ip, femVecGet(d, ip) - h );
          femVecPut(d, iq, femVecGet(d, iq) + h );
          
          femMatPut(a, ip, iq, 0.0);

          for (j=1; j<=ip-1; j++)
          {
            femMatJacRotate(a, j,ip,j,iq, g,h,s,tau);
          }

          for (j=ip+1; j<=iq-1; j++)
          {
            femMatJacRotate(a, ip,j,j,iq, g,h,s,tau);
          }

          for (j=iq+1; j<=n; j++)
          {
            femMatJacRotate(a, ip,j,iq,j, g,h,s,tau);
          }

          for (j=1; j<=n; j++)
          {
            femMatJacRotate(v, j,ip,j,iq, g,h,s,tau);
          }
          *nrot = *nrot + 1 ;
        }
      }
    }

    for (ip=1; ip<=n; ip++)
    {
      femVecAdd (&b, ip, femVecGet(&z, ip) );
      femVecPut ( d, ip, femVecGet(&b, ip) );
      femVecPut (&z, ip, 0.0 );

    }
  }

#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s\n", _("Out of iterations for eigendata"));
#endif

  return(-1);
}


/** Conjugate gradient method with SSOR preconditioner 
 *  (for symetric matrices only!) 
 *  @param a      matrix
 *  @param b      "load" vector
 *  @param x      results (vector - given as first iteration)
 *  @param eps    error (min.)
 *  @param maxIt  max. number of iterations
 *  @return state value
 */
int femEqsCGwSSOR(tMatrix *a, tVector *b, tVector *x, double eps, long maxIt)
{
	tVector  M; /* Jacobi preconditioner (diag[A] ;-) */
	tVector  r;
	tVector  z;
	tVector  zz;
	tVector  p;
	tVector  q;
	double   ro, alpha, beta;
	double   roro = 0.0 ;
	long     n=0;
	long     i,ii,j, ipos;
	int      rv = AF_OK;
	int      converged = AF_NO;
	double   normRes, normX, normA, normB, val;

	if ((a->cols != x->rows) || (x->rows != b->rows)) {return(AF_ERR_SIZ);}
	
	n = a->rows;

	normA = femMatNormBig(a);
	normB = femVecNormBig(b);

	if (normB <= 0.0)
	{
		femVecSetZeroBig(x);
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"[ ]  %s!\n",_("solution done without iterations because of zero load"));
#endif
		return(AF_OK);
	}

	/* vector initialization */
  femVecNull(&M);
  femVecNull(&r);
  femVecNull(&z);
  femVecNull(&zz);
  femVecNull(&p);
  femVecNull(&q);
	/* memory allocation */
  if ((rv=femVecFullInit(&M,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&r,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&z,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&zz,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&p,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&q,   n))!=AF_OK) { goto memFree; }

	/* Jacobi preconditioner creation: */
	for (i=1; i<=n; i++) 
	{ 
    val = femMatGet(a,i,i) ;
		if (fabs(val) < FEM_ZERO)
		{
			rv = AF_ERR_ZER;
#ifdef RUN_VERBOSE
 fprintf(msgout,"[ ]   %s[%li][%li] %s\n",_("matrix member"),i,i, _("has zero size"));
#endif
			goto memFree;
		}
		M.data[i-1] = 1.0 / val ; /* NOTE: M includes inverse of diagonal */
	}

	
	/* next two lines mean: r = b - A*x  */
  femMatVecMultBig(a, x, &r);
	for (i=0; i<n; i++) { r.data[i] = b->data[i] - r.data[i] ; }


	/* main loop */
	for (i=1; i<= maxIt; i++)
	{
#ifdef DEVEL_VERBOSE
 fprintf(msgout,"[ ]   %s %li\n",_("linear step"),i);
#endif

	  /* using preconditioner: */
 		if (a->type != MAT_SPAR)
		{
    	for (ii=1; ii<=n; ii++)
    	{
      	val = 0.0 ;
      	for (j=1; j<ii; j++)
      	{
        	val += femMatGet(a,ii, j) * femVecGet(&zz, j) ;
      	}

      	femVecPut(&zz, ii, femVecGet(&M,ii)*(femVecGet(&r,ii) - val) );
    	}

    	for (ii=n; ii>=1; ii--)
    	{
      	val = 0.0 ;
      	for (j=(ii+1); j<=n; j++)
      	{
        	val += femMatGet(a,ii, j) * femVecGet(&z, j) ;
      	}

      	femVecPut(&z, ii, femVecGet(&zz, ii) - femVecGet(&M,ii)* val );
    	}
		}
		else
		{
			/* faster code for MAT_SPAR: */
			for (ii=1; ii<=n; ii++)
    	{
      	val = 0.0 ;
				for (j=a->frompos[ii-1]; j<(a->frompos[ii-1]+a->defpos[ii-1]); j++)
				{
					ipos = a->pos[j] ;
					if ((ipos >= ii)||(ipos<1)) { continue; }
					val += a->data[j] * zz.data[ipos - 1] ;
				}
      	femVecPut(&zz, ii, femVecGet(&M,ii)*(femVecGet(&r,ii) - val) );
    	}

    	for (ii=n; ii>=1; ii--)
    	{
      	val = 0.0 ;
				for (j=a->frompos[ii-1]; j<(a->frompos[ii-1]+a->defpos[ii-1]); j++)
				{
					ipos = a->pos[j] ;
					if (ipos > ii)
					{
						val += a->data[j] * z.data[ipos - 1] ;
					}
				}
      	femVecPut(&z, ii, femVecGet(&zz, ii) - femVecGet(&M,ii)* val );
    	}
		}
    /* end of preconditioning */

		ro = femVecVecMultBig(&r,&z);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"ro = %f\n",ro);
#endif

		if (i == 1)
		{
		  for (j=0; j<n; j++) { p.data[j] = z.data[j]; }
		}
		else
		{
			beta = ro / roro ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"beta = %f\n",beta);
#endif

		  for (j=0; j<n; j++) 
			    { p.data[j] = (z.data[j] + (beta*p.data[j])) ; }

		}

		femMatVecMultBig(a,&p,&q);

		alpha = ro / femVecVecMultBig(&p,&q);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"alpha = %f\n",alpha);
#endif
		
		for (j=0; j<n; j++) 
		{ 
			x->data[j] = x->data[j] + (alpha * p.data[j])  ; 
			r.data[j] = r.data[j] - (alpha * q.data[j])  ; 
		}

		/* Convergence testing */

	  normRes = femVecNormBig(&r);
	  normX   = femVecNormBig(x);

		/* convergence test */
		/*if (fabs(norm - norm0) < eps )*/
    if (normRes  <= (eps*((normA*normX) + normB)) ) 
		{
			converged = AF_YES;
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[ ]  %s %li %s!\n",_("linear solution done in"),i,_("iterations"));
#endif
			break;
		}

#ifdef DEVEL_VERBOSE
#if 1
fprintf(msgout,"[i] Convergence test %f < %f (step %li from %li)\n",
				normRes  , (eps*((normA*normX) + normB)) , i, maxIt
				);
#endif
#endif

		roro = ro;

	} /* end of main loop */

	/*fprintf(msgout,"[I] normRes = %f\n",normRes);*/

	if (converged != AF_YES)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n",_("unconverged solution"));
#endif
		rv=AF_ERR;
	}
	
	/* freeing memory: */
memFree:
	femVecFree(&M);
	femVecFree(&r);
	femVecFree(&z);
	femVecFree(&zz);
	femVecFree(&p);
	femVecFree(&q);

	return(rv);
}

/* end of fem_eqs.c */

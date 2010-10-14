/*
   File name: fem_meqs.c
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

	 FEM Solver - linear equation system solver(s) - MPI version

	 $Id: fem_meqs.c,v 1.10 2004/08/01 11:08:57 jirka Exp $
*/

#include "fem_math.h"
#ifdef USE_MPI
#include "fem_mpi.h"

/** Matrix by vector multiplication (a[m,n]*b[n] = c[m])
 * @param a matrix
 * @param b vector
 * @param c vector (result)
 * @return state value
 * works only if a = MAT_SPAR, b,c = VEC_FULL !!
 */
int femMatVecMultMPI(tMatrix *a, tVector *b, tVector *c)
{
	long   i, ii, j;
	double val;

	if ((a->cols != b->rows)||(c->rows != a->rows)) { return(AF_ERR_SIZ); }

	if (a->type != MAT_SPAR) {return(AF_ERR_VAL);}
	if (b->type != VEC_FULL) {return(AF_ERR_VAL);}
	if (c->type != VEC_FULL) {return(AF_ERR_VAL);}

	
	femVecSetZero(c);
	for (i=ppDOFfrom; i<=ppDOFto; i++)
	{
		ii = i - ppDOFfrom ;

		val = 0;
		for (j=a->frompos[ii]; j<a->frompos[ii]+a->defpos[ii]; j++)
		{
		  if (a->pos[j] <= 0) {break; /*speedup*/} 
			val +=  ( a->data[j] * b->data[a->pos[j]-1] ) ;
		}
		c->data[ii] = val ;
	}
	
	return(AF_OK);
}

/** Computes Euclide norm of vector sum(sqrt(a*a)) 
 *  @param a     vector
 *  @return norm
 */
double femVecNormMPI(tVector *a)
{
	double Norm = 0 ;
	double val  = 0 ;
	int i;

	Norm = 0.0;

	if (a->type == VEC_FULL)
	{
		for (i=0; i<a->rows; i++)
		{
			Norm += (a->data[i]*a->data[i]) ;
		}
	}
	else
	{
		for (i=1; i<=a->rows; i++)
		{
			val = femVecGet(a, i);
			Norm += (val*val);
		}
	}

	return((Norm));
}


/** Parallel version of the Conjugate Gradient Method works? */
#if 0
int femMPIeqsPCGwJ(tMatrix *a, tVector *b, tVector *x, tVector *x_big, double eps, long maxIt)
{
	int      rv = AF_OK ;
	double   nui, dei ;
	double   lambda ;
	double   alpha ;
	double   normRes, normX, normA, normB, my_normA; /* norms */
	tVector  p  ;
	tVector  r  ;
	tVector  d  ;
	tVector  M  ; /* Jacobi preconditioner */
	tVector  ap ; /* a*p result vector  */
	tVector  p_big ; /* a*p result vector  */
	long     n ;  /* number of rows */
	long     sum   = 0 ;
	int     *sizes = NULL;
	int     *disps = NULL;
	double	 val2[4];    /* some stuff for messages */
	double	 my_val2[4]; /* some stuff for messages */
	int      converged = AF_NO ;
	long     i, ii, j ;

	if ((a->rows != x->rows) || (x->rows != b->rows)) {return(AF_ERR_SIZ);}
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"SIZES (%i): a[%li,%li] x[%li] b[%li]\n", 
			ppRank, 
			a->rows, a->cols, x->rows, b->rows
			);
#endif

	if ((sizes = femDOFsizes()) == NULL) {return(AF_ERR_MEM);}
	if ((disps = (int *)malloc(sizeof(int)*ppSize)) == NULL) {return(AF_ERR_MEM);}
	sum = 0 ;
	disps[0]=0;
	for (i=1; i<ppSize; i++)
	{
		sum+=sizes[i-1];
		disps[i] = sum ;
	} 
	
	n = a->rows;

	my_normA = femMatNorm(a);

	normB = femVecNormMPI(b);
	normX = femVecNormMPI(x);
	my_val2[0] = normB; val2[0] = 0.0 ;
	my_val2[1] = normX; val2[1] = 0.0 ;

	MPI_Allreduce(&my_normA, &normA, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
	MPI_Allreduce(my_val2, val2, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	normB = sqrt(val2[0]) ;
	normX = sqrt(val2[1]) ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"NORMS (%i): %f %f %f\n", ppRank, normA, normB, normX);
#endif

	if (normB <= 0.0)
	{
		femVecSetZero(x);
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
	femVecNull(&p_big);

	/* memory allocation */
  if ((rv=femVecFullInit(&p,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&r,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&d,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&M,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&ap,  n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&p_big,  a->cols))!=AF_OK) { goto memFree; }

	/* Jacobi preconditioner */
	for (i=ppDOFfrom; i<=ppDOFto; i++) 
	{ 
		ii = i - ppDOFfrom + 1 ;

		M.data[ii-1] = femMatGet(a, ii, i ); 
		if (fabs(M.data[ii-1]) < FEM_ZERO)
		{
			rv = AF_ERR_ZER;
#ifdef DEVEL_VERBOSE
 fprintf(msgout,"[ ]   %s[%li][%li] %s\n",_("matrix member"),ii,i, _("has zero size"));
#endif
			goto memFree;
		}
	}

	femVecSetZero(x_big);
	/* all processes should be informed (really?)  */
	if (normX > 0.0)
	{
		MPI_Allgatherv(
					x->data, 
					x->rows, 
					MPI_DOUBLE, 
					x_big->data,
					sizes,
					disps,
					MPI_DOUBLE,
					MPI_COMM_WORLD
					) ;
	}

	/* next several lines mean: r = b - A*x  */
  femMatVecMultMPI(a, x_big, &r);
	for (i=0; i<n; i++) { r.data[i] = b->data[i] - r.data[i] ; }

	/* using preconditioner: */
	for (j=0; j<n; j++) 
	{ 
		d.data[j] = (r.data[j] / M.data[j]) ; 
		p.data[j] = d.data[j]  ;
	}
	
	/* unfortunatelly, p_big is needed */
	MPI_Allgatherv(
					p.data, 
					p.rows, 
					MPI_DOUBLE, 
					p_big.data,
					sizes,
					disps,
					MPI_DOUBLE,
					MPI_COMM_WORLD
					);

	for (i=1; i<= maxIt; i++)
	{
#ifdef DEVEL_VERBOSE
 		fprintf(msgout,"[ ]   %s %li (%s %li)\n",_("linear step"),i,_("from"),maxIt);
#endif

 		/* untested code follows... */

		femMatVecMultMPI(a, &p_big, &ap) ;
		
		nui = femVecVecMult(&r, &d) ;
		dei = femVecVecMult(&p, &ap) ;

		/* another communication:  */
		my_val2[0] = nui; val2[0] = 0.0 ;
		my_val2[1] = dei; val2[1] = 0.0 ;
		MPI_Allreduce(my_val2, val2, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
		nui = val2[0] ;
		dei = val2[1] ;

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

	  normRes = femVecNormMPI(&r);
	  normX   = femVecNormMPI(x);
		
		nui = femVecVecMult(&r, &d) ;
		dei = femVecVecMult(&p, &ap) ;

		/* one more communication:  */
		my_val2[0] = nui; val2[0] = 0.0 ;
		my_val2[1] = dei; val2[1] = 0.0 ;
		my_val2[2] = normRes; val2[2] = 0.0 ;
		my_val2[3] = normX; val2[3] = 0.0 ;
		MPI_Allreduce(my_val2, val2, 4, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
		nui     = val2[0] ;
		dei     = val2[1] ;
		normRes = sqrt(val2[2]) ;
		normX   = sqrt(val2[3]) ;

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
		
		/* p[i] -> p_big HERE! */
		MPI_Allgatherv(
					p.data, 
					p.rows, 
					MPI_DOUBLE, 
					p_big.data,
					sizes,
					disps,
					MPI_DOUBLE,
					MPI_COMM_WORLD
					);

	} /* end of "for i" */

	/* when we are finished: */
	MPI_Allgatherv(
					x->data, 
					x->rows, 
					MPI_DOUBLE, 
					x_big->data,
					sizes,
					disps,
					MPI_DOUBLE,
					MPI_COMM_WORLD
					);
		femVecPrn(x_big, "X_BIG");

memFree:
	femVecFree(&p);
	femVecFree(&r);
	femVecFree(&d);
	femVecFree(&M);
	femVecFree(&ap);
	femVecFree(&p_big);
  return(rv);
}
#endif

/** Parallel?? version of the Conjugate Gradient Method */
int femMPIeqsPCGwJ_New(tMatrix *a, tVector *b, tVector *x, tVector *x_big, double eps, long maxIt)
{
	int      rv = AF_OK ;
	double   alpha, beta, rho, my_rho, rho0, gamma, my_gamma ;
	double   normRes, normX, normA, normB, my_normA; /* norms */
	tVector  p  ;
	tVector  r  ;
	tVector  s  ;
	tVector  w  ;
	tVector  q  ;
	tVector  L  ; /* Jacobi preconditioner */
	tVector  p_big ; /* a*p result vector  */
	long     n ;  /* number of rows */
	long     sum   = 0 ;
	int     *sizes = NULL;
	int     *disps = NULL;
	double	 val2[2];    /* some stuff for messages */
	double	 my_val2[2]; /* some stuff for messages */
	int      converged = AF_NO ;
	long     i, ii, j ;

	if ((a->rows != x->rows) || (x->rows != b->rows)) {return(AF_ERR_SIZ);}
#ifdef DEVEL
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"SIZES (%i): a[%li,%li] x[%li] b[%li]\n", 
			ppRank, 
			a->rows, a->cols, x->rows, b->rows
			);
#endif
#endif

	if ((sizes = femDOFsizes()) == NULL) {return(AF_ERR_MEM);}
	if ((disps = (int *)malloc(sizeof(int)*ppSize)) == NULL) {return(AF_ERR_MEM);}
	sum = 0 ;
	disps[0]=0;
	for (i=1; i<ppSize; i++)
	{
		sum+=sizes[i-1];
		disps[i] = sum ;
	} 
	
	n = a->rows;

	my_normA = femMatNorm(a);

	normB = femVecNormMPI(b);
	normX = femVecNormMPI(x);
	my_val2[0] = normB; val2[0] = 0.0 ;
	my_val2[1] = normX; val2[1] = 0.0 ;

	MPI_Allreduce(&my_normA, &normA, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
	MPI_Allreduce(my_val2, val2, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	normB = sqrt(val2[0]) ;
	normX = sqrt(val2[1]) ;

#ifdef DEVEL
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"NORMS (%i): %f %f %f\n", ppRank, normA, normB, normX);
#endif
#endif

	if (normB <= 0.0)
	{
		femVecSetZero(x);
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"[ ]  %s!\n",_("solution done without iterations because of zero load"));
#endif
		return(AF_OK);
	}

	/* vector initializations */
	femVecNull(&p);
	femVecNull(&r);
	femVecNull(&s);
	femVecNull(&w);
	femVecNull(&q);
	femVecNull(&L);
	femVecNull(&p_big);

	/* memory allocation */
  if ((rv=femVecFullInit(&p,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&r,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&s,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&w,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&q,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&L,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&p_big,  a->cols))!=AF_OK) { goto memFree; }

	/* Jacobi preconditioner */
	for (i=ppDOFfrom; i<=ppDOFto; i++) 
	{ 
		ii = i - ppDOFfrom + 1 ;

		L.data[ii-1] = sqrt(fabs(femMatGet(a, ii, i ))); /* it should be always >=0, but.. */
		if (L.data[ii-1] < FEM_ZERO)
		{
			rv = AF_ERR_ZER;
#ifdef DEVEL_VERBOSE
 fprintf(msgout,"[ ]   %s[%li][%li] %s\n",_("matrix member"),ii,i, _("has zero size"));
#endif
			goto memFree;
		}
	}

	/* all processes should be informed */
	femVecSetZero(x_big);
	MPI_Allgatherv(
				x->data, 
				x->rows, 
				MPI_DOUBLE, 
				x_big->data,
				sizes,
				disps,
				MPI_DOUBLE,
				MPI_COMM_WORLD
				) ;

	alpha = 0.0 ;
	beta  = 0.0 ;
	rho   = 0.0 ;
	rho0  = 0.0 ;
  gamma = 0.0 ;

	/* next several lines mean: r = b - A*x  */
  femMatVecMultMPI(a, x_big, &r);
	for (j=0; j<n; j++) 
	{ 
		r.data[j] = b->data[j] - r.data[j] ; 
		
		s.data[j] = (1.0/L.data[j]) * r.data[j] ; /* L-1 */
	}

	my_rho = femVecVecMult(&s, &s) ;
	rho = 0.0 ;
	/* another communication:  */
	MPI_Allreduce(&my_rho, &rho, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

/*if (ppRank == 0) { printf("RHO IS %e\n", rho); }*/


	/* main loop */
	for (i=1; i<= maxIt; i++)
	{
#ifdef DEVEL_VERBOSE
 		fprintf(msgout,"[ ]   %s %li (%s %li)\n",_("linear step"),i,_("from"),maxIt);
#endif

		for (j=0; j<n; j++)
		{
			w.data[j] = (1.0/L.data[j]) * s.data[j] ; /* L-T */
			p.data[j] = w.data[j] + (beta * p.data[j]) ;
		}
	
		/* should be replaced with something less drastic !!! */
		MPI_Allgatherv(
					p.data, 
					p.rows, 
					MPI_DOUBLE, 
					p_big.data,
					sizes,
					disps,
					MPI_DOUBLE,
					MPI_COMM_WORLD
					) ;

		femMatVecMultMPI(a, &p_big, &q) ;

		my_gamma = femVecVecMult(&p, &q) ;
		gamma = 0.0 ;
		/* another communication:  */
		MPI_Allreduce(&my_gamma, &gamma, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

		alpha = rho / gamma ;

		for (j=0; j<n; j++)
		{
			x->data[j] += alpha * p.data[j] ;
		}

		for (j=0; j<n; j++)
		{
			r.data[j] -= alpha * q.data[j] ;
		}
		
		for (j=0; j<n; j++)
		{
			s.data[j] = (1.0/L.data[j]) * r.data[j] ; /* L-1 */
		}

		rho0 = rho ;
		my_rho = femVecVecMult(&s, &s) ;
		rho = 0.0 ;
		/* another communication:  */
		MPI_Allreduce(&my_rho, &rho, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	  normRes = femVecNormMPI(&r);
	  normX   = femVecNormMPI(x);

		/* one more communication:  */
		my_val2[0] = normRes; val2[0] = 0.0 ;
		my_val2[1] = normX; val2[1] = 0.0 ;
		MPI_Allreduce(my_val2, val2, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

		normRes = sqrt(val2[0]) ;
		normX   = sqrt(val2[1]) ;

#ifdef DEVEL
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"XNORMS (%i): %f %f \n", ppRank, normRes, normX);
#endif
#endif

#ifdef DEVEL_VERBOSE
		printf("NORMS: A=%f X=%f B=%f <> R=%f\n",
				normA, normX, normB, normRes
				);
#endif

		/* convergence test */
    if (normRes  < (eps*((normA*normX) + normB)) ) 
		{
			converged = AF_YES;
#if 0
			for (j=0; j<n; j++) { x->data[j] += alpha * p.data[j] ; }
#endif

#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[ ]  %s %li %s!\n",_("solution done in"),i,_("iterations"));
#endif
			break;
		}
		else
		{
			beta = rho/rho0 ; ;
		}

	} /* end of "for i" */

	/* when we are finished: */
  femVecSetZero(x_big);
	MPI_Allgatherv(
					x->data, 
					x->rows, 
					MPI_DOUBLE, 
					x_big->data,
					sizes,
					disps,
					MPI_DOUBLE,
					MPI_COMM_WORLD
					);

memFree:
	femVecFree(&p);
	femVecFree(&r);
	femVecFree(&s);
	femVecFree(&w);
	femVecFree(&q);
	femVecFree(&L);
	femVecFree(&p_big);

	free(sizes) ; sizes = NULL ;
	free(disps) ; sizes = NULL ;
  return(rv);
}

/* ##################################################### */

/** Conjugate gradient method with Jacobi preconditioner 
 *  (for symetric matrices only!) 
 *  @param a      matrix
 *  @param b      "load" vector
 *  @param x      results (vector - given as first iteration)
 *  @param x_big  results (full size vector - shoul be filled with 0's)
 *  @param eps    error (min.)
 *  @param maxIt  max. number of iterations
 *  @return state value
 */
int femMPIeqsCGwJ(tMatrix *a, tVector *b, tVector *x, tVector *x_big, double eps, long maxIt)
{
	tVector  M; /* Jacobi preconditioner (diag[A] ;-) */
	tVector  r; /* small size vectors */
	tVector  z;
	tVector  p;
	tVector  q;
	/*tVector  x_big;*/ /* full size vectors */
	tVector  p_big;
	double   ro, myro, roro, alpha, beta;
	long     n=0;
	long     i,j,ii;
	int      rv = AF_OK;
	int      converged = AF_NO;
	double   normRes, normX, normA, normB, my_normA;
	/*double	 val[3];
	double	 my_val[3];*/
	double	 val2[32];
	double	 my_val2[32];
	int     *sizes = NULL;
	int     *disps = NULL;
	int      sum = 0 ;
	double	 pq = 0 ;
	double	 mypq = 0 ;

	if ((a->rows != x->rows) || (x->rows != b->rows)) {return(AF_ERR_SIZ);}
#ifdef DEVEL
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"SIZES (%i): a[%li,%li] x[%li] b[%li]\n", 
			ppRank, 
			a->rows, a->cols, x->rows, b->rows
			);
#endif
#endif

	if ((sizes = femDOFsizes()) == NULL) {return(AF_ERR_MEM);}
	if ((disps = (int *)malloc(sizeof(int)*ppSize)) == NULL) {return(AF_ERR_MEM);}
	sum = 0 ;
	disps[0]=0;
	for (i=1; i<ppSize; i++)
	{
		sum+=sizes[i-1];
		disps[i] = sum ;
	} 
	
	n = a->rows;

	my_normA = femMatNorm(a);
	
	MPI_Allreduce(&my_normA, &normA, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);


	normB = femVecNormMPI(b);
	normX = femVecNormMPI(x);

	/* MPI_Reduce: */

	my_val2[0] = normB; val2[0] = 0.0 ;
	my_val2[1] = normX; val2[1] = 0.0 ;

	MPI_Allreduce(my_val2, val2, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	normB = sqrt(val2[0]) ;
	normX = sqrt(val2[1]) ;

#ifdef DEVEL
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"NORMS (%i): %f %f %f\n", ppRank, normA, normB, normX);
#endif
#endif

	if (normB <= 0.0)
	{
		femVecSetZero(x);
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
	/* large v: */
  femVecNull(&p_big);

	
	/* memory allocation */
  if ((rv=femVecFullInit(&M,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&r,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&z,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&p,   n))!=AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&q,   n))!=AF_OK) { goto memFree; }
	/* large vectors: */
  if ((rv=femVecFullInit(&p_big,  a->cols))!=AF_OK) { goto memFree; }


	/* Jacobi preconditioner creation: */
	for (i=ppDOFfrom; i<=ppDOFto; i++) 
	{ 
		ii = i - ppDOFfrom + 1 ;

		M.data[ii-1] = femMatGet(a, ii, i ); 
		if (fabs(M.data[ii-1]) < FEM_ZERO)
		{
			rv = AF_ERR_ZER;
#ifdef DEVEL_VERBOSE
 fprintf(msgout,"[ ]   %s[%li][%li] %s\n",_("matrix member"),ii,i, _("has zero size"));
#endif
			goto memFree;
		}
	}


	femVecSetZero(x_big);
	MPI_Allgatherv(
				x->data, 
				x->rows, 
				MPI_DOUBLE, 
				x_big->data,
				sizes,
				disps,
				MPI_DOUBLE,
				MPI_COMM_WORLD
				);

	
	/* next two lines mean: r = b - A*x  */
  femMatVecMultMPI(a, x_big, &r);

#if 0
	printf("_##_ %i: n=%li\n",ppRank,n);
		 femVecPrn(&r,"R");
#endif

	for (i=0; i<n; i++) { r.data[i] = b->data[i] - r.data[i] ; }

#if 0
	femVecPrn(b,"B");
	femVecPrn(&r,"R");
#endif


	/* main loop */
	for (i=1; i<= maxIt; i++)
	{
#ifdef DEVEL_VERBOSE
 fprintf(msgout,"[ ]   %s %li (%s %li)\n",_("linear step"),i,_("from"),maxIt);
#endif

	  /* using preconditioner: */
		for (j=0; j<n; j++) { z.data[j] = (r.data[j] / M.data[j]) ; }

		ro = 0 ;
		myro = femVecVecMult(&r,&z);

		MPI_Allreduce(&myro,&ro,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

#ifdef DEVEL
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"ro = %f\n",ro);
#endif
#endif

		if (i == 1)
		{
		  for (j=0; j<n; j++) { p.data[j] = z.data[j]; }
		}
		else
		{
			beta = ro / roro ;

#ifdef DEVEL
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"beta = %f\n",beta);
#endif
#endif

		  for (j=0; j<n; j++) 
			    { p.data[j] = (z.data[j] + (beta*p.data[j])) ; }
		}

		/* p[i] -> p_big HERE! */
		MPI_Allgatherv(
					p.data, 
					p.rows, 
					MPI_DOUBLE, 
					p_big.data,
					sizes,
					disps,
					MPI_DOUBLE,
					MPI_COMM_WORLD
					);

#ifdef DEVEL
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"SSS: \n");
		for (j=0; j<ppSize; j++) { fprintf(msgout,"%i %i \n",sizes[j], disps[j]); }
		femVecPrn(&p_big,"P_BIG");
#endif
#endif


		femMatVecMultMPI(a,&p_big,&q);

		mypq =  femVecVecMult(&p,&q);

		MPI_Allreduce(&mypq,&pq,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
	
		alpha = ro / pq ;

#ifdef DEVEL
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"alpha = %f\n",alpha);
#endif
#endif
		
		for (j=0; j<n; j++) 
		{ 
			x->data[j] = x->data[j] + (alpha * p.data[j])  ; 
			r.data[j] = r.data[j] - (alpha * q.data[j])  ; 
		}

		MPI_Allgatherv(
					x->data, 
					x->rows, 
					MPI_DOUBLE, 
					x_big->data,
					sizes,
					disps,
					MPI_DOUBLE,
					MPI_COMM_WORLD
					);
		femVecPrn(x_big, "X_BIG");

		/* Convergence testing */

	  normRes = femVecNormMPI(&r);
	  normX   = femVecNormMPI(x);

	my_val2[0] = normRes; val2[0] = 0.0 ;
	my_val2[1] = normX; val2[1] = 0.0 ;

	MPI_Allreduce(my_val2, &val2, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	normRes = sqrt(val2[0]) ;
	normX   = sqrt(val2[1]) ;

#ifdef DEVEL
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"XNORMS (%i): %f %f \n", ppRank, normRes, normX);
#endif
#endif
	/*printf("XNORMS (%i)[%i]: %f %f \n", ppRank,i, normRes, normX);*/

		/* convergence test */
		/*if (fabs(norm - norm0) < eps )*/
    if (normRes  < (eps*((normA*normX) + normB)) ) 
		{
			converged = AF_YES;
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[ ]  %s %li %s!\n",_("solution done in"),i,_("iterations"));
#endif
			break;
		}

#ifdef DEVEL
#ifdef DEVEL_VERBOSE
#if 0
	fprintf(msgout,"[i] (%i) Convergence test %f < %f (step %li from %li)\n",
				ppRank,
				normRes  , (eps*((normA*normX) + normB)) , i, maxIt
				);
#endif
#endif
#endif

		roro = ro;

	} /* end of main loop */

#if 0
	printf("[I] normRes = %f\n",normRes);
#endif

	if (converged != AF_YES)
	{
#ifdef DEVEL_VERBOSE
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


	/*femVecFree(&x_big);*/
	femVecFree(&p_big);

	free(sizes) ; sizes = NULL ;
	free(disps) ; sizes = NULL ;
	return(rv);
}

#endif
/* end of fem_eqs.c */

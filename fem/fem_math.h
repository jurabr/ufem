/*
   File name: fem_math.h
   Date:      2003/04/12 12:45
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

	 FEM Software - matrix library - header file

	 $Id: fem_math.h,v 1.21 2005/07/11 17:56:16 jirka Exp $
*/


#ifndef __FEM_MATH_H__
#define __FEM_MATH_H__

#define MAT_FULL 0
#define MAT_SPAR 1

#define VEC_FULL 0
#define VEC_SPAR 1

#define FEM_PUT 0
#define FEM_ADD 1

#define FEM_TINY_VAL 1.0e-20

#include "fem.h"
#include "fem_mem.h"
#include <math.h>

typedef struct
{
	long    type; /* 0 = dense; 1 = sparse (rows) */
	long    rows;
	long    cols;
	long    len; /* lenght of "pos" and "data" (if used) fields */
	long   *pos;
	double *data;
	long   *frompos; /* from in "pos" and "data" - sparse only sizeof(frompos) = rows */
	long   *defpos;  /* number in "pos" and "data" - sparse only */
}tMatrix;


typedef struct
{
	long    type; /* 0 = dense; 1 = sparse (rows) */
	long    rows;
	long    len; /* lenght of "pos" and "data" (if used) fields */
	long   *pos;
	double *data;
}tVector;

#ifdef _USE_THREADS_
typedef struct  /* data for threads  */
{
  long     from ;
  long     to ;
  tMatrix *m_a ;
  tMatrix *m_b ;
  tMatrix *m_c ;
  tVector *v_a ;
  tVector *v_b ;
  tVector *v_c ;
  double   n_a ;
  double   n_b ;
  double   n_c ;
}tThData;
#endif



/* Functions: */

extern void femMatNull(tMatrix *mat);
extern void femMatFree(tMatrix *mat);
extern int femMatAlloc(tMatrix *mat, long type, long rows, long cols, long bandwidth, long *rowdesc);
#define femFullMatInit(mat, rows, cols) femMatAlloc(mat, MAT_FULL, rows, cols, 0, NULL)
#define femSparMatInitBand(mat, rows, cols, bandwidth) femMatAlloc(mat, MAT_SPAR, rows, cols, bandwidth, NULL)
#define femSparMatInitDesc(mat, rows, cols, desc) femMatAlloc(mat, MAT_SPAR, rows, cols, 0, desc)
extern int femMatPutAdd(tMatrix *mat, long row, long col, double val, int mode);
#define femMatAdd(mat, row, col, val) femMatPutAdd(mat, row, col, val, FEM_ADD)
#define femMatPut(mat, row, col, val) femMatPutAdd(mat, row, col, val, FEM_PUT)
extern double femMatGet(tMatrix *mat, long row, long col);
extern void femMatPrn(tMatrix *mat, char *name);
extern int femMatPrnF(char *fname, tMatrix *mat);
extern int femSparseMatPrnF(char *fname, tMatrix *mat);
extern int femSparseMatReadF(char *fname, tMatrix *mat);
extern int femSparseMarketMatPrnF(char *fname, tMatrix *mat);
extern int femMatOut(tMatrix *a, FILE *fw);
extern void femMatSetZero(tMatrix *a);
extern void femMatSetZeroBig(tMatrix *a);
extern void femMatSetZeroRow(tMatrix *a, long row);
extern void femMatSetZeroCol(tMatrix *a, long Col);
extern double femMatNorm(tMatrix *a);
extern double femMatNormBig(tMatrix *a);

extern void femVecNull(tVector *mat);
extern void femVecFree(tVector *mat);
extern int femVecAlloc(tVector *mat, long type, long rows, long items);
#define femVecFullInit(mat, rows) femVecAlloc(mat, VEC_FULL, rows, rows)
extern int femVecPutAdd(tVector *vec, long row, double val, int mode);
#define femVecPut(vec, row, val) femVecPutAdd(vec, row, val, FEM_PUT)
#define femVecAdd(vec, row, val) femVecPutAdd(vec, row, val, FEM_ADD)
extern double femVecGet(tVector *vec, long pos);
extern void femVecPrn(tVector *mat, char *name);
extern int femVecPrnF(char *fname, tVector *mat);
extern int femVecOut(tVector *a, FILE *fw);
extern void femVecSetZero(tVector *a);
extern void femVecSetZeroBig(tVector *a);
extern double femVecNorm(tVector *a);
extern double femVecNormBig(tVector *a);
extern int femVecClone(tVector *src, tVector *dest);
extern int femVecAddVec(tVector *orig, double mult, tVector *addt) ;

extern double femVecVecMult(tVector *a, tVector *b);
extern double femVecVecMultBig(tVector *a, tVector *b);
extern int femVecVecMulttoMat(tVector *a, tVector *b, tMatrix *c);
extern int femValVecMult(double val, tVector *a, tVector *b);
extern int femValVecMultSelf(double val, tVector *a);
extern int femValMatMultSelf(double val, tMatrix *a);
extern int femVecSwitch(tVector *a, tVector *b);

extern int femVecMatMult(tVector *a, tMatrix *b, tVector *c);
extern double femVecMatVecMult(tVector *a, tMatrix *b, tVector *c);
extern int femMatVecMult(tMatrix *a, tVector *b, tVector *c);
extern int femMatVecMultBig(tMatrix *a, tVector *b, tVector *c);
extern int femVecLinComb(double amult, tVector *a, double bmult, tVector *b, tVector *c);
extern int femVecLinCombBig(double amult, tVector *a, double bmult, tVector *b, tVector *c);
extern int femMatMatMult(tMatrix *a, tMatrix *b, tMatrix *c);
extern int femMatLinComb(double am, tMatrix *a, double bm, tMatrix *b, tMatrix *c);
extern int femMatTran(tMatrix *a, tMatrix *b);


extern int femMatInv(tMatrix *a);

extern int femEqsCGwJ(tMatrix *a, tVector *b, tVector *x, double eps, long maxIt);
extern int femEqsCGwSSOR(tMatrix *a, tVector *b, tVector *x, double eps, long maxIt);
extern int femEqsBiCCSwJ(tMatrix *a, tVector *b, tVector *x, double eps, long maxIt);

extern int femEqsChol(tMatrix *a, tVector *b, tVector *x);
extern int femMatCholFact(tMatrix *a, tVector *a_diag);

extern int femLUdecomp(tMatrix *a, tVector *index);
extern int femLUback(tMatrix *a, tVector *index, tVector *b);
extern int femLUinverse(tMatrix *a);

/* Use with care:  (!!)*/
extern int femVecCloneDiff(tVector *orig, tVector *clone);
extern int femMatCloneDiffToEmpty(tMatrix *orig, tMatrix *clone);
extern int femMatCloneDiffToSame(tMatrix *orig, tMatrix *clone);

#endif

/* end of fem_math.h */

/*
   File name: fem_math.c
   Date:      2003/04/12 12:44
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

	 FEM Solver - matrix library

	 $Id: fem_math.c,v 1.46 2005/07/11 17:56:16 jirka Exp $
*/

#include "fem_math.h"


/* MATRIX *** */

void femMatNull(tMatrix *mat)
{
	mat->type    = 0 ;
	mat->rows    = 0 ;
	mat->cols    = 0 ;
	mat->len     = 0 ;
	mat->pos     = NULL ;
	mat->data    = NULL ;
	mat->frompos = NULL ;
	mat->defpos  = NULL ;
}

void femMatFree(tMatrix *mat)
{
	mat->type    = 0 ;
	mat->rows    = 0 ;
	mat->cols    = 0 ;
	mat->len     = 0 ;
	femIntFree(mat->pos     ) ;
	femDblFree(mat->data    ) ;
	femIntFree(mat->frompos ) ;
	femIntFree(mat->defpos  ) ;
}

int femMatAlloc(tMatrix *mat, long type, long rows, long cols, long bandwidth, long *rowdesc)
{
	long sum = 0 ;
	long i;

  femMatNull(mat);
		
	if ((type >= MAT_FULL) && (type <= MAT_SPAR))
	{
		mat->type    = type ;
		switch (type)
		{
			case MAT_FULL: 
											mat->rows = rows;
											mat->cols = cols;
											mat->len	= cols*rows;
											if ((mat->data = femDblAlloc(mat->len)) == NULL) 
											   { goto memFree; }
											mat->pos = NULL;
											mat->frompos = NULL;
											mat->defpos  = NULL;
											break;
			case MAT_SPAR: 
											mat->rows = rows;
											mat->cols = cols;
											if ((mat->defpos = femIntAlloc(mat->rows)) == NULL) 
											   { goto memFree; }
											if ((mat->frompos = femIntAlloc(mat->rows)) == NULL) 
											   { goto memFree; }

											if ((bandwidth > 0) && (rowdesc == NULL))
											{
												mat->len	= rows*bandwidth;
												if ((mat->data = femDblAlloc(mat->len)) == NULL) 
											   { goto memFree; }
												if ((mat->pos = femIntAlloc(mat->len)) == NULL) 
											   { goto memFree; }

												for (i=0; i<rows; i++)
												{
													mat->frompos[i] = bandwidth*i;
												}
											}
											else
											{
												sum = 0;
												for (i=0; i<rows; i++) 
												{ 
													sum += rowdesc[i]; 
													mat->defpos[i]=rowdesc[i];
													mat->frompos[i]=sum-rowdesc[i];
												}

												mat->len	= sum;

												if ((mat->data = femDblAlloc(mat->len)) == NULL) 
											   { goto memFree; }
												if ((mat->pos = femIntAlloc(sum)) == NULL) 
											   { goto memFree; }
												
											}
											break;
		}
		return(AF_OK);
	}
	else
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s: %li!\n", _("Matrix type unsupported"),type);
#endif
		return(AF_ERR_VAL);
	}

memFree:
	femMatFree(mat);
	return(AF_ERR_MEM);
}


/** Gets value from matrix
 * @param mat matrix
 * @param row row
 * @param row collumn
 * @return value
 */
double femMatGet(tMatrix *mat, long row, long col)
{
	long pos,i;

#ifdef DEVEL
	if ((row < 0) || (col < 0)) {return(0);}
	if ((row > mat->rows) || (col > mat->cols)) {return(0);}
#endif

	switch(mat->type)
	{
		case MAT_FULL: 
										pos = (row-1)*(mat->cols) + (col-1) ;
										return(mat->data[pos]) ; 
										break;
		case MAT_SPAR: 
										for (i=mat->frompos[row-1]; i<mat->frompos[row-1]+mat->defpos[row-1]; i++)
										{
											if (mat->pos[i] == 0) {break;}
											if (mat->pos[i] == col)
											{
												return(mat->data[i]);
												break;
											}
										}
										break;
		default: 
#ifdef RUN_VERBOSE
										fprintf(msgout,"[E] %s!\n", _("Invalid matrix type"));
#endif
									 return(0.0); break;
	}
	return(0.0);
}

/** Adds value to matrix
 * @param mat matrix
 * @param row row 
 * @param col column
 * @param val value
 * @param mode FEM_PUT for putting ("=") FEM_ADD for adding ("+=")
 * @return  status
 */
int femMatPutAdd(tMatrix *mat, long row, long col, double val, int mode)
{
	long pos;
	int i;

#ifdef DEVEL
	if ((row < 0) || (col < 0)) {return(AF_ERR_SML);}
	if ((row > mat->rows) || (col > mat->cols)) {return(AF_ERR_BIG);}
#endif

	switch(mat->type)
	{
		case MAT_FULL: 
										pos = ((row-1)*(mat->cols)) + (col-1) ;
										if (mode == FEM_ADD) { mat->data[pos] += val ; }
										else                 { mat->data[pos]  = val ; }
										return(AF_OK);
										break;
		case MAT_SPAR: /* this is more complicated */
										for (i=mat->frompos[row-1]; i<mat->frompos[row-1]+mat->defpos[row-1]; i++)
										{
											if (mat->pos[i] == col)
											{
												if (mode == FEM_ADD) { mat->data[i] += val ; }
												else                 { mat->data[i]  = val ; }
												return(AF_OK);
											}
											if (mat->pos[i] == 0) /* empty field found */
											{
												mat->pos[i] = col ;
												if (mode == FEM_ADD) { mat->data[i] += val ; }
												else                 { mat->data[i]  = val ; }
												return(AF_OK);
											}
										}
										/* if we are here 
										 * because reallocation is needed !
										 */
#ifdef RUN_VERBOSE
										fprintf(msgout,"[E] %s [%li,%li]!\n", _("Matrix reallocation needed - requested unwritten code"),row,col);
										exit(AF_ERR_BIG);
#endif
										return(AF_ERR_BIG);
										break;
		default: 
#ifdef RUN_VERBOSE
										fprintf(msgout,"[E] %s!\n", _("Invalid matrix type"));
#endif
									 return(AF_ERR_VAL); break;
	}
}

/** Prints matrix to stdout, works only in DEVEL mode */
void femMatPrn(tMatrix *mat, char *name)
{
#ifdef DEVEL_VERBOSE
	int i,j;

	fprintf(msgout,"\n%s %s %s[%li,%li]:\n", _("Matrix"), name, _("listing"),mat->rows, mat->cols);
	for (i=1; i<=mat->rows; i++)
	{
		for (j=1; j<=mat->cols; j++)
		{
			fprintf(msgout," %f ",femMatGet(mat, i,j));
		}
		fprintf(msgout,"\n");
	}
	fprintf(msgout,"\n");
#endif
}

/** Saves matrix to file 
 * @param fname name of file
 * @param mat matrix to be printed
 * @return status
 */
int femMatPrnF(char *fname, tMatrix *mat)
{
  FILE *fw ;
  int   rv = AF_OK ;
	int   i,j;
  
  if ((fw = fopen(fname, "w")) == NULL) {return(AF_ERR_IO);}

	for (i=1; i<=mat->rows; i++)
	{
		for (j=1; j<=mat->cols; j++)
		{
			fprintf(fw," %e ",femMatGet(mat, i,j));
		}
		fprintf(fw,"\n");
	}

  if (fclose(fw) != 0 ) {rv = AF_ERR_IO;}
  
  return(rv);
}

/** Saves matrix to file IN SPARSE FORM
 * @param fname name of file
 * @param mat matrix to be printed
 * @return status
 */
int femSparseMatPrnF(char *fname, tMatrix *mat)
{
  FILE *fw ;
  long  rv = AF_OK ;
	long  i,j;
  long  sum = 0 ;

  if (mat->type != MAT_SPAR) {return(AF_ERR_VAL);}
  
  if ((fw = fopen(fname, "w")) == NULL) {return(AF_ERR_IO);}

  fprintf(fw, "%li %li\n", mat->rows, mat->cols);
  for (i=0; i<mat->rows; i++)
  {
    sum = 0 ;
    for (j=mat->frompos[i]; j<mat->frompos[i]+mat->defpos[i]; j++)
    {
      if (mat->pos[j] >= 0) { sum++ ; }
      else                  { break ; }
    }

    fprintf(fw, "%li %li ", i+1, sum);

    for (j=mat->frompos[i]; j<mat->frompos[i]+sum; j++)
    {
      fprintf(fw, "%li %e ", mat->pos[j], mat->data[j]);
    }

    fprintf(fw, "\n");
  }

  if (fclose(fw) != 0 ) {rv = AF_ERR_IO;}
  
  return(rv);
}

/** Saves matrix to file IN SPARSE FORM (MatrixMarket file standard)
 * @param fname name of file
 * @param mat matrix to be printed
 * @return status
 */
int femSparseMarketMatPrnF(char *fname, tMatrix *mat)
{
  FILE *fw ;
  long  rv = AF_OK ;
	long  i,j;
  long  sum = 0 ;

  if (mat->type != MAT_SPAR) {return(AF_ERR_VAL);}
  
  if ((fw = fopen(fname, "w")) == NULL) {return(AF_ERR_IO);}

  fprintf(fw,"%%%%MatrixMarket matrix coordinate real general\n");
  fprintf(fw, "%li %li %li\n", mat->rows, mat->cols, mat->len);
  for (i=0; i<mat->rows; i++)
  {
    sum = 0 ;
    for (j=mat->frompos[i]; j<mat->frompos[i]+mat->defpos[i]; j++)
    {
      if (mat->pos[j] >= 0) { sum++ ; }
      else                  { break ; }
    }

    for (j=mat->frompos[i]; j<mat->frompos[i]+sum; j++)
    {
      fprintf(fw, "%li %li %e\n",i+1, mat->pos[j], mat->data[j]);
    }
  }

  if (fclose(fw) != 0 ) {rv = AF_ERR_IO;}
  
  return(rv);
}


/** Reads matrix from file IN SPARSE FORM
 * @param fname name of file
 * @param mat matrix (must be unallocated)
 * @return status
 */
int femSparseMatReadF(char *fname, tMatrix *mat)
{
  FILE   *fw ;
  long    rv    = AF_OK ;
	long    i,j, k;
  long    tmp ;
  long    sum   = 0 ;
  long    size  = 0 ;
  long    ensize= 0 ;
  long   *pos0  = NULL ;
  double *data0 = NULL ;

  if ((fw = fopen(fname, "r")) == NULL) {return(AF_ERR_IO);}

  fscanf(fw, "%li %li\n", &mat->rows, &mat->cols);

  if ((mat->rows <= 0)||(mat->cols <=0)) { return(AF_ERR_IO); }

  if ((mat->frompos = femIntAlloc(mat->rows)) == NULL) { rv = AF_ERR_MEM; goto memFree ; }
  if ((mat->defpos = femIntAlloc(mat->rows)) == NULL) { rv = AF_ERR_MEM; goto memFree ; }
  size = mat->rows*300 ;
  if ((mat->pos = femIntAlloc(size)) == NULL) { rv = AF_ERR_MEM; goto memFree ; }
  if ((mat->data = femDblAlloc(size)) == NULL) { rv = AF_ERR_MEM; goto memFree ; }

  mat->type = MAT_SPAR ;

  sum = 0 ;
  for (i=0; i<mat->rows; i++)
  {
    fscanf(fw, "%li %li ", &tmp, &mat->defpos[i]);

    if (i > 0)
    {
      mat->frompos[i] = mat->frompos[i-1] + mat->defpos[i-1] ;
    }
    else /* first row */
    {
      mat->frompos[i] = 0 ;
    }

    for (j=0; j<mat->defpos[i]; j++)
    {
      if (sum >=size)
      {
        /* enlarge "data" and "pos" */
        ensize = size + (long)(2*size*(i/mat->rows));
        if((pos0 = femIntAlloc(ensize))==NULL) { rv = AF_ERR_MEM; goto memFree ; }
        if((data0 = femDblAlloc(ensize))==NULL) { rv = AF_ERR_MEM; goto memFree ; }
        for (k=0; k<sum; k++)
        {
          pos0[k] = mat->pos[k] ;
          data0[k] = mat->data[k] ;
        }
        free(mat->pos);
        free(mat->data);
        mat->pos=pos0;
        mat->data=data0;
        pos0 = NULL ;
        data0 = NULL ;
      }
      fscanf(fw, "%li %lf ", &mat->pos[sum], &mat->data[sum]);
      sum++ ;
    }
  }

  if (fclose(fw) != 0 ) {rv = AF_ERR_IO;}
  
  return(rv);
memFree:
  femMatFree(mat);
  return(rv);
}


/** Writes matrix to stream (FILE *)
 * @param a matrix
 * @param fw stream
 * @return stave value
 */
int femMatOut(tMatrix *a, FILE *fw)
{
	int rv = AF_OK;
	long i,j;

	fprintf(fw," %li %li\n", a->rows, a->cols);

	for (i=1; i<=a->rows; i++)
	{
		for (j=1; j<=a->cols; j++)
		{
			fprintf(fw," %e \n",femMatGet(a,i,j));
		}
	}

	return(rv);
}

#ifdef _USE_THREADS_
void *thFemMatSetZero (void *param)
{
  tThData *p = (tThData*) param ;
	long i;

  if (p->to <= 0) {return(NULL);}

	for (i=p->from; i<p->to; i++) 
  { 
    p->m_a->data[i] = 0.0 ; 
  }

  return(NULL);
}
#endif

/** Sets all of matrix contents to 0 */
void femMatSetZeroBig(tMatrix *a)
{
	long i;
#ifdef _USE_THREADS_
  /* threaded version */
  tThData data[AF_MAX_THREADS];
  pthread_t Thread_ID[AF_MAX_THREADS];
  ldiv_t  dnum ;

  if ((femUseThreads != AF_YES)||(a->len < femThreadMin)||(a->type != MAT_SPAR))
  {
	  for (i=0; i<a->len; i++) { a->data[i] = 0.0 ; }
  }
  else
  {
    dnum = ldiv(a->len, femThreadNum) ;
    if (dnum.quot < 1)
    {
      dnum.quot = 0 ;
      dnum.rem  = a->len ;
    }

    for (i=0; i<femThreadNum; i++)
    {
      data[i].m_a = a ;
      data[i].from = i*dnum.quot ;

      data[i].to = (i+1)*dnum.quot ;
      if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

      pthread_create(&Thread_ID[i],NULL,thFemMatSetZero,&data[i]);
    }

    for (i=0; i<femThreadNum; i++)
    {
      pthread_join(Thread_ID[i], NULL) ;
    }
  }
#else
	for (i=0; i<a->len; i++) { a->data[i] = 0.0 ; }
#endif
}

/** Sets all of matrix contents to 0 FOR SMALL DATA */
void femMatSetZero(tMatrix *a)
{
	long i;
	for (i=0; i<a->len; i++) { a->data[i] = 0.0 ; }
}


/** Sets matrix row to 0 */
void femMatSetZeroRow(tMatrix *a, long row)
{
	long i;
#ifdef DEVEL
	if ((row < 1)||(row > a->rows)) {return;}
#endif

	if (a->type == MAT_SPAR)
	{
		for (i=a->frompos[row-1]; i<a->frompos[row-1]+a->defpos[row-1]; i++)
		{
			if (a->pos[i] == 0) {break;}
			a->data[i] = 0 ;
			/*fprintf(msgout,"zero on %li\n",i);*/
		}
	}
	else
	{
	   for (i=1; i<=a->cols; i++) { femMatPut(a, row,i, 0); }
	}
}

#ifdef _USE_THREADS_
void *thFemMatSetZeroCol (void *param)
{
  tThData *p = (tThData*) param ;
	long i, j, ifrom, ito, ipos ;
  long Col;

  if (p->to <= 0) {return(NULL);}

  Col = (long) p->n_a ;

#if 0 /* non-symmetric */
	for (i=p->from; i<p->to; i++) 
  {
    if (p->m_a->pos[i] == Col) { p->m_a->data[i] = 0.0; }
  }
#else /* this is for symmetric matrices only!!! */
		ifrom = p->from ;
		ito   = p->to ;

		for (i=ifrom; i<ito; i++) 
		{
			ipos = p->m_a->pos[i]-1 ;
			for (j=p->m_a->frompos[ipos]; j<p->m_a->frompos[ipos]+p->m_a->defpos[ipos]; j++)
			{
				if (p->m_a->pos[j] == Col) { p->m_a->data[j] = 0.0 ; }
			}
		}
#endif

  return(NULL);
}
#endif

/** Sets all of matrix contents to 0 */
void femMatSetZeroCol(tMatrix *a, long Col)
{
	long i, j, ifrom, ito, ipos ;
#ifdef _USE_THREADS_
  tThData data[AF_MAX_THREADS];
  pthread_t Thread_ID[AF_MAX_THREADS];
  ldiv_t  dnum ;
#endif

	if (a->type == MAT_SPAR)
	{
#ifdef _USE_THREADS_
  if ((femUseThreads != AF_YES)||(a->len < femThreadMin))
  {
#if 0
    for (i=0;i<a->len;i++){if(a->pos[i]==Col) {a->data[i]=0;}}
		return;
#else
		ifrom = a->pos[a->frompos[Col-1]]-1 ;
		ito   = a->pos[a->frompos[Col-1]]+a->defpos[Col-1]-1 ;
		for (i=ifrom; i<ito; i++) 
		{
			for (j=a->frompos[ipos]; j<a->frompos[ipos]+a->defpos[ipos]; j++)
			{
				if (a->pos[j] == Col) { a->data[j] = 0.0 ; }
			}
		}
#endif
  }
  else
  {

#if 0 /* non-symetric - see thFemMatSetZeroCol too! */
	  dnum = ldiv(a->len, femThreadNum) ;
    if (dnum.quot < 1)
    {
      dnum.quot = 0 ;
      dnum.rem  = a->len ;
    }

    for (i=0; i<femThreadNum; i++)
    {
      data[i].v_a = a ;
      data[i].from = i*dnum.quot ;

      data[i].to = (i+1)*dnum.quot ;

#else /* symmetric matrices only: */
		ifrom = a->frompos[Col-1] ;
		ito   = a->frompos[Col-1]+a->defpos[Col-1]-1 ;

    dnum = ldiv((a->defpos[Col-1]), femThreadNum) ;
    if (dnum.quot < 1)
    {
      dnum.quot = 0 ;
      dnum.rem  = a->defpos[Col-1] ;
    }

    for (i=0; i<femThreadNum; i++)
    {
      data[i].n_a = Col ;

      data[i].m_a = a ;

      data[i].from = ifrom + i*dnum.quot  ;
      data[i].to   = ifrom + (i+1)*dnum.quot ;
#endif
			
      if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }
      pthread_create(&Thread_ID[i],NULL,thFemMatSetZeroCol,&data[i]);
    }

    for (i=0; i<femThreadNum; i++)
    {
      pthread_join(Thread_ID[i], NULL) ;
    }
  }
#else
#if 0 /* alternative algorithm - original */
		for (i=0; i<a->len; i++) { if (a->pos[i] == Col) { a->data[i] = 0 ; } }
#else  /* alternative algorithm - modified (symmetric matrices only) */
		ifrom = a->pos[a->frompos[Col-1]]-1 ;
		ito   = a->pos[a->frompos[Col-1]+a->defpos[Col-1]-1]-1 ;
		for (i=ifrom; i<ito; i++) 
		{
			for (j=a->frompos[i]; j<a->frompos[i]+a->defpos[i]; j++)
			{
				if (a->pos[j] == Col) { a->data[j] = 0.0 ; }
			}
		}

#endif /* end of alternative algorithm */

#endif
	}
	else
	{
		for (i=1; i<=a->rows; i++) 
		{ 
			femMatPut(a, i,Col, 0);
		}
	}
}


/* VECTOR *** */

void femVecNull(tVector *mat)
{
	mat->type    = 0 ;
	mat->rows    = 0 ;
	mat->len     = 0 ;
	mat->pos     = NULL ;
	mat->data    = NULL ;
}

void femVecFree(tVector *mat)
{
	mat->type    = 0 ;
	mat->rows    = 0 ;
	mat->len     = 0 ;
	femIntFree(mat->pos     ) ;
	femDblFree(mat->data    ) ;
}


int femVecAlloc(tVector *mat, long type, long rows, long items)
{
  femVecNull(mat);
		
	if ((type >= VEC_FULL) && (type <= VEC_SPAR))
	{
		mat->type    = type ;
		switch (type)
		{
			case VEC_FULL: 
											mat->rows = rows;
											mat->len	= rows;
											if ((mat->data = femDblAlloc(mat->len)) == NULL) 
											   { goto memFree; }
											mat->pos = NULL;
											break;
			case VEC_SPAR: 
											exit(AF_ERR_VAL); /* VEC_SPAR cannot be used ;-) */
											mat->rows = rows;
											if (items > 0)
											{
												mat->len	= items;
												if ((mat->data = femDblAlloc(mat->len)) == NULL) 
											   { goto memFree; }
												if ((mat->pos = femIntAlloc(mat->len)) == NULL) 
											   { goto memFree; }
											}
											else
											{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("Number of sparse vector items MUST BE nonzero"));
#endif
												goto memFree;
											}
											break;
		}
		return(AF_OK);
	}
	else
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s: %li!\n", _("Matrix type unsupported"),type);
#endif
		return(AF_ERR_VAL);
	}

memFree:
	femVecFree(mat);
	return(AF_ERR_MEM);
}

/** Adds value to vector
 * @param vec vector
 * @param pos row to add value
 * @param val value
 * @param mode FEM_PUT for putting ("=") FEM_ADD for adding ("+=")
 * @return  status
 */
int femVecPutAdd(tVector *vec, long pos, double val, int mode)
{
	if (pos > vec->rows)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s: %li > %li!\n", _("Index outside vector (Add/Put)"),pos, vec->rows);
#endif
		return(AF_ERR_BIG);
	}

	switch (vec->type)
	{
		case VEC_FULL:
										if (mode == FEM_PUT) /* put */
										   { vec->data[pos-1] = val; }
										else /* add */
										   { vec->data[pos-1] += val; }
			              break;
		case VEC_SPAR:  /* unimplemented */
										exit(AF_ERR_VAL);
			              break;
		default: 
#ifdef RUN_VERBOSE
										fprintf(msgout,"[E] %s!\n", _("Invalid vector type (Add/Put)"));
#endif
										return(AF_ERR_TYP); break;
	}
	return(AF_OK);
}

/** Gets value from vector
 * @param vec vector
 * @param pos row to add value
 * @return value
 */
double femVecGet(tVector *vec, long pos)
{
	if (pos > vec->rows)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s: %li/%li!\n", _("Index outside vector (Get)"),pos,vec->rows);
#endif
		return(0);
	}

	switch (vec->type)
	{
		case VEC_FULL:
										return(vec->data[pos-1]);
			              break;
		case VEC_SPAR:  /* unimplemented */
										exit(0);
			              break;
		default: 
#ifdef RUN_VERBOSE
										fprintf(msgout,"[E] %s!\n", _("Invalid vector type (Get)"));
#endif
										return(0); break;
	}
	return(AF_OK);
}

/** Prints vector to stdout, works only in DEVEL mode */
void femVecPrn(tVector *mat, char *name)
{
#ifdef DEVEL_VERBOSE
	int i;

	fprintf(msgout,"\n%s %s %s[%li]:\n", _("Vector"), name, _("listing"),mat->rows);
	for (i=1; i<=mat->rows; i++)
	{
		fprintf(msgout," %f ",femVecGet(mat, i));
	}
	fprintf(msgout,"\n");
#endif
}

/** Saves vector to file 
 * @param fname name of file
 * @param mat vector to be printed
 * @return status
 */
int femVecPrnF(char *fname, tVector *mat)
{
  FILE *fw ;
  int   rv = AF_OK ;
	int   i;
  
  if ((fw = fopen(fname, "w")) == NULL) {return(AF_ERR_IO);}

	for (i=1; i<=mat->rows; i++)
	{
		fprintf(fw," %e ",femVecGet(mat, i));
	}
	fprintf(fw,"\n");

  if (fclose(fw) != 0 ) {rv = AF_ERR_IO;}
  
  return(rv);
}


/** Writes vector to stream (FILE *)
 * @ a vector
 * @ fw stream
 * @return stave value
 */
int femVecOut(tVector *a, FILE *fw)
{
	int rv = AF_OK;
	long i;

	fprintf(fw," %li\n", a->rows);

	for (i=1; i<=a->rows; i++)
	{
		fprintf(fw," %e \n",femVecGet(a,i));
	}

	return(rv);
}

#ifdef _USE_THREADS_
void *thFemVecSetZero (void *param)
{
  tThData *p = (tThData*) param ;
	long i;

  if (p->to <= 0) {return(NULL);}

	for (i=p->from; i<p->to; i++) 
  { 
    p->v_a->data[i] = 0.0 ; 
  }

  return(NULL);
}
#endif


/** Sets all of vertor contents to 0 */
void femVecSetZeroBig(tVector *a)
{
	long i;
#ifdef _USE_THREADS_
  /* threaded version */
  tThData data[AF_MAX_THREADS];
  pthread_t Thread_ID[AF_MAX_THREADS];
  ldiv_t  dnum ;

  if ((femUseThreads != AF_YES)||(a->len < femThreadMin))
  {
	  for (i=0; i<a->len; i++) { a->data[i] = 0.0 ; }
  }
  else
  {
    dnum = ldiv(a->len, femThreadNum) ;
    if (dnum.quot < 1)
    {
      dnum.quot = 0 ;
      dnum.rem  = a->len ;
    }

    for (i=0; i<femThreadNum; i++)
    {
      data[i].v_a = a ;
      data[i].from = i*dnum.quot ;

      data[i].to = (i+1)*dnum.quot ;
      if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

      pthread_create(&Thread_ID[i],NULL,thFemVecSetZero,&data[i]);
    }

    for (i=0; i<femThreadNum; i++)
    {
      pthread_join(Thread_ID[i], NULL) ;
    }
  }
#else
	for (i=0; i<a->len; i++) { a->data[i] = 0.0 ; }
#endif
}

/** Sets all of vertor contents to 0 FOR SMALL DATA */
void femVecSetZero(tVector *a)
{
	long i;
	for (i=0; i<a->len; i++) { a->data[i] = 0.0 ; }
}

/** Clones vectors: src to dest both must be VEC_FULL, same size and allocated 
 * @param src original vector
 * @param dest moditied vector
 */
int femVecClone(tVector *src, tVector *dest)
{
	int i;

	if ( (src->type != VEC_FULL) || (dest->type != VEC_FULL) ) { return(AF_ERR_TYP); }
	if ( (src->len != dest->len) ) { return(AF_ERR_SIZ); }

	for (i=0; i<src->len; i++) { dest->data[i] = src->data[i] ; }
	return(AF_OK);
}

/* ------------------    Matrix Operations    -------------------- */

#ifdef _USE_THREADS_
void *thFemVecVecMult (void *param)
{
  tThData *p = (tThData*) param ;
	long i;
	double mult = 0.0 ;

	mult = 0.0 ;

  if (p->to <= 0) 
	{
		p->n_a = 0.0 ;
		return(NULL);
	}

  for (i=p->from; i<p->to; i++)
	{
		mult += p->v_a->data[i] * p->v_b->data[i] ;
	}

	p->n_a = mult ;
	return(NULL);
}
#endif

/** vector multiplication (scalar) (a[n]^t * b[n])
 * @param a vector
 * @param b vector
 * @return multiplication product
 */
double femVecVecMultBig(tVector *a, tVector *b)
{
	long i;
	static double mult;
#ifdef _USE_THREADS_
  tThData data1 ;
  tThData data[AF_MAX_THREADS];
  ldiv_t  dnum ;
  pthread_t Thread_ID[AF_MAX_THREADS];
  pthread_t Thread_1  ;
#endif

#ifdef DEVEL
	if (a->rows != b->rows) { return(0); }
	if ((a->rows <= 0) || (b->rows <= 0)) {return(0);}
#endif

	mult = 0.0;

	if ((a->type == VEC_FULL) && (b->type == VEC_FULL))
	{
#ifdef _USE_THREADS_
      data1.v_a  = a ;
      data1.v_b  = b ;
      data1.from = 0; 
      data1.to   = a->rows ;
      data1.n_a  = 0.0 ;
      
      if ((femUseThreads != AF_YES)||(a->rows < femThreadMin))
      {
        pthread_create(&Thread_1,NULL,thFemVecVecMult,&data1);
        pthread_join(Thread_1, NULL) ;
				mult = data1.n_a ;
      }
      else
      {
        dnum = ldiv(a->rows, femThreadNum) ;
        if (dnum.quot < 1)
        {
          dnum.quot = 0 ;
          dnum.rem  = a->rows ;
        }

        for (i=0; i<femThreadNum; i++)
        {
          data[i].v_a  = a ;
          data[i].v_b  = b ;
          data[i].from = i*dnum.quot ;
          data[i].to   = (i+1)*dnum.quot ;
      		data[i].n_a  = 0.0 ;

          if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

          pthread_create(&Thread_ID[i],NULL,thFemVecVecMult,&data[i]);
        }

        for (i=0; i<femThreadNum; i++)
        {
          pthread_join(Thread_ID[i], NULL) ;
        }

        mult = 0.0 ;

        for (i=0; i<femThreadNum; i++)
        {
					mult += data[i].n_a ;
        }
      }
#else
		for (i=0; i<a->rows; i++)
		{
			mult += (a->data[i]*b->data[i]) ;
		}
#endif
	}
	else
	{
		for (i=1; i<=a->rows; i++)
		{
			mult += (femVecGet(a,i)*femVecGet(b,i));
		}
	}
	return(mult);
}

/** vector multiplication (scalar) (a[n]^t * b[n])  FOR SMALL VECTORS
 * @param a vector
 * @param b vector
 * @return multiplication product
 */
double femVecVecMult(tVector *a, tVector *b)
{
	long i;
	static double mult;

#ifdef DEVEL
	if (a->rows != b->rows) { return(0); }
	if ((a->rows <= 0) || (b->rows <= 0)) {return(0);}
#endif

	mult = 0.0;

	if ((a->type == VEC_FULL) && (b->type == VEC_FULL))
	{
		for (i=0; i<a->rows; i++)
		{
			mult += (a->data[i]*b->data[i]) ;
		}
	}
	else
	{
		for (i=1; i<=a->rows; i++)
		{
			mult += (femVecGet(a,i)*femVecGet(b,i));
		}
	}
	return(mult);
}


/** vector multiplication (matrix) (a[n] * b[n]^t)
 * @param a vector
 * @param b vector
 * @param c matrix (result)
 * @return status
 */
int femVecVecMulttoMat(tVector *a, tVector *b, tMatrix *c)
{
	long i,j;

#ifdef DEVEL
	if (a->rows != b->rows) { return(AF_ERR_SIZ); }
	if ((a->rows != c->rows) || (b->rows != c->cols)) {return(AF_ERR_SIZ);}
  if (a->type != MAT_FULL) {return(AF_ERR_TYP);}
#endif


	if ((a->type == VEC_FULL) && (b->type == VEC_FULL))
	{
		for (i=0; i<a->rows; i++)
		{
		  for (j=0; j<a->rows; j++)
		  {
			  c->data[(i*c->cols)+j] = (a->data[i]*b->data[j]) ;
			}
		}
	}
	else
	{
		for (i=1; i<=a->rows; i++)
		{
			for (j=1; j<=a->rows; j++)
			{
      	femMatPut(c, i,j, (femVecGet(a,i) * femVecGet(b,j)) );
			}
		}
	}
	return(AF_OK);
}

/** number by vector multiplication (b[n] = val * a[n]) 
 * @param val number
 * @param a original vector (will not be modified)
 * @param b result (vector) - must be allocated and must have proper size
 * @return status
 */
int femValVecMult(double val, tVector *a, tVector *b)
{
	int i;

#ifdef DEVEL
	if (a->rows != b->rows) {return(AF_ERR_SIZ);}
	if (a->type != b->type) {return(AF_ERR_VAL);}
	if (a->type != VEC_FULL) {return(AF_ERR_VAL);} /* will be fixed */
#endif

	for (i=0; i<a->len; i++)
	{
		b->data[i] = a->data[i] * val;
	}
	return(AF_OK);
}

/** number by vector multiplication (a[n] = val * a[n]) 
 * @param val number
 * @param a original vector (WILL BE modified)
 * @return status
 */
int femValVecMultSelf(double val, tVector *a)
{
	int i;

	for (i=0; i<a->len; i++) { a->data[i] *= val; }
	return(AF_OK);
}

/** number by matrix multiplication (a[n] = val * a[n]) 
 * @param val number
 * @param a original number (WILL BE modified)
 * @return status
 */
int femValMatMultSelf(double val, tMatrix *a)
{
	int i;

	for (i=0; i<a->len; i++) { a->data[i] *= val; }
	return(AF_OK);
}

/** vector by matrix multiplication (a[n]^t * b[n,m]  = c[m])
 * @param a vector
 * @param b matrix
 * @param vector (result)
 * @return status
 */
int femVecMatMult(tVector *a, tMatrix *b, tVector *c)
{
	long i,j;
	double val;

	if ((a->rows != b->rows) || (b->cols != c->rows)) { return(AF_ERR_SIZ); }
	if (c->type != VEC_FULL) {return(AF_ERR_VAL);}

	if ((a->type == VEC_FULL) && (b->type == MAT_FULL) && (c->type == VEC_FULL))
	{
		for (i=0; i<b->cols; i++ )
		{
			val = 0;
			for (j=0; j<a->rows; j++ )
			{
				val += a->data[j] * b->data[i+ (b->cols*j)];
			}
			c->data[i] = val ;
		}
	}
	else
	{
		for (i=1; i<=b->cols; i++ )
		{
			val = 0;
			for (j=1; j<=a->rows; j++ )
			{
				val += femVecGet(a,j) * femMatGet(b,j,i);
			}
			femVecPut(c,i, val);
		}
	}

	return(AF_OK);
}

/** Vector by matrix by vector multiplication (a[n]^t * b[n,m] * c[m]  = d)
 * For small full matrices only (it is slow).
 * @param a vector
 * @param b matrix
 * @param c vector
 * @return constant (result)
 */
double femVecMatVecMult(tVector *a, tMatrix *b, tVector *c)
{
	long i,j;
	double val;
  static double sum_tot ;

  sum_tot = 0.0 ;

	if ((a->rows != b->rows) || (b->cols != c->rows)) { return(AF_ERR_SIZ); }
	if (c->type != VEC_FULL) {return(AF_ERR_VAL);}

	if ((a->type == VEC_FULL) && (b->type == MAT_FULL) && (c->type == VEC_FULL))
	{
		for (i=0; i<b->cols; i++ )
		{
			val = 0;
			for (j=0; j<a->rows; j++ )
			{
				val += a->data[j] * b->data[i+ (b->cols*j)];
			}
#if 0
			c->data[i] = val ;
#endif
      sum_tot += c->data[i] * val ;
		}
	}
	else
	{
		for (i=1; i<=b->cols; i++ )
		{
			val = 0;
			for (j=1; j<=a->rows; j++ )
			{
				val += ( femVecGet(a,j) * femMatGet(b,j,i) ) ;
			}
#if 0
			femVecPut(c,i, val);
#endif
      sum_tot += ( femVecGet(c, i) * val ) ;
		}
	}

	return(sum_tot);
}

#ifdef _USE_THREADS_
void *thFemMatVecMult(void *param)
{
  tThData *p = (tThData*) param ;
	long i,j;
  double val = 0.0 ;

  if (p->to <= 0) {return(NULL);}

	for (i=p->from; i<p->to; i++)
	{
		val = 0.0;
		for (j=p->m_a->frompos[i]; j<p->m_a->frompos[i]+p->m_a->defpos[i]; j++)
		{
			if (p->m_a->pos[j] <= 0) {break;} 
			val +=  ( p->m_a->data[j] * p->v_b->data[p->m_a->pos[j]-1] ) ;
	  }
	  p->v_c->data[i] = val ;
	}

  return(NULL);
}
#endif

/** Matrix by vector multiplication (a[m,n]*b[n] = b[n])
 * @param a matrix
 * @param b vector
 * @param c vector (result)
 * @return status
 */
int femMatVecMultBig(tMatrix *a, tVector *b, tVector *c)
{
	long   i,j;
	double val;
#ifdef _USE_THREADS_
  tThData data1 ;
  tThData data[AF_MAX_THREADS];
  ldiv_t  dnum ;
  pthread_t Thread_ID[AF_MAX_THREADS];
  pthread_t Thread_1  ;
#endif

	if ((a->cols != b->rows)||(c->rows != a->rows)) { return(AF_ERR_SIZ); }
	if (c->type != VEC_FULL) {return(AF_ERR_VAL);}

	if ((a->type == MAT_FULL) && (b->type == VEC_FULL))
	{
		for (i=0; i<a->rows; i++)
		{
			val = 0.0;
			for (j=0; j<a->cols; j++)
			{
				val += ( b->data[j] * a->data[j + (i*a->cols)] );
			}
			c->data[i] = val;
		}
	}
	else
	{
		if ((a->type == MAT_SPAR) && (b->type == VEC_FULL))
		{
			femVecSetZero(c);
#ifdef _USE_THREADS_
      data1.m_a = a ;
      data1.v_b = b ;
      data1.v_c = c ;
      data1.from = 0; 
      data1.to = a->rows ;
      
      if ((femUseThreads != AF_YES)||(a->len < femThreadMin))
      {
        pthread_create(&Thread_1,NULL,thFemMatVecMult,&data1);
        pthread_join(Thread_1, NULL) ;
        return(AF_OK);
      }
      else
      {
        dnum = ldiv(a->rows, femThreadNum) ;
        if (dnum.quot < 1)
        {
          dnum.quot = 0 ;
          dnum.rem  = a->rows ;
        }

        for (i=0; i<femThreadNum; i++)
        {
          data[i].m_a = a ;
          data[i].v_b = b ;
          data[i].v_c = c ;

          data[i].from = i*dnum.quot ;

          data[i].to = (i+1)*dnum.quot ;
          if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

          pthread_create(&Thread_ID[i],NULL,thFemMatVecMult,&data[i]);
        }

        for (i=0; i<femThreadNum; i++)
        {
          pthread_join(Thread_ID[i], NULL) ;
        }
      }
#else
			for (i=0; i<a->rows; i++)
			{
				val = 0.0;
				for (j=a->frompos[i]; j<a->frompos[i]+a->defpos[i]; j++)
				{
				  if (a->pos[j] <= 0) {break;} 
					val +=  ( a->data[j] * b->data[a->pos[j]-1] ) ;
				}
				c->data[i] = val ;
			}
#endif
		}
		else
		{
			for (i=1; i<=a->rows; i++)
			{
				val = 0;
				for (j=1; j<=a->cols; j++)
				{
					val += femMatGet(a, i,j) * femVecGet(b, j);
				}
				femVecPut(c,i, val);
			}
		}
	}
	
	return(AF_OK);
}

/** Matrix by vector multiplication (a[m,n]*b[n] = b[n]) FOR SMALL DATA
 * @param a matrix
 * @param b vector
 * @param c vector (result)
 * @return status
 */
int femMatVecMult(tMatrix *a, tVector *b, tVector *c)
{
	long   i,j;
	double val;

	if ((a->cols != b->rows)||(c->rows != a->rows)) { return(AF_ERR_SIZ); }
	if (c->type != VEC_FULL) {return(AF_ERR_VAL);}

	if ((a->type == MAT_FULL) && (b->type == VEC_FULL))
	{
		for (i=0; i<a->rows; i++)
		{
			val = 0.0;
			for (j=0; j<a->cols; j++)
			{
				val += ( b->data[j] * a->data[j + (i*a->cols)] );
			}
			c->data[i] = val;
		}
	}
	else
	{
		if ((a->type == MAT_SPAR) && (b->type == VEC_FULL))
		{
			femVecSetZero(c);
			for (i=0; i<a->rows; i++)
			{
				val = 0.0;
				for (j=a->frompos[i]; j<a->frompos[i]+a->defpos[i]; j++)
				{
				  if (a->pos[j] <= 0) {break;} 
					val +=  ( a->data[j] * b->data[a->pos[j]-1] ) ;
				}
				c->data[i] = val ;
			}
		}
		else
		{
			for (i=1; i<=a->rows; i++)
			{
				val = 0;
				for (j=1; j<=a->cols; j++)
				{
					val += femMatGet(a, i,j) * femVecGet(b, j);
				}
				femVecPut(c,i, val);
			}
		}
	}
	
	return(AF_OK);
}


#ifdef _USE_THREADS_
void *thFemVecLinComb (void *param)
{
  tThData *p = (tThData*) param ;
	long i;

  if (p->to < 0) 
	{
		return(NULL);
	}

  for (i=p->from; i<p->to; i++)
	{
		p->v_c->data[i] 
      = (p->n_a * p->v_a->data[i]) + (p->n_b * p->v_b->data[i]) ;
	}

	return(NULL);
}
#endif


/** linear combination of vectors am*a[m,n]+ bm*b[m,n] = c[m,n] (c..MAT_FULL)
 * @param am  "a" vector multiplier
 * @param a vector
 * @param bm  "b" vector multiplier
 * @param b vector
 * @param c vector (result)
 * @return status
 */
int femVecLinCombBig(double amult, tVector *a, double bmult, tVector *b, tVector *c)
{
	long i;
#ifdef _USE_THREADS_
  tThData data1 ;
  tThData data[AF_MAX_THREADS];
  ldiv_t  dnum ;
  pthread_t Thread_ID[AF_MAX_THREADS];
  pthread_t Thread_1  ;
#endif

#ifdef DEVEL
	if (a->rows != b->rows) { return(0); }
	if ((a->rows <= 0) || (b->rows <= 0)) {return(0);}
#endif

	if ((a->type == VEC_FULL) && (b->type == VEC_FULL) && (c->type == VEC_FULL))
	{
#ifdef _USE_THREADS_
      data1.v_a  = a ;
      data1.v_b  = b ;
      data1.v_c  = c ;
      data1.from = 0; 
      data1.to   = a->rows ;
      data1.n_a  = amult ;
      data1.n_b  = bmult ;
      
      if ((femUseThreads != AF_YES)||(a->rows < femThreadMin))
      {
        pthread_create(&Thread_1,NULL,thFemVecLinComb,&data1);
        pthread_join(Thread_1, NULL) ;
      }
      else
      {
        dnum = ldiv(a->rows, femThreadNum) ;
        if (dnum.quot < 1)
        {
          dnum.quot = 0 ;
          dnum.rem  = a->rows ;
        }

        for (i=0; i<femThreadNum; i++)
        {
          data[i].v_a  = a ;
          data[i].v_b  = b ;
          data[i].v_c  = c ;
          data[i].from = i*dnum.quot ;
          data[i].to   = (i+1)*dnum.quot ;
          data[i].n_a  = amult ;
          data[i].n_b  = bmult ;

          if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

          pthread_create(&Thread_ID[i],NULL,thFemVecLinComb,&data[i]);
        }

        for (i=0; i<femThreadNum; i++)
        {
          pthread_join(Thread_ID[i], NULL) ;
        }
      }
#else /* sequential code */
		for (i=0; i<a->rows; i++)
		{
			c->data[i] = (amult * a->data[i]) + (bmult * b->data[i]) ;
    }
#endif
	}
	else  /* VERY SLOW CODE: */
	{
    for (i=1; i<=a->rows; i++)
		{
			femVecPut(c,i, femVecGet(a,i)*amult+femVecGet(b,i)*bmult);
		}
	}

  return(AF_OK);
}



/** linear combination of vectors am*a[m,n]+ bm*b[m,n] = c[m,n] (c..MAT_FULL)
 * @param am  "a" vector multiplier
 * @param a vector
 * @param bm  "b" vector multiplier
 * @param b vector
 * @param c vector (result)
 * @return status
 */
int femVecLinComb(double amult, tVector *a, double bmult, tVector *b, tVector *c)
{
	long i;

#ifdef DEVEL
	if ((a->rows != b->rows) || (b->rows != c->rows)) {return(AF_ERR_SIZ);}
	if (c->type != VEC_FULL){return(AF_ERR_VAL);}
#endif

	if ((a->type == VEC_FULL) && (b->type == VEC_FULL) && (c->type == VEC_FULL) )
	{
		for (i=0; i<a->rows; i++)
		{
			c->data[i] = (amult * a->data[i]) + (bmult * b->data[i]) ;
		}
	}
	else /* SLOW CODE: */
	{
		for (i=1; i<=a->rows; i++)
		{
			femVecPut(c,i, femVecGet(a,i)*amult+femVecGet(b,i)*bmult);
		}
	}

	return(AF_OK);
}

/** matrix by matrix multiplication a[m,n]*b[n,h] = c[m,h]
 * @param a matrix
 * @param b matrix
 * @param c matrix (result)
 * @return status
 */
int femMatMatMult(tMatrix *a, tMatrix *b, tMatrix *c)
{
	long i,j,k;
	double val;

	if ((a->cols != b->rows) || (b->cols != c->cols)||(a->rows!=c->rows)) 
     { return(AF_ERR_SIZ); }
	if (c->type != MAT_FULL){return(AF_ERR_VAL);}

	if ((a->type == MAT_FULL) && (b->type == MAT_FULL) && (c->type == MAT_FULL) )
	{
		for (i=0; i<a->rows; i++)
		{
			for (j=0; j<b->cols; j++)
			{
				val = 0;
				for (k=0; k<a->cols; k++)
				{
					/*val += femMatGet(a, i,k)*femMatGet(b, k,j);*/
					val += a->data[(i*a->cols)+k] * b->data[(k*b->cols)+j];
				}
				/*femMatPut(c, i,j, val);*/
				c->data[(i*c->cols)+j] = val;
			}
		}
	}
	else
	{
		for (i=1; i<=a->rows; i++)
		{
			for (j=1; j<=b->cols; j++)
			{
				val = 0;
				for (k=1; k<=a->cols; k++)
				{
					val += femMatGet(a, i,k)*femMatGet(b, k,j);
				}
				femMatPut(c, i,j, val);
			}
		}
	}
	return(AF_OK);
}

/** linear combination of matrices am*a[m,n]+ bm*b[m,n] = c[m,n] (c..MAT_FULL)
 * @param am  "a" matrix multiplier
 * @param a matrix
 * @param bm  "b" matrix multiplier
 * @param b matrix
 * @param c matrix (result)
 * @return status
 */
int femMatLinComb(double am, tMatrix *a, double bm, tMatrix *b, tMatrix *c)
{
	long i,j;
	double val;

	if ((a->cols != b->cols) || (a->rows != b->rows)||(a->rows!=c->rows)||(a->cols!=c->cols)) 
     { return(AF_ERR_SIZ); }

	if (c->type != MAT_FULL) {return(AF_ERR_VAL);}

	if ((a->type == MAT_FULL) && (b->type == MAT_FULL) && (c->type == MAT_FULL) )
	{
		for (i=0; i<(a->rows*a->cols); i++)
		{
			c->data[i] = am * a->data[i] + bm * b->data[i] ;
		}
	}
	else
	{
		for (i=1; i<=c->rows; i++)
		{
			for (j=1; j<=c->cols; j++)
			{
				val = am*femMatGet(a, i,j) + bm*femMatGet(b, i,j);
				femMatPut(c, i,j, val);
			}
		}
	}
	return(AF_OK);
}

/** matrix transposition - works only on dense matrices (MAT_FULL)
 * @param a matrix (original)
 * @param b matrix (result - must be allocated)
 * @return status
 */
int femMatTran(tMatrix *a, tMatrix *b)
{
	long i,j;

	if ((a->cols != b->rows) || (b->cols != a->rows)) { return(AF_ERR_SIZ); }
	if ((a->type != MAT_FULL) || (b->type != MAT_FULL)) { return(AF_ERR_SIZ); }

#if 1
	for (i=0; i<a->rows; i++)
	{
		for (j=0; j<a->cols; j++)
		{
			if (a->cols == a->rows)
			{
				b->data[(j*a->cols)+i] = a->data[(i*a->cols)+j] ;
			}
			else
			{
				femMatPut(b,j+1,i+1, femMatGet(a, i+1, j+1));
			}
		}
	}
#else
	for (i=1; i<=a->rows; i++)
	{
		for (j=1; j<=a->cols; j++)
    {
      femMatPut(b, i,j, femMatGet(a, j,i));
    }
  }
#endif
	return(AF_OK);
}

#ifdef _USE_THREADS_
void *thFemMatNorm (void *param)
{
  tThData *p = (tThData*) param ;
	long i,j;
	double Norm = 0.0 ;
	double MaxNorm = 0.0 ;

	Norm = 0.0 ;
	MaxNorm = 0.0 ;

  if (p->to <= 0) 
	{
		p->n_a = 0.0 ;
		return(NULL);
	}

	for (i=p->from; i<p->to; i++)
	{
	  Norm = 0.0;
		for (j= p->m_a->frompos[i]; j< (p->m_a->frompos[i]+p->m_a->defpos[i]); j++)
		{
			if (p->m_a->pos[j] <= 0) {break;}
		  Norm += pow(p->m_a->data[j],2);
		}
		Norm = sqrt(Norm);
		if (Norm > MaxNorm) {MaxNorm = Norm;}
	}

	p->n_a = MaxNorm ;
	return(NULL);
}
#endif

/** Computes norm of sparse matrix
 *  @param a matrix
 *  @return norm
 */
double femMatNormBig(tMatrix *a)
{
	double Norm, MaxNorm,val;
	long i,j;
#ifdef _USE_THREADS_
  tThData data1 ;
  tThData data[AF_MAX_THREADS];
  ldiv_t  dnum ;
  pthread_t Thread_ID[AF_MAX_THREADS];
  pthread_t Thread_1  ;
#endif

	MaxNorm = 0.0;

	if (a->type == MAT_SPAR)
	{
#ifdef _USE_THREADS_
      data1.m_a  = a ;
      data1.from = 0; 
      data1.to   = a->rows ;
      data1.n_a  = 0.0 ;
      
      if ((femUseThreads != AF_YES)||(a->len < femThreadMin))
      {
        pthread_create(&Thread_1,NULL,thFemMatNorm,&data1);
        pthread_join(Thread_1, NULL) ;
				MaxNorm = data1.n_a ;
      }
      else
      {
        dnum = ldiv(a->rows, femThreadNum) ;
        if (dnum.quot < 1)
        {
          dnum.quot = 0 ;
          dnum.rem  = a->rows ;
        }

        for (i=0; i<femThreadNum; i++)
        {
          data[i].m_a  = a ;
          data[i].from = i*dnum.quot ;
          data[i].to   = (i+1)*dnum.quot ;
      		data[i].n_a  = 0.0 ;

          if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

          pthread_create(&Thread_ID[i],NULL,thFemMatNorm,&data[i]);
        }

        for (i=0; i<femThreadNum; i++)
        {
          pthread_join(Thread_ID[i], NULL) ;
        }

				MaxNorm = 0.0 ;

        for (i=0; i<femThreadNum; i++)
        {
					if (data[i].n_a > MaxNorm)
					{
						MaxNorm = data[i].n_a ;
					}
        }
      }
#else
		for (i=0; i<a->rows; i++)
		{
	  	Norm = 0.0;
			for (j= a->frompos[i]; j< (a->frompos[i]+a->defpos[i]); j++)
			{
				if (a->pos[j] <= 0) {break;}
		  	Norm += (a->data[j]*a->data[j]);
			}
			Norm = sqrt(Norm);
			if (Norm > MaxNorm) {MaxNorm = Norm;}
		}
#endif
	}
	else
	{
		for (i=1; i<=a->rows; i++)
		{
	  	Norm = 0.0;
			for (j=1; j<= a->cols; j++)
			{
				val = femMatGet(a, i,j);
		  	Norm += (val*val) ;
			}
			Norm = sqrt(Norm);
			if (Norm > MaxNorm) {MaxNorm = Norm;}
		}
	}
	return(MaxNorm);
}

/** Computes norm of sparse matrix FOR SMALL DATA
 *  @param a matrix
 *  @return norm
 */
double femMatNorm(tMatrix *a)
{
	double Norm, MaxNorm,val;
	long i,j;

	MaxNorm = 0.0;

	if (a->type == MAT_SPAR)
	{
		for (i=0; i<a->rows; i++)
		{
	  	Norm = 0.0;
			for (j= a->frompos[i]; j< (a->frompos[i]+a->defpos[i]); j++)
			{
				if (a->pos[j] <= 0) {break;}
		  	Norm += (a->data[j]*a->data[j]);
			}
			Norm = sqrt(Norm);
			if (Norm > MaxNorm) {MaxNorm = Norm;}
		}
	}
	else
	{
		for (i=1; i<=a->rows; i++)
		{
	  	Norm = 0.0;
			for (j=1; j<= a->cols; j++)
			{
				val = femMatGet(a, i,j);
		  	Norm += (val*val) ;
			}
			Norm = sqrt(Norm);
			if (Norm > MaxNorm) {MaxNorm = Norm;}
		}
	}
	return(MaxNorm);
}

#ifdef _USE_THREADS_
void *thFemVecNorm (void *param)
{
  tThData *p = (tThData*) param ;
	long i;
	double Norm = 0.0 ;

	Norm = 0.0 ;

  if (p->to <= 0) 
	{
		p->n_a = 0.0 ;
		return(NULL);
	}

  for (i=p->from; i<p->to; i++)
	{
		Norm += pow(p->v_a->data[i],2);
	}

	p->n_a = Norm ;
	return(NULL);
}
#endif

/** Computes Euclide norm of vector sum(sqrt(a*a)) 
 *  @param a     vector
 *  @return norm
 */
double femVecNormBig(tVector *a)
{
	double Norm, val;
	int i;
#ifdef _USE_THREADS_
  tThData data1 ;
  tThData data[AF_MAX_THREADS];
  ldiv_t  dnum ;
  pthread_t Thread_ID[AF_MAX_THREADS];
  pthread_t Thread_1  ;
#endif

	Norm = 0.0;

	if (a->type == VEC_FULL)
	{
#ifdef _USE_THREADS_
      data1.v_a  = a ;
      data1.from = 0; 
      data1.to   = a->rows ;
      data1.n_a  = 0.0 ;
      
      if ((femUseThreads != AF_YES)||(a->rows < femThreadMin))
      {
        pthread_create(&Thread_1,NULL,thFemVecNorm,&data1);
        pthread_join(Thread_1, NULL) ;
				Norm = data1.n_a ;
      }
      else
      {
        dnum = ldiv(a->rows, femThreadNum) ;
        if (dnum.quot < 1)
        {
          dnum.quot = 0 ;
          dnum.rem  = a->rows ;
        }

        for (i=0; i<femThreadNum; i++)
        {
          data[i].v_a  = a ;
          data[i].from = i*dnum.quot ;
          data[i].to   = (i+1)*dnum.quot ;
      		data[i].n_a  = 0.0 ;

          if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

          pthread_create(&Thread_ID[i],NULL,thFemVecNorm,&data[i]);
        }

        for (i=0; i<femThreadNum; i++)
        {
          pthread_join(Thread_ID[i], NULL) ;
        }

        Norm = 0.0 ;

        for (i=0; i<femThreadNum; i++)
        {
					Norm += data[i].n_a ;
        }
      }
#else
		for (i=0; i<a->rows; i++)
		{
			Norm += (a->data[i]*a->data[i]) ;
		}
#endif
	}
	else
	{
		for (i=1; i<=a->rows; i++)
		{
			val = femVecGet(a, i);
			Norm += (val*val);
		}
	}

	return(sqrt(Norm));
}

/** Computes Euclide norm of vector sum(sqrt(a*a)) FOR SMALL DATA
 *  @param a     vector
 *  @return norm
 */
double femVecNorm(tVector *a)
{
	double Norm, val;
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

	return(sqrt(Norm));
}

/** Adds vector "addt" to "orig" e.g. orig += mult*addt
 * @param orig original vector (to be modified)
 * @param mult scalar multiplier
 * @param addt addition vector
 * @return status
 */
int femVecAddVec(tVector *orig, double mult, tVector *addt)
{
	long i ;

	if (orig->rows != addt->rows)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout, "[E] %s!\n", _("Different vector sizes not allowed here"));
#endif
		return(AF_ERR_SIZ);
	}
	
	if ( (orig->type == VEC_FULL) && (addt->type == VEC_FULL) )
	{
		for (i=0; i<orig->len; i++)
		{
			orig->data[i] += (mult * addt->data[i]) ;
		}
	}
	else
	{
		for (i=1; i<=orig->len; i++)
		{
			femVecAdd( orig, i, (mult*femVecGet(addt, i)) ) ;
		}
	}

	return(AF_OK) ;
}

/** Does matrix inversion UNOPTIMIZED!
 *  @param a  matrix to be inverted
 */
int femMatInv(tMatrix *a)
{
	long m,n;
	long i,j,k;
	double f,f2;
	double val = 0;
	tVector f1;

#ifdef DEVEL
	if (a->rows != a->cols) {return(AF_ERR_SIZ);}
#endif

	n = a->cols;

	femVecNull(&f1);

	if (femVecFullInit(&f1,n) != AF_OK){return(AF_ERR_MEM);}
	
	m = n-1;

	val = femMatGet(a, 1,1);
	femMatPut(a, 1,1,  (1.0 / val));

	for (i = 1; i <= m; i++)
	{
		
	  for (j = 1; j <= i; j++)
		{
			f = 0.0;

			for (k=1; k <= i; k++)
			{
				f += ( femMatGet(a,j,k) * femMatGet(a,k,i+1) );
			}

			femVecPut(&f1,j,  (-f) );
		}
		
		f2 = femMatGet(a,i+1,i+1);
		
		for (j = 1; j <= i; j++)
		{
			f2 += ( femMatGet(a,j,i+1) * femVecGet(&f1,j) );
		}
		
		if(fabs(f2 / femMatGet(a,i+1,i+1)) < FEM_ZERO)
		{
			return(AF_ERR_VAL);
		}
		
		f2 = 1 / f2;

		femMatPut(a, i+1,i+1,  f2);
		
		for (j = 1; j <= i; j++)
		{
			for (k = 1; k <= i; k++)
			{
				femMatPut(a,j,k, ((femVecGet(&f1,j) * femVecGet(&f1,k) * f2) + femMatGet(a,j,k) ));
			}
		}
		
		for (j = 1; j <= i; j++)
		{
			femMatPut(a,j,i+1,  (femVecGet(&f1,j) * f2) );
			femMatPut(a,i+1,j,  femMatGet(a,j,i+1) );
		}

	}

	femVecFree(&f1);
	return(AF_OK);
}

/* L-U: */

/** Decomposition to L/U
 * @param a matrix (will be modified!)
 * @param index index vector
 * @param d modified index status (-1/+1)
 * @return status
 */
int femLUdecomp(tMatrix *a, tVector *index)
{
	int rv = AF_OK;
	long i, j, k;
  long imax = 0  ;
	long n;
	double big,dum,sum,temp;
	tVector vv ;

	femVecNull(&vv);

	if((n = a->rows) <= 0) {return(AF_ERR_SIZ);}
	if ((rv=femVecFullInit(&vv, n)) != AF_OK) {goto memFree;}

	for (i=1; i<=n; i++)
	{
		big = 0.0 ;

		for (j=1; j<=n; j++)
		{
			if ((temp=fabs(femMatGet(a, i,j))) > big) 
      {
        big = temp;
      }
		}

		if (big == 0.0)
		{
			/* singular matrix */
			return(AF_ERR_VAL);
		}

		femVecPut(&vv, i,  (1.0/big) );
	}

	for (j=1; j<=n; j++)
	{
		for (i=1; i<j; i++)
		{
			sum = femMatGet(a, i,j);
			for (k=1; k<i; k++)
			{
				sum -= ( femMatGet(a, i,k)*femMatGet(a, k,j) ) ;
			}
			femMatPut(a, i, j, sum);
		}

		big = 0.0 ;
		for (i=j; i<=n; i++)
		{
			sum = femMatGet(a, i,j);
			for (k=1; k<j; k++)
			{
				sum -= ( femMatGet(a, i,k)*femMatGet(a, k,j) ) ;
			}

			femMatPut(a, i, j, sum);

			if ((dum=femVecGet(&vv,i)*fabs(sum)) >= big)
			{
				big  = dum ;
				imax = i ;
			}
		}

		if (j != imax)
		{
			for (k=1; k<=n; k++)
			{
				dum = femMatGet(a, imax, k) ;
				femMatPut(a, imax, k, femMatGet(a, j, k)) ;
				femMatPut(a, j, k, dum) ;
			}

			femVecPut(&vv, imax, femVecGet(&vv, j)) ;
		}

		femVecPut(index, j,  imax) ;

		if (femMatGet(a, j, j) == 0.0) {femMatPut(a,j,j, FEM_TINY_VAL);}

		if (j != n)
		{
			dum = 1.0 / femMatGet(a, j, j) ;
			for (i = (j+1); i<=n; i++)
			{
				femMatPut( a, i,j, ( dum*femMatGet(a,i,j) ) ) ;
			}
		}
	}

memFree:
	femVecFree(&vv);
	return(rv);
}


/** Decomposition to L/U
 * @param a matrix (will be modified!)
 * @param index index vector
 * @param b right hand side/result vector (will be modified!)
 * @return status
 */
int femLUback(tMatrix *a, tVector *index, tVector *b)
{
	int    rv = AF_OK ;
	long   i,ii,ip,j;
	long   n ;
	double sum ;

	ii = 0 ;

	if ((n = a->rows) <= 0) {return(AF_ERR_SIZ);}

	for (i=1; i<=n; i++)
	{
		ip  = (long)femVecGet(index, i);
		sum = femVecGet(b, ip) ;
		femVecPut(b, ip, femVecGet(b, i) );

		if (ii) /* means ii > 0 */
		{
			for (j=ii; j<=i-1; j++)
			{
				sum -= femMatGet(a, i,j)*femVecGet(b, j) ;
			}
		}
		else
		{
			if (sum)
			{
				ii = i ;
			}
		}

		femVecPut(b, i, sum);
	}

	for (i=n; i>=1; i--)
	{
		sum = femVecGet(b, i);
		for (j=i+1; j<=n; j++)
		{
			sum -= femMatGet(a, i,j)*femVecGet(b, j) ;
		}
		femVecPut(b, i, (sum / femMatGet(a, i,i ) ) ) ;
	}

	return(rv);
}

/** Inversion of "a" matrix using L/U
 * @param a matrix (will be modified!)
 * @return status
 */
int femLUinverse(tMatrix *a)
{
	int     rv = AF_OK ;
  long    i,j ;
	long    n ;
	tVector col ;
	tVector index ;
	tMatrix b ;

	if ((n = a->rows) <= 0) {return(AF_ERR_SIZ);}

	femVecNull(&col);
	femVecNull(&index);
	femMatNull(&b);

	if ((rv=femVecFullInit(&col, n)) != AF_OK) {goto memFree;}
	if ((rv=femVecFullInit(&index, n)) != AF_OK) {goto memFree;}
	if ((rv=femFullMatInit(&b, n,n)) != AF_OK) {goto memFree;}

	if ((rv = femLUdecomp(a, &index)) != AF_OK){goto memFree;}

  for (j=1; j<=n; j++)
  {
    for (i=1; i<=n; i++) { femVecPut(&col, i, 0.0 ); }

    femVecPut(&col, j, 1.0 ) ;
	  if ((rv = femLUback(a, &index, &col)) != AF_OK){goto memFree;}

    for (i=1; i<=n; i++) { femMatPut(&b, i,j, femVecGet(&col, i) ); }
  }

  for (i=1; i<=n; i++)
  {
    for (j=1; j<=n; j++)
    {
      femMatPut(a, i,j, femMatGet(&b, i,j) ) ;
    }
  }
memFree:
	femVecFree(&col);
	femVecFree(&index);
	femMatFree(&b);
	return(rv);
}

/** Moves "a" to "b" and "b" to "a"
 * @param a vector
 * @param b vector
 * @return status
 */
int femVecSwitch(tVector *a, tVector *b)
{
	double val ;
	long i ;

	if ((a->rows != b->rows)||(a->type!=VEC_FULL)||(b->type!=VEC_FULL))
	{
		return(AF_ERR_SIZ);
	}
		
  for (i=0; i<a->len; i++)
	{
		val = a->data[i] ;
		a->data[i] = b->data[i] ;
		b->data[i] = val ;
	}

	return(AF_OK);
}

/** Copies vector content to a larger one (extra fields are left untouched)
 * @param orig original vector
 * @param clone target vector (to be modified)
 * @return status
 */ 
int femVecCloneDiff(tVector *orig, tVector *clone)
{
	long i ;
	long len ;

	if ( (orig->type != VEC_FULL) || (clone->type != VEC_FULL) )
	{
		return(AF_ERR_TYP);
	}

	if ((clone->rows < 1) || (orig->rows < 1))
	{
		return(AF_ERR_SIZ);
	}

	if (orig->rows > clone->rows)
	     { len = clone->rows ; }
	else { len = orig->rows ; }

	for (i=0; i< len; i++)
	{
		clone->data[i] = orig->data[i] ;
	}

	return(AF_OK);
}


/** TODO FIX!!! Copies sparse matrix content to a larger one (extra fields are left untouched)
 * it is assumed that a) there is a space for data in "clone", b) identical data
 * in both matrices are stored at identical places 
 * @param orig original vector
 * @param clone target vector (to be modified)
 * @return status
 */ 
int femMatCloneDiffToSame(tMatrix *orig, tMatrix *clone)
{
	long i, j, k, ko, kc ;

	if ( (orig->type != MAT_SPAR) || (clone->type != MAT_SPAR) )
	{
		return(AF_ERR_TYP);
	}

	if ((orig->rows > clone->rows) || (orig->rows < 1))
	{
		return(AF_ERR_SIZ);
	}

	if ((orig->cols > clone->cols) || (orig->cols < 1))
	{
		return(AF_ERR_SIZ);
	}

	for (i=0; i< orig->rows; i++)
	{
		k = 0 ;
		for (j=orig->frompos[i]; j<orig->frompos[i]+orig->defpos[i]; j++)
		{
			ko = k + orig->frompos[i] ; 
			kc = k + clone->frompos[i] ; 

#if 0
			if (orig->pos[ko] != clone->pos[kc])
			{
#ifdef DEVEL_VERBOSE
				fprintf(msgout,"[E] Matrix incompatibility at line %li: %li != %li\n",
						i+1, orig->pos[ko], clone->pos[kc]);
#endif
				return(AF_ERR_VAL);
			}
#endif

			clone->data[kc] = orig->data[ko] ;
      k++ ;
		}
	}

	return(AF_OK);
}

/** Copies sparse matrix content to a larger one (extra fields are left untouched)
 * it is assumed that a) there is a space for data in "clone", b) identical data
 * in both matrices are stored at identical places 
 * @param orig original vector
 * @param clone target vector (to be modified)
 * @return status
 */ 
int femMatCloneDiffToEmpty(tMatrix *orig, tMatrix *clone)
{
	long i, j ;

	if ( (orig->type != MAT_SPAR) || (clone->type != MAT_SPAR) )
	{
		return(AF_ERR_TYP);
	}

	if ((orig->rows > clone->rows) || (orig->rows < 1))
	{
		return(AF_ERR_SIZ);
	}

	if ((orig->cols > clone->cols) || (orig->cols < 1))
	{
		return(AF_ERR_SIZ);
	}

	for (i=0; i< orig->rows; i++)
	{
		for (j=orig->frompos[i]; j<orig->frompos[i]+orig->defpos[i]; j++)
		{
      femMatPut(clone, i+1, orig->pos[j], orig->data[j]) ;
		}
	}

	return(AF_OK);
}

/* end of fem_math.c */

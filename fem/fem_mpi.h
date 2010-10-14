/*
   File name: fem_mpi.h
   Date:      2003/04/24 12:33
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

	 FEM Solver - MPI Communication code

	 $Id: fem_mpi.h,v 1.11 2004/08/01 11:08:57 jirka Exp $
*/


#ifndef __FEM_MPI_H__
#define __FEM_MPI_H__


#include "fem.h"
#include "fem_mem.h"
#include "fem_data.h"
#include "fem_dama.h"
#include "fem_elem.h"
#include "fem_mat.h"

#ifdef USE_MPI /* only used if USE_MPI is defined: */
#include <mpi.h>

extern int ppRank ;
extern int ppSize ;

extern long  ppDOFfrom ;
extern long  ppDOFto   ;
extern long  ppDOFlen  ;

extern void femMPIwhoami(int *rank, int *size);
extern int femSendInitDataPP(void);
extern int femMyNodes(int myRank, int mySize, long *myFrom, long *myNum);

extern int femSolveMPI(void);
extern long *femMyElems(long myFrom, long myNum, long *myEnum);
extern int femDelOthElem(long myEnum, long *eList);
extern void femMyDOFs(long nFrom, long nNum, long *from, long *to);
extern int *femDOFsizes(void);
extern int mod_K_rows(long dofFrom, long dofTo, long nNumDOF);


extern long ppDataCpyInt(signed long int *dest, signed long int *src, long len, long from);
extern long ppDataCpyDbl(double *dest, double *src, long len, long from);
extern long ppDataCpyIntBack(signed long int *dest, signed long int *src, long len, long from);
extern long ppDataCpyDblBack(double *dest, double *src, long len, long from);

extern int femMPIeqsCGwJ(tMatrix *a, tVector *b, tVector *x, tVector *x_big, double eps, long maxIt);
extern int femMPIeqsPCGwJ_New(tMatrix *a, tVector *b, tVector *x, tVector *x_big, double eps, long maxIt);

extern int femSolveNRM_MPI(long incr_type) ;

#endif
#endif

/* end of fem_mpi.h */

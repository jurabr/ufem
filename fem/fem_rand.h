/*
   File name: fem_rand.h
   Date:      2004/01/31 19:46
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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

   FEM Solver - Monte Carlo Simulation

   $Id: fem_rand.h,v 1.5 2004/02/26 17:43:24 jirka Exp $
*/


#ifndef __FEM_RAND_H__
#define __FEM_RAND_H__

#include "fem.h"
#include "fem_mem.h"
#include "fem_data.h"
#include "fem_math.h"
#include "fem_elem.h"

typedef struct /* distribution function */
{
  long    len ; /* lenght of prob and size */
  double *prob; /* probability values      */
  double *size; /* data size multiplier    */
}tRanDF;

extern tRanDF *RandDF    ;
extern long    RandDFlen ;

extern long   RandValLen ;
extern long  *randVal    ;
extern long  *randData   ;
extern long  *randSet    ;
extern long  *randPos    ;
extern long  *randRpos   ;

extern double   *n_x_rand    ;
extern double   *n_y_rand    ;
extern double   *n_z_rand    ;
extern double   *rsValL_rand ;
extern double   *mpValL_rand ;
extern double   *nlVal_rand  ;
extern double   *elValL_rand ;

extern long fem_rand_count   ;


extern double fem_rand(long *inpl);
extern int fem_read_rand_data(char *fname);

extern void fem_rand_backup_free(void);
extern int fem_rand_backup_orig_data(void);
extern void fem_rand_free_res(void);
extern int fem_rand_alloc_res(void);
extern int fem_rand_add_res(void);
extern int fem_rand_swap_res1(void);
extern int fem_rand_swap_res2(void);
extern int fem_rand_swap_resmax(void);
extern int fem_rand_swap_resmin(void);
extern int fem_rand_create_random_data(void);

extern int fem_rand_test_fail(void);

#endif

/* end of fem_rand.h */

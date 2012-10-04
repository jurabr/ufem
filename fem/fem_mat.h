/*
   File name: fem_mat.h
   Date:      2003/04/16 20:37
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

	 FEM Solver - material definition - header file

  $Id: fem_mat.h,v 1.7 2004/11/11 21:39:24 jirka Exp $
*/


#ifndef __FEM_MAT_H__
#define __FEM_MAT_H__

/* Included headers: */
#include <stdlib.h>
#include "fem.h"
#include "fem_dama.h"
#include "fem_data.h"
#include "fem_math.h"

/* DOF type definitions: */
#define MAT_DENS     1
#define MAT_EX       2
#define MAT_EY       3
#define MAT_EZ       4
#define MAT_NU       5
#define MAT_G        6
#define MAT_STEN     7
#define MAT_SCMPR    8
#define MAT_GF       9
#define MAT_E1      10
#define MAT_STEN1   11
#define MAT_F_YC    12
#define MAT_F_YBC   13
#define MAT_F_YT    14
#define MAT_F_UC    15
#define MAT_F_UBC   16
#define MAT_F_UT    17
#define MAT_COHES   18
#define MAT_FRICT   19
#define MAT_HARD    20
#define MAT_NUXY    21
#define MAT_NUYZ    22
#define MAT_NUZX    23
#define MAT_GXY     24
#define MAT_GYZ     25
#define MAT_GZX     26
#define MAT_RAD     27
#define MAT_WX      28
#define MAT_SELE    29
#define MAT_RAMB_K  30
#define MAT_RAMB_N  31
#define MAT_SHTYPE  32
#define MAT_FCTYPE  33
#define MAT_CRTYPE  34
#define MAT_GFC     35
#define MAT_PRICE   36
#define MAT_KXX     37 /* thermal conductivity */
#define MAT_ALPHA   38 /* thermal expansion coefficient */
#define MAT_C       39 /* specitic heat */


/* structure for finite element */
typedef struct
{
	long  type;   /* material type number                    */
	long *val;    /* list of properties (val[num])           */
	long  num;    /* number of properties                    */
	long *val_rp; /* list of REPEATING properties (val[num]) */
	long  num_rp; /* number of REPEATING properties          */
	int (* test)(long);  /* material definition test         */
} tMat;

/* definitions of data structures */
extern tMat  *Mat;     /* elements */
extern long   matLen;  /* number of elements */

/* functions */
extern long femAddMat(long type);
extern int femMatTypeInit(void);

#endif

/* end of fem_elem.h */

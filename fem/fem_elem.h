/*
   File name: fem_elem.h
   Date:      2003/04/09 10:27
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

	 FEM Solver - element definition - header file

  $Id: fem_elem.h,v 1.16 2005/07/11 17:56:16 jirka Exp $
*/


#ifndef __FEM_ELEM_H__
#define __FEM_ELEM_H__

/* Included headers: */
#include <stdlib.h>
#include "fem.h"
#include "fem_dama.h"
#include "fem_data.h"
#include "fem_math.h"
#include "fem_mat.h"
#include "fem_eres.h"
#include "fem_sol.h"

/* DOF type definitions (used to describe force/moment direction, too): */
#define KNOWN_DOFS 7
#define U_X   1
#define U_Y   2
#define U_Z   3
#define ROT_X 4
#define ROT_Y 5
#define ROT_Z 6
#define TEMP  7

#define RS_AREA          1
#define RS_WIDTH         2
#define RS_HEIGHT        3 
#define RS_ROT1          4
#define RS_RFC1          5
#define RS_ROT2          6
#define RS_RFC2          7
#define RS_INERTIA_X     8 /* moment in torsion */
#define RS_INERTIA_Y     9 /* moment of inertia */
#define RS_INERTIA_Z    10 /* moment of inertia */
#define RS_STF1         11
#define RS_STF2         12
#define RS_WINKLER      13 /* Winkler elastic foundation stiffness */
#define RS_M_CRV        14
#define RS_N_CRV        15
#define RS_EI_CRV       16
#define RS_STF_X_1      17
#define RS_STF_Y_1      18
#define RS_STF_Z_1      19
#define RS_STF_X_2      20
#define RS_STF_Y_2      21
#define RS_STF_Z_2      22
#define RS_MTYPE        23 /* material type - if needed */
#define RS_LNUM         24 /* numbers of sublayers */

/* structure for finite element */
typedef struct
{
	long  type;   /* element type number */
	long  dim;    /* dimensionality: 1..link/2..wall/3..3D/4..slab,shell/5..beam */
	long  nodes;  /* number of nodes     */
	long  dofs;   /* number of degrees of freedom in one node */
	long *ndof;   /* list of DOFs in each node (ndof[nodes])  */
	long  rs;     /* number of real set values */
	long *real;   /* list of real set values (real[rs]) */
	long  rs_rp;  /* number of REPEATING real set values */
	long *real_rp;/* list of REPEATING real set values (real_rp[rs_rp]) */
	long  res;    /* number of results */
	long *nres;   /* list of results nres[res] */
	long  res_rp;    /* number of REPEATING results */
	long *nres_rp;   /* list of REPEATING results nres_rp[res_rp] */
	int (* stiff)(long, long, tMatrix *, tVector *, tVector *); /* element stifness matrix (position,mode,nodal forces,FE,residual) */
	int (* therm)(long, long *, tVector *, tVector *); /* element stifness matrix (position,mode,nodal forces,FE,residual) */
	int (* mass )(long, tMatrix *);  /* element mass matrix (position) */
	long (* rvals)(long);  /* number of element results (position) */
	int (* eload)(long, long, tVector *);  /* element load usage (elem. position, load pos.) */
	int (* res_p_loc)(long, long, double *, double *, double *); /* x,y,z of element result point */
  int (* res_node)(long, long, long, double *); /* element pos, node pos, result type, value(returned) */
  int (* volume)(long, double *); /* element pos, volume(returned) */
} tElem;

/* definitions of data structures */
extern tElem *Elem;     /* elements */
extern long   elemLen;  /* number of elements */

/* functions */
extern long femAddElem(long type);
extern int femElemTypeInit(void);

extern long femElemIPoints(long ePos);

extern int fem_D_2D(long ePos, long iPoint, double A, tVector *epsilon, tVector *sigma, tVector *sigma_r, long newM, long Problem, tMatrix *D);

extern int e000_res_p_loc(long ePos, long point, double *x, double *y, double *z);
extern int e000_res_node(long ePos, long nPos, long type, double *val);
extern int e000_therm(long ePos, long *use, tVector *Te, tVector *Fe);

#endif

/* end of fem_elem.h */

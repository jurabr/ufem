/*
   File name: fem_mont.h
   Date:      2010/11/29 14:00
   Author:    Jiri Brozovsky

   Copyright (C) 2010 Jiri Brozovsky

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

	 uFEM interface for Monte: variables definitions
*/


#ifndef __FEM_MONT_H__
#define __FEM_MONT_H__

#define MONTE_VTYPE_RS          0
#define MONTE_VTYPE_MAT         1
#define MONTE_VTYPE_N           2
#define MONTE_VTYPE_NLD         3
#define MONTE_VTYPE_EL          4
#define MONTE_VTYPE_NLPOS       5 /* node load with its ID */
#define MONTE_VTYPE_DAMP        6 /* Rayleigh damping data */
#define MONTE_VTYPE_MAX_D       7 /* maximum individual displacement*/

#define MONTE_VTYPE_RES_D       8 /* must be the first output value */
#define MONTE_VTYPE_RES_R       9
#define MONTE_VTYPE_RES_E      10
#define MONTE_VTYPE_RES_SUM_E  11 /* sum of element results */
#define MONTE_VTYPE_RES_MAX_E  12 /* max value of elem. res. */
#define MONTE_VTYPE_RES_MIN_E  13 /* min value of elem. res. */
#define MONTE_VTYPE_RES_FAIL_E 14 /* 1 for fail, 0 for OK (determined from given elem. res, fails for res>0) */
#define MONTE_VTYPE_RES_D_MAX  15 /* max. displacement in node (dynamics) */
#define MONTE_VTYPE_RES_D_MIN  16 /* min. displacement in node (dynamics) */
#define MONTE_VTYPE_RES_D_SUM  17 /* sum of displacements in node (dynamics) */
#define MONTE_VTYPE_RES_A_MAX  18 /* maximum acceleration (in dynamics)  */
#define MONTE_VTYPE_RES_FAIL   19 /* general failure function            */

extern long fem_asse_max_disp_simple(double disp_max); /* max. displacment test */
extern long fem_asse_fail_cond(void); /* failure condition */

#endif

/* end of fem_mont.h */

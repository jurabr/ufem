/*
   File name: fem_m003.c
   Date:      2004/01/10 17:50
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

	FEM Solver - Material 003 (3D concrete - Chen+Chen)
*/

#include "fem_mat.h"

int e003_test(long mPos) 
{ 
	/* Datatest needed ;-) */
	return(AF_OK); 
}


int addMat_003(void)
{
	int rv = AF_OK;
	static long  type      = 3 ;
	static long  num       = 11 ;
	static long  val[11]   = {MAT_DENS, MAT_EX, MAT_NU,
  MAT_F_YC,MAT_F_YBC,MAT_F_YT,MAT_F_UC,MAT_F_UBC,MAT_F_UT,MAT_E1, MAT_PRICE};
	static long  num_rp    = 2 ;
	static long  val_rp[2] = {MAT_RAMB_K, MAT_RAMB_N} ;

	if (type != femAddMat(type)) {return(AF_ERR_VAL);}
	Mat[type].num = num ;
	Mat[type].val = val ;
	Mat[type].num_rp = num_rp ;
	Mat[type].val_rp = val_rp ;

	Mat[type].test = e003_test;
	return(rv);
}



/* end of fem_m003.c */

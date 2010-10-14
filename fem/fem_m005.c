/*
   File name: fem_m005.c
   Date:      2003/04/16 21:01
   Author:    Jiri Brozovsky

   Copyright (C) 2003 

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

	FEM Solver - Material 005 (Hooke for assessment)
*/

#include "fem_mat.h"

int mat005_test(long mPos) 
{ 
	/* Some datatest needed ;-) */
	return(AF_OK); 
}

int addMat_005(void)
{
	int rv = AF_OK;
	static long  type      = 5 ;
	static long  num       = 8 ;
	static long  val[8]    = {MAT_DENS, MAT_EX, MAT_NU, MAT_G,MAT_F_YC,MAT_F_YT,MAT_F_UC,MAT_F_UT};
	static long  num_rp    = 8 ;
	static long  val_rp[8] = {MAT_DENS, MAT_EX, MAT_NU, MAT_G,MAT_F_YC,MAT_F_YT,MAT_F_UC,MAT_F_UT};

	if (type != femAddMat(type)) {return(AF_ERR_VAL);}
	Mat[type].num = num ;
	Mat[type].val = val ;
	Mat[type].num_rp = num_rp ;
	Mat[type].val_rp = val_rp ;

	Mat[type].test = mat005_test;
	return(rv);
}

/* end of fem_e005.c */

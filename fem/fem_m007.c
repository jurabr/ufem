/*
   File name: fem_m007.c
   Date:      2004/09/05 14:16
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

	FEM Solver - Material 007 (linear orthotropic)

	Note that material axes must be identical to x,y,z!
*/

#include "fem_mat.h"

int mat007_test(long mPos) 
{ 
	/* Some datatest needed ;-) */
	return(AF_OK); 
}


int addMat_007(void)
{
	int rv = AF_OK;
	static long  type      = 7 ;
	static long  num       = 10 ;
	static long  val[10]   = {MAT_DENS, MAT_EX,MAT_EY,MAT_EZ, MAT_NUXY,MAT_NUYZ,MAT_NUZX, MAT_GXY,MAT_GYZ,MAT_GZX};
	static long  num_rp    = 1 ;
	static long  val_rp[1] = {MAT_ANG} ;

	if (type != femAddMat(type)) {return(AF_ERR_VAL);}
	Mat[type].num = num ;
	Mat[type].val = val ;
	Mat[type].num_rp = num_rp ;
	Mat[type].val_rp = val_rp ;

	Mat[type].test = mat007_test;
	return(rv);
}

/* end of fem_e007.c */

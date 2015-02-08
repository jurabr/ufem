/*
   File name: fem_m011.c
   Date:      2015/02/05 20:46
   Author:    Jiri Brozovsky

   Copyright (C) 2015 Jiri Brozovsky

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

	FEM Solver - Material 011 (linear trnasversely isotropic)

	Note that material axes must be identical to x,y,z!
*/

#include "fem_mat.h"

int mat011_test(long mPos) 
{ 
	/* Some datatest needed ;-) */
	return(AF_OK); 
}


int addMat_011(void)
{
	int rv = AF_OK;
	static long  type      = 11 ;
	static long  num       = 6 ;
	static long  val[6]    = {MAT_DENS,MAT_EX,MAT_EZ,MAT_NUXY,MAT_NUYZ,MAT_GXY};
	static long  num_rp    = 0 ;
	static long *val_rp    = NULL ;

	if (type != femAddMat(type)) {return(AF_ERR_VAL);}
	Mat[type].num = num ;
	Mat[type].val = val ;
	Mat[type].num_rp = num_rp ;
	Mat[type].val_rp = val_rp ;

	Mat[type].test = mat011_test;
	return(rv);
}

/* end of fem_e011.c */

/*
   File name: fem_m009.c
   Date:      2006/11/26 16:04
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

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

	FEM Solver - Material 010 (compression-only material: concrete)
*/

#include "fem_mat.h"

int e010_test(long mPos) 
{ 
	/* Datatest needed ;-) */
	return(AF_OK); 
}

int addMat_010(void)
{
	int rv = AF_OK;
	static long  type      = 10 ;
	static long  num       = 6 ;
	static long  val[6]    = {MAT_DENS, MAT_EX, MAT_NU,
    MAT_STEN, MAT_SCMPR, MAT_E1};
	static long  num_rp    = 0 ;
	static long *val_rp    = NULL ;

	if (type != femAddMat(type)) {return(AF_ERR_VAL);}
	Mat[type].num = num ;
	Mat[type].val = val ;
	Mat[type].num_rp = num_rp ;
	Mat[type].val_rp = val_rp ;

	Mat[type].test = e010_test;
	return(rv);
}

/* end of fem_e009.c */

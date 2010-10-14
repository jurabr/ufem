/*
   File name: fem_m008.c
   Date:      2006/00/11 14:01
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

	FEM Solver - Material 008 (2D concrete fracture)

  $Id: fem_m008.c,v 1.4 2004/01/10 21:09:29 jirka Exp $
*/

#include "fem_mat.h"

int e008_test(long mPos) 
{ 
	/* Datatest needed ;-) */
	return(AF_OK); 
}


int addMat_008(void)
{
	int rv = AF_OK;
	static long  type      = 8 ;
	static long  num       = 12 ;
	static long  val[12]    = {MAT_DENS, MAT_EX, MAT_NU,
    MAT_STEN, MAT_SCMPR, MAT_GF, MAT_E1, MAT_STEN1,MAT_SELE,MAT_SHTYPE,MAT_FCTYPE, MAT_CRTYPE}; /* SELE=type, SHTYPE=shear reduction type, CRTYPE=fix/rot cracks */
	static long  num_rp    = 1 ;
	static long  val_rp[1] = {MAT_WX}; /* WX = softening parameter */

	if (type != femAddMat(type)) {return(AF_ERR_VAL);}
	Mat[type].num = num ;
	Mat[type].val = val ;
	Mat[type].num_rp = num_rp ;
	Mat[type].val_rp = val_rp ;

	Mat[type].test = e008_test;
	return(rv);
}

/* end of fem_e008.c */

/*
   File name: fem_m006.c
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

	FEM Solver - Material 006 (3D inelastic bone)

  $Id: fem_m006.c,v 1.1 2004/11/11 21:35:02 jirka Exp $
*/

#include "fem_mat.h"

int e006_test(long mPos) 
{ 
	/* Datatest needed ;-) */
	return(AF_OK); 
}


int addMat_006(void)
{
	int rv = AF_OK;
	static long  type      = 6 ;
	static long  num       = 8 ;
	static long  val[8]    = {MAT_DENS, MAT_EX, MAT_NU, MAT_G, MAT_E1, 
  MAT_F_UC,MAT_F_UBC,MAT_F_UT};
	static long  num_rp    = 0 ;
	static long *val_rp    = NULL ;

	if (type != femAddMat(type)) {return(AF_ERR_VAL);}
	Mat[type].num = num ;
	Mat[type].val = val ;
	Mat[type].num_rp = num_rp ;
	Mat[type].val_rp = val_rp ;

	Mat[type].test = e006_test;
	return(rv);
}

/* end of fem_m006.c */

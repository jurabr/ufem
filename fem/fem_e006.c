/*
   File name: fem_e006.c
   Date:      2003/04/09 13:11
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

	FEM Solver - Element 006 (2D link)
*/

#include "fem_elem.h"

extern tVector u;

extern int e001_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e);
extern int e001_mass(long ePos, tMatrix *M_e);
extern long e001_rvals(long ePos);
extern int e001_eload(long ePos, long mode, tVector *F_e);
extern int e001_res_p_loc(long ePos, long point, double *x, double *y, double *z);

int addElem_006(void)
{
	int rv = AF_OK;
	static long type    = 6 ;
	static long nodes   = 2 ;
	static long dofs    = 2 ;
	static long ndof[2] = {U_X,U_Y} ;
	static long rs      = 3 ;
	static long real[3] = {RS_AREA,RS_INERTIA_Y,RS_INERTIA_Z} ;
	static long rs_rp      = 0 ;
	static long *real_rp = NULL ;
	static long res      = 3 ;
	static long nres[3] = {RES_FX,RES_SX,RES_EX} ;
	static long res_rp      = 0 ;
	static long *nres_rp = NULL ;

	if (type != femAddElem(type)) {return(AF_ERR_VAL);}
	Elem[type].nodes = nodes ;
	Elem[type].dofs = dofs ;
	Elem[type].ndof = ndof ;
	Elem[type].rs = rs ;
	Elem[type].real = real ;
	Elem[type].rs_rp = rs_rp ;
	Elem[type].real_rp = real_rp ;
	Elem[type].res = res ;
	Elem[type].nres = nres ;
	Elem[type].res_rp = res_rp ;
	Elem[type].nres_rp = nres_rp ;

	Elem[type].stiff = e001_stiff;
	Elem[type].mass  = e001_mass;
	Elem[type].rvals = e001_rvals;
	Elem[type].eload = e001_eload;
	Elem[type].res_p_loc = e001_res_p_loc;
	Elem[type].res_node = e000_res_node;
	return(rv);
}


/* end of fem_e006.c */

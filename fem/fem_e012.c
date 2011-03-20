/*
   File name: fem_e008.c
   Date:      Sun May 16 14:18:29 CET 2004
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

	 FEM Solver - Element 012 (structural brick - 20 nodes)
   Orientation of crack is saved inside results.

   $Id: fem_e012.c,v 1.1 2004/11/11 21:33:56 jirka Exp $
*/

#include "fem_elem.h"
#include "fem_pl3d.h"

extern int e008_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e);
extern int e008_mass(long ePos, tMatrix *M_e);
extern int e008_eload(long ePos, long mode, tVector *F_e);
extern int e008_res_p_loc(long ePos, long point, double *x, double *y, double *z);
extern int e008_volume(long ePos, double *vol);

long e012_rvals(long ePos)
{
	return(26*27);
}


int addElem_012(void)
{
	int rv = AF_OK;
	static long type    = 12 ;
	static long nodes   = 20 ;
	static long dofs    = 3 ;
	static long ndof[3] = {U_X,U_Y,U_Z} ;
	static long rs      = 0 ;
	static long *real   = NULL ;
	static long rs_rp    = 0 ;
	static long *real_rp = NULL ;
	static long res      = 0 ;
	static long *nres    = NULL ;
	static long res_rp      = 26 ;
	static long nres_rp[26] = {
    RES_SX,RES_SY,RES_SZ,RES_SYZ,RES_SZX,RES_SXY, 
    RES_EX,RES_EY,RES_EZ,RES_EYZ,RES_EZX,RES_EXY,
    RES_CR1,RES_PSI,
    RES_ECR1, RES_ECR2, RES_GCR1,
    RES_P_M11,RES_P_M12,RES_P_M13,
    RES_P_M21,RES_P_M22,RES_P_M23,
    RES_P_M31,RES_P_M32,RES_P_M33};

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

	Elem[type].stiff = e008_stiff;
	Elem[type].mass  = e008_mass;
	Elem[type].rvals = e012_rvals;
	Elem[type].eload = e008_eload;
	Elem[type].res_p_loc = e008_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e008_volume;
	return(rv);
}

/* end of fem_e008.c */

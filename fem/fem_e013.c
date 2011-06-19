/*
   File name: fem_e013.c
   Date:      2005/03/02 20:42
   Author:    Jiri Brozovsky

   Copyright (C) 2003  Jiri Brozovsky

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

	 FEM Solver - Element 013 (2D beam with stiffness curve)

   Note that no element load is used (nodal forces/moments only)

	 $Id$
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_
extern tVector u;

extern int e003_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e);
extern int e003_mass(long ePos, tMatrix *M_e);
extern long e003_rvals(long ePos);
extern int e003_eload(long ePos, long mode, tVector *F_e);
extern int e003_res_p_loc(long ePos, long point, double *x, double *y, double *z);
extern int e003_volume(long ePos, double *vol);

int addElem_013(void)
{
	int rv = AF_OK;
	static long type    = 13 ;
	static long nodes   = 2 ;
	static long dofs    = 3 ;
	static long ndof[3] = {U_X,U_Y,ROT_Z} ;
	static long rs      = 4 ;
	static long real[4] = {RS_AREA, RS_INERTIA_Y, RS_STF1,RS_STF2} ;
	static long rs_rp   = 3 ;
	static long real_rp[3] = {RS_M_CRV, RS_N_CRV, RS_EI_CRV} ; /* EI curve data */
	static long res      = 4 ;
	static long nres[4]    = {RES_FX,RES_FY,RES_MZ,RES_EI};
	static long res_rp   = 4 ;
	static long nres_rp[4] = {RES_FX,RES_FY,RES_MZ,RES_EI};

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

	Elem[type].stiff = e003_stiff;
	Elem[type].mass  = e003_mass;
	Elem[type].rvals = e003_rvals;
	Elem[type].eload = e003_eload;
	Elem[type].res_p_loc = e003_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e003_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

#endif
/* end of fem_e003.c */

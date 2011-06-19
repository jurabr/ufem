/*
   File name: fem_e000.c
   Date:      2003/04/12 15:20
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

	FEM Solver - Element 000 (empty)
*/

#include "fem_elem.h"

int e000_stiff(long ePos, long Mode, tMatrix *Ke, tVector *Fe, tVector *Fre) { return(AF_OK); }
int e000_mass(long ePos, tMatrix *Me) { return(AF_OK); }
long e000_rvals(long ePos) { return(0); }
int e000_eload(long ePos, long mode, tVector *Fe) { return(AF_OK); }
int e000_res_p_loc(long ePos, long point, double *x, double *y, double *z){return(AF_OK);}
int e000_res_node(long ePos, long nPos, long type, double *val){return(AF_OK);}
int e000_volume(long ePos, double *val){return(AF_OK);}
int e000_therm(long ePos, long *use, tVector *Te, tVector *Fe) { *use = AF_NO; return(AF_OK); }




int addElem_000(void)
{
	int rv = AF_OK;
	static long type    = 0 ;
	static long nodes   = 0 ;
	static long dofs    = 0 ;
	static long *ndof = NULL;
	static long rs      = 0 ;
	static long *real = NULL;
	static long rs_rp      = 0 ;
	static long *real_rp = NULL ;
	static long res      = 1 ;
	static long *nres = NULL;
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

	Elem[type].stiff = e000_stiff;
	Elem[type].mass  = e000_mass;
	Elem[type].rvals = e000_rvals;
	Elem[type].eload = e000_eload;
	Elem[type].res_p_loc = e000_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e000_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}


/* end of fem_e000.c */

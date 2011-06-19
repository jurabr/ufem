/*
   File name: fem_e009.c
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

	 FEM Solver - Element 010 (structural brick - 8 nodes)
   Orientation of cracks is saved inside results.

   $Id: fem_e010.c,v 1.3 2004/12/31 00:31:06 jirka Exp $
*/

#include "fem_elem.h"
#include "fem_pl3d.h"

extern int e009_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e);
extern int e009_mass(long ePos, tMatrix *M_e);
extern int e009_eload(long ePos, long mode, tVector *F_e);
extern int e008_res_p_loc(long ePos, long point, double *x, double *y, double *z);
extern int e009_res_node(long ePos, long nPos, long type, double *val);
extern int e009_volume(long ePos, double *vol);

extern int e005_shape_mat(long ePos, double mult_val, tMatrix *M_e, int all, long pos_winkler);

long e010_rvals(long ePos)
{
	return(26*27);
}


int e010_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
  int     rv = AF_OK ;
  double  winkler ;

  rv = e009_stiff(ePos, Mode, K_e, F_e, Fr_e);
  
  /* Winkler's foundation (only 1-2-3-4 side and it MUST BE in x-y plane!) */
  if (Mode != AF_YES) 
  {
    winkler = femGetRSValPos(ePos, RS_WINKLER, 0) ;

    if (winkler > 0.0)
    {
      e005_shape_mat(ePos, winkler, K_e, AF_NO,3) ;
    }
  }

  return(rv);
}

int addElem_010(void)
{
	int rv = AF_OK;
	static long type    = 10 ;
	static long nodes   = 8 ;
	static long dofs    = 3 ;
	static long ndof[3] = {U_X,U_Y,U_Z} ;
	static long rs      = 1 ;
	static long real[1]  = {RS_WINKLER} ;
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

	Elem[type].stiff = e010_stiff;
	Elem[type].mass  = e009_mass;
	Elem[type].rvals = e010_rvals;
	Elem[type].eload = e009_eload;
	Elem[type].res_p_loc = e008_res_p_loc;
	Elem[type].res_node = e009_res_node;
	Elem[type].volume = e009_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

/* end of fem_e009.c */

/*
   File name: fem_e015.c
   Date:      2006/10/30 19:07
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

	 FEM Solver - element 015 constant strain/ plane element
	              linear solution ONLY
                Cosserat continuum
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_
extern int D_PlaneHookIso(long ePos, long iPoint, double A, tVector *epsilon, long newM, long Problem, tMatrix *D);
extern int e011_res_p_loc(long ePos, long point, double *x, double *y, double *z);
extern int e011_volume(long ePos, double *vol);

double e015_get_L(double x1, double y1, double x2, double y2)
{
  return ( sqrt( pow((y2*y2-y1*y1), 2) + pow((x2*x2-x1*x1), 2)));
}

int e015_stiff(long ePos, long Mode, tMatrix *K_e, tVector *Fe, tVector *Fre)
{
	int     rv = AF_OK;
  double  x[4] ;
  double  y[4] ;
  double l1, l2, l3 ;
  double l12, l23, l31 ;
  tMatrix D ;
  tMatrix DD ;
  long    i ;

	femMatNull(&D);
	femMatNull(&DD);

	if ((rv = femFullMatInit(&D, 4,4)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&DD, 2,2)) != AF_OK) {goto memFree;}

	for (i=1; i<= 3; i++)
	{
		x[i] = femGetNCoordPosX(femGetENodePos(ePos, i-1));
		y[i] = femGetNCoordPosY(femGetENodePos(ePos, i-1));
	}

  l1 = e015_get_L(x[2],y[2], x[3],y[3]) ;
  l2 = e015_get_L(x[1],y[1], x[3],y[3]) ;
  l3 = e015_get_L(x[1],y[1], x[2],y[2]) ;

  l12 = l1*l1 + l2*l2 - l3*l3 ;
  l23 = l2*l2 + l3*l3 - l1*l1 ;
  l31 = l3*l3 + l1*l1 - l2*l2 ;


memFree:
  femMatFree(&D);
  femMatFree(&DD);
	return(rv);
}

/** It should work - according to Zienkiewicz ;-)
 * Actually, it's for basic triangle, not for Cosserat one
 * */
int e015_mass(long ePos, tMatrix *M_e)
{
	int    rv = AF_OK ;
	long   i;
	double t,ro,A, mult;
	double  x[4] ;
	double  y[4] ;

	femMatSetZero(M_e);

	/* Geometry (width, area) and material (density) data: */
	t  = femGetRSValPos(ePos, RS_HEIGHT, 0) ;
	ro = femGetMPValPos(ePos, MAT_DENS, 0) ;

	for (i=1; i<= 3; i++)
	{
		x[i] = femGetNCoordPosX(femGetENodePos(ePos, i-1));
		y[i] = femGetNCoordPosY(femGetENodePos(ePos, i-1));
	}

	A=(0.5*(x[1]*y[2]- x[2]*y[1] + x[2]*y[3]- x[3]*y[2] + x[3]*y[1]- x[1]*y[3]));

	/* Multiplier: */
	mult = (ro * t * A) / 3 ;

	femMatPut(M_e,1,1 , mult * 0.5  );
	femMatPut(M_e,1,4 , mult * 0.25  );
	femMatPut(M_e,1,7 , mult * 0.25  );

	femMatPut(M_e,2,2 , mult * 0.5  );
	femMatPut(M_e,2,5 , mult * 0.25  );
	femMatPut(M_e,2,8 , mult * 0.25  );

	femMatPut(M_e,4,1 , mult * 0.25  );
	femMatPut(M_e,4,4 , mult * 0.5  );
	femMatPut(M_e,4,7 , mult * 0.25  );

	femMatPut(M_e,5,2 , mult * 0.25  );
	femMatPut(M_e,5,5 , mult * 0.5  );
	femMatPut(M_e,5,8 , mult * 0.25  );

	femMatPut(M_e,7,1 , mult * 0.25  );
	femMatPut(M_e,7,4 , mult * 0.25  );
	femMatPut(M_e,7,7 , mult * 0.5  );

	femMatPut(M_e,8,2 , mult * 0.25  );
	femMatPut(M_e,8,5 , mult * 0.25  );
	femMatPut(M_e,8,8 , mult * 0.5  );

	return(rv);
}


long e015_rvals(long ePos)
{
	return(6);
}

int e015_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

int addElem_015(void)
{
	int rv = AF_OK;
	static long type     = 15 ;
	static long nodes    = 3 ;
	static long dofs     = 3 ;
	static long ndof[3]  = {U_X,U_Y,ROT_Z} ;
	static long rs       = 1 ;
	static long real[1]  = {RS_HEIGHT} ;
	static long rs_rp    = 0 ;
	static long *real_rp = NULL ;
	static long res      = 6 ;
	static long nres[6]  = { RES_SX, RES_SY, RES_SXY, RES_EX, RES_EY, RES_EXY} ;
	static long res_rp   = 0 ;
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

	Elem[type].stiff = e015_stiff;
	Elem[type].mass  = e015_mass;
	Elem[type].rvals = e015_rvals;
	Elem[type].eload = e015_eload;
	Elem[type].res_p_loc = e011_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e011_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

#endif
/* end of fem_e015.c */

/*
   File name: fem_e018.c
   Date:      2011/06/07 18:21
   Author:    Jiri Brozovsky

   Copyright (C) 2011 Jiri Brozovsky

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

	 FEM Solver - element 018 temperature plane element
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_

extern double e011_area(long ePos);
extern int e011_mass(long ePos, tMatrix *M_e);
extern int e011_volume(long ePos, double *vol);
extern int e011_res_p_loc(long ePos, long point, double *x, double *y, double *z);

int e018_stiff(long ePos, long Mode, tMatrix *K_e, tVector *Fe, tVector *Fre)
{
	int     rv = AF_OK;
	double  A, thick, mult, kxx ;
	double  x[4] ;
	double  y[4] ;
	int     i ;
	tMatrix B ;      /*[3][2]*/
	tMatrix Bt ;     /*[2][3]*/
	tMatrix S ;     /*[3][3]*/
	tMatrix St;     /*[3][3]*/
	tMatrix D ;      /*[2][2]*/
	tMatrix BtD ;    /*[6][3]*/
	tMatrix StBt ;    /*[3][2]*/
	tMatrix BS ;    /*[2][3]*/
	tVector u_e;     /*[3]*/

	femVecNull(&u_e);

	femMatNull(&B);
	femMatNull(&Bt);
	femMatNull(&S);
	femMatNull(&St);
	femMatNull(&D);

	femMatNull(&StBt);
	femMatNull(&BS);
	femMatNull(&BtD);

	if ((rv = femVecFullInit(&u_e, 3)) != AF_OK) {goto memFree;}

	if ((rv = femFullMatInit(&B, 2,3)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&Bt, 3,2)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&S, 3,3)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&St, 3,3)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&D, 2,2)) != AF_OK) {goto memFree;}

	if ((rv = femFullMatInit(&StBt, 3,2)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&BS, 2,3)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&BtD, 3,2)) != AF_OK) {goto memFree;}


	for (i=1; i<= 3; i++)
	{
		x[i] = femGetNCoordPosX(femGetENodePos(ePos, i-1));
		y[i] = femGetNCoordPosY(femGetENodePos(ePos, i-1));
	}

	A = e011_area(ePos);
	thick = femGetRSValPos(ePos, RS_HEIGHT, 0) ;

  femMatPut(&B, 1,1, 1.0) ;
  femMatPut(&B, 2,2, 1.0) ;
	femMatTran(&B, &Bt) ;

  femMatPut(&S, 1, 1,(y[2]-y[3]) );
  femMatPut(&S, 1, 2,(y[3]-y[1]) );
  femMatPut(&S, 1, 3,(y[1]-y[2]) );

  femMatPut(&S, 2,1, (x[3]-x[2]) );
  femMatPut(&S, 2,2, (x[1]-x[3]) );
  femMatPut(&S, 2,3, (x[2]-x[1]) );

  femMatPut(&S, 3,1, (x[2]*y[3]-x[3]*y[2]) );
  femMatPut(&S, 3,2, (x[3]*y[1]-x[1]*y[3]) );
  femMatPut(&S, 3,3, (x[1]*y[2]-x[2]*y[1]) );

  mult = 1.0 / (
      x[1]*y[2] + x[3]*y[1] + x[2]*y[3] - x[3]*y[2] - x[2]*y[1] - x[1]*y[3]
      ) ;

  femValMatMultSelf(mult, &S);

	femMatTran(&S, &St) ;

  kxx = femGetMPValPos(ePos, MAT_KXX, 0) ;
  femMatPut(&D, 1,1, kxx );
  femMatPut(&D, 2,2, kxx );

	/* stiffness matrix is computed */
	femMatMatMult(&St,&Bt, &StBt) ;
	femMatMatMult(&StBt, &D, &BtD) ;

	femMatMatMult(&B,&S, &BS) ;
	femMatMatMult(&BtD,  &BS, K_e) ;

  femValMatMultSelf(A*thick, K_e);

	/* Control prints: */
	femMatPrn(K_e,"Ke local:");


	if (Mode == AF_YES) /* results are computed */
	{
  	femLocUtoU_e(&u, ePos, &u_e);

	  femVecPrn(&u,"u global:");
	  femVecPrn(&u_e,"u local:");

    mult =  (femVecGet(&u_e, 1) + femVecGet(&u_e, 2) + femVecGet(&u_e, 3)) / 3.0 ;

		if (femTangentMatrix == AF_YES)
	     { femAddEResVal(ePos, RES_TEMP,  0, mult); }
	  else
	     { femPutEResVal(ePos, RES_TEMP,  0, mult); }
		
		femMatVecMult(K_e, &u_e, Fe) ;
		femVecSetZero(Fre);
	}
	
memFree:
	femVecFree(&u_e);

	femMatFree(&B);
	femMatFree(&Bt);
	femMatFree(&S);
	femMatFree(&St);
	femMatFree(&D);

	femMatFree(&StBt);
	femMatFree(&BS);
	femMatFree(&BtD);
	return(rv);
}

long e018_rvals(long ePos) { return(1); }

int e018_eload(long ePos, long mode, tVector *F_e) { return(AF_OK); }

int addElem_018(void)
{
	int rv = AF_OK;
	static long type     = 18 ;
	static long dim      = 0 ;
	static long nodes    = 3 ;
	static long dofs     = 1 ;
	static long ndof[1]  = {TEMP} ;
	static long rs       = 1 ;
	static long real[1]  = {RS_HEIGHT} ;
	static long rs_rp    = 0 ;
	static long *real_rp = NULL ;
	static long res      = 1 ;
	static long nres[1]  = { RES_TEMP } ;
	static long res_rp   = 0 ;
	static long *nres_rp = NULL ;

	if (type != femAddElem(type)) {return(AF_ERR_VAL);}
	Elem[type].dim = dim ;
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

	Elem[type].stiff = e018_stiff;
	Elem[type].mass  = e011_mass;
	Elem[type].rvals = e018_rvals;
	Elem[type].eload = e018_eload;
	Elem[type].res_p_loc = e011_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e011_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

#endif
/* end of fem_e018.c */

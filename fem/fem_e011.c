/*
   File name: fem_e011.c
   Date:      2004/10/22 23:40
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

	 FEM Solver - element 011 constant strain/ plane element
	              linear solution ONLY
	 $Id: fem_e011.c,v 1.2 2005/07/11 17:56:16 jirka Exp $
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_
extern int D_PlaneHookIso(long ePos, long iPoint, double A, tVector *epsilon, long newM, long Problem, tMatrix *D);
extern int femGetPrincStress2D(tVector *sigma_x, tVector *sigma_1, double *phi);
extern double femCEB_FIP_Test( double  s_01, double  s_02, double  f_uc, double  f_ut); /* from fem_ccr2.c */

int e01_compute_B(double *x, double *y, double A, tMatrix *B)
{
	femMatSetZero(B) ;

	femMatPut(B, 1,1, y[2]-y[3] );
	femMatPut(B, 1,3, y[3]-y[1] );
	femMatPut(B, 1,5, y[1]-y[2] );

	femMatPut(B, 2,2, x[3]-x[2] );
	femMatPut(B, 2,4, x[1]-x[3] );
	femMatPut(B, 2,6, x[2]-x[1] );

	femMatPut(B, 3,1, x[3]-x[2] );
	femMatPut(B, 3,2, y[2]-y[3] );
	femMatPut(B, 3,3, x[1]-x[3] );
	femMatPut(B, 3,4, y[3]-y[1] );
	femMatPut(B, 3,5, x[2]-x[1] );
	femMatPut(B, 3,6, y[1]-y[2] );

	femValMatMultSelf(1.0 / (2.0 * A), B);

	return(AF_OK);
}

int e011_stiff(long ePos, long Mode, tMatrix *K_e, tVector *Fe, tVector *Fre)
{
	int     rv = AF_OK;
	long		ProblemType = 0 ; /* plane stress ;-) */
	double  thick ;
	double  A ;
	double  x[4] ;
	double  y[4] ;
	int     i ;
	tMatrix B ;      /*[3][6]*/
	tMatrix Bt ;     /*[6][3]*/
	tMatrix D ;      /*[3][3]*/
	tMatrix BtD ;    /*[6][3]*/
	tVector u_e;     /*[6]*/
	tVector epsilon; /*[6]*/
	tVector sigma;   /*[6]*/
	tVector sigma_1;   /*[6]*/
	double  phi ;

	/* sets problem type for "D" computation: */
	switch (eType[ePos])
	{
		case 11 : ProblemType = 0 ; break; /* plane stress */
		default : ProblemType = 0 ; break; /* plane stress */
	}

	femVecNull(&u_e);
	femVecNull(&epsilon);
	femVecNull(&sigma);

	femMatNull(&B);
	femMatNull(&Bt);
	femMatNull(&BtD);
	femMatNull(&D);

	if ((rv = femVecFullInit(&u_e, 6)) != AF_OK) {goto memFree;}
	if ((rv = femVecFullInit(&epsilon, 3)) != AF_OK) {goto memFree;}
	if ((rv = femVecFullInit(&sigma, 3)) != AF_OK) {goto memFree;}

	if ((rv = femFullMatInit(&B, 3,6)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&Bt, 6,3)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&BtD, 6,3)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&D, 3,3)) != AF_OK) {goto memFree;}


	thick = femGetRSValPos(ePos, RS_HEIGHT, 0) ;

	if (thick <= 0.0) {ProblemType = 1 ;} /* plane strain */

	for (i=1; i<= 3; i++)
	{
		x[i] = femGetNCoordPosX(femGetENodePos(ePos, i-1));
		y[i] = femGetNCoordPosY(femGetENodePos(ePos, i-1));
	}

	A = (0.5*(x[1]*y[2]- x[2]*y[1] + x[2]*y[3]- x[3]*y[2] + x[3]*y[1]- x[1]*y[3]));

	e01_compute_B(x, y, A, &B);

	femMatTran(&B, &Bt) ;

	if ((rv=D_PlaneHookIso(ePos, 0, A, &epsilon, AF_NO, ProblemType, &D)) != AF_OK) 
		 { goto memFree; }

	/* stiffness matrix is computed */
	femMatMatMult(&Bt,&D, &BtD) ;
	femMatMatMult(&BtD, &B, K_e) ;
	femValMatMultSelf(thick * A, K_e) ;

	if (Mode == AF_YES) /* results are computed */
	{
  	femLocUtoU_e(&u, ePos, &u_e);
		femMatVecMult(&B, &u_e, &epsilon) ;
		femMatVecMult(&D, &epsilon, &sigma) ;

		if (femTangentMatrix == AF_YES)
	  {
	    femAddEResVal(ePos, RES_SX,  0, femVecGet(&sigma, 1));
	    femAddEResVal(ePos, RES_SY,  0, femVecGet(&sigma, 2));
	    femAddEResVal(ePos, RES_SXY, 0, femVecGet(&sigma, 3));

	    femAddEResVal(ePos, RES_EX,  0, femVecGet(&epsilon, 1));
	    femAddEResVal(ePos, RES_EY,  0, femVecGet(&epsilon, 2));
	    femAddEResVal(ePos, RES_EXY, 0, femVecGet(&epsilon, 3));
	  }
	  else
	  {
	    femPutEResVal(ePos, RES_SX,  0, femVecGet(&sigma, 1));
	    femPutEResVal(ePos, RES_SY,  0, femVecGet(&sigma, 2));
	    femPutEResVal(ePos, RES_SXY, 0, femVecGet(&sigma, 3));

	    femPutEResVal(ePos, RES_EX,  0, femVecGet(&epsilon, 1));
	    femPutEResVal(ePos, RES_EY,  0, femVecGet(&epsilon, 2));
	    femPutEResVal(ePos, RES_EXY, 0, femVecGet(&epsilon, 3));
	  }

		/* for material 2 compute CEB-FIP */
		if (Mat[femGetMPTypePos(femGetEMPPos(ePos))].type == 2)
		{
			femVecNull(&sigma_1);
			if ((rv = femVecFullInit(&sigma_1, 3)) == AF_OK) 
			{
			  femGetPrincStress2D(&sigma, &sigma_1, &phi);

				femPutEResVal(ePos, RES_STAT1,  0, 
				femCEB_FIP_Test(
		                 femVecGet(&sigma_1,1),
										 femVecGet(&sigma_1,2),
										 femGetMPValPos(ePos, MAT_SCMPR, 0),
										 femGetMPValPos(ePos, MAT_STEN, 0)
		               ));

			  femVecFree(&sigma_1);
			}
		}

		femMatVecMult(K_e, &u_e, Fe) ;
		femVecSetZero(Fre);
	}
	
memFree:
	femVecFree(&u_e);
	femVecFree(&epsilon);
	femVecFree(&sigma);

	femMatFree(&B);
	femMatFree(&Bt);
	femMatFree(&BtD);
	femMatFree(&D);
	return(rv);
}

/** Computes temperature load for element 011 */
int e011_therm(long ePos, long *use, tVector *Te, tVector *Fe)
{
	int     rv = AF_OK;
	long		ProblemType = 0 ; /*  0 = plane stress  */
	double  thick, temp ;
  double  alpha = 0.0 ;     /* thermal expansion coeff. */
	double  A ;
	double  x[4] ;
	double  y[4] ;
	int     i ;
	tMatrix B ;      /*[3][6]*/
	tMatrix Bt ;     /*[6][3]*/
	tMatrix D ;      /*[3][3]*/
	tMatrix BtD ;    /*[6][3]*/
	tVector epsilon; /*[6]*/

  *use = AF_YES;

	/* sets problem type for "D" computation: */
	switch (eType[ePos])
	{
		case 11 : ProblemType = 0 ; break; /* plane stress */
		default : ProblemType = 0 ; break; /* plane stress */
	}

	femVecNull(&epsilon);

	femMatNull(&B);
	femMatNull(&Bt);
	femMatNull(&BtD);
	femMatNull(&D);

	if ((rv = femVecFullInit(&epsilon, 3)) != AF_OK) {goto memFree;}

	if ((rv = femFullMatInit(&B, 3,6)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&Bt, 6,3)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&BtD, 6,3)) != AF_OK) {goto memFree;}
	if ((rv = femFullMatInit(&D, 3,3)) != AF_OK) {goto memFree;}

	thick = femGetRSValPos(ePos, RS_HEIGHT, 0) ;
  alpha = femGetMPValPos(ePos, MAT_ALPHA, 0) ;

	if (thick <= 0.0) { ProblemType = 1 ; } /* plane strain */ 

	for (i=1; i<= 3; i++)
	{
		x[i] = femGetNCoordPosX(femGetENodePos(ePos, i-1));
		y[i] = femGetNCoordPosY(femGetENodePos(ePos, i-1));
	}

	A = (0.5*(x[1]*y[2]- x[2]*y[1] + x[2]*y[3]- x[3]*y[2] + x[3]*y[1]- x[1]*y[3]));


	e01_compute_B(x, y, A, &B);

	femMatTran(&B, &Bt) ;

	if ((rv=D_PlaneHookIso(ePos, 0, A, &epsilon, AF_NO, ProblemType, &D)) != AF_OK) 
		 { goto memFree; }

  /* set temperature strains: */
  if (thick <= 0.0) {thick = 1.0;}
  temp = (femVecGet(Te, 1) + femVecGet(Te, 2) + femVecGet(Te, 3)) / 3.0 ;
  femVecPut(&epsilon, 1, alpha * temp );
  femVecPut(&epsilon, 2, alpha * temp );
  femVecPut(&epsilon, 3, 0.0 );

	/* stiffness matrix is computed */
	femMatMatMult(&Bt,&D, &BtD) ;
	femMatVecMult(&BtD, &epsilon, Fe) ;
	femValVecMultSelf(thick * A, Fe) ; 
	
memFree:
	femVecFree(&epsilon);

	femMatFree(&B);
	femMatFree(&Bt);
	femMatFree(&BtD);
	femMatFree(&D);
	return(rv);
}

/** It should work - according to Zienkiewicz ;-) */
double e011_area(long ePos)
{
	double  x[4] ;
	double  y[4] ;
	long   i;

	for (i=1; i<= 3; i++)
	{
		x[i] = femGetNCoordPosX(femGetENodePos(ePos, i-1));
		y[i] = femGetNCoordPosY(femGetENodePos(ePos, i-1));
	}

	return(0.5*(x[1]*y[2]- x[2]*y[1] + x[2]*y[3]- x[3]*y[2] + x[3]*y[1]- x[1]*y[3]));
}

/** It should work - according to Zienkiewicz ;-) */
int e011_mass(long ePos, tMatrix *M_e)
{
	int    rv = AF_OK ;
	double t,ro,A, mult;

	femMatSetZero(M_e);

	/* Geometry (width, area) and material (density) data: */
	t  = femGetRSValPos(ePos, RS_HEIGHT, 0) ;
	ro = femGetMPValPos(ePos, MAT_DENS, 0) ;

	A=e011_area(ePos);

	/* Multiplier: */
	mult = (ro * t * A) / 3 ;

	femMatPut(M_e,1,1 , mult * 0.5  );
	femMatPut(M_e,1,3 , mult * 0.25  );
	femMatPut(M_e,1,5 , mult * 0.25  );

	femMatPut(M_e,2,2 , mult * 0.5  );
	femMatPut(M_e,2,4 , mult * 0.25  );
	femMatPut(M_e,2,6 , mult * 0.25  );

	femMatPut(M_e,3,1 , mult * 0.25  );
	femMatPut(M_e,3,3 , mult * 0.5  );
	femMatPut(M_e,3,5 , mult * 0.25  );

	femMatPut(M_e,4,2 , mult * 0.25  );
	femMatPut(M_e,4,4 , mult * 0.5  );
	femMatPut(M_e,4,6 , mult * 0.25  );

	femMatPut(M_e,5,1 , mult * 0.25  );
	femMatPut(M_e,5,3 , mult * 0.25  );
	femMatPut(M_e,5,5 , mult * 0.5  );

	femMatPut(M_e,6,2 , mult * 0.25  );
	femMatPut(M_e,6,4 , mult * 0.25  );
	femMatPut(M_e,6,6 , mult * 0.5  );

	return(rv);
}

int e011_volume(long ePos, double *vol)
{
	int    rv = AF_OK ;
	double t,A;

	t  = femGetRSValPos(ePos, RS_HEIGHT, 0) ;

	A=e011_area(ePos);
	*vol = (t * A)  ;

	return(rv);
}

long e011_rvals(long ePos)
{
	return(7);
}

int e011_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}


int e011_res_p_loc(long ePos, long point, double *x, double *y, double *z)
{
	long i ;

	*x = 0 ;
	*y = 0 ;
	*z = 0 ;

	for (i=3; i<4; i++)
	{
  	*x += femGetNCoordPosX(femGetENodePos(ePos,i));
  	*y += femGetNCoordPosY(femGetENodePos(ePos,i));
	}

	*x = *x / 3.0 ;
	*y = *y / 3.0 ;

	return(AF_OK);
}

int addElem_011(void)
{
	int rv = AF_OK;
	static long type     = 11 ;
	static long dim      = 2 ;
	static long nodes    = 3 ;
	static long dofs     = 2 ;
	static long ndof[2]  = {U_X,U_Y} ;
	static long rs       = 1 ;
	static long real[1]  = {RS_HEIGHT} ;
	static long rs_rp    = 0 ;
	static long *real_rp = NULL ;
	static long res      = 7 ;
	static long nres[7]  = { RES_SX, RES_SY, RES_SXY, RES_EX, RES_EY, RES_EXY, RES_STAT1} ;
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

	Elem[type].stiff = e011_stiff;
	Elem[type].mass  = e011_mass;
	Elem[type].rvals = e011_rvals;
	Elem[type].eload = e011_eload;
	Elem[type].res_p_loc = e011_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e011_volume;
	Elem[type].therm = e011_therm;
	return(rv);
}

#endif
/* end of fem_e011.c */

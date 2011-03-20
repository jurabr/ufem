/*
   File name: fem_e017.c
   Date:      2017/05/18 20:15
   Author:    Jiri Brozovsky

   Copyright (C) 2017  Jiri Brozovsky

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

	 FEM Solver - Element 017 (2D grille)

   Note that no element load is used (nodal forces/moments only)

	 $Id: fem_e017.c,v 1.11 2005/07/11 17:56:16 jirka Exp $
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_
extern tVector u;

extern int e003_volume(long ePos, double *vol);
extern double fem_beam2d_ei_val(long epos, double M, double N) ;

int e017_local_stiff_matrix(
		long ePos, 
		long Mode, 
		double Lx, 
		double Ex, 
		double Gx, 
		double Ax, 
		double Ix, 
		double Itx, 
		tMatrix *k_0)
{
  int rv = AF_OK ;
  int i,j ;

  femMatSetZero(k_0);

	/* filling of stiffness matrix: */
	femMatPut(k_0, 1, 1,  (12.0*Ex*Ix)/pow(Lx,3) );
	femMatPut(k_0, 1, 3,  (-6.0*Ex*Ix)/pow(Lx,2) );
	femMatPut(k_0, 1, 4,  (-12.0*Ex*Ix)/pow(Lx,3) );
	femMatPut(k_0, 1, 6,  (-6.0*Ex*Ix)/pow(Lx,2) );

	femMatPut(k_0, 2, 2,  (Gx*Itx)/Lx );
	femMatPut(k_0, 2, 5,  (-Gx*Itx)/Lx );

	femMatPut(k_0, 3, 3,  (4.0*Ex*Ix)/Lx );
	femMatPut(k_0, 3, 4,  (6.0*Ex*Ix)/pow(Lx,2) );
	femMatPut(k_0, 3, 6,  (2.0*Ex*Ix)/Lx );

	femMatPut(k_0, 4, 4,  (12.0*Ex*Ix)/pow(Lx,3) );
	femMatPut(k_0, 4, 6,  (6.0*Ex*Ix)/pow(Lx,2) );

	femMatPut(k_0, 5, 5,  (Gx*Itx)/Lx );

	femMatPut(k_0, 6, 6,  (4.0*Ex*Ix)/Lx );

  for (i=2; i<=6; i++)
  {
    for (j=1; j<=i; j++)
    {
      femMatPut( k_0, i, j, femMatGet(k_0, j, i) ) ;
    }
  }

	femMatPrn(k_0,"K_0");
  return(rv);
}

int e017_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
	int     rv = AF_OK ;
	double  x1,y1,x2,y2;
	double  Ex = 0;  /* Young's modulus */
	double  Gx = 0;  /* shear modulus*/
	double  Ax = 0 ; /* area */
	double  Ix = 0 ; /* moment of inertia */
	double  Itx = 0 ; /* moment in torsion */
	double  Lx = 0 ; /* lenght of beam */
  double  Wink = 0 ; /* Winkler constant */
  double  width = 0 ; /* width for winkler */
	tMatrix k_0;
	tMatrix k_g;
	tMatrix k_1;
	tMatrix Tk_0;
	tMatrix T;
	tMatrix T_T;
	tVector F_0 ;
	tVector u_e ;
	double  cos_a, sin_a, nu ;
#if 0
  int     i ;
  double  f0_Norm, f1_Norm;
#endif

	femMatNull(&k_0);
	femMatNull(&Tk_0);
	femMatNull(&T);
	femMatNull(&T_T);

	femMatNull(&k_g);
	femMatNull(&k_1);

	if ((rv=femFullMatInit(&k_0,6,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&Tk_0,6,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&T,6,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&T_T,6,6)) != AF_OK) { goto memFree; }

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));

  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));

	if ( (Lx = sqrt( pow((y2 - y1), 2) + pow((x2 - x1), 2) )) <= 0 )
	{
#ifdef RUN_VERBOSE
		fprintf(msgout, "[E] %s!\n", _("Zero element lenght"));
#endif
		return(AF_ERR_VAL);
	}

	cos_a = (x2 - x1) / Lx ;
	sin_a = (y2 - y1) / Lx ;

	/* geometric properties: */
	Ax  = femGetRSValPos(ePos, RS_AREA,      0) ;
	Ix  = femGetRSValPos(ePos, RS_INERTIA_Y, 0) ;
	Itx = femGetRSValPos(ePos, RS_INERTIA_X,   0) ;

	Wink = femGetRSValPos(ePos, RS_WINKLER,   0) ;
	width = femGetRSValPos(ePos, RS_WIDTH,   0) ;

	/* material properties: */
	Ex = femGetMPValPos(ePos, MAT_EX, 0) ;
	nu = femGetMPValPos(ePos, MAT_NU, 0) ;

	Gx = Ex / ( 2.0*(1.0 + nu )) ;

	e017_local_stiff_matrix( ePos, Mode, Lx, Ex, Gx, Ax, Ix, Itx, &k_0);

  if ((Wink*width) > 0.0)
  {
	  femMatPut(&k_0, 1, 1,  0.5*Wink*width*Lx ) ;
	  femMatPut(&k_0, 4, 4,  0.5*Wink*width*Lx ) ;
  }


	/* transformation matrix */
	femMatSetZero(&T) ;
	femMatSetZero(&T_T) ;

	femMatPut(&T, 1, 1,  cos_a ) ;
	femMatPut(&T, 1, 2,  sin_a ) ;

	femMatPut(&T, 2, 1, -sin_a ) ;
	femMatPut(&T, 2, 2,  cos_a ) ;

	femMatPut(&T, 3, 3,  1.0 ) ;

	femMatPut(&T, 4, 4,  cos_a ) ;
	femMatPut(&T, 4, 5,  sin_a ) ;

	femMatPut(&T, 5, 4, -sin_a ) ;
	femMatPut(&T, 5, 5,  cos_a ) ;

	femMatPut(&T, 6, 6,  1.0 ) ;


	/* transposition of "T" */
	femMatTran(&T, &T_T) ;

	/* global stiffness matrix: */
	femMatMatMult(&T_T, &k_0, &Tk_0);

	/* finishing of global stiffness matrix: */
	femMatMatMult(&Tk_0, &T, K_e);

#ifdef DEVEL_VERBOSE
	femMatPrn(K_e, "K_e");
#endif

	/* ====================================================================== */
	if (Mode == AF_YES) /* result, F_e, Fr_e */
	{

		femVecNull(&F_0);
		femVecFullInit(&F_0,6);
		femVecNull(&u_e);
		femVecFullInit(&u_e,6);

    femLocUtoU_e(&u, ePos, &u_e);

		femVecPrn(&u_e, "U_e");

		/* nodal forces: */
		femMatVecMult(K_e, &u_e, F_e) ; /* global */

    /* to fix problem with moment signs: */
#if 0
    femVecPut(F_e, 3, -1.0*femVecGet(F_e,3)) ;
    femVecPut(F_e, 6, -1.0*femVecGet(F_e,6)) ;
#endif

		femMatVecMult(&T, F_e, &F_0) ; /* local */

		femVecSetZero(Fr_e); /* residual forces ;-) */

		femVecPrn(F_e, "F_e");
		femVecPrn(&F_0, "F_0");

	  if (femTangentMatrix == AF_YES)
	  {
	    femAddEResVal(ePos, RES_FZ, 0, femVecGet(&F_0,1));
	    femAddEResVal(ePos, RES_MX, 0, femVecGet(&F_0,2));
	    femAddEResVal(ePos, RES_MY, 0, femVecGet(&F_0,3));

	    femAddEResVal(ePos, RES_FZ, 1, femVecGet(&F_0,4));
	    femAddEResVal(ePos, RES_MX, 1, femVecGet(&F_0,5));
	    femAddEResVal(ePos, RES_MY, 1, femVecGet(&F_0,6));
		}
		else
		{
	    femPutEResVal(ePos, RES_FZ, 0, femVecGet(&F_0,1));
	    femPutEResVal(ePos, RES_MX, 0, femVecGet(&F_0,2));
	    femPutEResVal(ePos, RES_MY, 0, femVecGet(&F_0,3));

	    femAddEResVal(ePos, RES_FZ, 1, femVecGet(&F_0,4));
	    femAddEResVal(ePos, RES_MX, 1, femVecGet(&F_0,5));
	    femAddEResVal(ePos, RES_MY, 1, femVecGet(&F_0,6));
		}

		/*printf("RR %f %f \n",
	    femGetEResVal(ePos, RES_MZ, 0),
	    femGetEResVal(ePos, RES_MZ, 1)
		);*/

		femVecFree(&u_e);
		femVecFree(&F_0);
  }
	
memFree:
	femMatFree(&k_0);
	femMatFree(&Tk_0);
	femMatFree(&T);
	femMatFree(&T_T);
  if (fem2ndOrder == AF_YES)
  {
	  femMatFree(&k_g);
	  femMatFree(&k_1);
  }
	return(AF_OK);
}

int e017_mass(long ePos, tMatrix *M_e)
{
	int    rv = AF_OK ;

	double x1,y1,x2,y2;
	double Lx  = 0 ;
	double Ax  = 0 ;
	double dens = 0 ;
	double mass = 0 ;
	int    i ;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));

  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));

	if ( (Lx = sqrt( pow((y2 - y1), 2) + pow((x2 - x1), 2) )) <= 0 )
	{
#ifdef RUN_VERBOSE
		fprintf(msgout, "[E] %s!\n", _("Zero element lenght"));
#endif
		return(AF_ERR_VAL);
	}

	Ax = femGetRSValPos(ePos, RS_AREA, 0) ;
	dens = femGetMPValPos(ePos, MAT_DENS, 0) ;

	mass = dens * Ax * Lx ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"mass = %e \n",mass);
#endif

	for (i=1; i<3; i++)
	{
		femMatPut(M_e,i,  i,   ( (mass)/(2.0) ) ) ;
		femMatPut(M_e,i+3,i+3, ( (mass)/(2.0) ) ) ;
	}

#if 1
	femMatPut(M_e,3,  3,   ( (mass*Lx*Lx)/(24.0) ) ) ;
	femMatPut(M_e,6,  6,   ( (mass*Lx*Lx)/(24.0) ) ) ;
#endif

	return(rv);
}

long e017_rvals(long ePos)
{
	return(6);
}

int e017_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

int e017_res_p_loc(long ePos, long point, double *x, double *y, double *z)
{
	*x = 0.0 ;
	*y = 0.0 ;
	*z = 0.0 ;

	switch(point)
	{
		case 0:
  			*x = femGetNCoordPosX(femGetENodePos(ePos,0));
  			*y = femGetNCoordPosY(femGetENodePos(ePos,0));
				break;
		case 1:
  			*x = femGetNCoordPosX(femGetENodePos(ePos,1));
  			*y = femGetNCoordPosY(femGetENodePos(ePos,1));
				break;
	}
	return(AF_OK);
}

int addElem_017(void)
{
	int rv = AF_OK;
	static long type    = 17 ;
	static long nodes   = 2 ;
	static long dofs    = 3 ;
	static long ndof[3] = {U_Z,ROT_X,ROT_Y} ;
	static long rs      = 5 ;
	static long real[5] = {RS_AREA, RS_INERTIA_X, RS_INERTIA_Y, RS_WINKLER, RS_WIDTH} ;
	static long rs_rp      = 0 ;
	static long *real_rp   = NULL ;
	static long res      = 3 ;
	static long nres[3]    = {RES_FZ,RES_MX,RES_MY};
	static long res_rp   = 3 ;
	static long nres_rp[3] = {RES_FZ,RES_MX,RES_MY};

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

	Elem[type].stiff = e017_stiff;
	Elem[type].mass  = e017_mass;
	Elem[type].rvals = e017_rvals;
	Elem[type].eload = e017_eload;
	Elem[type].res_p_loc = e017_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e003_volume;
	return(rv);
}

#endif
/* end of fem_e017.c */

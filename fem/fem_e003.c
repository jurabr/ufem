/*
   File name: fem_e003.c
   Date:      2003/05/18 20:15
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

	 FEM Solver - Element 003 (2D beam)

   Note that no element load is used (nodal forces/moments only)

	 $Id: fem_e003.c,v 1.11 2005/07/11 17:56:16 jirka Exp $
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_
extern tVector u;

extern double fem_beam2d_ei_val(long epos, double M, double N) ;

int e003_geom_matrix(long ePos, long Mode, double L, tMatrix *K_s)
{
	int rv = AF_OK ;

	femMatSetZero(K_s);

#if 0
	femMatPut(K_s, 2, 2, 6.0/(5.0*L) ) ;
	femMatPut(K_s, 2, 3, 0.1 ) ;
	femMatPut(K_s, 2, 5, -6.0/(5.0*L) ) ;
	femMatPut(K_s, 2, 6, 0.1 ) ;
	femMatPut(K_s, 3, 2, 0.1 ) ;
	femMatPut(K_s, 3, 3, (2.0*L)/(15.0) ) ;
	femMatPut(K_s, 3, 5, -0.1 ) ;
	femMatPut(K_s, 3, 6, -(L*1.0)/(30.0) ) ;

	femMatPut(K_s, 5, 2, -6.0/(5.0*L) ) ;
	femMatPut(K_s, 5, 3, -0.1 ) ;
	femMatPut(K_s, 5, 5, 6.0/(5.0*L) ) ;
	femMatPut(K_s, 5, 6, -0.1 ) ;
	femMatPut(K_s, 6, 2, 0.1 ) ;
	femMatPut(K_s, 6, 3, -(L*1.0)/(30.0) ) ;
	femMatPut(K_s, 6, 5, -0.1 ) ;
	femMatPut(K_s, 6, 6, (2.0*L)/(15.0) ) ;
#else /* According to Cook: */
	femMatPut(K_s, 1, 1, 30 ) ;
	femMatPut(K_s, 1, 4, -30 ) ;
	femMatPut(K_s, 4, 1, -30 ) ;
	femMatPut(K_s, 4, 4, 30 ) ;

	femMatPut(K_s, 2, 2, 36 ) ;
	femMatPut(K_s, 2, 3, 3*L ) ;
	femMatPut(K_s, 2, 5, -36 ) ;
	femMatPut(K_s, 2, 6, 3*L ) ;

	femMatPut(K_s, 3, 2, 3*L ) ;
	femMatPut(K_s, 3, 3, 4*L*L ) ;
	femMatPut(K_s, 3, 5, -3*L ) ;
	femMatPut(K_s, 3, 6, -L*L ) ;

	femMatPut(K_s, 5, 2, -36 ) ;
	femMatPut(K_s, 5, 3, -3*L ) ;
	femMatPut(K_s, 5, 5, 36 ) ;
	femMatPut(K_s, 5, 6, -3*L ) ;

	femMatPut(K_s, 6, 2, 3*L ) ;
	femMatPut(K_s, 6, 3, -L*L ) ;
	femMatPut(K_s, 6, 5, -3*L ) ;
	femMatPut(K_s, 6, 5, 4*L*L ) ;

  femValMatMultSelf((1/(30*L)), K_s);
#endif

	femMatPrn(K_s,"K_s");

#if 0
	femMatSetZero(K_s);
#endif
	return(rv);
}

int e003_local_stiff_matrix(long ePos, long Mode, double Lx, double Ex, double Ax, double Ix, double EI, double kl1, double kl2,  tMatrix *k_0)
{
  int rv = AF_OK ;
  int i,j ;

  femMatSetZero(k_0);

	/* filling of stiffness matrix: */

	femMatPut(k_0, 1, 1,  (Ax*Ex) / Lx              );  
	femMatPut(k_0, 1, 4, (-1.0) * (Ax*Ex) / Lx        );
	femMatPut(k_0, 4, 4,  (Ax * Ex) / Lx            );

  if ((kl1 > 0.0) && (kl2 > 0.0))
  {
	  femMatPut(k_0, 2, 2,  (12.0 * Ex*Ix) / (Lx*Lx*Lx)  );
	  femMatPut(k_0, 2, 3,  (6.0 * Ex*Ix) / (Lx*Lx)  );
	  femMatPut(k_0, 2, 5,  (-12.0 * Ex*Ix) / (Lx*Lx*Lx) );
	  femMatPut(k_0, 2, 6,  (6.0 * Ex*Ix) / (Lx*Lx)  );

	  femMatPut(k_0, 3, 3,  (4.0 * Ex*Ix) / Lx       );
	  femMatPut(k_0, 3, 5,  (-6.0 * Ex*Ix) / (Lx*Lx)     );
	  femMatPut(k_0, 3, 6,  (2.0 * Ex*Ix) / Lx   );

	  femMatPut(k_0, 5, 5,  (12.0 * Ex*Ix) / (Lx*Lx*Lx)  );
	  femMatPut(k_0, 5, 6,  (-6.0 * Ex*Ix) / (Lx*Lx) );

	  femMatPut(k_0, 6, 6,  (4.0 * Ex*Ix) / Lx       );
  }
  else
  {
    if ((kl1 == 0.0) && (kl2 > 0.0)) /* o--| */
    {
      femMatPut(k_0, 2, 2,  (3.0 * EI) / (Lx*Lx*Lx)  );
	    femMatPut(k_0, 2, 5,  (-3.0 * EI) / (Lx*Lx*Lx) );
	    femMatPut(k_0, 2, 6,  (-3.0 * EI) / (Lx*Lx)  );

	    femMatPut(k_0, 5, 5,  (3.0 * EI) / (Lx*Lx*Lx)  );
	    femMatPut(k_0, 5, 6,  (3.0 * EI) / (Lx*Lx) );

	    femMatPut(k_0, 6, 6,  (3.0 * EI) / Lx       );
    }

    if ((kl1 > 0.0) && (kl2 == 0.0)) /* |--o */
    {
      femMatPut(k_0, 2, 2,  (3.0 * EI) / (Lx*Lx*Lx)  );
	    femMatPut(k_0, 2, 3,  (-3.0 * EI) / (Lx*Lx)  );
	    femMatPut(k_0, 2, 5,  (-3.0 * EI) / (Lx*Lx*Lx) );

	    femMatPut(k_0, 3, 2,  (-3.0 * EI) / (Lx*Lx)   );
	    femMatPut(k_0, 3, 3,  (3.0 * EI) / Lx       );
	    femMatPut(k_0, 3, 5,  (3.0 * EI) / (Lx*Lx)     );

	    femMatPut(k_0, 5, 5,  (3.0 * EI) / (Lx*Lx*Lx) );
    }

    if ((kl1 == 0.0) && (kl2 == 0.0)) /* 0-0 - probably a user error */
    {
#ifdef RUN_VERBOSE
      fprintf(msgout,"[W] %s %li!\n", _("Beam with two hinges found - element"), eID[ePos]);
#endif
    }
  }
  
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

int e003_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
	int     rv = AF_OK ;
	double  x1,y1,x2,y2;
	double  Ex = 0;  /* Young's modulus */
	double  Ax = 0 ; /* area */
	double  Ix = 0 ; /* moment of inertia */
	double  Lx = 0 ; /* lenght of beam */
	tMatrix k_0;
	tMatrix k_g;
	tMatrix k_1;
	tMatrix Tk_0;
	tMatrix T;
	tMatrix T_T;
	tVector F_0 ;
	tVector F_1 ;
	tVector u_e ;
	tVector u_0 ;
	double  cos_a, sin_a ;
  double  kl1 = 1 ;
  double  kl2 = 1 ;
  double  N ;
  double  EI ;
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

  if (fem2ndOrder == AF_YES)
  {
	  if ((rv=femFullMatInit(&k_g,6,6)) != AF_OK) { goto memFree; }
	  if ((rv=femFullMatInit(&k_1,6,6)) != AF_OK) { goto memFree; }
  }

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
	Ax = femGetRSValPos(ePos, RS_AREA, 0) ;
	Ix = femGetRSValPos(ePos, RS_INERTIA_Y, 0) ;

	kl1 = femGetRSValPos(ePos, RS_STF1, 0) ;
	kl2 = femGetRSValPos(ePos, RS_STF2, 0) ;

  /* stiffness of left and right joint: */
  if (kl1 < 0.5) { kl1 = 0 ; }
  else           { kl1 = 1 ; }

  if (kl2 < 0.5) { kl2 = 0 ; }
  else           { kl2 = 1 ; }

	/* material properties: */
	Ex = femGetMPValPos(ePos, MAT_EX, 0) ;

  e003_local_stiff_matrix(ePos, Mode, Lx, Ex, Ax, Ix, Ex*Ix, kl1, kl2, &k_0);


	/* transformation matrix */
	femMatSetZero(&T) ;
	femMatSetZero(&T_T) ;

	femMatPut(&T, 1, 1,  cos_a ) ;
	femMatPut(&T, 1, 2,  sin_a ) ;

	femMatPut(&T, 2, 1, -sin_a ) ;
	femMatPut(&T, 2, 2,  cos_a ) ;

	femMatPut(&T, 3, 3,  1.0 ) ;

	femMatPut(&T, 4, 4,  cos_a ) ;
	femMatPut(&T, 4, 5, sin_a ) ;

	femMatPut(&T, 5, 4, -sin_a ) ;
	femMatPut(&T, 5, 5,  cos_a ) ;

	femMatPut(&T, 6, 6,  1.0 ) ;

	/* transposition of "T" */
	femMatTran(&T, &T_T) ;

  if (fem2ndOrder == AF_YES)
  {
    N = femGetEResVal(ePos, RES_FX, 1),

    e003_geom_matrix(ePos, Mode, Lx, &k_g);
    femMatLinComb(1.0, &k_0, (-1.0*N), &k_g, &k_1);

	  /* global stiffness matrix: */
	  femMatMatMult(&T_T, &k_1, &Tk_0);
  }
  else
  {
	  /* global stiffness matrix: */
	  femMatMatMult(&T_T, &k_0, &Tk_0);
  }

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
		femVecNull(&F_1);
		femVecFullInit(&F_1,6);

    if (fem2ndOrder == AF_YES)
    {
		  femVecNull(&u_0);
		  femVecFullInit(&u_0,6);
    }

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

    EI = Ex * Ix ;

    /* layered beam */
    if ((femGetETypePos(ePos) == 3) && (femRSNumPos(femGetERSPos(ePos)) > 4))
    {
      /* recompute stiffness matrix (won't work for 2nd order!): */
      /* TODO */
#if 0
      EI = 
#endif
      e003_local_stiff_matrix(ePos, Mode, Lx, Ex, Ax, Ix, EI, kl1, kl2, &k_0);
	    femMatMatMult(&T_T, &k_0, &Tk_0);
	    femMatMatMult(&Tk_0, &T, K_e);
		  femMatVecMult(K_e, &u_e, F_e) ; /* global */
		  femMatVecMult(&T, F_e, &F_0) ; /* local */
      femVecLinComb(1.0, &F_0, -1.0, &F_1, Fr_e) ;

    }
    

    if (fem2ndOrder == AF_YES)
    {
#if 0
		  femMatVecMult(&T, &u_e, &u_0) ; /* local */

		  femVecPrn(&u_0, "u_0");

      f0_Norm = femVecNorm(&F_0) ;
      femVecClone(&F_0, &F_1);

      femMatSetZero(&k_0) ;
      e003_local_stiff_matrix(ePos, Mode, Lx, Ex, Ax, Ix, kl1, kl2, &k_0);

      femMatSetZero(&k_g) ;
      e003_geom_matrix(ePos, Mode, Lx, &k_g);

      for (i=0; i<10e4; i++)
      {
        femMatSetZero(&k_1);

#if 0
        N = 0.5 * ( femVecGet(&F_1,1) + femVecGet(&F_1,4) ) ;
#else
        N = femVecGet(&F_1,4) ;
#endif
        femMatLinComb(1.0, &k_0, (-1.0*N), &k_g, &k_1);

        printf("NNN N = %f (%f,%f)\n", N,femVecGet(&F_1,1) , femVecGet(&F_1,4));
		    femVecPrn(&F_1, "F1");
        femMatPrn(&k_0,"K0");
        femMatPrn(&k_1,"K1");

        femVecSetZero(&F_1);
		    femMatVecMult(&k_1, &u_0, &F_1) ;

        f1_Norm = femVecNorm(&F_1) ;

        if (fabs(f0_Norm - f1_Norm) < 1e-4) { break ; }
        f0_Norm = f1_Norm ;
      }

      femVecPrn(&F_0,"F0") ;
      femVecPrn(&F_1,"F1") ;

      femVecLinComb(1.0, &F_0, -1.0, &F_1, Fr_e) ;
      femVecClone(&F_1, &F_0);

      femVecClone(Fr_e, &F_1);
      femMatVecMult(&T_T, &F_1, Fr_e);
      femVecPrn(Fr_e,"RESIDUAL FORCES") ;

      /* is it usefull? :*/
#if 0
	    femMatMatMult(&T_T, &k_1, &Tk_0);
	    femMatMatMult(&Tk_0, &T, K_e);
#endif
#endif
    }

	  if (femTangentMatrix == AF_YES)
	  {
	    femAddEResVal(ePos, RES_FX, 0, femVecGet(&F_0,1));
	    femAddEResVal(ePos, RES_FY, 0, femVecGet(&F_0,2));
	    femAddEResVal(ePos, RES_MZ, 0, femVecGet(&F_0,3));

	    femAddEResVal(ePos, RES_FX, 1, femVecGet(&F_0,4));
	    femAddEResVal(ePos, RES_FY, 1, femVecGet(&F_0,5));
	    femAddEResVal(ePos, RES_MZ, 1, femVecGet(&F_0,6));
		}
		else
		{
	    femPutEResVal(ePos, RES_FX, 0, femVecGet(&F_0,1));
	    femPutEResVal(ePos, RES_FY, 0, femVecGet(&F_0,2));
	    femPutEResVal(ePos, RES_MZ, 0, femVecGet(&F_0,3));

	    femAddEResVal(ePos, RES_FX, 1, femVecGet(&F_0,4));
	    femAddEResVal(ePos, RES_FY, 1, femVecGet(&F_0,5));
	    femAddEResVal(ePos, RES_MZ, 1, femVecGet(&F_0,6));
		}

		/*printf("RR %f %f \n",
	    femGetEResVal(ePos, RES_MZ, 0),
	    femGetEResVal(ePos, RES_MZ, 1)
		);*/

		femVecFree(&u_e);
		femVecFree(&F_0);
    femVecFree(&F_1); 
    if (fem2ndOrder == AF_YES) 
    { 
      femVecFree(&u_0); 
    }
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

int e003_mass(long ePos, tMatrix *M_e)
{
	int    rv = AF_OK ;

	double x1,y1,x2,y2;
	double Lx  = 0 ;
	double Ax  = 0 ;
	double dens = 0 ;
	double mass = 0 ;
  double sin_a,cos_a;
	int    i, j ;
  tMatrix Me0 ;
	tMatrix Tk_0;
	tMatrix T;
	tMatrix T_T;

	femMatNull(&Tk_0);
	femMatNull(&T);
	femMatNull(&T_T);

	if ((rv=femFullMatInit(&Me0,6,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&Tk_0,6,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&T,6,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&T_T,6,6)) != AF_OK) { goto memFree; }


  femFullMatInit(&Me0,6,6);

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

  femMatSetZero(M_e);

#if 0
	mass = dens * Ax * Lx ;
	for (i=1; i<=3; i++)
	{
		femMatPut(&Me0,i,  i,   ( (mass)/(2.0) ) ) ;
		femMatPut(&Me0,i+3,i+3, ( (mass)/(2.0) ) ) ;
	}
#else
	mass = dens * Ax * Lx / 420.0 ;
  femMatPut(&Me0, 1,1,mass*140.0  ) ;
  femMatPut(&Me0, 1,4,mass*70.0  ) ;

	femMatPut(&Me0, 2,2,mass*156.0 ) ;
  femMatPut(&Me0, 2,3,22.0*Lx*mass  ) ;
  femMatPut(&Me0, 2,5,54.0*mass  ) ;
  femMatPut(&Me0, 2,6,(-13.0)*Lx*mass  ) ;

  femMatPut(&Me0, 3,3,mass * 4.0*Lx*Lx ) ;
  femMatPut(&Me0, 3,5,mass * 13.0*Lx ) ;
  femMatPut(&Me0, 3,6,mass * (-3.0)*Lx*Lx ) ;

  femMatPut(&Me0, 4,4,140.0*mass ) ;

  femMatPut(&Me0, 5,5,mass * 156.0 ) ;
  femMatPut(&Me0, 5,6,mass* (-22.0)*Lx ) ;

  femMatPut(&Me0, 6,6,mass * 4.0*Lx*Lx ) ;

	for (i=1; i<=6; i++)
	{
		for (j=i; j<=6; j++)
		{
			femMatPut(&Me0, j,i, femMatGet(&Me0,i,j)) ;
		}
	}

#endif

  /* TODO: transformation */
	cos_a = (x2 - x1) / Lx ;
	sin_a = (y2 - y1) / Lx ;

	/* transformation matrix */
	femMatSetZero(&T) ;
	femMatSetZero(&T_T) ;

	femMatPut(&T, 1, 1,  cos_a ) ;
	femMatPut(&T, 1, 2,  sin_a ) ;

	femMatPut(&T, 2, 1, -sin_a ) ;
	femMatPut(&T, 2, 2,  cos_a ) ;

	femMatPut(&T, 3, 3,  1.0 ) ;

	femMatPut(&T, 4, 4,  cos_a ) ;
	femMatPut(&T, 4, 5, sin_a ) ;

	femMatPut(&T, 5, 4, -sin_a ) ;
	femMatPut(&T, 5, 5,  cos_a ) ;

	femMatPut(&T, 6, 6,  1.0 ) ;

	/* transposition of "T" */
	femMatTran(&T, &T_T) ;

	femMatMatMult(&T_T, &Me0, &Tk_0);
	femMatMatMult(&Tk_0, &T, M_e);

memFree:
  femMatFree(&Me0);
	femMatFree(&Tk_0);
	femMatFree(&T);
	femMatFree(&T_T);
	return(rv);
}

int e003_volume(long ePos, double *vol)
{
	int    rv = AF_OK ;

	double x1,y1,x2,y2;
	double Lx  = 0 ;
	double Ax  = 0 ;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));

  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));

	Lx = sqrt( pow((y2 - y1), 2) + pow((x2 - x1), 2) );
	Ax = femGetRSValPos(ePos, RS_AREA, 0) ;
	*vol = Ax * Lx  ;

	return(rv);
}

long e003_rvals(long ePos)
{
	return(6);
}

int e003_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

int e003_res_p_loc(long ePos, long point, double *x, double *y, double *z)
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

int addElem_003(void)
{
	int rv = AF_OK;
	static long type    = 3 ;
	static long dim     = 5 ;
	static long nodes   = 2 ;
	static long dofs    = 3 ;
	static long ndof[3] = {U_X,U_Y,ROT_Z} ;
	static long rs      = 4 ;
	static long real[4] = {RS_AREA, RS_INERTIA_Y, RS_STF1,RS_STF2} ;
	static long rs_rp      = 7 ;
	static long real_rp[7] = {RS_AREA, RS_INERTIA_Y, RS_HEIGHT, RS_STF1, RS_STF2, RS_MTYPE, RS_LNUM} ;
	static long res      = 3 ;
	static long nres[3]    = {RES_FX,RES_FY,RES_MZ};
	static long res_rp   = 3 ;
	static long nres_rp[3] = {RES_FX,RES_FY,RES_MZ};

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

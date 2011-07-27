/*
   File name: fem_e016.c
   Date:      2007/07/18 16:15
   Author:    Jiri Brozovsky

   Copyright (C) 2007  Jiri Brozovsky

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

	 FEM Solver - Element 016 (2D beam with layers)

   PLEASE REMEBER: 
   * layers are numbert from BOTTOM
   * there have to be always AT LEAST ONE LAYER

   Note that no element load is used (nodal forces/moments only)
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_
extern tVector u;

extern long sb1d_get_check_epsilon(
    double epsilon, 
    double prev_epsilon,
    double epsilon_el_tension,
    double epsilon_el_compression
    );

extern double sb1d_get_Rt_tension(long ePos,
    double E0, 
    double smax, 
    double Gf, 
    double L, 
    double epsilon);

extern int e003_volume(long ePos, double *vol);


int e016_local_stiff_matrix(long ePos, long Mode, double Lx, double Ex, double Ax, double EI, double kl1, double kl2,  tMatrix *k_0)
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

	  femMatPut(k_0, 2, 2,  (12.0 * EI) / (Lx*Lx*Lx)  );
	  femMatPut(k_0, 2, 3,  kl1*(-6.0 * EI) / (Lx*Lx)  );
	  femMatPut(k_0, 2, 5,  (-12.0 * EI) / (Lx*Lx*Lx) );
	  femMatPut(k_0, 2, 6,  kl2*(-6.0 * EI) / (Lx*Lx)  );

	  femMatPut(k_0, 3, 3,  kl1*(4.0 * EI) / Lx       );
	  femMatPut(k_0, 3, 5,  (6.0 * EI) / (Lx*Lx)     );
	  femMatPut(k_0, 3, 6,  kl1*kl2*(2.0 * EI) / Lx   );

	  femMatPut(k_0, 5, 5,  (12.0 * EI) / (Lx*Lx*Lx)  );
	  femMatPut(k_0, 5, 6,  kl2*(6.0 * EI) / (Lx*Lx) );

	  femMatPut(k_0, 6, 6,  kl2*(4.0 * EI) / Lx       );
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

double e016_compute_EI(
    long ePos, long Mode, 
    double epsilon_top, double epsilon_bot,
    double *yy, double *htop, double *hbot,
    double *Ax, double *Ix
    )
{
  static double EI = 0.0 ;
  long lnum = 0 ;
  long i, stat ;
  double y, A, S, b, h, E, I, E0;
  double epsilon, epsilon_prev ;
  double e_ten = 0.0 ;
  double e_com = 0.0 ;
  double Gf    = 0.0 ;
  double smax  = 0.0;

  E0 = femGetMPValPos(ePos, MAT_EX, 0) ;

  *Ax = 0.0 ;
  *Ix = 0.0 ;

  /* get center of gravity */
  S = 0.0 ;
  A = 0.0 ;
  y = 0.0 ;

  if ((lnum = femGetRSRepNumPos(ePos)) < 1)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout,"[W] %s %li!\n",_("No layers on beam"),femGetEIDPos(ePos));
#endif
    return(0.0);
  }

  for (i=1; i<=lnum; i++)
  {
    b = femGetRSValPos(ePos, RS_WIDTH, i) ; 
    h = femGetRSValPos(ePos, RS_HEIGHT, i) ; 

    A += b*h ;
    S += A*((0.5*h) + y) ;
    y += h ;
  }

  if (A <= 0.0)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout,"[W] %s %li!\n",_("Empty layers on beam"),femGetEIDPos(ePos));
#endif
    return(0.0);
  }

  *yy = S/A ;
  
  *htop = fabs (*yy - y) ;
  *hbot = fabs (y) ;

  *Ax = A ;

  /* Compute EI */
  EI = 0.0 ;
  E  = 0.0 ;
  I  = 0.0 ;

  if (Mode == AF_YES)
  {
    e_ten = femGetMPValPos(ePos, MAT_STEN,  0) / E0 ;
    e_com = femGetMPValPos(ePos, MAT_SCMPR, 0) / E0 ;

    Gf    = femGetMPValPos(ePos, MAT_GF, 0) ;
    smax  = femGetMPValPos(ePos, MAT_STEN1, 0) ;
  }

  for (i=1; i<=lnum; i++)
  {
    b = femGetRSValPos(ePos, RS_WIDTH, i) ; 
    h = femGetRSValPos(ePos, RS_HEIGHT, i) ; 

    I = ( (1.0/12.0)*b*pow(h,3) + b*h*(y+(0.5*h))) ;
    *Ix +=  I ;
      
    E = 0.0 ;

    if (Mode == AF_YES)
    {
      /* new */
      if ((y+0.5*h) <= *yy)
      {
        /* bottom */
        epsilon = epsilon_bot * fabs((y+0.5*h) / (*yy)) ;
      }
      else
      {
        /* up */
        epsilon = epsilon_top * fabs(((y+0.5*h)-(*hbot))/(*htop)) ;
      }

      if (femGetMPTypePos(ePos) == 8)
      {
        epsilon_prev = femGetEResVal(ePos, RES_EX, i) ;

        stat = sb1d_get_check_epsilon(
          epsilon, epsilon_prev,
          e_ten, e_com);

        femPutEResVal(ePos, RES_ECR2, I, stat);

        switch (stat)
        {
          case 2: /* inelastic tension */
            /* TODO !!! */
            E = sb1d_get_Rt_tension(ePos, E0, 
            smax, 
            Gf/b, /* is it OK ? */
            h, 
            epsilon) ;

            break ;

          case 3: /* inelastic compression */
            E = E0 ; 
            break ;

          case -1: /* inelastic tension unloading */ 
            /* TODO add something better here !!! TODO */
            if ((E=femGetEResVal(ePos, RES_ECR1,i)) <= 0.0) {E=E0;}
            break ;

          case -3: /* inelastic compression unloading */
            E = E0 ;
            break ;

          case 1: /* elastic */
          case 0:
          default:
            E = E0 ; break ;
        }
      }
      else
      {
        E = E0 ;

      }

      femPutEResVal(ePos, RES_ECR1, I, E);

    }
    else
    {
      /* old */
      stat = (long)femGetEResVal(ePos, RES_ECR2, i) ;
      if (stat == 0.0) 
      { 
        E = E0; 
      } 
      else
      {
        E = femGetEResVal(ePos, RES_ECR1, i) ;
      }
    }

    EI += (E*I);

    y += h ;
  }

  return(EI);
}

int e016_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
	int     rv = AF_OK ;
	double  x1,y1,x2,y2;
	double  Ex = 0;  /* Young's modulus */
	double  Ax = 0 ; /* area */
	double  Ix = 0 ; /* moment of inertia */
	double  Lx = 0 ; /* lenght of beam */
	tMatrix k_0;
	tMatrix Tk_0;
	tMatrix T;
	tMatrix T_T;
	tVector F_0 ;
	tVector F_1 ;
	tVector u_e ;
	double  cos_a, sin_a ;
  double  kl1 = 1 ;
  double  kl2 = 1 ;
  double  EI ;
  double  htop, hbot, yy ; /* cross-section parameters */
  double  Mx, e_top, e_bot ;

	femMatNull(&k_0);
	femMatNull(&Tk_0);
	femMatNull(&T);
	femMatNull(&T_T);

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

#if 1
  EI = e016_compute_EI(ePos,AF_NO,0,0, &yy, &htop, &hbot, &Ax, &Ix);
#endif

  e016_local_stiff_matrix(ePos, Mode, Lx, Ex, Ax, EI, kl1, kl2, &k_0);


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

    Mx = 0.5 * (femVecGet(F_e,3)+femVecGet(F_e,6)) ;
    e_top = (Mx * htop) / EI ;
    e_bot = (Mx * (-hbot)) / EI ;

#ifdef DEVEL_VERBOSE
    fprintf(msgout,"e(%li) top=%e bot=%e\n",femGetEIDPos(ePos),e_top, e_bot);
#endif

		femMatVecMult(&T, F_e, &F_0) ; /* local */

		femVecSetZero(Fr_e); /* residual forces ;-) */

		femVecPrn(F_e, "F_e");
		femVecPrn(&F_0, "F_0");

    EI = Ex * Ix ;

    /* layered beam */
    /* TODO: compute epsilon _top and _bot here!!! */
#if 1
      EI = e016_compute_EI(ePos,Mode,e_top,e_bot, &yy, &htop, &hbot, &Ax, &Ix);
#endif
      e016_local_stiff_matrix(ePos, Mode, Lx, Ex, Ax, EI, kl1, kl2, &k_0);
	    femMatMatMult(&T_T, &k_0, &Tk_0);
	    femMatMatMult(&Tk_0, &T, K_e);
		  femMatVecMult(K_e, &u_e, F_e) ; /* global */
		  femMatVecMult(&T, F_e, &F_0) ; /* local */
      femVecLinComb(1.0, &F_0, -1.0, &F_1, Fr_e) ;

    
#if 0
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
#endif

		femVecFree(&u_e);
		femVecFree(&F_0);
  }
	
memFree:
	femMatFree(&k_0);
	femMatFree(&Tk_0);
	femMatFree(&T);
	femMatFree(&T_T);
  return(AF_OK);
}

int e016_mass(long ePos, tMatrix *M_e)
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
		femMatPut(M_e,i,  i,   ( (mass)/(4.0) ) ) ;
		femMatPut(M_e,i+3,i+3, ( (mass)/(4.0) ) ) ;
	}

	return(rv);
}

long e016_rvals(long ePos)
{
	return( 3 + (femGetRSRepNumPos(ePos) * 4) );
}

int e016_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

int e016_res_p_loc(long ePos, long point, double *x, double *y, double *z)
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

int addElem_016(void)
{
	int rv = AF_OK;
	static long type    = 16 ;
	static long dim     = 5 ;
	static long nodes   = 2 ;
	static long dofs    = 3 ;
	static long ndof[3] = {U_X,U_Y,ROT_Z} ;
	static long rs      = 2 ;
	static long real[2] = {RS_STF1,RS_STF2} ;
	static long rs_rp      = 4 ;
	static long real_rp[4] = { RS_WIDTH, RS_HEIGHT, RS_MTYPE, RS_LNUM };
	static long res      = 3 ;
	static long nres[3]    = {RES_FX,RES_FY,RES_MZ}; /* in element centroid */
	static long res_rp   = 4 ;
	static long nres_rp[4] = {RES_EX,RES_SX,RES_ECR1,RES_ECR2}; /* dtto but in layers */

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

	Elem[type].stiff = e016_stiff;
	Elem[type].mass  = e016_mass;
	Elem[type].rvals = e016_rvals;
	Elem[type].eload = e016_eload;
	Elem[type].res_p_loc = e016_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e003_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

#endif
/* end of fem_e016.c */

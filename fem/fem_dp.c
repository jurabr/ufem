/*
   File name: fem_dp.c
   Date:      2004/01/24 14:53
   Author:    Jiri Brozovsky

   Copyright (C) 2004 

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

   FEM Solver - 3D Drucker-Prager plasticity condition

   $Id: fem_dp.c,v 1.3 2004/11/11 21:39:23 jirka Exp $
*/

#include "fem_pl3d.h"

/* from "fem_chen.c": */
extern double stress3D_I1(tVector *stress);
extern double stress3D_J2(tVector *stress);
extern int chen_Dep(tVector *deriv, double H, tMatrix *De, tMatrix *Dep);


/* elasticity condition derivatives: */
int dp_deriv(tVector *deriv, tVector *stress, double c, double phi)
{
  double s_x, s_y, s_z, t_xy, t_yz, t_zx ;
  double alpha, J2, multJ2;

  J2 = stress3D_J2(stress);

  multJ2 = 1.0 / (2.0*sqrt(J2)) ;

  alpha =  (2.0*sin(phi)) / (sqrt(3)*(3.0+sin(phi))) ;

  s_x  = femVecGet (stress, 1 ) ;
  s_y  = femVecGet (stress, 2 ) ;
  s_z  = femVecGet (stress, 3 ) ;
  t_yz = femVecGet (stress, 4 ) ;
  t_zx = femVecGet (stress, 5 ) ;
  t_xy = femVecGet (stress, 6 ) ;

  femVecPut(deriv,1, alpha + multJ2*fabs((2.0/3.0)*s_x - s_y - s_z)) ;
  femVecPut(deriv,2, alpha + multJ2*fabs((2.0/3.0)*s_y - s_x - s_z)) ;
  femVecPut(deriv,3, alpha + multJ2*fabs((2.0/3.0)*s_z - s_y - s_x)) ;
  femVecPut(deriv,4, alpha + multJ2*fabs(2.0 * t_yz)) ;
  femVecPut(deriv,5, alpha + multJ2*fabs(2.0 * t_zx)) ;
  femVecPut(deriv,6, alpha + multJ2*fabs(2.0 * t_xy)) ;

  return(AF_OK);
}


/** Drucker-Prager elastoplastic matrix */
int fem_dp_D_3D(long ePos, 
                  long e_rep, 
                  long eT, 
                  long mT, 
                  tVector *sigma, 
                  tVector *epsilon, 
                  long Mode, 
                  tMatrix *Dep)
#if 0
{ /* old code */
  int rv = AF_OK ;
  long   state = 0 ;
  double Ex, nu, c, phi ;
  double I1, J2, f, alpha, K;
  double H = 0.0 ;
  tVector deriv ;
  tVector old_sigma ;
  tMatrix De ;
  long i ;


  femVecNull(&deriv) ;
  femVecNull(&old_sigma) ;
  femMatNull(&De) ;

  if ((rv=femVecFullInit(&deriv, 6)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&old_sigma, 6)) != AF_OK){goto memFree;}
  if ((rv=femFullMatInit(&De, 6, 6)) != AF_OK){goto memFree;}

  femVecPut(&old_sigma,1, femGetEResVal(ePos,RES_SX,e_rep));
  femVecPut(&old_sigma,2, femGetEResVal(ePos,RES_SY,e_rep));
  femVecPut(&old_sigma,3, femGetEResVal(ePos,RES_SZ,e_rep));
  femVecPut(&old_sigma,4, femGetEResVal(ePos,RES_SYZ,e_rep));
  femVecPut(&old_sigma,5, femGetEResVal(ePos,RES_SZX,e_rep));
  femVecPut(&old_sigma,6, femGetEResVal(ePos,RES_SXY,e_rep));


  Ex  = femGetMPValPos(ePos, MAT_EX,    0)  ;
  nu  = femGetMPValPos(ePos, MAT_NU,    0)  ;
  c   = femGetMPValPos(ePos, MAT_COHES, 0)  ;
  phi = femGetMPValPos(ePos, MAT_FRICT, 0)  ;


  if (Mode != AF_YES)
  {
	  state =  femGetEResVal(ePos, RES_CR1, e_rep);

    if (state == 0.0)
    {
      femD_3D_iso(ePos, Ex, nu, Dep);
    }
    else
    {
      femD_3D_iso(ePos, Ex, nu, &De);
      H = femGetEResVal(ePos,RES_PSI,e_rep) ;
      dp_deriv(&deriv, &old_sigma, c, phi) ;
      chen_Dep(&deriv, H, &De, Dep) ;
    }
  }
  else
  {
    for (i=1; i<=6; i++)
    {
      femVecAdd(&old_sigma,i, femVecGet(sigma, i)) ;
    }

    I1 = stress3D_I1(&old_sigma);
    J2 = stress3D_J2(&old_sigma);

    alpha = (2.0*sin(phi)) / (sqrt(3)*(3.0+sin(phi))) ;
    K     = (6.0 * c * cos(phi)) / (sqrt(3.0)*(3.0+sin(phi))) ;

    f     = alpha*I1 + sqrt(fabs(J2)) - K ;

    if (f <= 0.0)
    {
      /* elastic */
      femD_3D_iso(ePos, Ex, nu, Dep);
      state = 0 ;
    }
    else
    {
      /* plastic */
      femD_3D_iso(ePos, Ex*1.001, nu, &De);
      chen_Dep(&deriv, H, &De, Dep) ;
      state = +1 ;
    }

	  femPutEResVal(ePos, RES_CR1, e_rep, state);
	  femPutEResVal(ePos, RES_PSI, e_rep, H);
  }

memFree:
  femVecFree(&deriv) ;
  femVecFree(&old_sigma) ;
  femMatFree(&De) ;
  return(rv) ;
}
#else
{ /* new code */
  int rv = AF_OK ;
  long   state = 0 ;
  double Ex, nu, c, phi, E1 ;
  double I1,J2,alpha,K,f ;
  double H = 0.0 ;
  tVector deriv ;
  tVector old_sigma ;
  tMatrix De ;
  long i ;


  femVecNull(&deriv) ;
  femVecNull(&old_sigma) ;
  femMatNull(&De) ;

  if ((rv=femVecFullInit(&deriv, 6)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&old_sigma, 6)) != AF_OK){goto memFree;}
  if ((rv=femFullMatInit(&De, 6, 6)) != AF_OK){goto memFree;}

  femVecPut(&old_sigma,1, femGetEResVal(ePos,RES_SX,e_rep));
  femVecPut(&old_sigma,2, femGetEResVal(ePos,RES_SY,e_rep));
  femVecPut(&old_sigma,3, femGetEResVal(ePos,RES_SZ,e_rep));
  femVecPut(&old_sigma,4, femGetEResVal(ePos,RES_SYZ,e_rep));
  femVecPut(&old_sigma,5, femGetEResVal(ePos,RES_SZX,e_rep));
  femVecPut(&old_sigma,6, femGetEResVal(ePos,RES_SXY,e_rep));



  Ex  = femGetMPValPos(ePos, MAT_EX,   0)  ;
  nu  = femGetMPValPos(ePos, MAT_NU,   0)  ;
  c   = femGetMPValPos(ePos, MAT_COHES, 0)  ;
  phi = femGetMPValPos(ePos, MAT_FRICT, 0)  ;
  E1  = femGetMPValPos(ePos, MAT_HARD, 0)  ;

	if (E1 == Ex) 
	{
    return(femD_3D_iso(ePos, Ex, nu, Dep)); /* linear solution */
	}
	else
	{
		H = E1 / (1.0 - (E1/Ex) ) ; /* hardening parameter for bilinear behaviour */
	}


	state =  (long)femGetEResVal(ePos, RES_CR1, e_rep);

  if (state == 0)
  {
    femD_3D_iso(ePos, Ex, nu, Dep);
  }
  else
  {
    femD_3D_iso(ePos, Ex, nu, &De);
    dp_deriv(&deriv, &old_sigma, c, phi) ;

    chen_Dep(&deriv, H, &De, Dep) ;
  }
  
  if (Mode == AF_YES)
  {
		femMatVecMult(Dep, epsilon, sigma) ;
    for (i=1; i<=6; i++) { femVecAdd(sigma,i, femVecGet(&old_sigma, i)) ; }

    I1 = stress3D_I1(&old_sigma);
    J2 = stress3D_J2(&old_sigma);

    alpha = (2.0*sin(phi)) / (sqrt(3)*(3.0+sin(phi))) ;
    K     = (6.0 * c * cos(phi)) / (sqrt(3.0)*(3.0+sin(phi))) ;

    f     = alpha*I1 + sqrt(fabs(J2)) - K ;

    if (f < 0.0)
    {
      /* elastic */
      femD_3D_iso(ePos, Ex, nu, Dep);
      state = 0 ;
    }
    else
    {
      /* plastic */
      femD_3D_iso(ePos, Ex, nu, &De);

      dp_deriv(&deriv, sigma, c, phi) ;

      chen_Dep(&deriv, H, &De, Dep) ;

		  state = 1 ;
    }

		femMatVecMult(Dep, epsilon, sigma) ;
    for (i=1; i<=6; i++) { femVecAdd(sigma,i, femVecGet(&old_sigma, i)) ; }
	
	  femPutEResVal(ePos, RES_CR1, e_rep, state);
	  femPutEResVal(ePos, RES_PSI, e_rep, H);
  }

memFree:
  femVecFree(&deriv) ;
  femVecFree(&old_sigma) ;
  femMatFree(&De) ;
  return(rv) ;
}
#endif

/* end of fem_dp.c */

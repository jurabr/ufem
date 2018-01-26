/*
   File name: fem_vmis.c
   Date:      2004/03/27 13:41
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

   FEM Solver - 3D von Mises plasticity condition
*/

#include "fem_pl3d.h"

/* from "fem_chen.c": */
extern double stress3D_I1(tVector *stress);
extern double stress3D_J2dev(tVector *stress);
extern int chen_Dep(tVector *deriv, double H, tMatrix *De, tMatrix *Dep);
extern int D_HookIso_planeRaw(double E, double nu, long Problem, tMatrix *D);
extern double stress2D_I1(tVector *stress);
extern double stress2D_J2dev(tVector *stress) ;

extern double fem_plast_H_linear(long ePos, 
                      double E0,
                      double E1,
											double fy,
                      double sigma);
extern double fem_plast_H_RO(long ePos, 
                      double k, 
                      double n, 
                      double E,
                      double sigma);

/* Material status values: 0=linear, 1=plastic, -1=unloading */

/** Return mapping procedure: CPA method ("consistent elastoplatic
 ** modulus")
 * @param sigma: stress vector (returned with total sigma)
 * @param epsilon: strain vector (addition from current step)
 * @param Mode: AF_YES=new state, AF_NO=old 
 */
int retmap_CPA( long ePos, 
                long e_rep, 
                long eT, 
                long mT, 
                tVector *sigma, 
                tVector *epsilon, 
                long Mode, 
                tMatrix *Dep )
{
  int rv = AF_OK ;
  long d_dim = 3 ;
  long i ;
  double lambda, F ;
  double Ex, nu, fy ;
  tVector epsilon_n ;
  tVector epsilon_p ;
  tVector epsilon_d ;
  tVector d_sigma ;
  tVector a ;
  tMatrix De ;
  tMatrix Dei ;

  /* check dimensionality of problem: */
  if (Elem[eT].dim == 3) { d_dim = 6 ;        /* 3D element */ }
  else { if (Elem[eT].dim == 2) { d_dim = 3 ; /* wall element*/ }
    else
    {
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s: %li!\n", _("Return mapping - bad dimension for element"),ePos);
#endif
      return(AF_ERR_VAL);
    }
  }

  lambda = 0.0 ; /* sed lambda to zero */
  femVecSetZero(sigma);

  /* TODO: allocate sigma, epsilon vectors */
  femVecNull(&epsilon_n) ;
  femVecNull(&epsilon_p) ;
  femVecNull(&epsilon_d) ;
  femVecNull(&d_sigma) ;
  femVecNull(&a) ;
  femMatNull(&De) ;
  femMatNull(&Dei) ;

  if ((rv=femVecFullInit(&epsilon_n, d_dim)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&epsilon_p, d_dim)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&epsilon_d, d_dim)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&d_sigma,   d_dim)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&a,         d_dim)) != AF_OK){goto memFree;}
  if ((rv=femFullMatInit(&De, d_dim, d_dim)) != AF_OK){goto memFree;}
  if ((rv=femFullMatInit(&Dei,d_dim, d_dim)) != AF_OK){goto memFree;}

  Ex = femGetMPValPos(ePos, MAT_EX, 0); 
  nu = femGetMPValPos(ePos, MAT_NU, 0); 
  fy = femGetMPValPos(ePos, MAT_F_YC, 0)  ;

  /* get previous PLASTIC and TOTAL strain and set De: */
  switch (d_dim)
  {
    case 3:
      femVecPut(&epsilon_p,1, femGetEResVal(ePos,RES_EPX,e_rep));
      femVecPut(&epsilon_p,2, femGetEResVal(ePos,RES_EPY,e_rep));
      femVecPut(&epsilon_p,3, femGetEResVal(ePos,RES_EPXY,e_rep));

      femVecPut(&epsilon_n,1, femGetEResVal(ePos,RES_EX,e_rep));
      femVecPut(&epsilon_n,2, femGetEResVal(ePos,RES_EY,e_rep));
      femVecPut(&epsilon_n,3, femGetEResVal(ePos,RES_EXY,e_rep));

      D_HookIso_planeRaw(Ex, nu, 0, &De); /* hardcoded plane stress ! */
      break ;
    case 6:
      femVecPut(&epsilon_p,1, femGetEResVal(ePos,RES_EPX,e_rep));
      femVecPut(&epsilon_p,2, femGetEResVal(ePos,RES_EPY,e_rep));
      femVecPut(&epsilon_p,3, femGetEResVal(ePos,RES_EPZ,e_rep));
      femVecPut(&epsilon_p,4, femGetEResVal(ePos,RES_EPYZ,e_rep));
      femVecPut(&epsilon_p,5, femGetEResVal(ePos,RES_EPZX,e_rep));
      femVecPut(&epsilon_p,6, femGetEResVal(ePos,RES_EPXY,e_rep));

      femVecPut(&epsilon_n,1, femGetEResVal(ePos,RES_EX,e_rep));
      femVecPut(&epsilon_n,2, femGetEResVal(ePos,RES_EY,e_rep));
      femVecPut(&epsilon_n,3, femGetEResVal(ePos,RES_EZ,e_rep));
      femVecPut(&epsilon_n,4, femGetEResVal(ePos,RES_EYZ,e_rep));
      femVecPut(&epsilon_n,5, femGetEResVal(ePos,RES_EZX,e_rep));
      femVecPut(&epsilon_n,6, femGetEResVal(ePos,RES_EXY,e_rep));

      femD_3D_iso(ePos, Ex, nu, &De) ;
      break ;
  }

  /* make inversion of De: */
  femMatCloneDiffToEmpty(&De, &Dei);
  femLUinverse(&Dei);

  /* total initial epsilon (n+1): */
  femVecAddVec(&epsilon_n, 1.0, epsilon);

  /* initial elastic stress: */
  femVecLinComb(1.0, &epsilon_n, -1.0, &epsilon_p, &epsilon_d);

  /* initial sigma: */
  femMatVecMult(&De, &epsilon_d, sigma);

  /* iteration loop: */
  for (i=0; i< 100; i++)
  {
    /* check plasticity condition: */
    if ((F = (3.0 * stress3D_J2dev(sigma) )- (fy*fy) ) < FEM_ZERO)
    {
      break ; /* we are done */
    }

    /* compute lambda: */
    switch (d_dim) /* check what "alpha=0.0" does! */
    {
      case 3: vmis_deriv2D(&a, sigma, 1, 0.0) ; break;
      case 6: vmis_deriv(&a, sigma, 1, 0.0) ; break;
    }
    lambda = femVecMatVecMult(&a, &De, &a) ;
    if (lambda >= FEM_ZERO)
    {
      lambda = F / lambda ;
    }
    else
    {
      rv = AF_ERR_ZER ; goto memFree ;
    }

    /* compute d_sigma: */
    femMatVecMult(&De, &a, &d_sigma) ;
    femValVecMultSelf(-1.0*lambda, &d_sigma) ;

    /* compute new epsilon_p: */
    femMatVecMult(&Dei, &d_sigma, &a) ; /* using "a" as mid-product!*/
    femVecAddVec(&epsilon_p, -1.0, &a) ;

    /* compute total sigma: */
    femVecAddVec(sigma, 1.0, &d_sigma) ;

  } /* end "for i" */

  /* TODO write plastic stress ! */
  switch (d_dim)
  {
    case 3:
      femPutEResVal(ePos,RES_EPX,e_rep,femVecGet(&epsilon_p,1));
      femPutEResVal(ePos,RES_EPY,e_rep,femVecGet(&epsilon_p,2));
      femPutEResVal(ePos,RES_EPXY,e_rep,femVecGet(&epsilon_p,3));
      break ;
    case 6:
      femPutEResVal(ePos,RES_EPX,e_rep,femVecGet(&epsilon_p,1));
      femPutEResVal(ePos,RES_EPY,e_rep,femVecGet(&epsilon_p,2));
      femPutEResVal(ePos,RES_EPZ,e_rep,femVecGet(&epsilon_p,3));
      femPutEResVal(ePos,RES_EPYZ,e_rep,femVecGet(&epsilon_p,4));
      femPutEResVal(ePos,RES_EPZX,e_rep,femVecGet(&epsilon_p,5));
      femPutEResVal(ePos,RES_EPXY,e_rep,femVecGet(&epsilon_p,6));
      break ;
  }


memFree:
  femVecFree(&epsilon_n);
  femVecFree(&epsilon_p);
  femVecFree(&epsilon_d);
  femVecFree(&d_sigma);
  femVecFree(&a);
	femMatFree(&De);
	femMatFree(&Dei);
  return(rv);
}

/** elasticity condition derivatives: */
int vmis_deriv(tVector *deriv, tVector *stress, long type, double alpha)
{
  double s_x, s_y, s_z, s_q, t_xy, t_yz, t_zx, mult, mult1, J2 ;

  s_q =  (femVecGet(stress,1) + femVecGet(stress,2) + femVecGet(stress,3)) / 3.0 ;

  s_x  = femVecGet (stress, 1 ) - s_q ;
  s_y  = femVecGet (stress, 2 ) - s_q ;
  s_z  = femVecGet (stress, 3 ) - s_q ;
  t_yz = femVecGet (stress, 4 ) ;
  t_zx = femVecGet (stress, 5 ) ;
  t_xy = femVecGet (stress, 6 ) ;

  J2 = 0.5*(s_x*s_x + s_y*s_y + s_z*s_z) + t_xy*t_xy + t_yz*t_yz + t_zx*t_zx ;

  if (type == 1) /* von mises */
  {
    mult = sqrt(3.0) / (2.0 * sqrt(J2) ) ;
    mult1 = 0 ;
  }
  else /* drucker-prager */
  {
    mult = 1.0 / (2.0 * sqrt(J2) ) ;
    mult1 = 1.0 ;
  }

  femVecPut(deriv,1, alpha*mult1 + mult*(s_x )) ;
  femVecPut(deriv,2, alpha*mult1 + mult*(s_y )) ;
  femVecPut(deriv,3, alpha*mult1 + mult*(s_z )) ;
  femVecPut(deriv,4, mult*(2.0 * t_yz)) ;
  femVecPut(deriv,5, mult*(2.0 * t_zx)) ;
  femVecPut(deriv,6, mult*(2.0 * t_xy)) ;

  return(AF_OK);
}

double compute_sigma_e(tVector *sigma)
{
	return(  sqrt( 0.5 * (
				pow ((femVecGet(sigma,1) - femVecGet(sigma,2)), 2) +
				pow ((femVecGet(sigma,2) - femVecGet(sigma,3)), 2) +
				pow ((femVecGet(sigma,3) - femVecGet(sigma,1)), 2) +
				6.0 * (
					pow(femVecGet(sigma,4), 2) +
					pow(femVecGet(sigma,5), 2) +
					pow(femVecGet(sigma,6), 2)
					)
				)));
}

/** von Mises elastoplastic matrix */
int fem_vmis_D_3D(long ePos, 
                  long e_rep, 
                  long eT, 
                  long mT, 
                  tVector *sigma, 
                  tVector *epsilon, 
                  long Mode, 
                  tMatrix *Dep)
{
  int rv = AF_OK ;
  long   state = 0 ;
  double Ex, E1, nu, fy, n, k ;
  double s_eqv2 ;
  double f ;
  double cohes, frict ;
  double H = 0.0 ;
  tVector deriv ;
  tVector old_sigma ;
  tMatrix De ;
  long type    = 1 ;
  double alpha = 0.0 ;
  double kk    = 0.0 ;
  double prev ;
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

	state =  (long)femGetEResVal(ePos, RES_CR1, e_rep);
	H     = femGetEResVal(ePos, RES_PSI, e_rep);

  Ex  = femGetMPValPos(ePos, MAT_EX,   0)  ;
  nu  = femGetMPValPos(ePos, MAT_NU,   0)  ;
  fy  = femGetMPValPos(ePos, MAT_F_YC, 0)  ;
  E1  = femGetMPValPos(ePos, MAT_HARD, 0)  ;

  if (fabs(fy) > FEM_ZERO)
  {
    type = 1 ;
  }
  else
  {
    type = 0;
    cohes  = femGetMPValPos(ePos, MAT_COHES, 0)  ;
    frict  = femGetMPValPos(ePos, MAT_FRICT, 0)  ;
    alpha = ( 2.0 * sin(frict)) / (sqrt(3.0) * (3.0 - sin(frict)) ) ;
    kk    = ( 6.0 * cohes * cos(frict)) / (sqrt(3.0) * (3.0 - sin(frict)) ) ;
  }

  if (E1 <= FEM_ZERO)
  {
    n  = femGetMPValPos(ePos, MAT_RAMB_N, 0)  ;
    k  = femGetMPValPos(ePos, MAT_RAMB_K, 0)  ;
    E1 = 0.0 ;
  }
  else
  {
    k = 0.0 ;
    n = 0.0 ;
  }

	if ((state < 1)||(E1 == Ex)) 
	{
    return(femD_3D_iso(ePos, Ex, nu, Dep)); /* linear solution */
	}
	else
	{
    femD_3D_iso(ePos, Ex, nu, &De);
    vmis_deriv(&deriv, &old_sigma, type, alpha) ;
    chen_Dep(&deriv, H, &De, Dep) ;
	}
    
  if (Mode == AF_YES)
  {
    if  (E1 == Ex)
	  {
      femD_3D_iso(ePos,Ex, nu, Dep); /* linear solution */
	  }
	  else
	  {
      femD_3D_iso(ePos,Ex, nu, &De); 

	    femMatVecMult(Dep, epsilon, sigma) ;
      for (i=1; i<=6; i++) { femVecAdd(sigma,i, femVecGet(&old_sigma, i)) ; }

      if (type == 1)
      {
        s_eqv2 = (3.0 * stress3D_J2dev(sigma) ) ;
        prev = (3.0*stress3D_J2dev(&old_sigma)) ;

        f = s_eqv2 - (fy*fy) ;
      }
      else
      {
        s_eqv2 = alpha*stress3D_I1(sigma) + (stress3D_J2dev(sigma) ) ;
        prev   = alpha*stress3D_I1(&old_sigma) + (stress3D_J2dev(&old_sigma) ) ;

        f = s_eqv2 - kk ;
      }

      if (state == 0)
      {
        /* still elastic */
        if (f <= 0.0)
        {
          state = 0 ; /* continues to be elastic */
        }
        else
        {
          state = 1 ; /* yields */
        }
      }
      else
      {
        if (s_eqv2 < prev)
        {
          state = -1 ; /* unloading */
        }
        else
        {
          state = 1 ; /* plastic */
        }
      }
  
      switch (state) /* new matrix computation: */
      {
        case 1: /* plastic */
          H = fem_plast_H_linear(ePos, Ex, E1, fy, compute_sigma_e(&old_sigma) );
          vmis_deriv(&deriv, &old_sigma, type, alpha) ; /* works better with old_sigma !? */
          chen_Dep(&deriv, H, &De, Dep) ;
          break;
        case 0: /* elastic */
        case -1: /* plastic unloading */
        default: /* error */
          femD_3D_iso(ePos,Ex, nu, Dep);
          break ;
      }

	    femPutEResVal(ePos, RES_CR1, e_rep, state);
	    femPutEResVal(ePos, RES_PSI, e_rep, H);
	  }
  }

memFree:
  femVecFree(&deriv) ;
  femVecFree(&old_sigma) ;
  femMatFree(&De) ;
  return(rv) ;
}

/** 2D VERSION OF VON MISES *************************** */

/* elasticity condition derivatives: */
int vmis_deriv2D(tVector *deriv, tVector *stress, long type, double alpha)
{
  double s_x, s_y, t_xy, s_q, J2, mult, mult1 ;

  s_q =  (femVecGet(stress,1) + femVecGet(stress,2)) / 3.0 ;
  s_x  = femVecGet (stress, 1 ) - s_q ;
  s_y  = femVecGet (stress, 2 ) - s_q ;
  t_xy = femVecGet (stress, 3 ) ;

  J2 = 0.5*(s_x*s_x + s_y*s_y) + t_xy*t_xy ;

  if (type == 1) /* von mises */
  {
    mult = sqrt(3.0) / (2.0 * sqrt(J2) ) ;
    mult1 = 0 ;
  }
  else /* drucker-prager */
  {
    mult = 1.0 / (2.0 * sqrt(J2) ) ;
    mult1 = 1.0 ;
  }

  femVecPut(deriv,1, alpha*mult1 + mult*(s_x  )) ;
  femVecPut(deriv,2, alpha*mult1 + mult*(s_y )) ;
  femVecPut(deriv,3, mult*(2.0 * t_xy)) ;

  return(AF_OK);
}

double sigma_vmis2D(tVector *sigma)
{
	double sx, sy, sxy;

	sx = femVecGet(sigma,1);
	sy = femVecGet(sigma,2);
	sxy = femVecGet(sigma,3);

	return( sqrt((sx*sx + sy*sy -sx*sy + 3*sxy*sxy)));
}

/** von Mises elastoplastic matrix */
int fem_vmis_D_2D(long ePos, 
                  long e_rep, 
                  long Problem, 
                  tVector *epsilon, 
                  long Mode, 
                  tMatrix *Dep)
{
  int rv = AF_OK ;
  long   state = 0 ;
  double Ex, E1, nu, fy ;
  double f ;
  double H = 0.0 ;
  double cohes, frict ;
  double k, n ;
  double s_eqv2 ;
  tVector deriv ;
  tVector sigma ;
  tVector old_sigma ;
  tMatrix De ;
  long type    = 1 ;
  double alpha = 0.0 ;
  double kk    = 0.0 ;
  double prev ;
  long i ;

  femVecNull(&deriv) ;
  femVecNull(&old_sigma) ;
  femVecNull(&sigma) ;
  femMatNull(&De) ;

  if ((rv=femVecFullInit(&deriv, 3)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&sigma, 3)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&old_sigma, 3)) != AF_OK){goto memFree;}
  if ((rv=femFullMatInit(&De, 3, 3)) != AF_OK){goto memFree;}

  femVecPut(&old_sigma,1, femGetEResVal(ePos,RES_SX,e_rep));
  femVecPut(&old_sigma,2, femGetEResVal(ePos,RES_SY,e_rep));
  femVecPut(&old_sigma,3, femGetEResVal(ePos,RES_SXY,e_rep));

	state =  (long)femGetEResVal(ePos, RES_STAT1, e_rep);
	H     = femGetEResVal(ePos, RES_STAT2, e_rep);

  Ex  = femGetMPValPos(ePos, MAT_EX,   0)  ;
  nu  = femGetMPValPos(ePos, MAT_NU,   0)  ;
  fy  = femGetMPValPos(ePos, MAT_F_YC, 0)  ;
  E1  = femGetMPValPos(ePos, MAT_HARD, 0)  ;

  if (fabs(fy) > FEM_ZERO)
  {
    type = 1 ;
  }
  else
  {
    type = 0;
    cohes  = femGetMPValPos(ePos, MAT_COHES, 0)  ;
    frict  = femGetMPValPos(ePos, MAT_FRICT, 0)  ;
    alpha = ( 2.0 * sin(frict)) / (sqrt(3.0) * (3.0 - sin(frict)) ) ;
    kk    = ( 6.0 * cohes * cos(frict)) / (sqrt(3.0) * (3.0 - sin(frict)) ) ;
  }

  if (E1 <= FEM_ZERO)
  {
    n  = femGetMPValPos(ePos, MAT_RAMB_N, 0)  ;
    k  = femGetMPValPos(ePos, MAT_RAMB_K, 0)  ;
    E1 = 0.0 ;
  }
  else
  {
    k = 0.0 ;
    n = 0.0 ;
  }

  if ((state < 1)||(E1 == Ex))
  {
    D_HookIso_planeRaw(Ex, nu, Problem, Dep);
  }
  else
  {
    D_HookIso_planeRaw(Ex, nu, Problem, &De);
    vmis_deriv2D(&deriv, &old_sigma, type, alpha) ;
    chen_Dep(&deriv, H, &De, Dep) ; /* should work in 2D, too */
  }
  
	/* NEW matrix: */
  if (Mode == AF_YES)
  {
    if  (E1 == Ex)
	  {
      D_HookIso_planeRaw(Ex, nu, Problem, Dep); /* linear solution */
	  }
	  else
	  {
      D_HookIso_planeRaw(Ex, nu, Problem, &De);

	    femMatVecMult(Dep, epsilon, &sigma) ;
      for (i=1; i<=3; i++) { femVecAdd(&sigma,i, femVecGet(&old_sigma, i)) ; }

      if (type == 1)
      {
        s_eqv2 = (3.0 * stress2D_J2dev(&sigma) ) ;
        prev = (3.0*stress2D_J2dev(&old_sigma)) ;

        f = s_eqv2 - (fy*fy) ;
      }
      else
      {
        s_eqv2 = alpha*stress2D_I1(&sigma) + (stress2D_J2dev(&sigma) ) ;
        prev   = alpha*stress2D_I1(&old_sigma) + (stress2D_J2dev(&old_sigma) ) ;

        f = s_eqv2 - kk ;
      }

      if (state == 0)
      {
        /* still elastic */
        if (f <= 0.0)
        {
          state = 0 ; /* continues to be elastic */
        }
        else
        {
          state = 1 ; /* yields */
        }
      }
      else
      {
        if (s_eqv2 < prev)
        {
          state = -1 ; /* unloading */
        }
        else
        {
          state = 1 ; /* plastic */
        }
      }
  
      switch (state) /* new matrix computation: */
      {
        case 1: /* plastic */
          H = fem_plast_H_linear(ePos, Ex, E1, fy, sigma_vmis2D(&old_sigma) );
          vmis_deriv2D(&deriv, &old_sigma, type, alpha) ; /* works better with old_sigma !? */
          chen_Dep(&deriv, H, &De, Dep) ;
          break;
        case 0: /* elastic */
        case -1: /* plastic unloading */
        default: /* error */
          D_HookIso_planeRaw(Ex, nu, Problem, Dep);
          break ;
      }
      
	    femPutEResVal(ePos, RES_STAT1, e_rep, state);
	    femPutEResVal(ePos, RES_STAT2, e_rep, H);
	  }
  }

memFree:
  femVecFree(&deriv) ;
  femVecFree(&old_sigma) ;
  femVecFree(&sigma) ;
  femMatFree(&De) ;
  return(rv) ;
}

/* end of fem_vmis.c */

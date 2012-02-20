/*
   File name: fem_chen.c
   Date:      2004/01/03 18:26
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

   FEM Solver - 3D plasticity based on Chen-Chen

   $Id: fem_chen.c,v 1.5 2005/01/16 22:59:32 jirka Exp $
*/

#include "fem_pl3d.h"

#if 0
extern double c2d_get_H_Ohtani(
    double E0,
    double s_c_old, /* s_y[c|t|bc] values - input data */
    double s_bc_old,
    double s_t_old,
    double *k, /* Ramberg-Osgood parameters - input values */
    double *n,
    long cczone,
    double s1,
    double s2,
    double I1,
    double J2
    );
#endif


/* stress invariants etc.: */

double stress3D_I1(tVector *stress)
{
  return(femVecGet(stress,1)+femVecGet(stress,2)+femVecGet(stress,3));
}

double stress3D_J2(tVector *stress)
{
  double val = 0.0 ;
  double s_x, s_y, s_z, t_xy, t_yz, t_zx ;

  s_x  = femVecGet (stress, 1 ) ;
  s_y  = femVecGet (stress, 2 ) ;
  s_z  = femVecGet (stress, 3 ) ;
  t_zx = femVecGet (stress, 4 ) ;
  t_yz = femVecGet (stress, 5 ) ;
  t_xy = femVecGet (stress, 6 ) ;

  val = (
      (pow((s_x-s_y),2) + 
       pow((s_y-s_z),2) + 
       pow((s_z-s_x),2))
      /6.0) + 
    t_xy*t_xy + t_yz*t_yz + t_zx*t_zx ;
  
  return(val) ;
}


/** Chen's constants: */

double chen_Ac(double f_c0, double f_bc0, double f_t0)
{
  double f_c, f_bc, f_t ;

  f_t = fabs(f_t0);
  f_c = (-1.0)*fabs(f_c0);
  f_bc = (-1.0)*fabs(f_bc0);

  if ((2*f_bc - f_c) == 0.0)
  {
    return(0.0);
  }
  else
  {
    return ( ((f_bc*f_bc)-(f_c*f_c)) / (2.0*f_bc - f_c));
  }
}

double chen_At(double f_c0, double f_bc0, double f_t0)
{
  double f_c, f_t ;

  f_t = fabs(f_t0);
  f_c = (-1.0)*fabs(f_c0);

  return ( 0.5*(f_c - f_t) ) ;
}

double chen_tau_c(double f_c0, double f_bc0, double f_t0)
{
  double f_c, f_bc, f_t ;

  f_t = fabs(f_t0);
  f_c = (-1.0)*fabs(f_c0);
  f_bc = (-1.0)*fabs(f_bc0);

  if ((2*f_bc - f_c) == 0.0)
  {
    return(0.0);
  }
  else
  {
    return(fabs(f_bc*f_c*(2.0*f_c - f_bc))/(3.0*(2.0*f_bc - f_c)));
  }
}

double chen_tau_t(double f_c0, double f_bc0, double f_t0)
{
  double f_c, f_t ;

  f_t = fabs(f_t0);
  f_c = (-1.0)*fabs(f_c0);

  return ( fabs((f_c * f_t) / 6.0) ) ;
}

/* constants alpha_[ct], beta_[ct] */

double chen_alpha(double A_y, double tau_y, double A_u, double tau_u)
{
  if ( ((tau_u) - (tau_y)) == 0)
  {
    return(0.0);
  }
  else
  {
    return((A_u - A_y)/((tau_u)-(tau_y)));
  }
}

double chen_beta(double A_y, double tau_y, double A_u, double tau_u)
{
  if ( ((tau_u) - (tau_y)) == 0)
  {
    return(0.0);
  }
  else
  {
    return( ((A_y*tau_u)-(A_u*tau_y)) / (tau_u-tau_y));
  }
}

/* derivatives of the Chen's condition: */

/* TODO FIX THIS: A_c*/
int chen_deriv_c(tVector *deriv, tVector *stress, double alfac, double tauc0, double betac)
{
  double s_x, s_y, s_z, t_xy, t_yz, t_zx ;
  double tauc ;
  double a;

  tauc = sqrt(fabs(tauc0)) ;

  /*a = A_c / 3.0 ;*/
  a = (alfac*tauc+betac) / 3.0 ;

  s_x  = femVecGet (stress, 1 ) ;
  s_y  = femVecGet (stress, 2 ) ;
  s_z  = femVecGet (stress, 3 ) ;
  t_yz = femVecGet (stress, 4 ) ;
  t_zx = femVecGet (stress, 5 ) ;
  t_xy = femVecGet (stress, 6 ) ;

  femVecPut(deriv,1, (2*s_x - s_y - s_z)/3.0 + a ) ;
  femVecPut(deriv,2, (2*s_y - s_x - s_z)/3.0 + a ) ;
  femVecPut(deriv,3, (2*s_z - s_x - s_y)/3.0 + a ) ;
  femVecPut(deriv,4, 2*t_yz) ;
  femVecPut(deriv,5, 2*t_zx) ;
  femVecPut(deriv,6, 2*t_xy) ;

  return(AF_OK);
}

/* TODO FIX THIS: A_t*/
int chen_deriv_t(tVector *deriv, tVector *stress, double alfat, double taut0, double betat, double I1)
{
  double s_x, s_y, s_z, t_xy, t_yz, t_zx ;
  double a;
  double taut ;

  taut = sqrt(fabs(taut0));



  /*a = A_t / 3.0 ;*/
  a = (alfat*taut+betat-I1) / 3.0 ;

  s_x  = femVecGet (stress, 1 ) ;
  s_y  = femVecGet (stress, 2 ) ;
  s_z  = femVecGet (stress, 3 ) ;
  t_yz = femVecGet (stress, 4 ) ;
  t_zx = femVecGet (stress, 5 ) ;
  t_xy = femVecGet (stress, 6 ) ;

  femVecPut(deriv,1, a - 0.5*(s_y + s_z) ) ;
  femVecPut(deriv,2, a - 0.5*(s_x + s_z) ) ;
  femVecPut(deriv,3, a - 0.5*(s_x + s_y) ) ;
  femVecPut(deriv,4, 2*t_yz) ;
  femVecPut(deriv,5, 2*t_zx) ;
  femVecPut(deriv,6, 2*t_xy) ;

  return(AF_OK);
}

/** Tests if material is plastic/destroyed or not
 * @param I1 first stress invariant
 * @param J2 second  invariant od stress deviator
 * @param f_c uniaxial compressive stress
 * @param f_bc biaxial compressive stress
 * @param f_t uniaxial tension stress
 * @param cczone AF_YES if we are in C-C zone, AF_NO for other stress zones
 * @return AF_YES (over limit) or AF_NO (under limit)
 * */
long chen_limit_test(
    double I1,
    double J2,
    double f_c,
    double f_bc,
    double f_t,
    long *cczone)
{
  double sqJ2I1, A, tau ;

  sqJ2I1 = sqrt(J2) + (I1/3.0) ;

  if ( (I1 < 0.0) && (sqJ2I1 < 0.0) )
  {
    /* compression - compression zone */
    *cczone = AF_YES ;

    A = fabs ( chen_Ac(f_c, f_bc, f_t) ) ;
    tau = fabs( chen_tau_c(f_c, f_bc, f_t) ) ; /* means tau^2 ! */

#if 0
printf("COMPRESSION c=%e | A=%e tau2=%e | I1=%e J2=%e |fc=%e!\n", (J2 + (A/3.0)*I1 - tau), A, tau, I1, J2, f_c );
#endif

    if ( (J2 + (A/3.0)*I1 - tau ) < 0.0 )
    {
      return(AF_NO) ;
    }
    else
    {
#if 0
printf("COMPRESSION DAMAGE!\n");
#endif
      return(AF_YES) ;
    }
  }
  else
  {
    /* all other zones */
    *cczone = AF_NO ;

    A   = fabs ( chen_At(f_c, f_bc, f_t) ) ;
    tau = fabs( chen_tau_t(f_c, f_bc, f_t) ) ; /* means tau^2 ! */

#if 0
printf("TENSION c=%e | A=%e tau2=%e | I1=%e J2=%e, ft=%e!\n", ( J2 - ((I1*I1)/6.0) + (A/3.0)*I1 - tau), A, tau, I1, J2, f_t);
#endif

    if ( ( J2 - ((I1*I1)/6.0) + (A/3.0)*I1 - tau ) < 0.0 )
    {
      return(AF_NO) ;
    }
    else
    {
#if 0
printf("TENSION DAMAGE!\n");
#endif
      return(AF_YES) ;
    }
  }

  return(AF_NO) ;
}

/** Computes elastoplastic material stiffness  matrix
 * @param deriv derivatives of the plasticity condition
 * @param H hardening parameter
 * @param De elastic matrix
 * @param Dep elastoplastic matrix (result)
 * @return status
 * */
int chen_Dep(tVector *deriv, double H, tMatrix *De, tMatrix *Dep)
{
  int rv = AF_OK ;
  long size = 3 ;
  tMatrix Dtmp;
  tVector Df, fD ;
  double fDf, ff ;
  double mult ;

  size = deriv->rows ;
  if ((size != 3) && (size != 6))
  {
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s (%li)!\n",_("Chen Dep: Invalid size of derivatives vector"),size);
#endif
    return(AF_ERR_SIZ);
  }

  femMatNull(&Dtmp) ;
  femVecNull(&Df) ;
  femVecNull(&fD) ;

  if ((rv=femFullMatInit(&Dtmp, size, size)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&Df, size)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&fD, size)) != AF_OK){goto memFree;}

  femMatVecMult     (  De,     deriv, &Df) ;
  femVecMatMult     (  deriv,  De,    &fD) ;
  femVecVecMulttoMat( &Df,    &fD,    &Dtmp);

  fDf = femVecVecMult(deriv, &Df) ;
  ff  = sqrt(fabs(femVecVecMult(deriv, deriv))) ;

#if 0
  mult = fDf - fabs(H*ff) ; /* is fabs() OK here? */
#else
  mult = fDf + H ;
#endif

#if 1
#ifdef DEVEL_VERBOSE
  femMatPrn(De,"De");
  femVecPrn(deriv,"deriv");
  femVecPrn(&fD,"fD");
  femVecPrn(&Df,"Df");
	femMatPrn(&Dtmp,"D_TMP");
  fprintf(msgout,"fDf = %f H=%f (H*ff) = %f ff = %f mult = %f \n",fDf,H,H*ff,ff,mult);
#endif
#endif

  if (mult == 0.0) 
  {
    mult = 1.0 ;
#ifdef DEVEL_VERBOSE
    fprintf(msgout, "[E] %s!\n", _("Zero size multiplier in che_Dep"));
#endif
  }

  femMatLinComb(1.0, De, -1.0/mult, &Dtmp, Dep) ;

#if 1
	femMatPrn(De,"D_E");
	femMatPrn(Dep,"D_EP");
#endif

memFree:
  femMatFree(&Dtmp) ;
  femVecFree(&Df) ;
  femVecFree(&fD) ;
  return(rv) ;
}

int fem_chen_D_3D(long ePos, 
                  long e_rep, 
                  long eT, 
                  long mT, 
                  tVector *sigma_unused, 
                  tVector *epsilon, 
                  long Mode, 
                  tMatrix *Dep)
{
  int rv = AF_OK ;
  long   cczone = AF_YES ;
  long   state = 0 ;
  double Ex, nu ;
  double f_yc, f_ybc, f_yt ;
  double f_uc, f_ubc, f_ut ;
  double A_y, A_u, alfa, tau_y, tau_u, beta ;
  double I1, J2 ;
  double H = 0.0 ;
  tVector deriv ;
  tVector old_sigma ;
 	tVector sigma ;
  tVector sigma_1 ;
  tMatrix De ;
  double k[4];
  double n[4] ;
  long i, jj ;


  femVecNull(&deriv) ;
  femVecNull(&old_sigma) ;
  femVecNull(&sigma) ;
  femVecNull(&sigma_1) ;
  femMatNull(&De) ;

  if ((rv=femVecFullInit(&deriv, 6)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&old_sigma, 6)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&sigma, 6)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&sigma_1, 6)) != AF_OK){goto memFree;}
  if ((rv=femFullMatInit(&De, 6, 6)) != AF_OK){goto memFree;}

  femVecPut(&old_sigma,1, femGetEResVal(ePos,RES_SX,e_rep));
  femVecPut(&old_sigma,2, femGetEResVal(ePos,RES_SY,e_rep));
  femVecPut(&old_sigma,3, femGetEResVal(ePos,RES_SZ,e_rep));
  femVecPut(&old_sigma,4, femGetEResVal(ePos,RES_SYZ,e_rep));
  femVecPut(&old_sigma,5, femGetEResVal(ePos,RES_SZX,e_rep));
  femVecPut(&old_sigma,6, femGetEResVal(ePos,RES_SXY,e_rep));


  Ex = femGetMPValPos(ePos, MAT_EX, 0)  ;
  nu = femGetMPValPos(ePos, MAT_NU, 0)  ;

  f_yc  = femGetMPValPos(ePos, MAT_F_YC, 0)  ;
  f_ybc = femGetMPValPos(ePos, MAT_F_YBC, 0) ;
  f_yt  = femGetMPValPos(ePos, MAT_F_YT, 0)  ;

  f_uc  = femGetMPValPos(ePos, MAT_F_UC, 0)  ;
  f_ubc = femGetMPValPos(ePos, MAT_F_UBC, 0) ;
  f_ut  = femGetMPValPos(ePos, MAT_F_UT, 0)  ;

#if 0
  if (Mode != AF_YES)
  {
#endif

    /* old  */
	  state = (long) femGetEResVal(ePos, RES_CR1, e_rep);
    H = femGetEResVal(ePos,RES_PSI,e_rep) ;

    if (state == 0.0)
    {
      femD_3D_iso(ePos, Ex, nu, Dep);
    }
    else
    {
      femD_3D_iso(ePos, Ex, nu, &De);

      if (state < 0)
      {
        /* compression  */
 
        A_y = chen_Ac(f_yc, f_ybc, f_yt) ;
        A_u = chen_Ac(f_uc, f_ubc, f_ut) ;
        
        tau_y = chen_tau_c(f_yc, f_ybc, f_yt);
        tau_u = chen_tau_c(f_uc, f_ubc, f_ut);

        alfa = chen_alpha(A_y, tau_y, A_u, tau_u);
        beta = chen_beta(A_y, tau_y, A_u, tau_u);

        chen_deriv_c(&deriv, &old_sigma, alfa, tau_y, beta) ;
      }
      else
      {
        /* tension  */
				A_y = chen_At(f_yc, f_ybc, f_yt) ;
        A_u = chen_At(f_uc, f_ubc, f_ut) ;
        
        tau_y = chen_tau_t(f_yc, f_ybc, f_yt);
        tau_u = chen_tau_t(f_uc, f_ubc, f_ut);

        alfa = chen_alpha(A_y, tau_y, A_u, tau_u);
        beta = chen_beta(A_y, tau_y, A_u, tau_u);

        I1 = stress3D_I1(&old_sigma) ;
        chen_deriv_t(&deriv, &old_sigma, alfa, tau_y, beta, I1) ;
      }

#if 1
      chen_Dep(&deriv, H, &De, Dep) ;
#else

			if (H > 0.0)
			{
				femMatSetZero(Dep);
      	femD_3D_iso(ePos, H, nu, &De);
			}
#endif
    }
#if 0
  }
  else
#else

  if (Mode == AF_YES)
#endif
  {

    /* new */
    femVecSetZero(&sigma);
    femMatVecMult(Dep, epsilon, &sigma);

    for (i=1; i<=6; i++)
    {
      femVecAdd(&old_sigma,i, femVecGet(&sigma, i)) ;
    }

    I1 = stress3D_I1(&old_sigma) ;
    J2 = stress3D_J2(&old_sigma) ;

    femPrinc3D(&sigma, &sigma_1);

    if (chen_limit_test(I1, J2, f_yc, f_ybc, f_yt, &cczone) == AF_YES)
    {
      /* plastic */

      femD_3D_iso(ePos, Ex, nu, &De);

      if (cczone == AF_YES)
      {
        /* compression - compression zone */
				A_y = chen_Ac(f_yc, f_ybc, f_yt) ;
        A_u = chen_Ac(f_uc, f_ubc, f_ut) ;
        
        tau_y = chen_tau_c(f_yc, f_ybc, f_yt);
        tau_u = chen_tau_c(f_uc, f_ubc, f_ut);

        alfa = chen_alpha(A_y, tau_y, A_u, tau_u);
        beta = chen_beta(A_y, tau_y, A_u, tau_u);

        chen_deriv_c(&deriv, &old_sigma, alfa, tau_y, beta) ;

        state = -1 ;
      }
      else
      {
        /* other zones */
        A_y = chen_At(f_yc, f_ybc, f_yt) ;
        A_u = chen_At(f_uc, f_ubc, f_ut) ;
        
        tau_y = chen_tau_t(f_yc, f_ybc, f_yt);
        tau_u = chen_tau_t(f_uc, f_ubc, f_ut);

        alfa = chen_alpha(A_y, tau_y, A_u, tau_u);
        beta = chen_beta(A_y, tau_y, A_u, tau_u);

        chen_deriv_t(&deriv, &old_sigma, alfa, tau_y, beta, I1) ;

        state = (+1) ;
      }

			/* obtain k, n HERE */
      for (jj=1; jj<=3; jj++)
      {
        k[jj] = femGetMPValPos(ePos, MAT_RAMB_K, jj) ;
        n[jj] = femGetMPValPos(ePos, MAT_RAMB_N, jj) ;
      }

#if 0
      H = ( 
					c2d_get_H_Ohtani(
      			Ex,
      			f_yc,
      			f_ybc,
      			f_yt,
      			k,
      			n,
      			cczone,
      			femVecGet(&sigma_1,1),
      			femVecGet(&sigma_1,3),
      			I1,
            J2
    			) 
			);

#if 0
printf("H = %e\n", H);
#endif
#else
        H = 1e6 * FEM_ZERO ;
#endif

#if 0
			H = 0.0 ;
#endif


#if 1
     chen_Dep(&deriv, H, &De, Dep) ;
#else
		 if (H > 0.0)
		 {
			 femMatSetZero(Dep);
       femD_3D_iso(ePos, H, nu, &De);
		 }
#endif

     }
     else
     {
       /* elastic */
       femD_3D_iso(ePos, Ex, nu, Dep);
       state = 0 ;
     }

	   femPutEResVal(ePos, RES_CR1, e_rep, state);
	   femPutEResVal(ePos, RES_PSI, e_rep, H);

   }

memFree:
  femVecFree(&deriv) ;
  femVecFree(&old_sigma) ;
  femVecFree(&sigma) ;
  femVecFree(&sigma_1) ;
  femMatFree(&De) ;
  return(rv) ;
}

/* end of fem_chen.c */

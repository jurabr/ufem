/*
   File name: fem_ch2d.c
   Date:      2007/11/25 10:28
   Author:    Jiri Brozovsky

   Copyright (C) 2007 Jiri Brozovsky

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

   FEM solver: 2D Chen-Chen plasticity condition
*/

#include "fem_elem.h"                                         
#include "fem_mat.h"

extern int femGetPrincStress2D(tVector *sigma_x, tVector *sigma_1, double *phi);
extern int chen_Dep(tVector *deriv, double H, tMatrix *De, tMatrix *Dep);

/* from fem_stf2.c */
extern int D_HookIso_planeRaw(double E, double nu, long Problem, tMatrix *D);

/** Computes 1st stress invariant I1
 * @param stress  3-compoment stress vector {s_x, s_y, s_xy}
 * @return I1 */
double stress2D_I1(tVector *stress)
{
  return(femVecGet(stress,1) + femVecGet(stress,2));
}

/** Computes 2nd invariant of stress deviator J2
 * @param stress  3-compoment stress vector {s_x, s_y, s_xy}
 * @return J2 */
double stress2D_J2(tVector *stress)
{
  double s_x, s_y, t_xy;

  s_x  = femVecGet (stress, 1 ) ;
  s_y  = femVecGet (stress, 2 ) ;
  t_xy = femVecGet (stress, 3 ) ;

  printf("SIGMAS: %e %e %e\n",s_x, s_y, t_xy);

  return(
   (pow((s_x-s_y), 2) + (s_x*s_x) + (s_y*s_y) )/6.0 + (t_xy*t_xy)
      );
}

/** Computes material data for Chen yield criteria
 * @param fc  compresive strenth
 * @param fbc  biaxial compresive strenth
 * @param ft  tensile strenth
 * @param Ac  Ac material constant
 * @param At  At material constant
 * @param tau2c  tau^2_c material constant
 * @param tau2t  tau^2_t material constant
 * @return status
 */ 
int chen2d_get_A_tau2_y(double fc, double fbc, double ft,
		  double *Ac, double *At, double *tau2c, double *tau2t)
{
	*Ac = ( (fbc*fbc) - (fc*fc) ) / (2.0*fbc - fc) ;
	*At =  (fc - ft)  / 2.0 ;

  *tau2c = (fbc*fc * ( 2.0*fc - fbc) ) / (3.0 * ( 2.0*fbc - fc ) ) ;
  *tau2t = ( fc * ft ) / 6.0 ;
	return(AF_OK) ;
}

/** Checks the yield condition
 * @param I1 1st stress invariant
 * @param J2 2nd stress deviator invariant
 * @param Ac  Ac material constant
 * @param At  At material constant
 * @param tau2c  tau^2_c material constant
 * @param tau2t  tau^2_t material constant
 * @return -1 for c-c plastic, +1 for ofthe plastic, 0 for elastic
 */
long chen2d_check_yield(double I1, double J2,
    double Ac, double At, double tau2c, double tau2t)
{
  if ( (I1 <= 0.0) && ((sqrt(J2) + (I1/3.0) ) <= 0.0) )
  {
printf("Test C-C %e\n",(J2 + (Ac*I1)/3.0 - tau2c));
    /* compression-compression zone: */
    if ((J2 + (Ac*I1)/3.0 - tau2c) > 0)
       { return(-1) ; /* plastic */ }
  }
  else
  {
    /* other zones: */
printf("Test C-T %e (I1=%e, J2=%e At=%e, tau2t=%e)\n",(J2 - (I1*I1)/6.0 + (At*I1)/3.0 - tau2t),I1,J2, At, tau2t);
    if ((J2 - ((I1*I1)/6.0) + ((At*I1)/3.0) - tau2t) > 0)
       { return(1) ; /* plastic */ }
  }
  return(0) ; /* must be elastic... */
}

/** Computes parameters for intermediate conditions (c or t)
 * @parameters Ay yield A Chen constant
 * @parameters Au ultimate A Chen constant
 * @parameters tau2y yield tau2 Chen constant
 * @parameters tau2u ultimate tau2 Chen constant
 * @return status
 */ 
int chen2d_alpha_beta(double Ay, double Au, double tau2y, double tau2u,
        double *alpha, double *beta)
{
  *alpha = (Au - Ay) / (tau2u - tau2y) ;
  *beta  = ( Ay*tau2u - Au*tau2y ) / (tau2u - tau2y) ; /* TODO: check! */
  return(AF_OK);
}

/** Computes equivalent limits (sigma_c, sigma_bc, sigma_t)
 * for intermediate yield conditions
 *
 * @param I1 1st stress invariant
 * @param J2 2nd stress deviator invariant
 * @param alpha_c alpha intermediate parameter for compression
 * @param beta_c beta intermediate parameter for compression
 * @param alpha_t alpha intermediate parameter for tension
 * @param beta_t beta intermediate parameter for tension
 * @param a relation a=sigma_bc/sigma_c (initial)
 * @param sigma_c equivalent unixal compression limit
 * @param sigma_bc equivalent biaxial compression limit
 * @param sigma_t equivalent unixal tension limit
 * @return status
 */
int chen2d_equivalent_limits(
    double I1, double J2,
    double alpha_c, double beta_c, 
    double alpha_t, double beta_t,
    double a,
    double *sigma_c, double *sigma_bc, double *sigma_t,
    double *tau2c, double *tau2t)
{ /* TODO: verify these equation: */
  *tau2c = fabs((J2 + (beta_c*I1)/3.0) / (1.0 - (alpha_c*I1)/3.0)) ;
  *tau2t = fabs((J2-((beta_t*I1)/3.0)+(I1*I1/6.0))/(1.0-(alpha_t*I1)/3.0)) ;

  *sigma_c  = sqrt ( ((*tau2c)*3.0*(2.0*a -1.0)) / (2.0 - a) );
  *sigma_bc = a * (*sigma_c) ;
  *sigma_t  = ((*tau2t)*6.0) / (*sigma_c) ; 

fprintf(msgout,"EQUIV I1=%e, J2=%e, ac=%e, bc=%e, at=%e bt=%e\n", 
    I1, J2, alpha_c, beta_c, alpha_t, beta_t
    );
  return(AF_OK);
}

/** Computes the derivations of plascicity condition for C-C
 * @param alpha_c alpha intermediate parameter for compression
 * @param beta_c beta intermediate parameter for compression
 * @param alpha_t alpha intermediate parameter for tension
 * @param beta_t beta intermediate parameter for tension
 * @param a relation a=sigma_bc/sigma_c (initial)
 * @param sigma_c equivalent unixal compression limit
 * @param sigma_bc equivalent biaxial compression limit
 * @param sigma_t equivalent unixal tension limit
 * @param sigma_x actual stress Sigma X
 * @param sigma_y actual stress Sigma Y
 * @param tau_xy actual stress TAu xy
 * @param deriv derivatives (result)
 * @return status
 */
int chen2d_derivations_cc(
		double alpha_c, double beta_c,
		double sigma_c, double sigma_bc, double sigma_t, 
		double sigma_x, double sigma_y, double tau_xy,
    tVector *deriv
    )
{
  femVecPut(deriv,1, 
		(-1.0*(sigma_bc - 2.0*sigma_c)*alpha_c*sigma_bc*sigma_c)
		/
		(9.0*(2.0*sigma_bc-sigma_c))
		+ (beta_c/3.0) + (2.0/3.0)*sigma_x - (sigma_y/3.0) );

  femVecPut(deriv,2, 
		(-1.0*(sigma_bc - 2.0*sigma_c)*alpha_c*sigma_bc*sigma_c)
		/
		(9.0*(2.0*sigma_bc-sigma_c))
		+ (beta_c/3.0) + (2.0/3.0)*sigma_y - (sigma_x/3.0) );

  femVecPut(deriv,3, 
    2.0 * tau_xy );
  return(AF_OK);
}

/** Computes the derivations of plascicity condition for C-T
 * @param alpha_c alpha intermediate parameter for compression
 * @param beta_c beta intermediate parameter for compression
 * @param alpha_t alpha intermediate parameter for tension
 * @param beta_t beta intermediate parameter for tension
 * @param a relation a=sigma_bc/sigma_c (initial)
 * @param sigma_c equivalent unixal compression limit
 * @param sigma_bc equivalent biaxial compression limit
 * @param sigma_t equivalent unixal tension limit
 * @param sigma_x actual stress Sigma X
 * @param sigma_y actual stress Sigma Y
 * @param tau_xy actual stress TAu xy
 * @param deriv derivatives (result)
 * @return status
 */
int chen2d_derivations_ct(
		double alpha_t, double beta_t,
		double sigma_c, double sigma_bc, double sigma_t, 
		double sigma_x, double sigma_y, double tau_xy,
    tVector *deriv
    )
{
  femVecPut(deriv,1, 
		(1.0/18.0)*alpha_t*sigma_c*sigma_t - (beta_t/3.0) + sigma_x );

  femVecPut(deriv,2, 
		(1.0/18.0)*alpha_t*sigma_c*sigma_t - (beta_t/3.0) + sigma_y );

  femVecPut(deriv,3, 2.0 * tau_xy ) ;
  return(AF_OK);
}

/** Computes hardening parameter according to Ohtani+Chen */
double chen2d_H_Ohtani(
    long   ePos,
    long   status,
    double E0,
    double s_c,
    double s_bc,
    double s_t,
    double I1,
    double J2
    )
{
  int jj ;
  double a1, a2, a3, Q1, Q2, Q3 ;
  double H1, H2, H3 ;
  double k[4];
  double n[4] ;

  /* alpha multipliers: */
  if (status < 0)
  {
    /* C-C */
    a1 = 1.0 ;
    a2 = 1.0 ;
    a3 = 0.0 ;
  }
  else
  {
    /* other areas */
    if ((I1 <= 0) && ( (sqrt(J2) + I1/3.0) >= 0))
    {
      /* T-T */
      a1 = 0.0 ; 
      a2 = 0.0 ;
      a3 = 1.0 ;
    }
    else
    {
      /* C-T, T-C */
      if ((s_c+s_t) != 0.0)
      {
        a1 = (s_t - I1) / (s_c + s_t)  ;
        a2 = (s_t - I1) / (s_c + s_t)  ;
        a3 = (s_c + I1) / (s_c + s_t)  ;
      }
      else
      {
#ifdef RUN_VERBOSE
				fprintf(msgout,"Impossible stress state in element %li!\n",ePos);
#endif
        a1 = FEM_ZERO ;
        a2 = FEM_ZERO ;
        a3 = FEM_ZERO ;
      }
    }
  }
  
  
  /* get Q derivatives */
  if (status < 0.0)
  {
    /* TODO - check this */
    Q1 = (s_c*s_c-4.0*s_c*s_bc+s_bc*s_bc)*(I1+2.0*s_bc)/(3.0*pow((2.0*s_bc - s_c),2)) ;
    Q2 = (s_c*s_c-4.0*s_c*s_bc+s_bc*s_bc)*(I1+s_bc)/(3.0*pow((2.0*s_bc - s_c),2)) ; /* ? */
    Q3 = 0.0 ;
  }
  else
  {
    Q1 = (1.0/6.0)*(I1 - s_c) ;
    Q2 = 0.0 ;
    Q3 = (1.0/6.0)*(I1 - s_c) ;
  }

	/* obtain k, n HERE */
  for (jj=1; jj<=3; jj++)
  {
    k[jj] = femGetMPValPos(ePos, MAT_RAMB_K, jj) ;
    n[jj] = femGetMPValPos(ePos, MAT_RAMB_N, jj) ;
fprintf(msgout,"KN[%i] %e %e\n",jj, k[jj], n[jj]);
  }

printf("Stresses s_c=%e s_bc=%e s_t=%e\n",s_c,s_bc,s_t);

  H1 = ( E0*pow(s_c/E0, 1.0 - n[1]) ) / ( k[1] * n[1] ) ;  /* C  */
  H2 = ( E0*pow(s_bc/E0, 1.0 - n[2]) ) / ( k[2] * n[2] ) ; /* BC */
  H3 = ( E0*pow(s_t/E0, 1.0 - n[3]) ) / ( k[3] * n[3] ) ;  /* T  */

fprintf(msgout,"H1=%e H2=%e H3=%e(%e)\n", H1, H2, H3 , s_t/E0);

  return( a1*Q1*(H1) + a2*Q2*(H2) + a3*Q3*(H3) );
}

/** computes stiffness matrix of Chen material
 * @param ePos element index
 * @param e_rem integration point
 * @param Problem - problem type (ignored here)
 * @param epsilon shear vector [3]
 * @param Mode if ==AY_YES then new matrix is created
 * @param Dep pointer to (allocated!) stiffness matrix (resul)
 */
int chen2d_D(long ePos, long e_rep, long Problem,
    tVector *epsilon, long Mode, 
    tMatrix *Dep)
{
  int rv = AF_OK ;
  long i, j ;
  double Ex, nu ;
  double f_yc, f_ybc, f_yt, f_uc, f_ubc, f_ut ;
  double alpha_c, beta_c, alpha_t, beta_t, a, H , sum ;
  double Auc, Aut, tauu2c, tauu2t ;
  double Ayc, Ayt, tauy2c, tauy2t ;
  double tau2c, tau2t ;
  double sigma_c, sigma_bc, sigma_t ;
  long   status = 0 ;
  long   status_old = 0 ;
  double I1, J2 ;
  tVector sigma ;
  tVector sigma_ps ;
  tVector deriv ;
  tMatrix De ;

  femVecNull(&sigma) ;
  femVecNull(&sigma_ps) ;
  femMatNull(&De) ;

  if ((rv=femVecFullInit(&sigma, 3)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&sigma_ps, 3)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&deriv, 3)) != AF_OK){goto memFree;}
  if ((rv=femFullMatInit(&De, 3, 3)) != AF_OK){goto memFree;}

  /* get current material status: 0=elastic ; -1=CC; +1=other */
	status = (long) femGetEResVal(ePos, RES_STAT1, e_rep);

  Ex = femGetMPValPos(ePos, MAT_EX, 0)  ;
  nu = femGetMPValPos(ePos, MAT_NU, 0)  ;

  f_yc  = femGetMPValPos(ePos, MAT_F_YC, 0)  ;
  f_ybc = femGetMPValPos(ePos, MAT_F_YBC, 0) ;
  f_yt  = femGetMPValPos(ePos, MAT_F_YT, 0)  ;

  f_uc  = femGetMPValPos(ePos, MAT_F_UC, 0)  ;
  f_ubc = femGetMPValPos(ePos, MAT_F_UBC, 0) ;
  f_ut  = femGetMPValPos(ePos, MAT_F_UT, 0)  ;

  a = 0.5 * ( (f_ubc/f_uc) + (f_ybc / f_yc) ) ;

  H = 0.0 ;

  /* get initial/previous matrix */
  if (status == 0)
  {
    /* elastic */
    D_HookIso_planeRaw(Ex, nu, Problem, Dep);
  }
  else
  {
    femVecPut(&deriv,1, femGetEResVal(ePos, RES_ECR1, e_rep)); 
    femVecPut(&deriv,2, femGetEResVal(ePos, RES_ECR2, e_rep)); 
    femVecPut(&deriv,3, femGetEResVal(ePos, RES_GCR1, e_rep)); 
	  H = femGetEResVal(ePos, RES_DIR3, e_rep);

    D_HookIso_planeRaw(Ex, nu, Problem, &De);
    chen_Dep(&deriv, H, &De, Dep) ;
  }
    
  
  if (Mode == AF_YES) /* new matrix */
  {
    status_old = status ;

    /* get current total stress: */
    femMatVecMult(Dep, epsilon, &sigma) ;

    femVecAdd(&sigma,1, femGetEResVal(ePos, RES_SX, e_rep)); 
    femVecAdd(&sigma,2, femGetEResVal(ePos, RES_SY, e_rep)); 
    femVecAdd(&sigma,3, femGetEResVal(ePos, RES_SXY, e_rep)); 

    I1 =  stress2D_I1(&sigma) ;
    J2 =  stress2D_J2(&sigma) ;

    /* get data for yield condition */
    chen2d_get_A_tau2_y(f_yc, f_ybc, f_yt,
		  &Ayc, &Ayt, &tauy2c, &tauy2t) ;

    /* check plasticity condition */
    status =  chen2d_check_yield(I1, J2, Ayc, Ayt, tauy2c, tauy2t);

printf("STATUS[%li,%li] = %li\n", ePos, e_rep, status);

    if (status == 0)
    {
      if (status_old == 0)
      {
        /* elastic matrix - nothing to do */
      }
      else
      {
        /* must be unloading */
        femMatSetZero(Dep) ;
        femMatCloneDiffToEmpty(&De, Dep);
      }
    }
    else /* plastic */
    {
      /* TODO: check unloading somewhere here! */
      
      chen2d_get_A_tau2_y(f_uc, f_ubc, f_ut,
		    &Auc, &Aut, &tauu2c, &tauu2t) ;

      chen2d_alpha_beta(Ayc, Auc, tauy2c, tauu2c, &alpha_c, &beta_c) ;
      chen2d_alpha_beta(Ayt, Aut, tauy2t, tauu2t, &alpha_t, &beta_t) ;

      for (j=0; j<100; j++) /* loop for precision */
      {
        /* compute intermediate equation parameters: */
        chen2d_equivalent_limits( I1, J2,
            alpha_c, beta_c, alpha_t, beta_t, a,
            &sigma_c, &sigma_bc, &sigma_t,
            &tau2c, &tau2t) ;

        /* derivatives */
        chen2d_derivations_ct( alpha_t, beta_t,
            sigma_c, sigma_bc, sigma_t, 
            femVecGet(&sigma, 1), 
            femVecGet(&sigma, 2), 
            femVecGet(&sigma, 3),
            &deriv );

        /* get H: */
#if 1
        H = chen2d_H_Ohtani(ePos, status, Ex,
            sigma_c, sigma_bc, sigma_t, I1, J2) ;
#else
        H = 1E6 * FEM_ZERO ;
#endif
fprintf(msgout,"H is %e\n", H);

        /* get elastoplastic matrix */
        chen_Dep(&deriv, H, &De, Dep) ;
      
        /* get NEW current total stress: */
        femMatVecMult(Dep, epsilon, &sigma) ;

        femVecAdd(&sigma,1, femGetEResVal(ePos, RES_SX, e_rep)); 
        femVecAdd(&sigma,2, femGetEResVal(ePos, RES_SY, e_rep)); 
        femVecAdd(&sigma,3, femGetEResVal(ePos, RES_SXY, e_rep)); 

        /* precision control:  */
        sum = 0 ;
        for (i=1; i<=3; i++)
        {
          sum += pow(femVecGet(&sigma_ps, i)-femVecGet(&sigma, i), 2);
        }
        sum = sqrt(sum) ;

        if (sum < 1e-6)
        {
          break ; /* OK */
        }
        else
        {
          femVecClone(&sigma, &sigma_ps) ; /* save current vector */

          /* new: I1, J2 */
          I1 =  stress2D_I1(&sigma) ;
          J2 =  stress2D_J2(&sigma) ;
        }
      }
    } /* if plastic */

    /* TODO */

    /* save status */
	  femPutEResVal(ePos, RES_STAT1, e_rep, (double)status);
    femPutEResVal(ePos, RES_ECR1, e_rep, femVecGet(&deriv,1)); 
    femPutEResVal(ePos, RES_ECR2, e_rep, femVecGet(&deriv,2)); 
    femPutEResVal(ePos, RES_GCR1, e_rep, femVecGet(&deriv,3)); 
	  femPutEResVal(ePos, RES_DIR3, e_rep, H );
  }

memFree:
  femVecFree(&sigma_ps);
  femVecFree(&sigma);
  femVecFree(&deriv);
  femMatFree(&De);
  return(rv);
}

/* end of fem_ch2d.c */

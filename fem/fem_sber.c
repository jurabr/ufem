/*
   File name: fem_sber.c
   Date:      Thu Oct 22 20:00:46 CEST 2009
   Author:    Jiri Brozovsky

   Copyright (C) 2005 Jiri Brozovsky

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

   An analog to the "SBETA" material model (Cervenka et al)
   Rotating crack model - divided from fem_sbet.c
*/

/* #define DEVEL_SBET */

#ifdef DEVEL_SBET
#define DEVEL_VERBOSE 1
#endif

#include "fem_elem.h"
#include "fem_mat.h"
#include "fem_math.h"

#ifndef _SMALL_FEM_CODE_

extern double sbet_zero ;

extern int femGetPrincStress2D(tVector *sigma_x, tVector *sigma_1, double *phi);
extern int D_Plane_OrthoCB(double R1, double R2, double nu, double G, double beta, double fi_1, tMatrix *Dcr);
extern int D_Plane_Ortho(double R1, double R2, double nu, double beta, double fi_1, tMatrix *Dcr);
extern int D_Plane_OrthoCerv(double R1, double R2, double nu, double G, double beta, double fi_1, tMatrix *Dcr);
extern int femTs_2d(tMatrix *Ts, double fi0);
extern int femTe_2d(tMatrix *Ts, double fi0);

extern double sb1d_get_Rt_tension(long ePos,
    double E0, 
    double smax, 
    double Gf, 
    double L, 
    double epsilon);

int sbet_rotcr_get_D(
    long ePos, 
    long iPoint, 
    double A, 
    tVector *epsilon, 
    tVector *sigma, 
    tVector *sigma_r, 
    long Mode, 
    long Problem, 
    tMatrix *D);



/** Status numbers:
 *  1: elastic tension
 *  2: softening in tension (cracking)
 *  3: elastic compression
 *  4: softening in compression (crushing)
 *  5: completely damaged (after cracking+crushing)
 * -1: unloading in tension
 * -3: unloading in compression
 */ 

/* What IS NOT included:
 * - reinforcement
 * - concrete-reinforcement interaction
 * - E1-E2 interaction
 * - cyclic loading
 * ...
 */ 


/** compute fc and ft according to Kupfer
 * @param s1 sigma 1 stress
 * @param s2 sigma 2 stress
 * @param f_ccyl cyllindric compression strenght (>0 ?)
 * @param f_tu uniaxial tension strenght (>0)
 * @param ttype tension-tension curve type [0,1,2]
 * @param fc pointer to compression limit
 * @param fc pointer to tension limit
 * */
extern int comp_2d_limits(
    double s1, 
    double s2,
    double f_ccyl,
    double f_tu,
    long   ttype,
    double *fc,
    double *ft
    );

/**TODO Computes E in compression (untested)
 * @param epsilon epsilon size in given direction
 * @param E0 initial Young modulus size
 * @param Ec Young modulus in compression (TODO: check this!)
 * @param fc compression strenght
 * @param L  lenght of cracking zone
 * @return modulus of cracking concrete
 */
extern double sbet_compr_E(
    double epsilon,
    double E0, 
    double Ec, 
    double fc,
    double L,
		long stat
    );

/**TODO Computes E in compression (untested)
 * @param epsilon epsilon size in given direction
 * @param E0 initial Young modulus size
 * @param Ec Young modulus in compression (TODO: check this!)
 * @param fc compression strenght
 * @return modulus of cracking concrete
 */
extern double sbet_compr_plast_E(
    double epsilon,
    double E0, 
    double Ec, 
    double fc,
		long   stat
    ) ;

/** TODO Compute shear reduction coefficient
 */
extern double sbeg_Gbeta(
    long type, 
    double p, 
    double eps,
    double eps_max,
    double Ecr,
    double E0
    ) ;


/** Material stiffness matrix according to Hook (plane stress)
 * @param E Young modullus
 * @param nu Poisson ratio
 * @param D matrix (result)
 */
extern void sbet_HookD(double E, double nu, tMatrix *D) ;

/** Computes Cervenka's skew factor (works ONLY for element 002!)
 * @param angle crack direction
 * @param ePos  element position
 * @return skew factor size
 */
extern long sbet_skew_factor_2D(double angle, long ePos) ;


/** TODO Computes D matrix for ROTATING CRACKS MODEL TODO
 * @param ePos element index
 * @param iPoint integration point index
 * @param A element area
 * @param epsilon strain vector
 * @param sigma total stress vector (result, if newM=AF_YES)
 * @param sigma_r residual stress vector (result, if newM=AF_YES)
 * @param newM - if AF_YES the make new matrix, else make old
 * @param Problem problem type (ignored - plane stress assumed)
 * @param D pointer to (allocated!!!) D matrix (result)
 */ 
int sbet_rotcr_get_D(
    long ePos, 
    long iPoint, 
    double A, 
    tVector *epsilon, 
    tVector *sigma, 
    tVector *sigma_r, 
    long Mode, 
    long Problem, 
    tMatrix *D)
{
  int rv = AF_OK ;
#if 0
  long prev_stat1, stat1 ;
  long prev_stat2, stat2 ;
  double E0, E1, E2, beta, angle, angle_e, epsilon01, epsilon02, nu, G ;
  double Gf, f_ccyl, f_tu, fc, ft, L ;
  double E1_old, E2_old, beta_old ;
  long i ;
	long kupf_type = 1 ;

  tVector tot_epsilon ;
  tVector tot_epsilon_1 ;
  tVector sigma_x ;
  tVector sigma_1 ;
	tMatrix Ts;

  /**
   * E1=Rt   ... RES_ECR1 (main direction stiffness) .. 28
   * E2      ... RES_ECR2 (stiffness in other direction) .. 29
   * status1 ... RES_STAT1 (material status 1st dir) .. 63
   * status2 ... RES_STAT2 (material status 2nd dir) .. 64
   * beta    ... RES_GCR1 (shear reduction coefficient) .. 31
   * angle   ... RES_DIR1 (angle of cracks [rad]) .. 25
   * epsilon1... RES_DIR2 (epsilon in direction of cracks) .. 26
   * epsilon2... RES_DIR3 (epsilon in other direction) .. 27
   * fc      ... RES_SCMAX (2D limit compression stress from Kupfer) .. 62
   * ft      ... RES_STMAX (2D limit tension stress from Kupfer) .. 61
   */ 

	femMatSetZero(D);

	prev_stat1 = (long) femGetEResVal(ePos, RES_STAT1, iPoint+1) ;
  stat1      = prev_stat1 ;

	prev_stat2 = (long) femGetEResVal(ePos, RES_STAT2, iPoint+1) ;
  stat2      = prev_stat2 ;

	E0        = femGetMPValPos(ePos, MAT_EX, 0) ;
	nu        = femGetMPValPos(ePos, MAT_NU, 0) ;
  G         = E0 / (2.0*(1.0+nu)) ;

	kupf_type = femGetMPValPos(ePos, MAT_FCTYPE, 0) ; if (kupf_type < 0) { kupf_type = 0 ;}

	E1        = femGetEResVal(ePos, RES_ECR1, iPoint+1) ;
	E2        = femGetEResVal(ePos, RES_ECR2, iPoint+1) ;
	beta      = femGetEResVal(ePos, RES_GCR1, iPoint+1) ;
	angle     = femGetEResVal(ePos, RES_DIR1, iPoint+1) ;
	epsilon01 = femGetEResVal(ePos, RES_DIR2, iPoint+1) ;
	epsilon02 = femGetEResVal(ePos, RES_DIR3, iPoint+1) ;

	L         = sqrt(A) ; /* TODO: check this! */

	if ((prev_stat1 == prev_stat2) && ((prev_stat1 == 1)||(prev_stat1 == 0)))
  {
    /* initial linear behaviour */
		E1 = E0 ;
		E2 = E0 ;
		beta = 1.0 ;
		angle = 0.0 ;
  }

	/* convergence stuff: */
#if 1
	if (E1 < FEM_ZERO*E0) {E1 = FEM_ZERO*E0;}
	if (E2 < FEM_ZERO*E0) {E2 = FEM_ZERO*E0;}
	if (beta < FEM_ZERO) {beta = FEM_ZERO ;}
#endif

	if ((stat1 != 2)&&(stat1 != -1)) { beta = 1.0 ; }
	else                             { beta = 0.0 ; }
	

	/* Compute old matrix */
#if 0
  D_Plane_OrthoCB(E1, E2, nu, G, beta, angle, D);
#else
  D_Plane_Ortho(E1, E2, nu, beta, angle, D);
#endif

	/* new matrix */
	if (Mode == AF_YES)
  {
    E1_old = E1 ;
    E2_old = E2 ;
    beta_old = beta ;

		femVecSetZero(sigma_r);
		femVecSetZero(sigma);

		/* pre-iterational stresses */
    femMatVecMult (D, epsilon, sigma_r) ;
		femVecAdd(sigma_r,1, femGetEResVal(ePos,RES_SX, iPoint+1));
    femVecAdd(sigma_r,2, femGetEResVal(ePos,RES_SY, iPoint+1));
    femVecAdd(sigma_r,3, femGetEResVal(ePos,RES_SXY,iPoint+1));

    f_ccyl = femGetMPValPos(ePos, MAT_SCMPR, 0);
    f_tu   = femGetMPValPos(ePos, MAT_STEN, 0);

    Gf     = femGetMPValPos(ePos, MAT_GF, 0) ;

		/* data preparation */
	  femVecNull(&tot_epsilon);
	  femVecNull(&tot_epsilon_1);
	  femVecNull(&sigma_x);
	  femVecNull(&sigma_1);
        
    femMatNull(&Ts) ;

	  if ((rv=femVecFullInit(&tot_epsilon, 3)) != AF_OK) {goto memFree; }
	  if ((rv=femVecFullInit(&tot_epsilon_1, 3)) != AF_OK) {goto memFree; }
	  if ((rv=femVecFullInit(&sigma_x, 3)) != AF_OK) {goto memFree; }
	  if ((rv=femVecFullInit(&sigma_1, 3)) != AF_OK) {goto memFree; }

	  if ((rv = femFullMatInit(&Ts,3,3)) != AF_OK) { goto memFree; }

		/* total strain and stress computation: */
	  femVecPut(&tot_epsilon,1,femGetEResVal(ePos, RES_EX, iPoint+1));
	  femVecPut(&tot_epsilon,2,femGetEResVal(ePos, RES_EY, iPoint+1));
	  femVecPut(&tot_epsilon,3,femGetEResVal(ePos, RES_EXY, iPoint+1));
    for (i=1; i<=3; i++) 
      { femVecAdd(&tot_epsilon, i, femVecGet(epsilon, i)) ; }

		/* OK /|\ ?*/

#if 0
		/* computing of the state of initially undamaged material  */
		if ( ((prev_stat1 == 1)||(prev_stat1 == 0)||(prev_stat1 == 3)) &&
				((prev_stat2 == 1)||(prev_stat2 == 0)||(prev_stat2 == 3)) )
		{
#endif
			femMatVecMult( D, &tot_epsilon, &sigma_x ) ;
			femGetPrincStress2D(&sigma_x, &sigma_1, &angle); /* also NEW angle */

			/* Kupfer */
			comp_2d_limits(
         femVecGet(&sigma_1, 1), 
         femVecGet(&sigma_1, 2), 
         f_ccyl,
         f_tu,
         kupf_type, /* can be 0,1,2 */
         &fc,
         &ft
       );

			/* checking stat change: */
			if (ft < femVecGet(&sigma_1,1)) { stat1 = 2 ; }
			if ((-1.0*fabs(fc)) > femVecGet(&sigma_1,1) && (femVecGet(&sigma_1,1) < 0.0)) { stat1 = 4 ; }

			if (ft < femVecGet(&sigma_1,2)) { stat2 = 2 ; }
			if (((-1.0*fabs(fc)) > femVecGet(&sigma_1,2)) && (femVecGet(&sigma_1,2) < 0.0)) { stat2 = 4 ; }
			
#if 0 /* strain direction can differ from stress direction! */
			femTe_2d(&Ts, angle) ;
		  femMatVecMult(&Ts, &tot_epsilon, &tot_epsilon_1);
#else
			femGetPrincStress2D(&tot_epsilon, &tot_epsilon_1, &angle_e);
#endif

#if 0
		}
		else
		{
			fc = femGetEResVal(ePos, RES_SCMAX, iPoint+1) ;
			ft = femGetEResVal(ePos, RES_STMAX, iPoint+1) ;

			/* previously damaged material */
			femTs_2d(&Ts, angle) ;
		  femMatVecMult(&Ts, &tot_epsilon, &tot_epsilon_1);

			/* sigma in previous direction: */
			femMatVecMult( D, &tot_epsilon, &sigma_x ) ;
		  femMatVecMult(&Ts, &sigma_x, &sigma_1);
#endif

			/* check for unloading */
			if ((prev_stat1 == 2) || (prev_stat1 == -1))
			{
				if (epsilon01 > femVecGet(&tot_epsilon_1, 1)) { stat1 = -1 ; }
				else { stat1 = 2 ; }
			}

			if ((prev_stat1 == 4) || (prev_stat1 == -3))
			{
				if (epsilon01 < femVecGet(&tot_epsilon_1, 1)) { stat1 = -3 ; }
				else { stat1 = 4 ; }

				if (femVecGet(&tot_epsilon_1, 1) > 0.0) {stat1 = 5 ;} 
			}


			if ((prev_stat2 == 2) || (prev_stat2 == -1))
			{
				if (epsilon02 > femVecGet(&tot_epsilon_1, 2)) { stat2 = -1 ; }
				else { stat2 = 2 ; }
			}

			if ((prev_stat2 == 4) || (prev_stat2 == -3))
			{
				if (epsilon02 < femVecGet(&tot_epsilon_1, 2)) { stat2 = -3 ; }
				else { stat2 = 4 ; }

				if (femVecGet(&tot_epsilon_1, 2) > 0.0) {stat2 = 5 ;} 
			}

			/* check of damage in undamaged direction */
			if ((prev_stat1 == 1) || (prev_stat1 == 0) || (prev_stat1 == 3))
			{
				if (ft < femVecGet(&sigma_1,1)) { stat1 = 2 ; }
				if (((-1.0*fabs(fc)) > femVecGet(&sigma_1,1)) && (femVecGet(&sigma_1,1) < 0.0)) 
					 { stat1 = 4 ; }
			}

			if ((prev_stat2 == 1) || (prev_stat2 == 0) || (prev_stat2 == 3))
			{
				if (ft < femVecGet(&sigma_1,2)) { stat2 = 2 ; }
				if (((-1.0*fabs(fc)) > femVecGet(&sigma_1,2)) && (femVecGet(&sigma_1,2) < 0.0)) 
					 { stat2 = 4 ; }
			}
#if 0
		}
#endif

		/* computation of E1 */ 
		switch (stat1)
		{
			case 2: E1 = sb1d_get_Rt_tension(
									ePos, E0, ft, Gf, L, 
									femVecGet(&tot_epsilon_1, 1) );
							break ;

			case 4: 
#if 1
							E1 = sbet_compr_E( femVecGet(&tot_epsilon_1,1),
									            E0, E0, fc, L, stat1);
#else
							E1 = sbet_compr_plast_E( femVecGet(&tot_epsilon_1,1),
									            E0, E0, fc, stat1);
#endif
							break;

			case 5:	E1 = 0.0 ; beta =0.0 ; break ;

			case -1: /* previous values are correct here */ break ;
			case -3: /* previous values are correct here -- TODO: check this! */ break ;

			case 3: /* 3 might have to be changed later! */
			case 1:
			case 0:
			default:	E1 = E0 ; break ;
		}

		/* computation of E2 */ 
		switch (stat2)
		{
			case 2: E2 = sb1d_get_Rt_tension(
									ePos, E0, ft, Gf, L, 
									femVecGet(&tot_epsilon_1, 2) );
							break ;

			case 4: 
#if 1
							E2 = sbet_compr_E( femVecGet(&tot_epsilon_1, 2) ,
									            E0, E0, fc, L,stat2);
#else
							E2 = sbet_compr_plast_E( femVecGet(&tot_epsilon_1, 2) ,
									            E0, E0, fc,stat2);
#endif
							break;

			case 5:	E2 = 0.0 ; beta = 0.0 ; break ;

			case -1: /* previous values are correct here */ break ;
			case -3: /* previous values are correct here -- TODO: check this! */ break ;

			case 3: /* 3 might have to be changed later! */
			case 1:
			case 0:
			default:	E2 = E0 ; break ;
		}

    /* E quality control (the cannot be better, only worse): */
    if (E1_old > 0.0) { if (E1 > E1_old) { E1 = E1_old ;}}
    if (E2_old > 0.0) {if (E2 > E2_old) { E2 = E2_old ;}}

		femMatSetZero(D);

		/* new D matrix */
		if ((stat1 != 2)&&(stat1 != -1)) { beta = 1.0 ; }
		else                             { beta = 0.0 ; }

		/* computation of residual beta */
		if ((stat1 != 0)&&(stat1 != 1)&&(stat1 != -3))
		{
			if (fabs(femVecGet(&tot_epsilon_1,1)-femVecGet(&tot_epsilon_1,2)) <= FEM_ZERO)
			{
				beta = FEM_ZERO ;
			}
			else
			{
				beta = fabs(femVecGet(&tot_epsilon_1,1)-femVecGet(&tot_epsilon_1,2))  /
					fabs( 2.0 * (femVecGet(&tot_epsilon_1,1)-femVecGet(&tot_epsilon_1,2)) ) ;
	
				if (beta < 0.0) 
				{
					beta = 0.0 ;
				}
				else
				{
					if (beta > G)
					{
						beta = 1.0 ;
					}
					else
					{
						beta = beta / G ;
					}
				}
			}
		}


#if 0
  	D_Plane_OrthoCB(E1, E2, nu, G, beta, angle, D); 
#else
  	D_Plane_Ortho(E1, E2, nu, beta, angle, D); 
#endif

		/* correct actual stress: */
    femMatVecMult (D, &tot_epsilon, sigma) ;

#if 1
		for (i=1; i<=3; i++)
    {
      femVecPut(sigma_r, i,
          (femVecGet(sigma_r, i) - femVecGet(sigma,i )) );
    }
#else
		femVecSetZero(sigma_r);
#endif

	  femPutEResVal(ePos, RES_USX,  iPoint+1, femVecGet(sigma_r,1)) ;
	  femPutEResVal(ePos, RES_USY,  iPoint+1, femVecGet(sigma_r,2)) ;
	  femPutEResVal(ePos, RES_USXY, iPoint+1, femVecGet(sigma_r,3)) ;

    femVecAdd(sigma, 1, (-1.0)*femGetEResVal(ePos, RES_SX,  iPoint+1));
	  femVecAdd(sigma, 2, (-1.0)*femGetEResVal(ePos, RES_SY,  iPoint+1));
	  femVecAdd(sigma, 3, (-1.0)*femGetEResVal(ePos, RES_SXY, iPoint+1));


    /* material status data */
    femPutEResVal(ePos, RES_STAT1, iPoint+1, stat1) ;
    femPutEResVal(ePos, RES_STAT2, iPoint+1, stat2) ;

	  femPutEResVal(ePos, RES_ECR1, iPoint+1, E1) ;
	  femPutEResVal(ePos, RES_ECR2, iPoint+1, E2) ;
	  femPutEResVal(ePos, RES_DIR1, iPoint+1, angle) ;
	  femPutEResVal(ePos, RES_GCR1, iPoint+1, beta) ;
	  femPutEResVal(ePos, RES_DIR2, iPoint+1, femVecGet(&tot_epsilon_1,1)) ;
	  femPutEResVal(ePos, RES_DIR3, iPoint+1, femVecGet(&tot_epsilon_1,2)) ;

	  femPutEResVal(ePos, RES_SCMAX, iPoint+1, fc);
	  femPutEResVal(ePos, RES_STMAX, iPoint+1, ft);
#if 0
printf("E1=%e, E2=%e, alpha=%e s1=%e, s1r=%e e=%e te=%e\n",E1,E2,angle,
				femVecGet(sigma,1),femVecGet(sigma_r,1),femVecGet(epsilon,1),femVecGet(&tot_epsilon,1));
#endif

memFree:
	  femVecFree(&tot_epsilon);
	  femVecFree(&tot_epsilon_1);
	  femVecFree(&sigma_x);
	  femVecFree(&sigma_1);
        
    femMatFree(&Ts) ;
	}
 
#endif
  return(rv);
}


#endif /* SMALL_FEM_CODE */
/* end of fem_sber.c */

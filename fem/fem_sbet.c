/*
   File name: fem_sbet.c
   Date:      2005/12/20 19:58
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
*/

/* #define DEVEL_SBET */

#ifdef DEVEL_SBET
#define DEVEL_VERBOSE 1
#endif

#include "fem_elem.h"
#include "fem_mat.h"
#include "fem_math.h"

#ifndef _SMALL_FEM_CODE_

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


double sbet_zero = FEM_ZERO ;

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
int comp_2d_limits(
    double s1, 
    double s2,
    double f_ccyl,
    double f_tu,
    long   ttype,
    double *fc,
    double *ft
    )
{
  double r ;
  double A,B,x,K ;

  *fc = f_ccyl ;
  *ft = f_tu ;

  /* compression-compression */
	if ((s1 < 0.0) && (s2 < 0.0))
  {
   	A   = s1 / s2 ;
    *fc = ((1.0 + 3.65*A) / pow(1.0+A, 2)) * f_ccyl ;
  }
	else
	{
  	/* compression-tension (compression strenght) */
		if ((s1*s2) <= 0.0)
		{
  		/* (compression strenght) */
			r = (1.0 - 5.3278*fabs(s1/f_ccyl)) ;

    	if (r > 1.0) {r = 1.0;}
    	if (r < 0.9) {r = 0.9;}
		
    	*fc = f_ccyl*r ;

			r = 0.0 ;

  		/* (tension strenght) */
    	switch (ttype)
    	{
     		case 1:
       		x = fabs(s2/f_ccyl) ;
       		A = 0.75 ;
       		K = 1.125 ;
	
       		B = K*x + A ;
       		r = (A + (A-1.0)*B)/(A*B);
       		break ;
	
     		case 2:
       		x = fabs(s2/f_ccyl) ;
       		A = 1.0635 ;
       		K = 6.0208;
	
       		B = K*x + A ;
       		r = (A + (A-1.0)*B)/(A*B);
       		break ;

     		case 0:
				default:
       		r = 1.0 - 0.8*fabs((s2/f_ccyl));
       		break ;
    	}
		
    	*ft = f_tu*r ;
		}
		else
		{
			/* default sizes (tension-tension zone) */
  		*fc = f_ccyl ;
  		*ft = f_tu ;
		}
	}

  *fc = fabs( *fc );
  *ft = fabs( *ft );

  return(AF_OK);
}

/**TODO Computes E in compression (untested)
 * @param epsilon epsilon size in given direction
 * @param E0 initial Young modulus size
 * @param Ec Young modulus in compression (TODO: check this!)
 * @param fc compression strenght
 * @param L  lenght of cracking zone
 * @return modulus of cracking concrete
 */
double sbet_compr_E(
    double epsilon,
    double E0, 
    double Ec, 
    double fc,
    double L,
		long stat
    )
{
  double e_c ;
  static double sig_ef;
	static double sigma;
	double x, k ;
  double e_d ; /* max. compr. strain */
  double Ez ;
  double w_d = 0.5 ; /* plastic strain for concrete ... van Mier */

#if 0
printf("EPSILON=%e\n",epsilon);
#endif

	if (epsilon > 0.0)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[w] %s: %e, %li!\n",_("Compression E used in tension zone"),epsilon,stat);
#endif
		return(E0);
	}

	e_c = fabs ( fc / Ec ) ;

  /* epsilon on peak */
  if (fabs(epsilon) <= e_c )
  {
    /* elastic zone - CEB-FIP Model Code 90 */
    x = (epsilon) / (e_c) ; /* normalized strain */
    k = E0 / Ec ;
    sig_ef = fc * ( (k*x - x*x) / (1.0 + (k-2.0)*x ) ) ;

#if 1
printf("SIGMA PRE %e, x = %e, k = %e (%e/%e)\n",fabs(sig_ef/epsilon),x,k,epsilon,e_c);
#endif
    return ( fabs (sig_ef / epsilon) );
  }
  else
  {
    /* plastic (post-peak) zone */
    e_d = e_c / w_d ;

		if ((epsilon < 0.0)&&(fabs(epsilon) > fabs(e_d))) {return(0.0);}
		
    Ez = fabs( fc / (e_d - e_c) ) ;
    /** sigma = -1.0*epsilon*Ez - fc*(1.0 - Ez/Ec); **/

		sigma = -fc + Ez * ( fabs(epsilon) - fabs(fc/Ec) ) ;
		if (sigma > 0.0) {sigma = 0.0 ;}

#if 1
printf("SIGMA POST %e (Ez = %e)%e(%e..%e..%e)\n",fabs(sigma/epsilon),Ez,epsilon,e_c,w_d,e_d);
#endif
    return ( fabs (sigma / epsilon) ) ;
  }

  /* impossible to reach this: */
  return(E0);
}

/**TODO Computes E in compression (untested)
 * @param epsilon epsilon size in given direction
 * @param E0 initial Young modulus size
 * @param Ec Young modulus in compression (TODO: check this!)
 * @param fc compression strenght
 * @return modulus of cracking concrete
 */
double sbet_compr_plast_E(
    double epsilon,
    double E0, 
    double Ec, 
    double fc,
		long   stat
    )
{
  double e_c ;

	if (epsilon >= 0.0)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[w] %s: %e, %li!\n",_("Compression E used in tension zone"),epsilon,stat);
#endif
		return(E0);
	}

	e_c = fabs ( fc / Ec) ;

  /* epsilon on peak */
  if (fabs(epsilon) <= e_c )
  {
    return ( fabs (E0) );
  }
  else
  {
    return ( fabs (fc / epsilon) ) ;
  }

  /* impossible to reach this: */
  return(E0);
}

/** TODO Compute shear reduction coefficient
 */
double sbeg_Gbeta(
    long type, 
    double p, 
    double eps,
    double eps_max,
    double Ecr,
    double E0
    )
{
	static double val = 0.0 ;

  switch (type)
  {
    case 0: return(1.0);
            break;
    case 2: /* Damjanic + Owen */
						if (fabs(eps/eps_max) > 1.0){return(0.0);}
            return(1.0 - fabs(eps / eps_max));
            break;
    case 3: /* Cervenka + Brdecko */
            return( 1.0 - pow( (eps / 0.005), 0.3) );
            break ;
    case 4: /* Cervenka + Mang */
            return( 1.0 - pow( (eps / 0.005), 0.2) );
            break ;
    case 5: /* Glemberg + Samuelsson */
            val =  Ecr / E0  ;
            if (val <= 0.0) {val = 2.0*FEM_ZERO ;}
            if (val >= 1.0) {val = 1.0 ;}
            return(val);
            break ;
		case 6: return(0.001); break;
		case 1: /* Kolmar */
		case 7: if (eps <= 0.0) {return(1.0);}
						val = ( 
							-1.0 * log((1000.0*eps)
									/
									(7.0+(333.0*0.005))) / (10.0 - 167.0*0.005) ) ;
						if (val > 1.0) {val = 1.0 ;}
						if (val < 0.0) {val = 0.0 ;}
						return(val);
						break;
    default: return(1.0);
             break ;
  }
  return(1.0);
}


/** Material stiffness matrix according to Hook (plane stress)
 * @param E Young modullus
 * @param nu Poisson ratio
 * @param D matrix (result)
 */
void sbet_HookD(double E, double nu, tMatrix *D)
{
  double Mul ;

  Mul = E / (1 - (nu*nu)) ;

  femMatPut(D,1,1,   (Mul) ) ;
  femMatPut(D,1,2,   (Mul * nu) ) ;
  femMatPut(D,2,1,   (Mul * nu) ) ;
  femMatPut(D,2,2,   (Mul) ) ;
  femMatPut(D,3,3,   (Mul*0.5*(1 - nu)) ) ;
}

/** Computes Cervenka's skew factor (works ONLY for element 002!)
 * @param angle crack direction
 * @param ePos  element position
 * @return skew factor size
 */
long sbet_skew_factor_2D(double angle, long ePos)
{
	static double skew_fact = 1.0 ;
	double skew_angle ;
	double max_fact = 1.5 ;
	double x1,x2,y1,y2 ;
	double ang[4], ang1, ang2, ang0 ;
	int    i, i1 ;

	for (i=0; i<4; i++)
	{
		i1 =i+1 ;
		if (i1 == 4) { i1 = 0 ;}
		x1 = femGetNCoordPosX(femGetENodePos(ePos, i)) ;
		x2 = femGetNCoordPosX(femGetENodePos(ePos, i1)) ;
		y1 = femGetNCoordPosY(femGetENodePos(ePos, i)) ;
		y2 = femGetNCoordPosY(femGetENodePos(ePos, i1)) ;
		
		if (fabs(x2-x1) <= FEM_ZERO) { ang[i] = 90.0 ; }
		else                         { ang[i] = fabs ( (y2-y1)/(x2-x1) ) ; }
	}

	ang1 =  0.5 * (ang[0]+ang[2] ) ;
	ang2 =  0.5 * (ang[1]+ang[3] ) ;

#if 0
	printf("A:  %e %e %e %e\n", ang[0], ang[1], ang[2], ang[3]);
	printf("A! = %e A2 =%e\n", ang1, ang2);
#endif

	if (ang2 < ang1) { ang0 = ang2 ; }
	else             { ang0 = ang1 ; }

	skew_angle = fabs(angle - ang0) ;
	if (skew_angle > 45.0) {skew_angle = 90.0 - skew_angle ;}

	skew_fact = 1.0 + ( max_fact - 1.0 ) * (skew_angle / 45.0) ;

#if 0
	printf("SKEW %e ANGLE SIDE %e   ANGLE CRCK %e\n", skew_fact, ang0, angle);
#endif

	if (skew_fact < 1.0) {skew_fact = 1.0 ;}
	if (skew_fact > max_fact) {skew_fact = max_fact ;}
	
	return(skew_fact);
}

/** Computes D matrix
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
int sbet_get_D(
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
  long prev_stat1, stat1 ;
  long prev_stat2, stat2 ;
  double E0, E1, E2, beta, angle, epsilon01, epsilon02, nu, G ;
  double Gf, f_ccyl, f_tu, fc, ft, L ;
  double E1_old, E2_old, beta_old ;
  double equi_e1, equi_e2, equi_eps, equi_eps2 ;
  long i, ii ;
	long beta_type = 1 ;
	long kupf_type = 1 ;
  long crck_type = 0 ;

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

#if  0
	if ((crck_type = femGetMPValPos(ePos, MAT_CRTYPE, 0)) > 0)
  {
    return(
        sbet_rotcr_get_D(
          ePos, iPoint, A, epsilon, sigma, sigma_r,
          Mode, Problem, D)) ; 
  }
#endif

	femMatSetZero(D);

	prev_stat1 = (long) femGetEResVal(ePos, RES_STAT1, iPoint+1) ;
  stat1      = prev_stat1 ;

	prev_stat2 = (long) femGetEResVal(ePos, RES_STAT2, iPoint+1) ;
  stat2      = prev_stat2 ;

	E0        = femGetMPValPos(ePos, MAT_EX, 0) ;
	nu        = femGetMPValPos(ePos, MAT_NU, 0) ;
  G         = E0 / (2.0*(1.0+nu)) ;

	beta_type = femGetMPValPos(ePos, MAT_SHTYPE, 0) ;
	kupf_type = femGetMPValPos(ePos, MAT_FCTYPE, 0) ; if (kupf_type < 0) { kupf_type = 0 ;}

	E1        = femGetEResVal(ePos, RES_ECR1, iPoint+1) ;
	E2        = femGetEResVal(ePos, RES_ECR2, iPoint+1) ;
	beta      = femGetEResVal(ePos, RES_GCR1, iPoint+1) ;
	angle     = femGetEResVal(ePos, RES_DIR1, iPoint+1) ;
	epsilon01 = femGetEResVal(ePos, RES_DIR2, iPoint+1) ;
	epsilon02 = femGetEResVal(ePos, RES_DIR3, iPoint+1) ;

	L         = sqrt(A) ; /* TODO: check this! */

	beta_type = 7 ;

  if (beta < FEM_ZERO) { beta = FEM_ZERO ; } /* (!) */
	if (beta > 1.0) { beta = 1.0 ; }

	if ((prev_stat1 == prev_stat2) &&
      ((prev_stat1 == 1)||(prev_stat1 == 0)))
  {
    /* initial linear behaviour */
		E1 = E0 ;
		E2 = E0 ;
		beta = 1.0 ;
		angle = 0.0 ;
  }

	/* convergence stuff: */
#if 1
	if (E1 < (0.001*E0)) {E1 = 0.001*E0;}
	if (E2 < (0.001*E0)) {E2 = 0.001*E0;}
	if (beta < (0.001)) {beta = 0.001 ;}
#endif
#if 0
	if (E1 < (FEM_ZERO*E0)) {E1 = FEM_ZERO*E0;}
	if (E2 < (FEM_ZERO*E0)) {E2 = FEM_ZERO*E0;}
	if (beta < (FEM_ZERO)) {beta = FEM_ZERO ;}
#endif

	/* Compute old matrix */
  D_Plane_OrthoCB(E1, E2, nu, G, beta, angle, D);

	/* new matrix */
	if (Mode == AF_YES)
  {
		E1        = femGetEResVal(ePos, RES_ECR1, iPoint+1) ;
		E2        = femGetEResVal(ePos, RES_ECR2, iPoint+1) ;
		beta      = femGetEResVal(ePos, RES_GCR1, iPoint+1) ;


		if (((prev_stat2==0)||(prev_stat2==1)) && ((prev_stat1==1)||(prev_stat1==0)))
  	{
    	/* initial linear behaviour -  repeating */
			E1 = E0 ;
			E2 = E0 ;
			beta = 1.0 ;
  	}

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

    /* begin of ii loop: */
    for (ii=0; ii<3; ii++) {

		if (ii > 0)
		{
			E1        = femGetEResVal(ePos, RES_ECR1, iPoint+1) ;
			E2        = femGetEResVal(ePos, RES_ECR2, iPoint+1) ;
			beta      = femGetEResVal(ePos, RES_GCR1, iPoint+1) ;
		}

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

		/* computing of the state of initially undamaged material  */
		if ( ((prev_stat1 == 1)||(prev_stat1 == 0)||(prev_stat1 == 3)) &&
				((prev_stat2 == 1)||(prev_stat2 == 0)||(prev_stat2 == 3)) )
		{
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
			if (ft < femVecGet(&sigma_1,2)) { stat2 = 2 ; }

			if ((-1.0*fabs(fc)) > femVecGet(&sigma_1,1) && (femVecGet(&sigma_1,1) < 0.0)) { stat1 = 4 ; }
			if (((-1.0*fabs(fc)) > femVecGet(&sigma_1,2)) && (femVecGet(&sigma_1,2) < 0.0)) { stat2 = 4 ; }
			
			femTe_2d(&Ts, angle) ;
		  femMatVecMult(&Ts, &tot_epsilon, &tot_epsilon_1);

      equi_e1 = femVecGet(&tot_epsilon_1, 1) ;
      equi_e2 = femVecGet(&tot_epsilon_1, 2) ;

#if 1
      if ( (E2 > 0.0)&&(E1 > 0.0) )
      {
        equi_e1 = femVecGet(&sigma_1, 1) / ( E1 / (1.0-nu*nu*(E1/E2)) ) ;
        equi_e2 = femVecGet(&sigma_1, 2) / ( E2 / (1.0-nu*nu*(E1/E2)) ) ;
      }
#endif

	  	femPutEResVal(ePos, RES_SCMAX, iPoint+1, fc);
	  	femPutEResVal(ePos, RES_STMAX, iPoint+1, ft);
		}
		else
		{
			fc = femGetEResVal(ePos, RES_SCMAX, iPoint+1) ;
			ft = femGetEResVal(ePos, RES_STMAX, iPoint+1) ;

			/* previously damaged material */
			femTe_2d(&Ts, angle) ;
		  femMatVecMult(&Ts, &tot_epsilon, &tot_epsilon_1);

			/* sigma in previous direction: */
			femTs_2d(&Ts, angle) ;
			femMatVecMult( D, &tot_epsilon, &sigma_x ) ;
		  femMatVecMult(&Ts, &sigma_x, &sigma_1);

			/* additional tests: */
			if (ft < femVecGet(&sigma_1,1)) { stat1 = 2 ; }
			if (ft < femVecGet(&sigma_1,2)) { stat2 = 2 ; }

			if ((-1.0*fabs(fc)) > femVecGet(&sigma_1,1) && (femVecGet(&sigma_1,1) < 0.0)) { stat1 = 4 ; }
			if (((-1.0*fabs(fc)) > femVecGet(&sigma_1,2)) && (femVecGet(&sigma_1,2) < 0.0)) { stat2 = 4 ; }

      /* compute equivalent strain here */
      equi_e1 = femVecGet(&tot_epsilon_1, 1) ;
      equi_e2 = femVecGet(&tot_epsilon_1, 2) ;

#if 1
      if ( (E2 > 0.0)&&(E1 > 0.0) )
      {
        equi_e1 = femVecGet(&sigma_1, 1) / ( E1 / (1.0-nu*nu*(E1/E2)) ) ;
        equi_e2 = femVecGet(&sigma_1, 2) / ( E2 / (1.0-nu*nu*(E1/E2)) ) ;
      }
#endif

			/* check for unloading */
#if 1
			if ((prev_stat1 == 2) || (prev_stat1 == -1))
			{
				if (epsilon01 > equi_e1) { stat1 = -1 ; }
				else { stat1 = 2 ; }
			}

			if ((prev_stat1 == 4) || (prev_stat1 == -3))
			{
				if (epsilon01 < equi_e1) { stat1 = -3 ; }
				else { stat1 = 4 ; }

				if (equi_e1 > 0.0) {stat1 = 5 ;} 
			}


			if ((prev_stat2 == 2) || (prev_stat2 == -1))
			{
				if (epsilon02 > equi_e2) { stat2 = -1 ; }
				else { stat2 = 2 ; }
			}

			if ((prev_stat2 == 4) || (prev_stat2 == -3))
			{
				if (epsilon02 < equi_e2) { stat2 = -3 ; }
				else { stat2 = 4 ; }

				if (equi_e2 > 0.0) {stat2 = 5 ;} 
			}
#endif

			/* check of damage in undamaged direction */
#if 1
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
#endif
		}

		if ( (stat1 == 2) || (stat1 == -1) || (stat2 == 2)|| (stat2 == -1) )
		{
			L = sqrt(A) * sbet_skew_factor_2D(angle, ePos) ; 
		}

		/* computation of E1 */ 
		switch (stat1)
		{
			case 2: E1 = sb1d_get_Rt_tension(
									ePos, E0, ft, Gf, L, equi_e1
                  );
							break ;

			case 4: 
#if 1
							E1 = sbet_compr_E( equi_e1,
									            E0, E0, fc, L, stat1);
#else
							E1 = sbet_compr_plast_E( equi_e1,
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

#if 1
		/* computation of E2 */ 
		switch (stat2)
		{
			case 2: E2 = sb1d_get_Rt_tension(
									ePos, E0, ft, Gf, L, 
									equi_e2
                  );
							break ;

			case 4: 
#if 1
							E2 = sbet_compr_E( equi_e2,
									            E0, E0, fc, L,stat2);
#else
							E2 = sbet_compr_plast_E( equi_e2,
									            E0, E0, fc,stat2);
#endif
							break;

			case 5:	E2 = 0.0 ; beta =0.0 ; break ;

			case -1: /* previous values are correct here */ break ;
			case -3: /* previous values are correct here -- TODO: check this! */ break ;

			case 3: /* 3 might have to be changed later! */
			case 1:
			case 0:
			default:	E2 = E0 ; break ;
		}
#else
		E2 = E0 ;
#endif


		/* computation of beta */ 
#if 1
		if ((stat1 == 2)||(stat1 == -1)||(stat2 == 2)||(stat2 == -1))
		{
			if ((stat1 == 2)||(stat1 == -1))
			{
#if 0
				equi_eps = femVecGet(&tot_epsilon_1,1) ;
#else
				equi_eps = equi_e1 ;
#endif
				
				beta = sbeg_Gbeta(beta_type, 0.02, equi_eps, 0.005, E1, E0) ;
				if (fabs(beta*G*equi_eps) > ft) {beta = (ft/(G*equi_eps));}
			}
			
			if ((stat2 == 2)||(stat2 == -1))
			{
#if 0
				equi_eps2 = femVecGet(&tot_epsilon_1,2) ;
#else
				equi_eps2 = equi_e2 ;
#endif
				if ((stat1 == 2)||(stat1 == -1))
				{
					if (equi_eps2 < equi_eps) {equi_eps = -1e6 ; ;}
					else {equi_eps = equi_eps2;}
				}
				else
				{
					equi_eps = equi_eps2 ;
				}
				
				if (equi_eps > 0)
				{
					beta = sbeg_Gbeta(beta_type, 0.02,
               equi_eps, 0.005, E2, E0) ;
					if (fabs(beta*G*equi_eps) > ft) {beta = (ft/(G*equi_eps));}
				}
			}

			if (beta > 1.0) {beta = 1.0 ;}
			if (beta < 0.0) {beta = 0.0 ;}
		} /* on other cases the beta remains unchanged */
		else
		{
			beta = 1.0 ;
		}
#else
			if ((stat1 == 2)||(stat1 == -1))
			{
				beta = sbeg_Gbeta(beta_type, 0.02,
               femVecGet(&tot_epsilon_1,1), 0.005, E1, E0) ;
				if (fabs(beta*G*femVecGet(&tot_epsilon_1,1)) > ft) 
           {beta = (ft/(G*femVecGet(&tot_epsilon_1,1)));}
			}
printf("BETA = %e\n", beta);

			if (beta > 1.0) {beta = 1.0 ;}
			if (beta < 0.0) {beta = 0.0 ;}
#endif

    /* E quality control (it cannot be better, only worse): */
    if (E1_old > 0.0) { if (E1 > E1_old) { E1 = E1_old ;}}
    if (E2_old > 0.0) {if (E2 > E2_old) { E2 = E2_old ;}}
    if (beta_old >= 0.0) {if (beta > beta_old) { beta = beta_old ;}}

		femMatSetZero(D);

		/* new D matrix */
  	D_Plane_OrthoCB(E1, E2, nu, G, beta, angle, D); 

		/* correct actual stress: */
    femMatVecMult (D, &tot_epsilon, sigma) ;

    /* material status data */
    femPutEResVal(ePos, RES_STAT1, iPoint+1, stat1) ;
    femPutEResVal(ePos, RES_STAT2, iPoint+1, stat2) ;

	  femPutEResVal(ePos, RES_ECR1, iPoint+1, E1) ;
	  femPutEResVal(ePos, RES_ECR2, iPoint+1, E2) ;
	  femPutEResVal(ePos, RES_GCR1, iPoint+1, beta) ;
	  femPutEResVal(ePos, RES_DIR1, iPoint+1, angle) ;
	  femPutEResVal(ePos, RES_DIR2, iPoint+1, femVecGet(&tot_epsilon_1,1)) ;
	  femPutEResVal(ePos, RES_DIR3, iPoint+1, femVecGet(&tot_epsilon_1,2)) ;

    }  /* end of ii loop */

		/* unballanced (residual) stresses */
		for (i=1; i<=3; i++)
    {
      femVecPut(sigma_r, i,
          (femVecGet(sigma_r, i) - femVecGet(sigma,i )) );
    }

	  femPutEResVal(ePos, RES_USX,  iPoint+1, femVecGet(sigma_r,1)) ;
	  femPutEResVal(ePos, RES_USY,  iPoint+1, femVecGet(sigma_r,2)) ;
	  femPutEResVal(ePos, RES_USXY, iPoint+1, femVecGet(sigma_r,3)) ;

    femVecAdd(sigma, 1, (-1.0)*femGetEResVal(ePos, RES_SX,  iPoint+1));
	  femVecAdd(sigma, 2, (-1.0)*femGetEResVal(ePos, RES_SY,  iPoint+1));
	  femVecAdd(sigma, 3, (-1.0)*femGetEResVal(ePos, RES_SXY, iPoint+1));



memFree:
	  femVecFree(&tot_epsilon);
	  femVecFree(&tot_epsilon_1);
	  femVecFree(&sigma_x);
	  femVecFree(&sigma_1);
        
    femMatFree(&Ts) ;
	}
 
  return(rv);
}

#endif /* SMALL_FEM_CODE */
/* end of fem_sbet.c */

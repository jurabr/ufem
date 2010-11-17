/*
   File name: fem_ccr2.c
   Date:      2003/05/03 19:39
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

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

	 FEM Solver - 2D concrete material

	 $Id: fem_ccr2.c,v 1.8 2004/05/30 13:53:36 jirka Exp $
*/

#include "fem_elem.h"
#include "fem_mat.h"

#ifndef _SMALL_FEM_CODE_

extern int femGetPrincStress2D(tVector *sigma_x, tVector *sigma_1, double *phi);



/** Computes Ts transformation matrix for 2D problem 
 * @param Ts matrix[3,3]  - must be allocated!
 * @param fi0 angle [grad]
 * @return state value
 */
int femTs_2d(tMatrix *Ts, double fi0)
{
  double c, s, fi;

	femMatSetZero(Ts);

#if 1
  fi = FEM_PI * (fi0 / 180.0);
  c = cos(fi);
  s = sin(fi);

  femMatPut(Ts,1,1, c * c ) ;
  femMatPut(Ts,1,2, s * s ) ;
  femMatPut(Ts,1,3, 2.0 * c * s ) ;

  femMatPut(Ts,2,1, s * s ) ;
  femMatPut(Ts,2,2, c * c ) ;
  femMatPut(Ts,2,3, (-2.0) * c * s ) ;

  femMatPut(Ts,3,1, (-1.0) * c * s ) ;
  femMatPut(Ts,3,2, c * s ) ;
  femMatPut(Ts,3,3, (c * c) - (s * s) ) ;
#else
  femMatPut(Ts,1,1, 1.0 ) ;
  femMatPut(Ts,2,2, 1.0 ) ;
  femMatPut(Ts,3,3, 1.0 ) ;
#endif

  return (AF_OK);
}

/** Computes Te transformation matrix for 2D problem 
 * @param Ts matrix[3,3]  - must be allocated!
 * @param fi0 angle [grad]
 * @return state value
 */
int femTe_2d(tMatrix *Ts, double fi0)
{
  double c, s, fi;

	femMatSetZero(Ts);

#if 1
  fi = FEM_PI * (fi0 / 180.0);
  c = cos(fi);
  s = sin(fi);

  femMatPut(Ts,1,1, c * c ) ;
  femMatPut(Ts,1,2, s * s ) ;
  femMatPut(Ts,1,3, c * s ) ;

  femMatPut(Ts,2,1, s * s ) ;
  femMatPut(Ts,2,2, c * c ) ;
  femMatPut(Ts,2,3, (-1.0) * c * s ) ;

  femMatPut(Ts,3,1, (-2.0) * c * s ) ;
  femMatPut(Ts,3,2, (2.0) * c * s ) ;
  femMatPut(Ts,3,3, (c * c) - (s * s) ) ;
#else
  femMatPut(Ts,1,1, 1.0 ) ;
  femMatPut(Ts,2,2, 1.0 ) ;
  femMatPut(Ts,3,3, 1.0 ) ;
#endif

  return (AF_OK);
}




/**
 * Computes reduction coefficient for G modulus (using Kolmar's theory)
 * @param  epsilon  relative deformation perpedicular to  crack
 * @return beta     reduction coefficient (0, 1>
 */
double betaKolmar(double epsilon)
{
	double c1, c2 ;

	if (epsilon <= 0.0) {return(1.0);}  /* uncracked                    */
	
	c1 =  7.0 + ( 333.0 * (0.0 - 0.005) ) ; 
	c2 = 10.0 - ( 167.0 * (0.0 - 0.005) ) ;
	
	if (c2 <= 0){return(0.0);}

	return ( log(fabs(epsilon/c1)) / c2 ) ;
}

/***
 * Computes reduction coefficient for G modulus (using Cervenka's method)
 * @param  epsilon  relative deformation perpedicular to  crack
 * @return beta     reduction coefficient (0, 1>
 */
double betaCerv(double epsilon)
{
	double k3 = 0.4 ;
	static double beta;

	k3 = 1.0 ;

	if (epsilon < 0) {return(1.0); }
	else
	{
	  beta = 1.0 - pow((epsilon/0.005), k3) ;
		if (beta > 1.0 ) {return(1.0);}
		if (beta < 0.001 ) {return(0.001);}
		else              {return(beta);}
	}
}

/** Computes "beta" reduction coefficient 
 * @param  epsilon relative deformation in needed direction
 * @return beta (result)
 */
double betaCoeff(double epsilon, double E, double Ecr)
{
	static int afMethodG = 1 ;
	double e_max = 0.0004 ;


	if ((Ecr/E) > 1.0){return(1.0);}
	if (epsilon < 0.0){return(1.0);}

	switch(afMethodG)
	{
		case 1:  /* Damjanic + Owen: */
 	           if (epsilon > e_max) { return(1e-6); }
	           else                 { return(1.0 - (epsilon/e_max)); }
			       break ;
		case 2: /* Kolmar */
            return(betaKolmar(epsilon));
						break;
		case 3: /* Mang: */
	          return(0.2) ; 
						break;
						break;
		case 4: /* Brdecko: */
            return(0.3) ; 
						break;
		case 5: /* Glemberk + Sammuelson: */
						if ((Ecr/E) < 1e-6){return(1e-6);}
						else                {return(Ecr/E);}
						break;
		case 6: /* Cervenka: */
	          return(betaCerv(epsilon));
						break;
		default: return(1.0);
						 break;
	}

  return(1.0);
}



/**
 * Computes residual stifness of cracked material (Rt)
 * using simplified crack band model
 * @param  E          initial stifness modulus
 * @param  Gf         fracture energy
 * @param  epsilon_el elastic part of relative deformation
 * @param  sigma_max  maximal (== failure) stress 
 * @param  L          crack band width
 * @return Rt modulus size (may be 0, too)
 */
double chenGetRt(
		             double E,
								 double Gf,
								 double epsilon_el,
								 double sigma_max,
								 double sigma_max_cr,
								 double L
		            )
{
	double sigma_i;
	double Ez;
	static double Rt;

	if (epsilon_el == 0.0) {return(E);}

	if (epsilon_el < 0.0) /* crushing */
	{
		if (fabs(sigma_max_cr / E) > fabs(epsilon_el))
		{
			return(E);
		}
		else
		{
			 return (fabs (sigma_max_cr / epsilon_el) ) ;
		}
	}


	if (fabs(sigma_max / E) > fabs(epsilon_el)){return(E);}

#if 1 /* Cervenka */
	/* Descending modulus: */
	Ez = ( -1.0 * E ) / ( 1.0 - ((2.0*Gf*E)/(L*sigma_max*sigma_max)) ) ;

	/* Actual equivalent stess: */
	sigma_i = ( epsilon_el * Ez ) + ( sigma_max * ( 1.0 - (Ez/E) ) ) ;

	Rt = sigma_i / epsilon_el ;

#else /* Samuelsson */
  sigma_i = sigma_max * exp((-(epsilon_el - (sigma_max/E))/(Gf/(sigma_max*L)))) ;
	sigma_i = fabs(sigma_i);

	Rt = sigma_i / epsilon_el ;
#endif
	
	/* Modulus is returned: */
	if (Rt > E) 
	{ 
		Rt = E ; 
	}
	else                            
	{ 
		if (Rt < (1e-8*E))
		{
			Rt = 1e-8*E ;
		}
		else
		{
			Rt = sigma_i/epsilon_el ; 
		}
	}

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[i] L=%f Gf=%f E=%f Ez=%f Rt=%f\n", L, Gf, E, Ez, Rt );
#endif

	return(Rt) ;
}

/* ---------------------------------------------- */


void  femCEB_FIP_Val(
		                 double  s_01,
										 double  s_02,
										 double  f_uc,
										 double  f_ut,
										 double *f_uc_new,
										 double *f_ut_new
		               )
{
	double crit, alpha;
	double s_1,s_2;

	*f_uc_new = f_uc ;
	*f_ut_new = f_ut ;

	/* always must be s_1 > s_2: */
	if (s_02 > s_01)
	{
		s_1 = s_02 ;
		s_2 = s_01 ;
	}
	else
	{
	  s_1 = s_01;
	  s_2 = s_02;
	}

	/* applying criteria: */
		
	if (s_2 < (-0.96*fabs(f_uc)))
	{
		alpha = s_1 / s_2 ;
		crit = ((-1.0) * ( (1.0 + 3.8*alpha) * f_uc )) / ((1.0 + alpha)*(1.0 + alpha)) ;
		
		if (s_2 <= crit) { *f_uc_new = fabs((-0.96*fabs(f_uc))); return; /* crushing */ }
		else             { return; /* nothing  */ }
	}

	if (s_2 < 0.0)
	{
		crit = ( 1.0 + 0.8*(s_2/f_uc) ) * f_ut ;

		if (s_1 >= crit) { *f_ut_new = crit ; return; /* cracking */ }
		else             { return; /* nothing  */ }
	}
	else /* s2 > 0 */
	{
		if (s_1 >= f_ut) { return; /* cracking */ }
		else             { return; /* nothing  */ }
	}
}

/** Test material status using CEB-FIP criteria
 * @return 0=uncracked, 1=tension, -1=compression
 * */
double  femCEB_FIP_Test(
		                 double  s_01,
										 double  s_02,
										 double  f_uc,
										 double  f_ut
		               )
{
	double crit, alpha;
	double s_1,s_2;

	/* always must be s_1 > s_2: */
	if (s_02 > s_01)
	{
		s_1 = s_02 ;
		s_2 = s_01 ;
	}
	else
	{
	  s_1 = s_01;
	  s_2 = s_02;
	}

	/* applying criteria: */
		
	if (s_2 < (-0.96*fabs(f_uc)))
	{
		alpha = s_1 / s_2 ;
		crit = ((-1.0) * ( (1.0 + 3.8*alpha) * f_uc )) / ((1.0 + alpha)*(1.0 + alpha)) ;

#ifdef DEVEL_VERBOSE
    fprintf(msgout," CEB-FIP MC90: CRIT=%e S2=%e\n", crit, s_2);
#endif
		
		if (s_2 <= crit) 
		{ 
			return(-1); /* crushing */ 
		}
		else             
		{ 
			return(0); /* nothing  */ 
		}
	}

	if ((s_2 < 0.0) && (s_2 >= (-0.96*fabs(f_uc))))
	{
		crit = ( 1.0 + 0.8*(s_2/f_uc) ) * f_ut ;
printf("CRIT=%e S1=%e\n", crit, s_1);

		if (s_1 >= crit) 
		{ 
			return(1); /* cracking */ 
		}
		else             
		{ 
			return(0); /* nothing  */ 
		}
	}
	else /* s2 > 0 */
	{
		if (s_1 >= f_ut) 
		{ 
			return(1); /* cracking */ 
		}
		else             
		{ 
			return(0); /* nothing  */ 
		}
	}
}



/* Computes Te transformation matrix for 2D problem */
int D_Fill_Te(tMatrix *Te, tMatrix *TeT, double fi0)
{
	double c,s,fi;
	int    i,j;

#ifdef DEVEL
	if ((Te->rows != TeT->rows) || (Te->cols != TeT->cols) || (Te->rows != 3) || (Te->cols != 3))
	{
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("Transformation matrix not allocated or has invalid size"));
#endif
		return(AF_ERR_SIZ);
	}
#endif

	fi = FEM_PI * (fi0/180.0);
	c = cos(fi);
	s = sin(fi);

	femMatPut(Te,1,1, (c*c) ) ;
	femMatPut(Te,1,2, (s*s) ) ;
	femMatPut(Te,1,3, (c*s) ) ;

	femMatPut(Te,2,1, (s*s) ) ;
	femMatPut(Te,2,2, (c*c) ) ;
	femMatPut(Te,2,3, ((-1.0)*c*s) ) ;

	femMatPut(Te,3,1, ((-2.0)*c*s) ) ;
	femMatPut(Te,3,2, (2.0*c*s) ) ;
	femMatPut(Te,3,3, ((c*c) - (s*s)) ) ;

	for (i=1; i<=3; i++)
	{
		for (j=1; j<=3; j++)
		{
			femMatPut(TeT, i,j,  femMatGet(Te,j,i));
		}
	}
	
	return(AF_OK);
}


/** 
 * Computes ortotropic D matrix (by me) )
 * @param R1 stifness in 1st direction
 * @param R2 stifness in 2nd direction
 * @param nu poisson ratio
 * @param G shear modulus
 * @param beta G modulus multiplier
 * @param fi_1 1st direction angle (grad)
 * @param Dcr material stifness matrix
 * @param 
 */
int D_Plane_OrthoCB(double R1, double R2, double nu, double G, double beta, double fi_1, tMatrix *Dcr)
{
	int rv = AF_OK;
	/*int i,j ;*/
	double   dMult = 0.0 ; /* "d" multiplier for Cervenka's matrix */
  tMatrix  Dc0 ;
	tMatrix  Dt  ;
	tMatrix  TeT ;
	tMatrix  Te  ;

	dMult = 0 ; /* this line is needed only to avoid compiler warnings */

  /* Allocating of matrix in local coordinates and transformation matrices: */
	if ((rv = femFullMatInit(&Dc0,3,3)) != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&Dt,3,3))  != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&TeT,3,3)) != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&Te,3,3))  != AF_OK) { goto memFree; }

	femMatSetZero(&Dc0);

	/* Cervenka and fixed by me: */
#if 1
	if (fabs(R1+R2) > 0.0)
	{
#endif
    dMult = R2 / (R2 - (nu*nu*R1)) ;
#if 0
		printf("DMULT %e (R1 = %e)(R2 = %e)(nu = %e)\n",dMult, R1, R2, nu);
#endif
#if 1
	}
	else
	{
	  dMult = 0.0 ;
	}
#endif

	femMatPut(&Dc0,1,1, (dMult *      R1) ) ;
	femMatPut(&Dc0,1,2, (dMult * nu * R1) ) ;
	femMatPut(&Dc0,2,1, (dMult * nu * R1) ) ;
	femMatPut(&Dc0,2,2, (dMult *      R2) ) ;
	
#if 0
	if (dMult == 0.0 )
	{
	  femMatPut(&Dc0,3,3, (0.0001) ) ;
	}
	else
	{
#endif
	  /*femMatPut(&Dc0,3,3, ((beta * G)  / dMult) ) ; */
	  femMatPut(&Dc0,3,3, ((beta * G)) ) ; 
#if 0
	}
#endif

	femMatPrn(&Dc0, "D_0");
	
	/* Transformation of Dc0 to global coordinates (Dcr): */
	D_Fill_Te(&Te, &TeT, fi_1);

	femMatPrn(&Te,"TE");
	femMatPrn(&TeT,"TET");

	femMatPrn(&Dc0, "D_00");

	femMatMatMult(&TeT, &Dc0, &Dt);
	femMatMatMult(&Dt,  &Te,   Dcr);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[!] R1=%f R2=%f nu=%f beta=%f phi=%f\n",
			R1,R2,nu,beta, 
			/*FEM_PI*fi_1/180.0*/ fi_1
			);
#endif

memFree:
	femMatFree(&Dc0);
	femMatFree(&Dt);
	femMatFree(&TeT);
	femMatFree(&Te);
	return(rv);
}

/** 
 * Computes ortotropic D matrix (Cervenka et al - SBETA) )
 * @param R1 stifness in 1st direction
 * @param R2 stifness in 2nd direction
 * @param nu poisson ratio
 * @param G shear modulus
 * @param beta G modulus multiplier
 * @param fi_1 1st direction angle (grad)
 * @param Dcr material stifness matrix
 * @param 
 */
int D_Plane_OrthoCerv(double R1, double R2, double nu, double G, double beta, double fi_1, tMatrix *Dcr)
{
	int rv = AF_OK;
  tMatrix  Dc0 ;
	tMatrix  Dt  ;
	tMatrix  TeT ;
	tMatrix  Te  ;

  /* Allocating of matrix in local coordinates and transformation matrices: */
	if ((rv = femFullMatInit(&Dc0,3,3)) != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&Dt,3,3))  != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&TeT,3,3)) != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&Te,3,3))  != AF_OK) { goto memFree; }

	femMatSetZero(Dcr);
	femMatSetZero(&Dc0);

	if (R1 < FEM_ZERO) {R1 = FEM_ZERO;}
	if (R2 < FEM_ZERO) {R2 = FEM_ZERO;}
	if (beta < FEM_ZERO) {beta = FEM_ZERO;}
	femMatPut(&Dc0, 1,1, 1.0/R1 );
	femMatPut(&Dc0, 2,2, 1.0/R2 );
	femMatPut(&Dc0, 1,2, -nu/R2 );
	femMatPut(&Dc0, 2,1, -nu/R2 );

	femMatPut(&Dc0, 3,3, 1.0/(beta*G) );
	femLUinverse(&Dc0) ;
	
	/* Transformation of Dc0 to global coordinates (Dcr): */
	D_Fill_Te(&Te, &TeT, fi_1);

	femMatPrn(&Te,"TE");
	femMatPrn(&TeT,"TET");

	femMatPrn(&Dc0, "D_00");

	femMatMatMult(&TeT, &Dc0, &Dt);
	femMatMatMult(&Dt,  &Te,   Dcr);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[!] R1=%f R2=%f nu=%f beta=%f phi=%f\n",
			R1,R2,nu,beta, 
			/*FEM_PI*fi_1/180.0*/ fi_1
			);
#endif

memFree:
	femMatFree(&Dc0);
	femMatFree(&Dt);
	femMatFree(&TeT);
	femMatFree(&Te);
	return(rv);
}

/** 
 * Computes ortotropic D matrix (Bittnar-Sejnoha Numericke met. I  pp.21) )
 * @param R1 stifness in 1st direction
 * @param R2 stifness in 2nd direction
 * @param nu poisson ratio
 * @param G shear modulus
 * @param beta G modulus multiplier
 * @param fi_1 1st direction angle (grad)
 * @param Dcr material stifness matrix
 * @param 
 */
int D_Plane_Ortho(double R1, double R2, double nu, double beta, double fi_1, tMatrix *Dcr)
{
	int rv = AF_OK;
	/*int i,j ;*/
	double   dMult = 0.0 ; /* "d" multiplier for Cervenka's matrix */
  tMatrix  Dc0 ;
	tMatrix  Dt  ;
	tMatrix  TeT ;
	tMatrix  Te  ;

	dMult = 0 ; /* this line is needed only to avoid compiler warnings */

  /* Allocating of matrix in local coordinates and transformation matrices: */
	if ((rv = femFullMatInit(&Dc0,3,3)) != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&Dt,3,3))  != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&TeT,3,3)) != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&Te,3,3))  != AF_OK) { goto memFree; }

	femMatSetZero(&Dc0);

  dMult = 1.0 / (1.0 -nu*nu) ;

	femMatPut(&Dc0,1,1, (dMult *      R1) ) ;
	femMatPut(&Dc0,1,2, (dMult * nu * sqrt(R1*R2)) ) ;
	femMatPut(&Dc0,2,1, (dMult * nu * sqrt(R1*R2)) ) ;
	femMatPut(&Dc0,2,2, (dMult *      R2) ) ;
	
	/*femMatPut(&Dc0,3,3, ((beta * G)  / dMult) ) ; */
	femMatPut(&Dc0,3,3,  beta * 0.25*(R1+R2 - 2.0*nu*sqrt(R1*R2)) ) ; 

	femMatPrn(&Dc0, "D_0");
	
	/* Transformation of Dc0 to global coordinates (Dcr): */
	D_Fill_Te(&Te, &TeT, fi_1);

	femMatPrn(&Te,"TE");
	femMatPrn(&TeT,"TET");

	femMatPrn(&Dc0, "D_00");

	femMatMatMult(&TeT, &Dc0, &Dt);
	femMatMatMult(&Dt,  &Te,   Dcr);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[!] R1=%f R2=%f nu=%f beta=%f phi=%f\n",
			R1,R2,nu,beta, 
			/*FEM_PI*fi_1/180.0*/ fi_1
			);
#endif

memFree:
	femMatFree(&Dc0);
	femMatFree(&Dt);
	femMatFree(&TeT);
	femMatFree(&Te);
	return(rv);
}

/** Computes stifness matrix of reinforcement */
int fem_Fill_Plane_Ds(double p_x, 
											double E0, 
											double phi,
											tVector *epsilon_sum,  /* unused */
											double s_max,          /* unused */
											long Mode,
											tMatrix *Ds)
{
	int rv = AF_OK ;
	tMatrix Te ;
	tMatrix TeT ;
	tMatrix D0 ;
	tMatrix D1 ;
	double E ;
	int i,j;

	E = E0 ;

	femFullMatInit(&Te, 3, 3) ;
	femFullMatInit(&TeT, 3, 3) ;
	femFullMatInit(&D0, 3, 3) ;
	femFullMatInit(&D1, 3, 3) ;

	if (Mode == AF_YES)
	{
		/* non-linear code here */
	}

	if ((rv = D_Fill_Te(&Te, &TeT, phi)) != AF_OK) { goto memFree; }
	femMatPut(&D0, 1,1, (p_x * E) ) ;

	femMatMatMult(&TeT, &D0, &D1) ;
	femMatSetZero(&D0) ;
	femMatMatMult(&D1, &Te, &D0) ;

	for (i=1; i<=3; i++)
	{
		for (j=1; j<=3; j++)
		{
			femMatAdd(Ds, i,j, (femMatGet(&D0, i,j)) ) ;
		}
	}

memFree:
	femMatFree(&Te) ;
	femMatFree(&TeT) ;
	femMatFree(&D0) ;
	femMatFree(&D1) ;
	return(rv);
}


/** Creates material stifness matrix for 2D plane stress (concrete)
 *  @param E    Young modulus
 *  @param nu   Poisson ratio
 *  @param Type   type of "D": 0..plane stress, 1..plane strain
 *  @param D   matrix (must be [4,4] and only indexes 1,2,3 are filled!)
 */
int D_ConcCrack2D(long ePos, long iPoint, double A, tVector *epsilon, long newM, long Problem, tMatrix *D)
{
	int    rv = AF_OK ;
	/*long   state = 0;*/
	double E, nu, G, s_tens, s_cmpr, Gf, phi;
	double res_E1, res_E2, res_phi, res_beta;
	double f_ut_new, f_uc_new ;
	tVector epsilon_1;
	tVector sigma_x;
	tVector sigma_1;
	tMatrix Ts;
	double E_s, ps_x1, ps_x2, phi_s1, phi_s2 ;

	femMatSetZero(D);

	femVecNull(&sigma_x);
	femVecNull(&sigma_1);
	femVecNull(&epsilon_1);

	if ((rv=femVecFullInit(&sigma_x, 3)) != AF_OK) {goto memFree; }
	if ((rv=femVecFullInit(&sigma_1, 3)) != AF_OK) {goto memFree; }
	if ((rv=femVecFullInit(&epsilon_1, 3)) != AF_OK) {goto memFree; }
	if ((rv=femFullMatInit(&Ts, 3,3)) != AF_OK) {goto memFree; }

	E      = femGetMPValPos(ePos, MAT_EX,    0) ;
	nu     = femGetMPValPos(ePos, MAT_NU,    0) ;
	s_tens = femGetMPValPos(ePos, MAT_STEN,  0) ;
	s_cmpr = femGetMPValPos(ePos, MAT_SCMPR, 0) ;
	Gf     = femGetMPValPos(ePos, MAT_GF,    0) ;

	E_s    = femGetMPValPos(ePos, MAT_E1,    0) ;


	phi_s1  = femGetRSValPos(ePos, RS_ROT1, 0) ;
	ps_x1   = femGetRSValPos(ePos, RS_RFC1, 0) ;
	phi_s2  = femGetRSValPos(ePos, RS_ROT2, 0) ;
	ps_x2   = femGetRSValPos(ePos, RS_RFC2, 0) ;

	G = E / ( 2.0*(1.0 + nu) ) ;

	res_E1   = femGetEResVal(ePos, RES_ECR1,  iPoint+1) ;
	res_E2   = femGetEResVal(ePos, RES_ECR2,  iPoint+1) ;
	res_phi  = femGetEResVal(ePos, RES_ECR3,  iPoint+1) ;
	res_beta = femGetEResVal(ePos, RES_GCR1,  iPoint+1) ;


	/* data initialization - if needed */
	if ( (res_E1 == 0) && (res_E2 == 0) && (res_phi == 0) && (newM == AF_NO) )
	{
		res_E1 = E ;
		res_E2 = E ;
		res_phi = 0 ;
		res_beta = 1.0 ;

		femPutEResVal(ePos, RES_ECR1,  iPoint+1, res_E1    ) ;
		femPutEResVal(ePos, RES_ECR2,  iPoint+1, res_E2    ) ;
		femPutEResVal(ePos, RES_ECR3,  iPoint+1, res_phi   ) ;
		femPutEResVal(ePos, RES_GCR1,  iPoint+1, res_beta   ) ;
	}

/*#ifndef PSEUDO_SEC*/
#if 0
 	if ((rv = D_Plane_OrthoCB(res_E1, res_E2,  nu,  G, res_beta, res_phi, D)) != AF_OK)
		 { goto memFree; }
#else
 	if ((rv = D_Plane_OrthoCB(E, E,  nu,  G, 1.0, 0.0, D)) != AF_OK)
		 { goto memFree; }
#endif


	if (newM == AF_YES) /* new stifness matrix */
	{
		femMatVecMult( D, epsilon, &sigma_x ) ;

		/* compute princ. stresses here ! */
		femGetPrincStress2D(&sigma_x, &sigma_1, &phi);

  	femTs_2d(&Ts, phi) ;
		femMatVecMult(&Ts, epsilon, &epsilon_1);

		femCEB_FIP_Val(
		                 femVecGet(&sigma_1, 1),
		                 femVecGet(&sigma_1, 2),
										 s_cmpr,
										 s_tens,
										 &f_uc_new,
										 &f_ut_new
		               ) ;


#if 1
	  res_E1 =  chenGetRt(E,Gf, femVecGet(&epsilon_1,1), f_ut_new, f_uc_new, sqrt(A));
		res_E2 =  chenGetRt(E,Gf, femVecGet(&epsilon_1,2), f_ut_new, f_uc_new, sqrt(A));
#else
	  res_E1 =  chenGetRt(E,Gf, femVecGet(&epsilon_1,1), s_tens, s_cmpr, sqrt(A));
		res_E2 =  chenGetRt(E,Gf, femVecGet(&epsilon_1,2), s_tens, s_cmpr, sqrt(A));
#endif

		if ( (femVecGet(&epsilon_1,1) > 0.0)  )
		{
			res_beta =  betaCoeff(femVecGet(&epsilon_1,1),  E, res_E1) ;
		}
		else
		{
			res_beta = 1.0 ;
		}

		if ((res_E1 < E) || (res_E2 < E))
		{
			res_phi  = phi ; 
			/*printf("PHI = %e\n",res_phi);*/
		}
		else
		{
			res_phi = 0 ;
		}

		/*printf("NORM: %f %f %f %f\n",res_E1, res_E2, f_ut_new, f_uc_new);*/


		femPutEResVal(ePos, RES_ECR1,  iPoint+1, res_E1    ) ;
		femPutEResVal(ePos, RES_ECR2,  iPoint+1, res_E2    ) ;
		femPutEResVal(ePos, RES_ECR3,  iPoint+1, res_phi   ) ;
		femPutEResVal(ePos, RES_GCR1,  iPoint+1, res_beta   ) ;

 		if ((rv = D_Plane_OrthoCB(res_E1, res_E2,  nu,  G, res_beta, res_phi, D)) != AF_OK)
		 	 { goto memFree; }
		/*femMatPrn(D,"D_new_00");*/
	}

	/* in all cases: */
#if 1
	fem_Fill_Plane_Ds(  ps_x1, 
											E_s, 
											phi_s1,
											NULL,  /* unused */
											0,          /* unused */
											newM,
											D) ;
	fem_Fill_Plane_Ds(  ps_x2, 
											E_s, 
											phi_s2,
											NULL,  /* unused */
											0,          /* unused */
											newM,
											D) ;

#endif

	
memFree:
	femVecFree(&sigma_x);
	femVecFree(&sigma_1);
	femVecFree(&epsilon_1);
	femMatFree(&Ts);
	return(rv);
}

#endif
/* end of fem_ccr2.c */

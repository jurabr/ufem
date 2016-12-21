/*
   File name: fem_stf2.c
   Date:      2003/05/01 17:52
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

	 FEM Solver - material stifness matrix for 2D problems (plane stress/strain)

	 $Id: fem_stf2.c,v 1.10 2003/10/18 16:45:19 jirka Exp $
*/

#include "fem_elem.h"
#include "fem_mat.h"

#ifndef _SMALL_FEM_CODE_

extern int D_ConcCrack2D(long ePos, long iPoint, double A, tVector *epsilon, long newM, long Problem, tMatrix *D);
extern int sbet_get_D( long ePos, long iPoint, double A, tVector *epsilon, tVector *sigma, tVector *sigma_r, long newM, long Problem, tMatrix *D); /* fem_sbet.c */
extern int fem_m09_D_2d( long ePos, long iPoint, double A, tVector *epsilon, long Mode, long Problem, tMatrix *D); 
extern int chen2d_D(long ePos, long e_rep, long Problem, tVector *epsilon, long Mode, tMatrix *Dep);
extern int fem_vmis_D_2D(long ePos, long e_rep, long Problem, tVector *epsilon, long Mode, tMatrix *Dep);
extern int D_Fill_Te(tMatrix *Te, tMatrix *TeT, double fi0);

int D_Plane_Ortho(double R1, double R2, double nu, double beta, double fi_1, tMatrix *Dcr);


int D_HookIso_planeRaw(double E, double nu, long Problem, tMatrix *D)
{
	double Mul = 0;

	if ((nu <= 0.0)||(nu >= 1))
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s: %e!\n",_("Invalid Poisson ration"),nu);
#endif
		return(AF_ERR_VAL);
	}

	switch (Problem)
	{
		case 0: 
			      Mul = E / (1 - (nu*nu)) ;

						femMatPut(D,1,1,   (Mul) ) ;
						femMatPut(D,1,2,   (Mul * nu) ) ;
						femMatPut(D,2,1,   (Mul * nu) ) ;
						femMatPut(D,2,2,   (Mul) ) ;
						femMatPut(D,3,3,   (Mul*0.5*(1 - nu)) ) ;
			      break;
						
		case 1: 
						Mul = E / ((1 + nu)*(1 - (2*nu))) ;

						femMatPut(D,1,1,  (Mul * (1 - nu)) ) ;
						femMatPut(D,1,2,  (Mul * nu) ) ;
						femMatPut(D,2,1,  (Mul * nu) ) ;
						femMatPut(D,2,2,  (Mul * (1 - nu)) ) ;
						femMatPut(D,3,3,  (Mul * 0.5 * (1 - nu)) ) ;
						break;
						
		default: return(AF_ERR_TYP); 
						 break;
	}

	femMatPut(D,1,3, 0.0 ) ;
	femMatPut(D,2,3, 0.0 ) ;
	femMatPut(D,3,1, 0.0 ) ;

	return(AF_OK);
}

/** Creates material stifness matrix for 2D plane stress/strain
 *  @param Type   type of "D": 0..plane stress, 1..plane strain
 *  @param D   matrix (must be [4,4] and only indexes 1,2,3 are filled!)
 */
int D_PlaneHookIso( long ePos, long iPoint, double A, tVector *epsilon, long newM, long Problem, tMatrix *D)
{
	double E, nu;

	E  = femGetMPValPos(ePos, MAT_EX, 0) ;
	nu = femGetMPValPos(ePos, MAT_NU, 0) ;
	
	return(D_HookIso_planeRaw(E, nu, Problem, D));
}

/** Creates orthotropic material stifness matrix for 2D plane stress/strain
 *  @param Problem   type of "D": 0..plane stress, 1..plane strain
 *  @param angle  matrix orientation
 *  @param D  matrix (must be [4,4] and only indexes 1,2,3 are filled!)
 */
int D_PlaneOrthoPlain(long ePos, long iPoint, long Problem, tMatrix *D)
{
  int rv = AF_OK ;
	double Ex, Ey, nuxy, nuyx, G, mult;
  double angle = 0.0 ;
  tMatrix  Dc0 ;
	tMatrix  Dt  ;
	tMatrix  TeT ;
	tMatrix  Te  ;

  /* Allocating of matrix in local coordinates and transformation matrices: */
	if ((rv = femFullMatInit(&Dc0,3,3)) != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&Dt,3,3))  != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&TeT,3,3)) != AF_OK) { goto memFree; }
	if ((rv = femFullMatInit(&Te,3,3))  != AF_OK) { goto memFree; }

	femMatSetZero(&Dc0);
  femMatSetZero(D) ;

	Ex   = femGetMPValPos(ePos, MAT_EX,   0) ;
	Ey   = femGetMPValPos(ePos, MAT_EY,   0) ;
  if (Ey<FEM_ZERO) {Ey = femGetMPValPos(ePos, MAT_EZ,   0) ;}
	nuxy = femGetMPValPos(ePos, MAT_NUXY, 0) ;
	G    = femGetMPValPos(ePos, MAT_GXY,  0) ;
	angle= femGetMPValPos(ePos, MAT_ANG,  1) ;

	nuyx = (Ey/Ex) * nuxy ;
#if 1
  mult = 1.0 / (1.0 - nuxy*nuyx) ;


	femMatPut(&Dc0, 1,1, mult*Ex      ) ;
	femMatPut(&Dc0, 1,2, mult*Ex*nuxy ) ;
	femMatPut(&Dc0, 2,1, mult*Ey*nuyx ) ;
	femMatPut(&Dc0, 2,2, mult*Ey      ) ;
	femMatPut(&Dc0, 3,3, G            ) ;
#else
  mult = sqrt(nuyx+nuxy) ;
	femMatPut(&Dc0, 3,3, 1.0 ) ;
  if (Probleolv == 0)
  {
	  femMatPut(&Dc0, 1,1, G*(2.0/(1.0-mult))      ) ;
	  femMatPut(&Dc0, 1,2, G*(2.0*mult/(1.0-mult))      ) ;
	  femMatPut(&Dc0, 2,1, G*(2.0*mult/(1.0-mult))      ) ;
	  femMatPut(&Dc0, 2,2, G*(2.0/(1.0-mult))      ) ;
  }
  else
  {
	  femMatPut(&Dc0, 1,1, G*(2.0*(1.0-mult)/(1.0-2.0*mult))      ) ;
	  femMatPut(&Dc0, 2,2, G*(2.0*(1.0-mult)/(1.0-2.0*mult))      ) ;
	  femMatPut(&Dc0, 1,2, G*(2.0*mult/(1.0-2.0*mult))      ) ;
	  femMatPut(&Dc0, 2,1, G*(2.0*mult/(1.0-2.0*mult))      ) ;
  }
#endif

  femMatPrn(&Dc0, "D_ort");

	/* Transformation of Dc0 to global coordinates (Dcr): */
	D_Fill_Te(&Te, &TeT, angle);

	femMatPrn(&Te,"TE");
	femMatPrn(&TeT,"TET");

	femMatPrn(&Dc0, "D_00");

	femMatMatMult(&TeT, &Dc0, &Dt);
	femMatMatMult(&Dt,  &Te,   D);

	femMatPrn(D, "D");

memFree:
	femMatFree(&Dc0);
	femMatFree(&Dt);
	femMatFree(&TeT);
	femMatFree(&Te);
	return(rv);

}


/** Computes principal stresses in 2D
 * @param sigma_x xy stresses
 * @param sigma_1 principal stresses s_1, s_2) - result
 * @param phi direction of s_1 stress
 * @return state value
 */
int femGetPrincStress2D(tVector *sigma_x, tVector *sigma_1, double *phi)
{
	int rv = AF_OK ;
	double Part1, Part2;
	double tmpVal;
	double s_x, s_y, s_xy, s_1, s_2;

	s_x  = femVecGet(sigma_x, 1 );
	s_y  = femVecGet(sigma_x, 2 );
	s_xy = femVecGet(sigma_x, 3 );

	Part1 = 0.5 * (s_x + s_y);
  Part2 = 0.5 * sqrt(((s_x - s_y)*(s_x - s_y))+(4.0*(s_xy * s_xy)));

  s_1 = Part1 + Part2;
  s_2 = Part1 - Part2;

  if (s_2 > s_1) /* because s1 > s2 */
  {
    tmpVal = s_1;
    s_1 = s_2;
    s_2 = tmpVal;
  }

  if (s_xy == 0.0)
  {
    *phi = 0.0;
		if (s_x < s_y) {*phi = 90.0 ;}
  }
  else
  {
    /* tmpVal = ((*phi - s_x)/(s_xy)); ??? WHAT PHI ???? */
    tmpVal = ((s_1 - s_x)/(s_xy));
    *phi = 180.0 * (atan(tmpVal) / FEM_PI);
  }

	femVecPut(sigma_1, 1, s_1 );
	femVecPut(sigma_1, 2, s_2 );
	femVecPut(sigma_1, 3, *phi );

	return(rv);
}


/** fills (allocated!) D matrix
 * @param ePos finite element position
 * @param iPoint finite element (integration) point position
 * @param A area size
 * @param epsilon strain vector (NULL if unused)
 * @param sigma stress vector for newM=AF_YES AND mtype=8 - result (NULL if unused)
 * @param sigma_r residual stress vector for newM=AF_YES AND mtype=8 - result (NULL if unused)
 * @param newM mode  AF_YES means changes, AF_NO=previous state
 * @param Problem 0=plane stress, 1=plane strain
 * @param D matrix to be filled
 */
int fem_D_2D(long ePos, long iPoint, double A, tVector *epsilon, tVector *sigma, tVector *sigma_r, long newM, long Problem, tMatrix *D)
{
	int rv = AF_OK;
	long mT;
	

	mT = mpType[femGetEMPPos(ePos)]; 

	switch (mT)
	{
		case 1: 
        if (femGetMPRepNumPos(ePos) > 0)
        {
				  rv =  D_Plane_Ortho(
              femGetMPValPos(ePos, MAT_EX, 0), 
              femGetMPValPos(ePos, MAT_EX, 1), 
              femGetMPValPos(ePos, MAT_NU, 0), 
              1.0, 
              0.0, /* angle !? */
              D);
        }
        else
        {
				  rv =  D_PlaneHookIso(ePos, iPoint, A, epsilon, newM, Problem, D) ;
        }
        if ((sigma != NULL)&&(newM==AF_YES)){femMatVecMult(D,epsilon,sigma);}
				break;
		case 2:
				rv =  D_ConcCrack2D(ePos, iPoint, A, epsilon, newM, Problem, D) ;
        if ((sigma != NULL)&&(newM==AF_YES)){femMatVecMult(D,epsilon,sigma);}
				break;
		case 3:
        rv = chen2d_D(ePos, iPoint+1, Problem, epsilon, newM, D); 
				break;
    case 4:
        rv = fem_vmis_D_2D(ePos, iPoint+1, Problem, epsilon, newM, D);
        break ;
    case 7:
    case 11:
        rv = D_PlaneOrthoPlain(ePos, iPoint+1, Problem, D);
        break ;
		case 8:
				rv = sbet_get_D( ePos, iPoint, A, epsilon, sigma, sigma_r, newM, Problem, D);
				break;

		case 9:
				rv = fem_m09_D_2d( ePos, iPoint, A, epsilon, newM, Problem, D);
        if ((sigma != NULL)&&(newM==AF_YES)){femMatVecMult(D,epsilon,sigma);}
				break;

		default:
				rv =  D_PlaneHookIso(ePos, iPoint, A, epsilon, newM, Problem, D) ;
        if ((sigma != NULL)&&(newM==AF_YES)){femMatVecMult(D,epsilon,sigma);}
				break;
	}
	
	return(rv);
}

#endif
/* end of fem_stf2.c */

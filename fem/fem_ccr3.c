/*
   File name: fem_ccr3.c
   Date:      2004/05/15 16:40
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
*/

#include "fem_pl3d.h"

/* from "fem_ccr2.c": */
extern double betaCoeff(double epsilon, double E, double Ecr);
extern double chenGetRt(
		             double E,
								 double Gf,
								 double epsilon_el,
								 double sigma_max,
								 double sigma_max_cr,
								 double L
		            );
/* from "fem_chen.h": */
extern long chen_limit_test(tVector *stress, double f_c, double f_bc, double f_t, long *cczone);


/** Computes orthotropic matrix for smeared crack model
 * @param Ex cracked stiffness modulus 
 * @param E1 stiffness modulus in other directions
 * @param nu Poisson's ratio
 * @param G  shear modulus
 * @param beta  here modulus reduction factor
 * @param D matrix (must me allocated)
 * @return status
 * */
int fem_ccr_ortho_matrix(double Ex, 
                         double E1, 
                         double nu, 
                         double G, 
                         double beta, 
                         tMatrix *D)
{
  int rv = AF_OK ;
#if 0
  int i,j ;
  tMatrix D0 ;

  femMatSetZero(D) ;

  if ((femFullMatInit(&D0, 6,6)) != AF_OK) {return(AF_ERR_MEM);} 

  if (Ex < 0.0)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout, "[E] %s!\n", _("Zero stiffness of cracked material") );
#endif
    femMatPut(D, 1,1, 1.0/FEM_ZERO) ;
  }
  else
  {
    femMatPut(D, 1,1, 1.0/Ex ) ;
  }

  femMatPut(D, 1,2, -1.0*nu/E1 ) ;
  femMatPut(D, 1,3, -1.0*nu/E1 ) ;

  femMatPut(D, 2,1, -1.0*nu/E1 ) ;
  femMatPut(D, 2,2, 1.0/E1 ) ;
  femMatPut(D, 2,3, -1.0*nu/E1 ) ;

  femMatPut(D, 3,1, -1.0*nu/E1 ) ;
  femMatPut(D, 3,2, -1.0*nu/E1 ) ;
  femMatPut(D, 3,3, 1.0/E1 ) ;

  femMatPut(D, 4,4, 1.0/(2.0*G) ) ;

  femMatPut(D, 5,5, 1.0/(2.0*beta*G) ) ;

  femMatPut(D, 6,6, 1.0/(2.0*beta*G) ) ;


  if ((rv = femLUinverse(D)) != AF_OK) { return(rv); }
  femMatTran(D, &D0) ;

  for (i=1; i<=6; i++)
  {
    for (j=1; j<=6; j++)
    {
      femMatPut(D,j,i, femMatGet(&D0,i,j)) ;
    }
  }

  femMatFree(&D0);
#else
	/* stiffness matrix derived by Lenka Lausova: */
	femMatSetZero(D) ;

  femMatPut(D, 1,1, ((1.0 - nu)*Ex) / (1.0 - nu - 2.0*(Ex/E1))*nu*nu  ) ;
  femMatPut(D, 1,2, (nu*Ex) / (1.0 - nu - 2.0*(Ex/E1))*nu*nu  ) ;
  femMatPut(D, 1,3, (nu*Ex) / (1.0 - nu - 2.0*(Ex/E1))*nu*nu  ) ;

  femMatPut(D, 2,1, (nu*Ex) / (1.0 - nu - 2.0*(Ex/E1))*nu*nu  ) ;
  femMatPut(D, 2,2, (E1 - nu*nu*Ex) / ( (1.0 + nu)*(1.0 - nu - 2.0*(Ex/E1))*nu*nu)  ) ;
  femMatPut(D, 2,3, (nu*(E1 + nu*Ex)) / ( (1.0 + nu)*(1.0 - nu - 2.0*(Ex/E1))*nu*nu)  ) ;

  femMatPut(D, 3,1, (nu*Ex) / (1.0 - nu - 2.0*(Ex/E1))*nu*nu  ) ;
  femMatPut(D, 3,2, (nu*(E1 + nu*Ex)) / ( (1.0 + nu)*(1.0 - nu - 2.0*(Ex/E1))*nu*nu)  ) ;
  femMatPut(D, 3,3, (E1 - nu*nu*Ex) / ( (1.0 + nu)*(1.0 - nu - 2.0*(Ex/E1))*nu*nu)  ) ;

  femMatPut(D, 4,4, 2.0*G      ) ;
  femMatPut(D, 5,5, 2.0*beta*G ) ;
  femMatPut(D, 6,6, 2.0*beta*G ) ;
#endif

	femMatPrn(D,"D_ortho");
  return(rv) ;
}

int D_fill_T_TT(
    double m11,
    double m12,
    double m13,
    double m21,
    double m22,
    double m23,
    double m31,
    double m32,
    double m33,
    tMatrix *Te,
    tMatrix *TeT
    )
{
  int rv = AF_OK ;

  femMatPut(Te, 1,1, m11*m11);
  femMatPut(Te, 1,2, m12*m12);
  femMatPut(Te, 1,3, m13*m13);
  femMatPut(Te, 1,4, 2.0*m12*m13);
  femMatPut(Te, 1,5, 2.0*m11*m13);
  femMatPut(Te, 1,6, 2.0*m11*m12);

  femMatPut(Te, 2,1, m21*m21);
  femMatPut(Te, 2,2, m22*m22);
  femMatPut(Te, 2,3, m23*m23);
  femMatPut(Te, 2,4, 2.0*m22*m23);
  femMatPut(Te, 2,5, 2.0*m21*m23);
  femMatPut(Te, 2,6, 2.0*m21*m22);

  femMatPut(Te, 3,1, m31*m31);
  femMatPut(Te, 3,2, m32*m32);
  femMatPut(Te, 3,3, m33*m33);
  femMatPut(Te, 3,4, 2.0*m32*m33);
  femMatPut(Te, 3,5, 2.0*m31*m33);
  femMatPut(Te, 3,6, 2.0*m31*m32);

  femMatPut(Te, 4,1, m21*m31);
  femMatPut(Te, 4,2, m22*m32);
  femMatPut(Te, 4,3, m23*m33);
  femMatPut(Te, 4,4, m22*m33 + m23*m32);
  femMatPut(Te, 4,5, m21*m33 + m23*m31);
  femMatPut(Te, 4,6, m21*m32 + m22*m31);

  femMatPut(Te, 5,1, m11*m31);
  femMatPut(Te, 5,2, m12*m32);
  femMatPut(Te, 5,3, m13*m33);
  femMatPut(Te, 5,4, m12*m33 + m13*m32);
  femMatPut(Te, 5,5, m11*m33 + m13*m31);
  femMatPut(Te, 5,6, m11*m32 + m12*m31);

  femMatPut(Te, 6,1, m11*m21);
  femMatPut(Te, 6,2, m12*m22);
  femMatPut(Te, 6,3, m13*m23);
  femMatPut(Te, 6,4, m12*m23 + m13*m22);
  femMatPut(Te, 6,5, m11*m23 + m13*m21);
  femMatPut(Te, 6,6, m11*m22 + m12*m21);

  if ((rv = femLUinverse(Te)) != AF_OK) { return(rv); }

  femMatTran(Te, TeT);

  return(rv);
}

/** Creates material stifness matrix for 3D
 *  @param E    Young modulus
 *  @param nu   Poisson ratio
 *  @param Type   type of "D": 0..plane stress, 1..plane strain
 *  @param D   matrix (must be [4,4] and only indexes 1,2,3 are filled!)
 */
int D_ConcCrack3D(long ePos, long iPoint, double A, tVector *sigma_x, tVector *epsilon, long newM, tMatrix *D)
{
	int    rv = AF_OK ;
	double E, nu, G, s_tens, s_cmpr, Gf;
	double res_E1, res_E2, res_beta;
	/*double f_ut_new, f_uc_new ;*/
  double m11,m12,m13,m21,m22,m23,m31,m32,m33;
	long   cczone ;
  tMatrix M;
  tMatrix Te;
  tMatrix TeT;
  tMatrix D0;
  tMatrix TD0;
	tVector epsilon_1;
	tVector sigma_1;

	femMatSetZero(D);

	femVecNull(&sigma_1);
	femVecNull(&epsilon_1);

	if ((rv=femVecFullInit(&sigma_1, 6)) != AF_OK) {goto memFree; }
	if ((rv=femVecFullInit(&epsilon_1, 6)) != AF_OK) {goto memFree; }

	if ((rv=femFullMatInit(&M, 3,3)) != AF_OK) {goto memFree; }
	if ((rv=femFullMatInit(&Te , 6,6)) != AF_OK) {goto memFree; }
	if ((rv=femFullMatInit(&TeT, 6,6)) != AF_OK) {goto memFree; }
	if ((rv=femFullMatInit(&D0 , 6,6)) != AF_OK) {goto memFree; }
	if ((rv=femFullMatInit(&TD0, 6,6)) != AF_OK) {goto memFree; }

	E      = femGetMPValPos(ePos, MAT_EX,    0) ;
	nu     = femGetMPValPos(ePos, MAT_NU,    0) ;
	s_tens = femGetMPValPos(ePos, MAT_STEN,  0) ;
	s_cmpr = femGetMPValPos(ePos, MAT_SCMPR, 0) ;
	Gf     = femGetMPValPos(ePos, MAT_GF,    0) ;

	G = E / ( 2.0*(1.0 + nu) ) ;

	res_E1   = femGetEResVal(ePos, RES_ECR1,  iPoint+1) ;
	res_E2   = femGetEResVal(ePos, RES_ECR2,  iPoint+1) ;
	res_beta = femGetEResVal(ePos, RES_GCR1,  iPoint+1) ;

	if (newM != AF_YES) /* new stifness matrix */
	{
	  m11 = femGetEResVal(ePos, RES_P_M11,  iPoint+1) ;
	  m12 = femGetEResVal(ePos, RES_P_M12,  iPoint+1) ;
	  m13 = femGetEResVal(ePos, RES_P_M13,  iPoint+1) ;
	  m21 = femGetEResVal(ePos, RES_P_M21,  iPoint+1) ;
	  m22 = femGetEResVal(ePos, RES_P_M22,  iPoint+1) ;
	  m23 = femGetEResVal(ePos, RES_P_M23,  iPoint+1) ;
	  m31 = femGetEResVal(ePos, RES_P_M31,  iPoint+1) ;
	  m32 = femGetEResVal(ePos, RES_P_M32,  iPoint+1) ;
	  m33 = femGetEResVal(ePos, RES_P_M33,  iPoint+1) ;


	  /* data initialization - if needed */
	  if ( (res_E1 == 0) && (res_E2 == 0))
	  {
		  res_E1 = E ;
		  res_E2 = E ;
		  res_beta = 1.0 ;

		  m11 = 1.0 ;
		  m12 = 0.0 ;
		  m13 = 0.0 ;
		  m21 = 0.0 ;
		  m22 = 1.0 ;
		  m23 = 0.0 ;
		  m31 = 0.0 ;
		  m32 = 0.0 ;
		  m33 = 1.0 ;

		  femPutEResVal(ePos, RES_ECR1,  iPoint+1, res_E1    ) ;
		  femPutEResVal(ePos, RES_ECR2,  iPoint+1, res_E2    ) ;
		  femPutEResVal(ePos, RES_GCR1,  iPoint+1, res_beta   ) ;

		  femPutEResVal(ePos, RES_P_M11,  iPoint+1, m11) ;
		  femPutEResVal(ePos, RES_P_M12,  iPoint+1, m12) ;
		  femPutEResVal(ePos, RES_P_M13,  iPoint+1, m13) ;
		  femPutEResVal(ePos, RES_P_M21,  iPoint+1, m21) ;
		  femPutEResVal(ePos, RES_P_M22,  iPoint+1, m22) ;
		  femPutEResVal(ePos, RES_P_M23,  iPoint+1, m23) ;
		  femPutEResVal(ePos, RES_P_M31,  iPoint+1, m31) ;
		  femPutEResVal(ePos, RES_P_M32,  iPoint+1, m32) ;
		  femPutEResVal(ePos, RES_P_M33,  iPoint+1, m33) ;
	  }
  }
  else /* new stifness matrix */
	{
    if ((rv=femPrinc3D(sigma_x, &sigma_1))!=AF_OK)
    {
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s?!\n",_("Computation of principal stresses failed"));
#endif
      goto memFree; 
    }

    femPrinc3dTranM(sigma_x, &sigma_1, &M);

	  m11 = femMatGet(&M, 1,1) ;
	  m12 = femMatGet(&M, 1,2) ;
	  m13 = femMatGet(&M, 1,3) ;
	  m21 = femMatGet(&M, 2,1) ;
	  m22 = femMatGet(&M, 2,2) ;
	  m23 = femMatGet(&M, 2,3) ;
	  m31 = femMatGet(&M, 3,1) ;
	  m32 = femMatGet(&M, 3,2) ;
	  m33 = femMatGet(&M, 3,3) ;

		femVecPrn(sigma_x, "S_X");
		femVecPrn(&sigma_1, "S_1");

#ifdef DEVEL_VERBOSE
	printf("m: %e %e %e %e %e %e %e %e %e\n",
			m11,m12,m13,m21,m22,m23,m31,m32,m33 );
#endif

		if ((rv = D_fill_T_TT(
    			m11, m12, m13,
    			m21, m22, m23,
    			m31, m32, m33,
    			&Te, &TeT))!=AF_OK) {goto memFree;}

  	femMatVecMult(&Te, epsilon, &epsilon_1);

#if 1
		if (chen_limit_test(sigma_x, s_cmpr, s_cmpr*1.2, s_tens, &cczone) == AF_YES)
		{
#endif
			if (cczone != AF_YES)
			{
				/* NOTE: this ignores peak stresses dependence on 3D stress state: */
	  		res_E1 =  chenGetRt(E,Gf, femVecGet(&epsilon_1,1), s_tens, s_cmpr, sqrt(A));
				res_E2 =  E ; /* should be changed later */

				if ( (femVecGet(&epsilon_1,1) > 0.0)  )
				{
					res_beta =  betaCoeff(femVecGet(&epsilon_1,1),  E, res_E1) ;
				}
				else
				{
					res_beta = 1.0 ;
				}

				femPutEResVal(ePos, RES_ECR1,  iPoint+1, res_E1    ) ;
				femPutEResVal(ePos, RES_ECR2,  iPoint+1, res_E2    ) ;
				femPutEResVal(ePos, RES_GCR1,  iPoint+1, res_beta   ) ;

				femPutEResVal(ePos, RES_P_M11,  iPoint+1, m11) ;
				femPutEResVal(ePos, RES_P_M12,  iPoint+1, m12) ;
				femPutEResVal(ePos, RES_P_M13,  iPoint+1, m13) ;
				femPutEResVal(ePos, RES_P_M21,  iPoint+1, m21) ;
				femPutEResVal(ePos, RES_P_M22,  iPoint+1, m22) ;
				femPutEResVal(ePos, RES_P_M23,  iPoint+1, m23) ;
				femPutEResVal(ePos, RES_P_M31,  iPoint+1, m31) ;
				femPutEResVal(ePos, RES_P_M32,  iPoint+1, m32) ;
				femPutEResVal(ePos, RES_P_M33,  iPoint+1, m33) ;
			}
		}
#if 1
	}
#endif

  /* matrix is created from computed data: */
 	if ((rv = fem_ccr_ortho_matrix(res_E1, res_E2,  nu,  G, res_beta, &D0)) != AF_OK)
		 { goto memFree; }

  /* matrix must be rotated HERE! */
  if ((rv = D_fill_T_TT(
    m11, m12, m13,
    m21, m22, m23,
    m31, m32, m33,
    &Te, &TeT))!=AF_OK) {goto memFree;}

  femMatMatMult(&Te, &D0, &TD0);
  femMatMatMult(&TD0, &TeT, D);

memFree:
	femVecFree(&sigma_1);
	femVecFree(&epsilon_1);
	femMatFree(&M);
	femMatFree(&Te);
	femMatFree(&TeT);
	femMatFree(&D0);
	femMatFree(&TD0);
	return(rv);
}


int fem_D_ccr3D(long ePos, 
                long e_rep, 
                long eT, 
                long mT, 
                tVector *sigma, 
                tVector *epsilon, 
                long Mode, 
                tMatrix *D)
{
  int    rv = AF_OK ;
  double A = 1.0 ;
  double x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  z1 = femGetNCoordPosZ(femGetENodePos(ePos,0));

  if (Elem[eT].nodes <= 8)
  {
    x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
    y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
    z2 = femGetNCoordPosZ(femGetENodePos(ePos,1));

    x3 = femGetNCoordPosX(femGetENodePos(ePos,4));
    y3 = femGetNCoordPosY(femGetENodePos(ePos,4));
    z3 = femGetNCoordPosZ(femGetENodePos(ePos,4));

    x4 = femGetNCoordPosX(femGetENodePos(ePos,3));
    y4 = femGetNCoordPosY(femGetENodePos(ePos,3));
    z4 = femGetNCoordPosZ(femGetENodePos(ePos,3));
  }
  else
  {
    if (Elem[eT].nodes <= 24)
    {
      x2 = femGetNCoordPosX(femGetENodePos(ePos,2));
      y2 = femGetNCoordPosY(femGetENodePos(ePos,2));
      z2 = femGetNCoordPosZ(femGetENodePos(ePos,2));

      x3 = femGetNCoordPosX(femGetENodePos(ePos,8));
      y3 = femGetNCoordPosY(femGetENodePos(ePos,8));
      z3 = femGetNCoordPosZ(femGetENodePos(ePos,8));

      x4 = femGetNCoordPosX(femGetENodePos(ePos,6));
      y4 = femGetNCoordPosY(femGetENodePos(ePos,6));
      z4 = femGetNCoordPosZ(femGetENodePos(ePos,6));
    }
    else
    {
      return(AF_ERR_SIZ);
    }
  }

  A = 
    pow(((
    sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2)) 
    *
    sqrt((x1-x3)*(x1-x3)+(y1-y3)*(y1-y3)+(z1-z3)*(z1-z3)) 
    *
    sqrt((x1-x4)*(x1-x4)+(y1-y4)*(y1-y4)+(z1-z4)*(z1-z4)) 
    ) / (1.0/3.0)), 2.0)
    ;

  rv = D_ConcCrack3D(ePos, e_rep-1, A, sigma, epsilon, Mode, D) ;

  return(rv);
}


/* end of fem_ccr3.c */

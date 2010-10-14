/*
   File name: fem_m009.c
   Date:      2006/11/26 16:04
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

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

	FEM Solver - Material 009 (brittle - clay brick)
*/

#include "fem_mat.h"

extern int D_HookIso_planeRaw(double E, double nu, long Problem, tMatrix *D);
#ifdef USE_NLMC
extern int D_Plane_OrthoCB(double R1, double R2, double nu, double G, double beta, double fi_1, tMatrix *Dcr); /* fem_ccr2 */
#endif

int e009_test(long mPos) 
{ 
	/* Datatest needed ;-) */
	return(AF_OK); 
}

/** fills (allocated!) D matrix
 * @param ePos finite element position
 * @param iPoint finite element (integration) point position
 * @param A area size
 * @param epsilon strain vector (NULL if unused)
 * @param Mode mode  AF_YES means changes, AF_NO=previous state
 * @param Problem 0=plane stress, 1=plane strain (1 is now ignored!)
 * @param D matrix to be filled
 */
int fem_m09_D_2d(
		long ePos, 
		long iPoint, 
		double A, 
		tVector *epsilon, 
		long Mode, 
		long Problem,
		tMatrix *D)
{
	int  rv = AF_OK ;
	double nu = 0.0 ;
	double E  = 0.0 ;
#ifdef USE_NLMC
	double E1    = 0.0 ;
  double G     = 0.0 ;
  double angle = 0.0 ;
  long   stat  = 0 ;
#endif

	E  = femGetMPValPos(ePos, MAT_EX, 0) ;
	nu = femGetMPValPos(ePos, MAT_NU, 0) ;

#ifdef USE_NLMC
  stat = (long)femGetEResVal(ePos, RES_STAT1, iPoint+1) ;
  if (stat != 0){angle = femGetEResVal(ePos, RES_DIR1, iPoint+1);}
  G = E / ( 2.0*(1.0 + nu) ) ;

  if (Mode == AF_YES)
  {
    switch (stat)
    {
      case  1: /* tension */
	      E1 = femGetEResVal(ePos, RES_ECR1, iPoint+1) ;
        if (E1 <= 0.0) {E1 = E ;}
 	      if ((rv = D_Plane_OrthoCB(E1, E, nu, G, 1.0, angle, D)) != AF_OK)
        break ;
      case -1: /*compression*/ 
	      E1 = femGetEResVal(ePos, RES_ECR1, iPoint+1) ;
        if (E1 <= 0.0) {E1 = E ;}
 	      if ((rv = D_Plane_OrthoCB(E1, E, nu, G, 1.0, angle, D)) != AF_OK)
        break ;
      case  0: /* undamaged */ 
      default:
	      E1 = E ;
	      rv = D_HookIso_planeRaw(E, nu, Problem, D); /* boring linear behaviour */
        break ;
    }
  }
  else
  {
    switch (stat) /* the same as for Mode=AF_YES, may be changed later */
    {
      case  1: /* tension */
	      E1 = femGetEResVal(ePos, RES_ECR1, iPoint+1) ;
        if (E1 <= 0.0) {E1 = E ;}
 	      if ((rv = D_Plane_OrthoCB(E1, E, nu, G, 1.0, angle, D)) != AF_OK)
        break ;
      case -1: /*compression*/ 
	      E1 = femGetEResVal(ePos, RES_ECR1, iPoint+1) ;
        if (E1 <= 0.0) {E1 = E ;}
 	      if ((rv = D_Plane_OrthoCB(E1, E, nu, G, 1.0, angle, D)) != AF_OK)
        break ;
      case  0: /* undamaged */ 
      default:
	      E1 = E ;
	      rv = D_HookIso_planeRaw(E, nu, Problem, D); /* boring linear behaviour */
        break ;
    }
  }
#else
	rv =  D_HookIso_planeRaw(E, nu, Problem, D); /* boring linear behaviour */
#endif

	return(rv);
}

int addMat_009(void)
{
	int rv = AF_OK;
	static long  type      = 9 ;
	static long  num       = 8 ;
	static long  val[8]    = {MAT_DENS, MAT_EX, MAT_NU,
    MAT_STEN, MAT_SCMPR, MAT_GF, MAT_E1, MAT_RAD};
	static long  num_rp    = 0 ;
	static long *val_rp    = NULL ;

	if (type != femAddMat(type)) {return(AF_ERR_VAL);}
	Mat[type].num = num ;
	Mat[type].val = val ;
	Mat[type].num_rp = num_rp ;
	Mat[type].val_rp = val_rp ;

	Mat[type].test = e009_test;
	return(rv);
}

/* end of fem_e009.c */

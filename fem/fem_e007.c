/*
   File name: fem_e007.c
   Date:      2003/04/09 13:11
   Author:    Jiri Brozovsky

   Copyright (C) 2003 

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

	FEM Solver - Element 007 (3D link)
*/

#include "fem_elem.h"

extern tVector u;

void FillTlink3d(double a, double b, double c, tMatrix *T)
{
  femMatSetZero(T);

	femMatPut(T, 1,1, a) ;
	femMatPut(T, 1,2, b) ;
	femMatPut(T, 1,3, c) ;

	femMatPut(T, 2,1, a) ;
	femMatPut(T, 2,2, b) ;
	femMatPut(T, 2,3, c) ;
}

int e007_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
	tMatrix T;  /* transformation matrix */
	tVector F_0;
	tVector u_e;
	double x1,y1,z1,x2,y2,z2,dx,dy,dz, L, Nas;
  double a,b,c;
	double Ax = 0;  /* area */
	double Ex = 0;  /* area */
	double Fx, sx, ex; /* results */
  long   i,j ;

	
	femMatNull(&T);
	femFullMatInit(&T,2,6);

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  z1 = femGetNCoordPosZ(femGetENodePos(ePos,0));
  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
  z2 = femGetNCoordPosZ(femGetENodePos(ePos,1));
	
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"E[%li]: x1 = %f, y1 = %f, x2 = %f y2 = %f\n",
			eID[ePos], x1,y1,x2,y2);
#endif

	dx = x2 - x1;
	dy = y2 - y1;
	dz = z2 - z1;
	L  = sqrt((dy*dy) + (dx*dx) + (dz*dz));

	a = dx/L;
	b = dy/L;
	c = dz/L;

	Ax = femGetRSValPos(ePos, RS_AREA, 0) ;
	Ex = femGetMPValPos(ePos, MAT_EX, 0) ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"Ax = %e, Ex = %e, L=%f a=%f b=%f c=%f\n",Ax,Ex,L,a,b,c);
#endif
	
	Nas = (Ex * Ax)/L;

  femMatSetZero(K_e) ;

  femMatPut( K_e, 1,1, a*a*Nas ) ;
  femMatPut( K_e, 1,2, a*b*Nas ) ;
  femMatPut( K_e, 1,3, a*c*Nas ) ;

  femMatPut( K_e, 2,1, a*b*Nas ) ;
  femMatPut( K_e, 2,2, b*b*Nas ) ;
  femMatPut( K_e, 2,3, b*c*Nas ) ;

  femMatPut( K_e, 3,1, a*c*Nas ) ;
  femMatPut( K_e, 3,2, b*c*Nas ) ;
  femMatPut( K_e, 3,3, c*c*Nas ) ;

  for (i=1; i<=3; i++)
  {
    for (j=1; j<=3; j++)
    {
      femMatPut(K_e, i+3, j, (-1.0)*femMatGet(K_e, i,j)) ;
      femMatPut(K_e, i+3, j+3,      femMatGet(K_e, i,j)) ;
      femMatPut(K_e, i, j+3, (-1.0)*femMatGet(K_e, i,j)) ;
    }
  }


	if (Mode == AF_YES) /* result, F_e, Fr_e */
	{
		femVecNull(&F_0);
		femVecFullInit(&F_0,2);
		femVecNull(&u_e);
		femVecFullInit(&u_e,6);
    
    femLocUtoU_e(&u, ePos, &u_e);

		femMatVecMult(K_e, &u_e, F_e); /* global c.s. */

 		FillTlink3d(a, b, c, &T);

		femVecSetZero(Fr_e); /* residual forces ;-) */

	  femMatVecMult(&T, F_e, &F_0);   /* local c.s.  */

		Fx = femVecGet(&F_0, 1) * (-1.0) ;
		sx = Fx / Ax ;
		ex = sx/Ex ;


	  if (femTangentMatrix == AF_YES)
	  {
	    femAddEResVal(ePos, RES_FX, 0, Fx);
	    femAddEResVal(ePos, RES_SX, 0, sx);
	    femAddEResVal(ePos, RES_EX, 0, ex);
	  }
	  else
	  {
	    femPutEResVal(ePos, RES_FX, 0, Fx);
	    femPutEResVal(ePos, RES_SX, 0, sx);
	    femPutEResVal(ePos, RES_EX, 0, ex);
	  }
		

		femVecFree(&F_0);
		femVecFree(&u_e);
	}

	femMatFree(&T);
	return(AF_OK);
}

double e007_length(long ePos)
{
	double x1,y1,x2,y2,dx,dy;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));

	dx = x2 - x1;
	dy = y2 - y1;
	return ( sqrt((dy*dy) + (dx*dx)) );
}


int e007_mass(long ePos, tMatrix *M_e)
{
	int rv = AF_OK;
	double Ax, ro, mass;
	double L;

	L  = e007_length(ePos);

	Ax = femGetRSValPos(ePos, RS_AREA, 0) ;
	ro = femGetMPValPos(ePos, MAT_DENS, 0) ;

	mass = Ax * L *ro ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"Ax = %f, ro = %f, L=%f mass=%f\n",Ax,ro,L,mass);
#endif

	femMatSetZero(M_e);

	femMatPut(M_e, 1,1, (mass /2.0) ) ;
	femMatPut(M_e, 2,2, (mass /2.0) ) ;
	femMatPut(M_e, 3,3, (mass /2.0) ) ;
	femMatPut(M_e, 4,4, (mass /2.0) ) ;
	femMatPut(M_e, 5,5, (mass /2.0) ) ;
	femMatPut(M_e, 5,5, (mass /2.0) ) ;

	return(rv);
}

int e007_volume(long ePos, double *vol)
{
	int rv = AF_OK;
	double Ax, ro, mass;
	double L;

	L  = e007_length(ePos);
	Ax = femGetRSValPos(ePos, RS_AREA, 0) ;

	*vol = Ax * L ;
	return(rv);
}

long e007_rvals(long ePos)
{
	return(3);
}

int e007_res_p_loc(long ePos, long point, double *x, double *y, double *z)
{
	double x1,x2,y1,y2, z1, z2 ;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  z1 = femGetNCoordPosZ(femGetENodePos(ePos,0));
  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
  z2 = femGetNCoordPosZ(femGetENodePos(ePos,1));

	*x = 0.5*(x1+x2) ;
	*y = 0.5*(y1+y2) ;
	*z = 0.5*(z1+z2) ;

	return(AF_OK);
}


int e007_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

int addElem_007(void)
{
	int rv = AF_OK;
	static long type    = 7 ;
	static long nodes   = 2 ;
	static long dofs    = 3 ;
	static long ndof[3] = {U_X,U_Y,U_Z} ;
	static long rs      = 3 ;
	static long real[3] = {RS_AREA,RS_INERTIA_Y,RS_INERTIA_Z} ;
	static long rs_rp      = 0 ;
	static long *real_rp = NULL ;
	static long res      = 3 ;
	static long nres[3] = {RES_FX,RES_SX,RES_EX} ;
	static long res_rp      = 0 ;
	static long *nres_rp = NULL ;

	if (type != femAddElem(type)) {return(AF_ERR_VAL);}
	Elem[type].nodes = nodes ;
	Elem[type].dofs = dofs ;
	Elem[type].ndof = ndof ;
	Elem[type].rs = rs ;
	Elem[type].real = real ;
	Elem[type].rs_rp = rs_rp ;
	Elem[type].real_rp = real_rp ;
	Elem[type].res = res ;
	Elem[type].nres = nres ;
	Elem[type].res_rp = res_rp ;
	Elem[type].nres_rp = nres_rp ;

	Elem[type].stiff = e007_stiff;
	Elem[type].mass  = e007_mass;
	Elem[type].rvals = e007_rvals;
	Elem[type].eload = e007_eload;
	Elem[type].res_p_loc = e007_res_p_loc;
	Elem[type].res_node = e000_res_node;
	return(rv);
}


/* end of fem_e007.c */

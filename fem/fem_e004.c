/*
   File name: fem_e004.c
   Date:      Tue Oct  7 14:18:04 CEST 2004
   Author:    Jiri Brozovsky

   Copyright (C) 2004  Jiri Brozovsky

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

	 FEM Solver - Element 004 (3D - tetrahedron by Kolar et al)

	 $Id: fem_e004.c,v 1.8 2004/01/11 18:23:25 jirka Exp $
*/

#include "fem_elem.h"
#include "fem_pl3d.h"

#ifndef _SMALL_FEM_CODE_
extern tVector u;


int e004_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
	int rv = AF_OK ;
	tMatrix D;      /* 6x6   */
	tMatrix D_n;    /* 6x6   */
	tMatrix S;      /* 12x12 */
	tMatrix St;     /* 12x12 */
	tMatrix B;      /* 6x12  */
	tMatrix Bt;     /* 12x6  */
  tMatrix StBt;   /* 12x6  */
  tMatrix StBtD;  /* 12x6  */
  tMatrix BS;     /* 6x12  */
  tVector F_0 ;
  tVector u_e ;
  tVector epsilon ;
  tVector sigma ;
  tVector d_sigma;
	double  volume ;
	double x1,y1,x2,y2,x3,y3,x4,y4,z1,z2,z3,z4;
	double a1,b1,c1,a2,b2,c2,a3,b3,c3 ;
  double val1, val2;
	long   eT, mT;
	long   i;

	eT = femGetETypePos(ePos); 
	mT = femGetEMPPos(ePos); 

  femMatSetZero(K_e);

	femMatNull(&D);
	femMatNull(&S);
	femMatNull(&St);
	femMatNull(&B);
	femMatNull(&Bt);
	femMatNull(&StBt);
	femMatNull(&StBtD);
	femMatNull(&BS);

	if ((rv=femFullMatInit(&D,6,6)) != AF_OK) { goto memFree; }
  if ((rv=femFullMatInit(&S,12,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&St,12,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&B,6,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&Bt,12,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&StBt,12,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&StBtD,12,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&BS,6,12)) != AF_OK) { goto memFree; }

  if (Mode == AF_YES)
  {
		femVecNull(&F_0);
		femVecNull(&u_e);
		femVecNull(&epsilon);
		femVecNull(&sigma);
		femVecNull(&d_sigma);
	  femMatNull(&D_n);

		if ((femVecFullInit(&F_0,12)) != AF_OK) { goto memFree; }
		if ((femVecFullInit(&u_e,12)) != AF_OK) { goto memFree; }
		if ((femVecFullInit(&epsilon,6)) != AF_OK) { goto memFree; }
		if ((femVecFullInit(&sigma,6)) != AF_OK) { goto memFree; }
		if ((femVecFullInit(&d_sigma,6)) != AF_OK) { goto memFree; }
	  if ((rv=femFullMatInit(&D_n,6,6)) != AF_OK) { goto memFree; }
  }

  /* coordinates */
  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  z1 = femGetNCoordPosZ(femGetENodePos(ePos,0));

  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
  z2 = femGetNCoordPosZ(femGetENodePos(ePos,1));

  x3 = femGetNCoordPosX(femGetENodePos(ePos,2));
  y3 = femGetNCoordPosY(femGetENodePos(ePos,2));
  z3 = femGetNCoordPosZ(femGetENodePos(ePos,2));

  x4 = femGetNCoordPosX(femGetENodePos(ePos,3));
  y4 = femGetNCoordPosY(femGetENodePos(ePos,3));
  z4 = femGetNCoordPosZ(femGetENodePos(ePos,3));
	
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"E[%li]: x1=%f y1=%f z1=%f, x2=%f y2=%f z2=%f. x3=%f y3=%f z3=%f x4=%f y4=%f z4=%f\n",
			eID[ePos],  x1,y1,z1,x2,y2,z2, x3,y3,z3,x4,y4,z4 );
#endif

	a1 = x2 - x1 ;
	a2 = y2 - y1 ;
	a3 = z2 - z1 ;

	b1 = x3 - x1 ;
	b2 = y3 - y1 ;
	b3 = z3 - z1 ;

	c1 = x4 - x1 ;
	c2 = y4 - y1 ;
	c3 = z4 - z1 ;

	/* "D" creation: */
  if ((rv = fem_D_3D(ePos, 0, eT, mT, NULL, NULL, AF_NO, &D) ) != AF_OK) {goto memFree;}


	/* "B" matrix: */
	femMatSetZero(&B);

	femMatPut(&B, 1,2,  1.0) ;  
	femMatPut(&B, 2,7,  1.0) ;  
	femMatPut(&B, 3,12, 1.0) ;  
	femMatPut(&B, 4,8,  1.0) ;  
	femMatPut(&B, 5,4,  1.0) ;  
	femMatPut(&B, 5,10, 1.0) ;  
	femMatPut(&B, 6,3,  1.0) ;  
	femMatPut(&B, 6,6,  1.0) ;  

	femMatTran(&B, &Bt);


	/* "S" matrix: */

  for (i=0; i<3; i++)
  {
	  femMatPut(&S, 1+i,1+(4*i),  1.0) ;  
	  femMatPut(&S, 1+i,2+(4*i),  x1) ;  
	  femMatPut(&S, 1+i,3+(4*i),  y1) ;  
	  femMatPut(&S, 1+i,4+(4*i),  z1) ;  

	  femMatPut(&S, 4+i,1+(4*i),  1.0) ;  
	  femMatPut(&S, 4+i,2+(4*i),  x2) ;  
	  femMatPut(&S, 4+i,3+(4*i),  y2) ;  
	  femMatPut(&S, 4+i,4+(4*i),  z2) ;  

	  femMatPut(&S, 7+i,1+(4*i),  1.0) ;  
	  femMatPut(&S, 7+i,2+(4*i),  x3) ;  
	  femMatPut(&S, 7+i,3+(4*i),  y3) ;  
	  femMatPut(&S, 7+i,4+(4*i),  z3) ;  

	  femMatPut(&S, 10+i,1+(4*i),  1.0) ;  
	  femMatPut(&S, 10+i,2+(4*i),  x4) ;  
	  femMatPut(&S, 10+i,3+(4*i),  y4) ;  
	  femMatPut(&S, 10+i,4+(4*i),  z4) ;  
  }


  /* hope this works (inversion of "S"): */
  if (femLUinverse(&S) != AF_OK)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout, "[E] %s!\n", _("Inversion of S failed"));
#endif
  }

  femMatTran(&S, &St);

	femMatMatMult(&St, &Bt, &StBt);
	femMatMatMult(&B, &S, &BS);
	
  /* element stiffness matrix: */
  femMatMatMult(&StBtD, &BS, K_e);
	volume = ( (a1*b2*c3 + c1*a2*b3 + a3*b1*c2) - (c1*b2*a3 + c3*b1*a2 + a1*b3*c2) ) / (6.0) ;
	femValMatMultSelf(volume, K_e);


	/* ====================================================================== */
	if (Mode == AF_YES) /* result, F_e, Fr_e */
	{
		/* 
		 probably epsilon = B* S^-1 * u_e ;-)
              6,1 =    6,12 12,12 12,1
		 */

    /* obtaining of element displacements */
    femLocUtoU_e(&u, ePos, &u_e);

    femMatVecMult(K_e, &u_e, F_e);/* nodal forces */
    

    /* strains and stresses: */
		femMatVecMult(&BS, &u_e, &epsilon);
		femMatVecMult(&D, &epsilon, &sigma);


    /* new stiffness matrix: */
    if ((rv = fem_D_3D(ePos, 0, eT, mT, &sigma, &epsilon, Mode, &D_n) ) != AF_OK) {goto memFree;}

    femMatVecMult(&D_n, &epsilon, &d_sigma) ;

    /* stresses: */
    for (i=1; i<=6; i++)
    {
      val1 = femVecGet(&sigma,   i) ;
      val2 = femVecGet(&d_sigma, i) ;

      val1 -= val2 ;

      femVecPut(&sigma,   i, val2) ; /* current stress  */
      femVecPut(&d_sigma, i, val1) ; /* residual stress */
    }

    /* saving of results: */
	  if (femTangentMatrix == AF_YES)
	  {
	    femAddEResVal(ePos, RES_SX, 0, femVecGet(&sigma,1));
	    femAddEResVal(ePos, RES_SY, 0, femVecGet(&sigma,2));
	    femAddEResVal(ePos, RES_SZ, 0, femVecGet(&sigma,3));

	    femAddEResVal(ePos, RES_SYZ, 0, femVecGet(&sigma,4));
	    femAddEResVal(ePos, RES_SZX, 0, femVecGet(&sigma,5));
	    femAddEResVal(ePos, RES_SXY, 0, femVecGet(&sigma,6));

	    femAddEResVal(ePos, RES_EX, 0, femVecGet(&epsilon,1));
	    femAddEResVal(ePos, RES_EY, 0, femVecGet(&epsilon,2));
	    femAddEResVal(ePos, RES_EZ, 0, femVecGet(&epsilon,3));

	    femAddEResVal(ePos, RES_EYZ, 0, femVecGet(&epsilon,4));
	    femAddEResVal(ePos, RES_EZX, 0, femVecGet(&epsilon,5));
	    femAddEResVal(ePos, RES_EXY, 0, femVecGet(&epsilon,6));
	  }
	  else
	  {
	    femPutEResVal(ePos, RES_SX, 0, femVecGet(&sigma,1));
	    femPutEResVal(ePos, RES_SY, 0, femVecGet(&sigma,2));
	    femPutEResVal(ePos, RES_SZ, 0, femVecGet(&sigma,3));

	    femPutEResVal(ePos, RES_SYZ, 0, femVecGet(&sigma,4));
	    femPutEResVal(ePos, RES_SZX, 0, femVecGet(&sigma,5));
	    femPutEResVal(ePos, RES_SXY, 0, femVecGet(&sigma,5));

	    femPutEResVal(ePos, RES_EX, 0, femVecGet(&epsilon,1));
	    femPutEResVal(ePos, RES_EY, 0, femVecGet(&epsilon,2));
	    femPutEResVal(ePos, RES_EZ, 0, femVecGet(&epsilon,3));

	    femPutEResVal(ePos, RES_EYZ, 0, femVecGet(&epsilon,4));
	    femPutEResVal(ePos, RES_EZX, 0, femVecGet(&epsilon,4));
	    femPutEResVal(ePos, RES_EXY, 0, femVecGet(&epsilon,6));
	  }
		

    /* residual forces:  */
		femVecSetZero(Fr_e); 
    femMatVecMult(&StBt, &d_sigma, Fr_e) ;

    femVecPrn(Fr_e,"RESIDUAL FORCES");
	}
	
memFree:
	femMatFree(&D);
	femMatFree(&S);
	femMatFree(&St);
	femMatFree(&B);
	femMatFree(&Bt);
	femMatFree(&StBt);
	femMatFree(&StBtD);
	femMatFree(&BS);

  if (Mode == AF_YES)
  {
	  femMatFree(&D_n);
		femVecFree(&epsilon);
		femVecFree(&sigma);
		femVecFree(&d_sigma);
		femVecFree(&u_e);
  }

	return(AF_OK);
}

int e004_mass(long ePos, tMatrix *M_e)
{
	int    rv = AF_OK ;
	double x1,y1,x2,y2,x3,y3,x4,y4,z1,z2,z3,z4 ;
	double a1,b1,c1,a2,b2,c2,a3,b3,c3 ;
	double volume = 0 ;
	double dens   = 0 ;
	int    i ;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  z1 = femGetNCoordPosZ(femGetENodePos(ePos,0));

  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
  z2 = femGetNCoordPosZ(femGetENodePos(ePos,1));

  x3 = femGetNCoordPosX(femGetENodePos(ePos,2));
  y3 = femGetNCoordPosY(femGetENodePos(ePos,2));
  z3 = femGetNCoordPosZ(femGetENodePos(ePos,2));

  x4 = femGetNCoordPosX(femGetENodePos(ePos,3));
  y4 = femGetNCoordPosY(femGetENodePos(ePos,3));
  z4 = femGetNCoordPosZ(femGetENodePos(ePos,3));


	a1 = x2 - x1 ;
	a2 = y2 - y1 ;
	a3 = z2 - z1 ;

	b1 = x3 - x1 ;
	b2 = y3 - y1 ;
	b3 = z3 - z1 ;

	c1 = x4 - x1 ;
	c2 = y4 - y1 ;
	c3 = z4 - z1 ;

	volume = ( (a1*b2*c3 + c1*a2*b3 + a3*b1*c2) - (c1*b2*a3 + c3*b1*a2 + a1*b3*c2) ) / (6.0) ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"volume = %e \n",volume);
#endif

	dens = femGetMPValPos(ePos, MAT_DENS, 0) ;

	for (i=1; i<=12; i++)
	{
		femMatPut(M_e,i,i, ( (dens*volume)/(12.0) ) ) ;
	}

	return(rv);
}

int e004_volume(long ePos, double *vol)
{
	int    rv = AF_OK ;
	double x1,y1,x2,y2,x3,y3,x4,y4,z1,z2,z3,z4 ;
	double a1,b1,c1,a2,b2,c2,a3,b3,c3 ;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  z1 = femGetNCoordPosZ(femGetENodePos(ePos,0));

  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
  z2 = femGetNCoordPosZ(femGetENodePos(ePos,1));

  x3 = femGetNCoordPosX(femGetENodePos(ePos,2));
  y3 = femGetNCoordPosY(femGetENodePos(ePos,2));
  z3 = femGetNCoordPosZ(femGetENodePos(ePos,2));

  x4 = femGetNCoordPosX(femGetENodePos(ePos,3));
  y4 = femGetNCoordPosY(femGetENodePos(ePos,3));
  z4 = femGetNCoordPosZ(femGetENodePos(ePos,3));


	a1 = x2 - x1 ;
	a2 = y2 - y1 ;
	a3 = z2 - z1 ;

	b1 = x3 - x1 ;
	b2 = y3 - y1 ;
	b3 = z3 - z1 ;

	c1 = x4 - x1 ;
	c2 = y4 - y1 ;
	c3 = z4 - z1 ;

	*vol = ( (a1*b2*c3 + c1*a2*b3 + a3*b1*c2) - (c1*b2*a3 + c3*b1*a2 + a1*b3*c2) ) / (6.0) ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"volume = %e \n", *vol);
#endif
	return(rv);
}


long e004_rvals(long ePos)
{
	return(14);
}

int e004_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

int e004_res_p_loc(long ePos, long point, double *x, double *y, double *z)
{
	long i ;

	*x = 0 ;
	*y = 0 ;
	*z = 0 ;

	for (i=0; i<4; i++)
	{

  	*x += femGetNCoordPosX(femGetENodePos(ePos,i));
  	*y += femGetNCoordPosY(femGetENodePos(ePos,i));
  	*z += femGetNCoordPosZ(femGetENodePos(ePos,i));
	}

	*x = *x / 4.0 ;
	*y = *y / 4.0 ;
	*z = *z / 4.0 ;

	return(AF_OK);
}

/** Computes temperature load for element 011 */
int e004_therm(long ePos, long *use, tVector *Te, tVector *Fe)
{
	int     rv = AF_OK; 
  tMatrix D;      /* 6x6   */
	tMatrix S;      /* 12x12 */
	tMatrix St;     /* 12x12 */
	tMatrix B;      /* 6x12  */
	tMatrix Bt;     /* 12x6  */
  tMatrix StBt;   /* 12x6  */
  tMatrix StBtD;  /* 12x6  */
  tVector epsilon;
	double  volume ;
	double x1,y1,x2,y2,x3,y3,x4,y4,z1,z2,z3,z4;
	double a1,b1,c1,a2,b2,c2,a3,b3,c3 ;
  double alpha, temp ;
	long   eT, mT;
	long   i;

  *use = AF_YES;

	eT = femGetETypePos(ePos); 
	mT = femGetEMPPos(ePos); 

  alpha = femGetMPValPos(ePos, MAT_ALPHA, 0) ;

	femMatNull(&D);
	femMatNull(&S);
	femMatNull(&St);
	femMatNull(&B);
	femMatNull(&Bt);
	femMatNull(&StBt);
	femMatNull(&StBtD);
	femVecNull(&epsilon);

	if ((rv=femFullMatInit(&D,6,6)) != AF_OK) { goto memFree; }
  if ((rv=femFullMatInit(&S,12,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&St,12,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&B,6,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&Bt,12,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&StBt,12,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&StBtD,12,6)) != AF_OK) { goto memFree; }
	if ((femVecFullInit(&epsilon,6)) != AF_OK) { goto memFree; }

  /* coordinates */
  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  z1 = femGetNCoordPosZ(femGetENodePos(ePos,0));

  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
  z2 = femGetNCoordPosZ(femGetENodePos(ePos,1));

  x3 = femGetNCoordPosX(femGetENodePos(ePos,2));
  y3 = femGetNCoordPosY(femGetENodePos(ePos,2));
  z3 = femGetNCoordPosZ(femGetENodePos(ePos,2));

  x4 = femGetNCoordPosX(femGetENodePos(ePos,3));
  y4 = femGetNCoordPosY(femGetENodePos(ePos,3));
  z4 = femGetNCoordPosZ(femGetENodePos(ePos,3));
	
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"E[%li]: x1=%f y1=%f z1=%f, x2=%f y2=%f z2=%f. x3=%f y3=%f z3=%f x4=%f y4=%f z4=%f\n",
			eID[ePos],  x1,y1,z1,x2,y2,z2, x3,y3,z3,x4,y4,z4 );
#endif

	a1 = x2 - x1 ;
	a2 = y2 - y1 ;
	a3 = z2 - z1 ;

	b1 = x3 - x1 ;
	b2 = y3 - y1 ;
	b3 = z3 - z1 ;

	c1 = x4 - x1 ;
	c2 = y4 - y1 ;
	c3 = z4 - z1 ;

	/* "D" creation: */
  if ((rv = fem_D_3D(ePos, 0, eT, mT, NULL, NULL, AF_NO, &D) ) != AF_OK) {goto memFree;}


	/* "B" matrix: */
	femMatSetZero(&B);

	femMatPut(&B, 1,2,  1.0) ;  
	femMatPut(&B, 2,7,  1.0) ;  
	femMatPut(&B, 3,12, 1.0) ;  
	femMatPut(&B, 4,8,  1.0) ;  
	femMatPut(&B, 5,4,  1.0) ;  
	femMatPut(&B, 5,10, 1.0) ;  
	femMatPut(&B, 6,3,  1.0) ;  
	femMatPut(&B, 6,6,  1.0) ;  

	femMatTran(&B, &Bt);


	/* "S" matrix: */
  for (i=0; i<3; i++)
  {
	  femMatPut(&S, 1+i,1+(4*i),  1.0) ;  
	  femMatPut(&S, 1+i,2+(4*i),  x1) ;  
	  femMatPut(&S, 1+i,3+(4*i),  y1) ;  
	  femMatPut(&S, 1+i,4+(4*i),  z1) ;  

	  femMatPut(&S, 4+i,1+(4*i),  1.0) ;  
	  femMatPut(&S, 4+i,2+(4*i),  x2) ;  
	  femMatPut(&S, 4+i,3+(4*i),  y2) ;  
	  femMatPut(&S, 4+i,4+(4*i),  z2) ;  

	  femMatPut(&S, 7+i,1+(4*i),  1.0) ;  
	  femMatPut(&S, 7+i,2+(4*i),  x3) ;  
	  femMatPut(&S, 7+i,3+(4*i),  y3) ;  
	  femMatPut(&S, 7+i,4+(4*i),  z3) ;  

	  femMatPut(&S, 10+i,1+(4*i),  1.0) ;  
	  femMatPut(&S, 10+i,2+(4*i),  x4) ;  
	  femMatPut(&S, 10+i,3+(4*i),  y4) ;  
	  femMatPut(&S, 10+i,4+(4*i),  z4) ;  
  }

  /* hope this works (inversion of "S"): */
  if (femLUinverse(&S) != AF_OK)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout, "[E] %s!\n", _("Inversion of S failed"));
#endif
  }

  femMatTran(&S, &St);

	volume = ( (a1*b2*c3 + c1*a2*b3 + a3*b1*c2) - (c1*b2*a3 + c3*b1*a2 + a1*b3*c2) ) / (6.0) ;

  /* set temperature strains: */
  temp = (  femVecGet(Te, 1) + femVecGet(Te, 2)
       + femVecGet(Te, 3) + femVecGet(Te, 4)  ) / 4.0 ;
  femVecPut(&epsilon, 1, alpha * temp );
  femVecPut(&epsilon, 2, alpha * temp );
  femVecPut(&epsilon, 3, 0.0 );

	/* stiffness matrix is computed */
	femMatMatMult(&St, &Bt, &StBt);
	femMatMatMult(&StBt, &D, &StBtD);
	femMatVecMult(&StBtD, &epsilon, Fe) ;
	femValVecMultSelf(volume, Fe) ; 
	
memFree:
	femVecFree(&epsilon);

  femMatFree(&D);
	femMatFree(&S);
	femMatFree(&St);
	femMatFree(&B);
	femMatFree(&Bt);
	femMatFree(&StBt);
	femMatFree(&StBtD);
	return(rv);
}

int addElem_004(void)
{
	int rv = AF_OK;
	static long type    = 4 ;
	static long nodes   = 4 ;
	static long dofs    = 3 ;
	static long ndof[3] = {U_X,U_Y,U_Z} ;
	static long rs      = 0 ;
	static long *real = NULL ;
	static long rs_rp      = 0 ;
	static long *real_rp = NULL ;
	static long res      = 14 ;
	static long nres[14]    = {
		RES_SX,RES_SY,RES_SZ,RES_SYZ,RES_SZX,RES_SXY,
		RES_EX,RES_EY,RES_EZ,RES_EYZ,RES_EZX,RES_EXY,
    RES_CR1,RES_PSI
	};
	static long res_rp   = 0 ;
	static long *nres_rp = NULL;

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

	Elem[type].stiff = e004_stiff;
	Elem[type].mass  = e004_mass;
	Elem[type].rvals = e004_rvals;
	Elem[type].eload = e004_eload;
	Elem[type].res_p_loc = e004_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e004_volume;
	Elem[type].therm = e004_therm;
	return(rv);
}

#endif
/* end of fem_e004.c */

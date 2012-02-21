/*
   File name: fem_e002.c
   Date:      2003/04/09 13:12
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

	 FEM Solver - element 002 (isoparametric plane) definition
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_

/** Evaluates shape functions and their derivatives
 *  @param deriv matrix with derivatives (2,9)
 *  @param nnode    number of nodes (4,8,9)
 *  @param etasp    
 *  @param exitsp  
 *  @param shape    shape funciton (result)[9]
 *  @param deriv    derivatives (result)[2,9]
 *  @return state value
 */
long sfr(
				 long     nnode, 
		     double   etasp, 
				 double   exitsp, 
				 tVector *shape,
         tMatrix *deriv
				 )
{
	double s,t,st,ss,tt, s2,t2, sst, stt, st2, s1, t1, s9, t9;

	s = exitsp;
	t = etasp;
	st = s * t;

	switch (nnode)
	{
		case 4:  	/* 4 - node element */
							femVecPut(shape,1, ( ( 1.0 - t - s + st ) * 0.25 ));
							femVecPut(shape,2, ( ( 1.0 - t + s - st ) * 0.25 ));
							femVecPut(shape,3, ( ( 1.0 - t - s + st ) * 0.25 ));
							femVecPut(shape,4, ( ( 1.0 - t - s - st ) * 0.25 ));
							
							femMatPut(deriv,1,1, ( ( -1.0 + t ) * 0.25 ));
							femMatPut(deriv,1,2, ( ( +1.0 - t ) * 0.25 ));
							femMatPut(deriv,1,3, ( ( +1.0 + t ) * 0.25 ));
							femMatPut(deriv,1,4, ( ( -1.0 - t ) * 0.25 ));
							femMatPut(deriv,2,1, ( ( -1.0 + s ) * 0.25 ));
							femMatPut(deriv,2,2, ( ( -1.0 - s ) * 0.25 ));
							femMatPut(deriv,2,3, ( ( +1.0 + s ) * 0.25 ));
							femMatPut(deriv,2,4, ( ( +1.0 - s ) * 0.25 ));
			       	break;
	  case 8:  	/* 8 - node element */
							s2 = s * 2.0;
							t2 = t * 2.0;
							ss = s * s;
							tt = t * t;
							st = s * t;
							sst = s * s * t;
							stt = s * t * t;
							st2 = s * t * 2.0;
							
							femVecPut(shape,1, ( ( -1.0 + st + ss + tt - sst - stt ) * 0.25 ));
							femVecPut(shape,2, ( ( +1.0 - t -ss + sst ) * 0.5 ));
							femVecPut(shape,3, ( ( -1.0 - st + ss + tt - sst + stt ) * 0.25 ));
							femVecPut(shape,4, ( ( +1.0 + s - tt - stt ) * 0.5 ));
							femVecPut(shape,5, ( ( -1.0 + st + ss + tt + sst + stt ) * 0.25 ));
							femVecPut(shape,6, ( ( +1.0 + t - ss - sst ) * 0.5 ));
							femVecPut(shape,7, ( ( -1.0 - st + ss + tt + sst - stt ) * 0.25 ));
							femVecPut(shape,8, ( ( +1.0 - s - tt + stt ) * 0.25 ));
						
							femMatPut(deriv,1,1, ( ( t + s2 - st2 - tt ) * 0.25 ));
							femMatPut(deriv,1,2, ( -s + st ));
							femMatPut(deriv,1,3, ( ( -t + s2 - st2 + tt ) * 0.25 ));
							femMatPut(deriv,1,4, ( ( 1.0 - tt ) * 0.5 ));
							femMatPut(deriv,1,5, ( ( t + s2 + st2 +tt ) * 0.25 ));
							femMatPut(deriv,1,6, ( -s - st ));
							femMatPut(deriv,1,7, ( ( -t + s2 + st2 - tt ) * 0.25 ));
							femMatPut(deriv,1,8, ( ( -1.0 + tt ) * 0.5 ));
							femMatPut(deriv,2,1, ( ( s + t2 - ss - st2 ) * 0.25 ));
							femMatPut(deriv,2,2, ( ( -1.0 + ss ) * 0.5 ));
							femMatPut(deriv,2,3, ( ( -s + t2 - ss + st2 ) * 0.25 ));
							femMatPut(deriv,2,4, ( -t - st ));
							femMatPut(deriv,2,5, ( ( s + t2 + ss + st2 ) * 0.25 ));
							femMatPut(deriv,2,6, ( ( 1.0 - ss ) * 0.5 ));
							femMatPut(deriv,2,7, ( ( -s + t2 +ss -st2 ) * 0.25 ));
							femMatPut(deriv,2,8, ( -t + st ));
						 	break;
	  case 9:  	/* 9 - node element */
							ss = s * s;
							st = s * t;
							tt = t * t;
							s1 = s + 1.0;
							t1 = t + 1.0;
							s2 = s * 2.0;
							t2 = t * 2.0;
							s9 = s - 1.0;
							t9 = t - 1.0;
							
							femVecPut(shape,1, ( 0.25 * s9 * st * t9 ));
							femVecPut(shape,2, ( 0.5 * ( 1.0 - ss ) * t * t9 ));
							femVecPut(shape,3, ( 0.25 * s1 * st * t9 ));
							femVecPut(shape,4, ( 0.5 * s * s1 * ( 1.0 - tt ) ));
							femVecPut(shape,5, ( 0.25 * s1 * st * t1 ));
							femVecPut(shape,6, ( 0.5 * (1.0 - ss ) * t * t1 ));
							femVecPut(shape,7, ( 0.5 * s9 * st * t1 ));
							femVecPut(shape,8, ( 0.5 * s *s9 * ( 1.0 - tt ) ));
							femVecPut(shape,8, ( (1.0 - ss) * ( 1.0 - tt ) ));
							
							femMatPut(deriv,1,1, ( 0.25 * t * t9 * ( -1.0 + s2 ) ));
							femMatPut(deriv,1,2, ( -st * t9 ));
							femMatPut(deriv,1,3, ( 0.25 * ( 1.0 + s2 ) * t * t9 ));
							femMatPut(deriv,1,4, ( 0.5 * ( 1.0 + s2 ) * ( 1.0 - tt ) ));
							femMatPut(deriv,1,5, ( 0.25 * ( 1.0 + s2 ) * t * t1 ));
							femMatPut(deriv,1,6, ( -st * t1 ));
							femMatPut(deriv,1,7, ( 0.25 * ( -1.0 + s2 ) * t * t1 ));
							femMatPut(deriv,1,8, ( 0.5 * ( -1.0 + s2 ) * ( 1.0 - tt ) ));
							femMatPut(deriv,1,9, ( -s2 * (1.0 - tt ) ));
							femMatPut(deriv,2,1, ( 0.25 * ( -1.0 + t2 ) * s * s9 ));
							femMatPut(deriv,2,2, ( 0.5 * ( 1.0 - ss ) * ( -1.0 +t2 ) ));
							femMatPut(deriv,2,3, ( 0.25 * s * s1 * ( -1.0 + t2 ) ));
							femMatPut(deriv,2,4, ( -st * s1 ));
							femMatPut(deriv,2,5, ( 0.25 * s * s1 * ( 1.0 + t2 ) ));
							femMatPut(deriv,2,6, ( 0.5  * ( 1.0 - ss ) * ( 1.0 + t2 ) ));
							femMatPut(deriv,2,7, ( 0.25 * s *s9 * ( 1.0 + t2 ) ));
							femMatPut(deriv,2,8, ( -st * s9 ));
							femMatPut(deriv,2,9, ( -t2 * ( 1.0 - ss ) ));
						 	break;
	}
	return(AF_OK);
}


/** Sets constant for Gauss integration 
 *  @param ngaus   number of gauss point (2,3)
 *  @param posgp  point coordinates [4]
 *  @param weigp   weights [4]
 *  @return status value
 */
long gaussq(long ngaus, tVector *posgp, tVector *weigp)
{
	long    kgaus;
	long    igash, jgash;
	
	if (ngaus <= 2)
	{
		femVecPut(posgp,1, ( -0.577350569189626 ));
		femVecPut(weigp,1, (  1.0 ));
	}
	else
	{
		femVecPut(posgp,1, ( -0.774596669241483 ));
		femVecPut(posgp,2, (  0.0 ));
		femVecPut(weigp,1, (  0.555555555555556 ));
		femVecPut(weigp,2, (  0.888888888888889 ));
	}
	
	kgaus = (long)(ngaus / 2);

	for (igash = 1; igash <= kgaus; igash++)
	{
		jgash = ngaus + 1 - igash ;
		femVecPut(posgp,jgash, ( -1 * femVecGet(posgp, igash) ));
		femVecPut(weigp,jgash, (  femVecGet(weigp, igash) ));
	}
	
	return(AF_OK);
}


/** Evaluates Jacobian matrix and carthesian shape function derivatives 
 *  @param cartd   carthesian s. f. derivatives[2][9]
 *  @param elcod   coordinates[2][9]
 *  @param djacb   determinant[?]
 *  @param nnode   number of nodes
 *  @return state value
 */
long jacob(
		       tMatrix *cartd, 
					 tMatrix *deriv, 
					 tMatrix *elcod,
	         double  *djacb,
					 long     nnode
					 )
{
	/* long    ielem; - potreba jen pri chybe - pro vyjasneni cisla prvku */

	double xjaci[3][3];
	double xjacm[3][3];
	long    idime, inode, jdime;

	for (idime = 1; idime <= 2; idime++)
	{
	  for (jdime = 1; jdime <= 2; jdime++) 
		{ 
			xjacm[idime][jdime] = 0 ; 

	    for (inode=1; inode<= nnode; inode++)
	    {
		    xjacm[idime][jdime] += (femMatGet(deriv,idime,inode)*femMatGet(elcod,jdime,inode));
	    }
		}
	}

	*djacb = (xjacm[1][1]*xjacm[2][2]) - (xjacm[1][2]*xjacm[2][1]) ;
	if (*djacb <= 0) 
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s: %e!\n",_("Jacobi matrix: zero determinant found"), *djacb);
#endif
		return(AF_ERR_VAL);
	}
	
	xjaci[1][1] = xjacm[2][2] / (*djacb) ;
	xjaci[2][2] = xjacm[1][1] / (*djacb) ;
	xjaci[1][2] = -xjacm[1][2] / (*djacb) ;
	xjaci[2][1] = -xjacm[2][1] / (*djacb) ;

	for (idime = 1; idime <= 2; idime++)
	{
		for (inode = 1; inode <= nnode; inode++)
		{
			femMatPut(cartd,idime,inode,  0 ) ;
			for (jdime = 1; jdime <=2; jdime++)
			{
				femMatAdd(cartd,idime,inode, (xjaci[idime][jdime]*femMatGet(deriv,jdime,inode)) );
			}
/*fprintf(msgout," %f ",cartd[idime][inode]);*/
		}
/*fprintf(msgout,"\n");*/
	}

	return(AF_OK);
}


/** Evaluates strain-displacement matrix
 *  @param cartd = cartesian derivations [2,10]
 *  @param nnode = number of nodes (4,8,9) 
 *  @param bmatx = B matrix (result) [2, 18]
 *  @return state value
 */
long bmatp(tMatrix *cartd, long nnode, tMatrix *bmatx)
{
	long ngash = 0; 
	long mgash = 0;
	long inode = 0;

	for (inode=1; inode <= nnode; inode++)
	{
		mgash = ngash + 1 ;
		ngash = mgash + 1 ;

		femMatPut(bmatx,1,mgash  , femMatGet(cartd,1,inode) );
		femMatPut(bmatx,1,ngash  , 0.0 );
		femMatPut(bmatx,2,mgash  , 0.0 );
		femMatPut(bmatx,2,ngash  , femMatGet(cartd,2,inode) );
		femMatPut(bmatx,3,mgash  , femMatGet(cartd,2,inode) );
		femMatPut(bmatx,3,ngash  , femMatGet(cartd,1,inode) );
	}

	return(AF_OK);
}

/** this doue somenthing
 *  @param D material stifness matrix [3,3]
 *  @param  bmatx [3,18]
 *  @param  dbmatx [3,18]
 *  @param  nat
 *  @return state value
 */
long dbe(tMatrix *D, tMatrix *bmatx, tMatrix *dbmatx, long nat)
{
	long istre,ievab,jstre;

	for (istre=1; istre<=3; istre++)
	{
		for(ievab=1; ievab<=nat; ievab++)
		{
			femMatPut(dbmatx,istre,ievab,  0.0 );
			for(jstre=1; jstre<=3; jstre++)
			{
				femMatAdd(dbmatx,istre,ievab, femMatGet(D,istre,jstre)*femMatGet(bmatx,jstre,ievab) );
			}
		}
	}

	return(AF_OK);
}


void mirrorK(tMatrix *Ke)
{
	long i,j;

	for (i=1;i<=Ke->rows; i++)
	{
		for (j=1; j<i; j++)
		{
			femMatPut(Ke,i,j,  femMatGet(Ke,j,i) ) ;
		}
	}
}


/** Prepares element sfifness matrix 
 *  @param ePos     element position
 *  @param eType    element type
 *  @param mType    material type
 *  @param optFrom  element options start
 *  @param optField field with element options
 *  @param rFrom    element real values start
 *  @param rFrom    element real values
 *  @param mFrom    material start
 *  @param mField   material data
 *  @param mNum     number of material data
 *  @param A        element area
 *  @param nFrom    starting polong in eNodes
 *  @param eNodes   element nodes description
 *  @param nField   nodal coordinates
 *  @param D        material stiffness matrix
 *  @param thick    element thickness
 *  @param ngaus    number of gauss polongs (2,3)
 *  @param nnode    number of nodes (4,8,9)
 *  @param Ke       element stiffness matrix (result)
 *  @param nRes     results in nodes (field for results)
 *  @param Ke       results in longegration polongs (field for results)
 */
int stiffp(
					 long     ePos,
					 double   thick, /* thickness of element         */
		       long     ngaus, /* number of gauss points (2,3) */ 
		       long     nnode, /* number of nodes (4,8,9)      */
		       tMatrix *Ke,    /* element stifness matrix      */
					 tVector *re,    /* displacement vector          */
					 tVector *Fe,    /* force vector                 */
					 tVector *Fe_r,  /* residual force vector        */
					 long    Mode
					 )
{
	int     rv = AF_OK ;
	double  A;     /* element area  */
	tMatrix coord; /* [10][3];  element coordinates */
	tMatrix elcod; /* [3][10]; */
	tVector posgp; /* [5]; */
	tVector weigp; /* [5]; */
	tVector shape; /*[10]; */
	tMatrix deriv; /* [3][10]; */
	tMatrix cartd; /* [3][10]; */
  tMatrix bmatx; /* [4][19]; */
  tMatrix dbmatx; /* [4][19]; */
	double djacb;
	double dvolu ;
	tVector epsilon; /* [4]={0,0,0,0}; */
	tVector sigma; /* [4]={0,0,0,0}; */
	tVector sigma_r; /* [4]={0,0,0,0};  residual */
	tVector sigma_0; /* [4]={0,0,0,0};  previous total*/
	tVector epsilon_0; /* [4]={0,0,0,0};  previous total*/
	tMatrix si2; /* [4][10]; */
	tMatrix si2_r; /* [4][10]; */
	tMatrix D; /*  [3][3] ; */
	tMatrix D_r; /*  [3][3] ; */
	long    nat ;
	long    i,j ;
#if 0
	long    k ;
#endif
	long    igaus, jgaus ;
	long    ievab, jevab, istre ;
	long    ipoint = 0;
	long		ProblemType = 0 ; /* plane stress ;-) */
	long    mtype ;

	/* sets problem type for "D" computation: */
  if (thick <= 0.0) 
  {
    ProblemType = 1 ; /* plane strain */ 
  }
  else
  {
    ProblemType = 0 ;
  }

	/* material type: */
  mtype = Mat[femGetMPTypePos(femGetEMPPos(ePos))].type ;

	/* number of DOFs */
	nat = 2*nnode ;

#ifdef DEVEL
	if (nat != Ke->rows)
	{
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"[E] %s (%li x %li)!\n", _("Invalid size of \"K\" matrix"),nat,Ke->rows);
#endif
		return(AF_ERR_SIZ);
	}
#endif

	if ((rv=femFullMatInit(&D, 3,3 )) != AF_OK){goto memFree;}
	if ((rv=femFullMatInit(&coord, 9,2 )) != AF_OK){goto memFree;}
	if ((rv=femFullMatInit(&elcod, 2,9 )) != AF_OK){goto memFree;}
	if ((rv=femVecFullInit(&posgp, 4)) != AF_OK){goto memFree;}
	if ((rv=femVecFullInit(&weigp, 4)) != AF_OK){goto memFree;}
	if ((rv=femVecFullInit(&shape, 9))  != AF_OK){goto memFree;}
	if ((rv=femFullMatInit(&deriv, 2,9)) != AF_OK){goto memFree;}
	if ((rv=femFullMatInit(&cartd, 2,9)) != AF_OK){goto memFree;}
  if ((rv=femFullMatInit(&bmatx, 3,18)) != AF_OK){goto memFree;}
  if ((rv=femFullMatInit(&dbmatx, 3,18)) != AF_OK){goto memFree;}

	if ((rv=femVecFullInit(&epsilon, 3)) != AF_OK){goto memFree;}
	if ((rv=femVecFullInit(&sigma, 3)) != AF_OK){goto memFree;}
	if ((rv=femVecFullInit(&sigma_r, 3)) != AF_OK){goto memFree;}
	if ((rv=femVecFullInit(&sigma_0, 3)) != AF_OK){goto memFree;}
	if ((rv=femVecFullInit(&epsilon_0, 3)) != AF_OK){goto memFree;}
	if ((rv=femFullMatInit(&si2, 3,9)) != AF_OK){goto memFree;}
	if ((rv=femFullMatInit(&si2_r, 3,9)) != AF_OK){goto memFree;}

	/* initialization(s): */
	femMatSetZero(Ke);

	if (Mode == AF_YES) 
	{ 
		femVecSetZero(Fe);
		
		if (Fe_r != NULL)
		{
			femVecSetZero(Fe_r);
		}

		if ((rv=femFullMatInit(&D_r, 3, 3)) != AF_OK){goto memFree;}
	}

	/* coordinates of element nodal points: */
	for (i=1; i<= nnode; i++)
	{
		femMatPut(&coord,i,1, femGetNCoordPosX(femGetENodePos(ePos, i-1)) );
		femMatPut(&coord,i,2, femGetNCoordPosY(femGetENodePos(ePos, i-1)) );

		for (j=1; j<=2; j++)
		{
		  femMatPut(&elcod,j,i,  femMatGet(&coord,i,j) );
		}
	}

	/* element area computation */
  A = ( 0.5 *
		  ( femMatGet(&coord,1 , 1 ) * femMatGet(&coord,2 , 2 )
			- femMatGet(&coord,2 , 1 ) * femMatGet(&coord,1  , 2 )
			+ femMatGet(&coord,2 , 1 ) * femMatGet(&coord,3 , 2 )
			- femMatGet(&coord,3 , 1 ) * femMatGet(&coord,2  , 2 )
			+ femMatGet(&coord,3 , 1 ) * femMatGet(&coord,1 , 2 )
			- femMatGet(&coord,1 , 1 ) * femMatGet(&coord,3 , 2 )
			)
		)
	+
		( 0.5 *
		  ( femMatGet(&coord,1 , 1 ) * femMatGet(&coord,3 , 2 )
			- femMatGet(&coord,3 , 1 ) * femMatGet(&coord,1 , 2 )
			+ femMatGet(&coord,3 , 1 ) * femMatGet(&coord,4 , 2 )
			- femMatGet(&coord,4 , 1 ) * femMatGet(&coord,3  , 2 )
			+ femMatGet(&coord,4 , 1 ) * femMatGet(&coord,1 , 2 )
			- femMatGet(&coord,1 , 1 ) * femMatGet(&coord,4 , 2 )
			)
		 );

	A = A / pow((double)ngaus,2) ; 

	/* values for gauss: */
	gaussq(ngaus, &posgp, &weigp);

	ipoint = (-1);

	/* numerical integration loop: */
	for (igaus=1; igaus<=ngaus; igaus++)
	{
	  for (jgaus=1; jgaus<=ngaus; jgaus++)
		{
			ipoint++;

			/* shape functions, element volume: */
			sfr(nnode, 
					femVecGet(&posgp,jgaus),
					femVecGet(&posgp,igaus), 
					&shape, 
					&deriv) ;

      if (jacob(&cartd,&deriv,&elcod, &djacb, nnode) != 0) 
			   {rv=AF_ERR_VAL; goto memFree;/* bad element */}


			dvolu = djacb * femVecGet(&weigp,igaus)*femVecGet(&weigp,jgaus) ;
      if (thick > 0.0) { dvolu = dvolu*thick; }

			/* B matrix: */
      bmatp(&cartd, nnode, &bmatx);
			
			femMatPrn(&bmatx,"BMAT");

			if (Mode == AF_YES)
			{
				femVecSetZero(&epsilon);
			  /* strain (epsilon) */
			  for (i=1; i<=3; i++)
			  {
				  femVecPut(&epsilon,i,  0.0 ) ;
				  for (j=1; j<=nat; j++)
				  {
					  femVecAdd(&epsilon,i, (femMatGet(&bmatx,i,j) * femVecGet(re,j)) );
				  }
			  }

				/* previous step matrix: */
				if ((rv=fem_D_2D(ePos, ipoint, A, &epsilon, NULL,NULL, AF_NO, ProblemType, &D_r)) != AF_OK) 
				   { goto memFree; }
			}
			else
			{
				femMatSetZero(&D);
				/* create (for mode != NEW) material stiffness matrix: */
				if ((rv=fem_D_2D(ePos, ipoint, A, &epsilon, NULL,NULL, AF_NO, ProblemType, &D)) != AF_OK) 
			   	  { goto memFree; }
			}

			if (Mode == AF_YES)
			{
				/* putting of data to iRes field */
				if (femTangentMatrix == AF_YES)
				{
					if (mtype == 2) /* "pseudo sec" */
					{
					femVecPut(&sigma_0, 1,  femGetEResVal(ePos, RES_SX, ipoint+1)) ;
					femVecPut(&sigma_0, 2,  femGetEResVal(ePos, RES_SY, ipoint+1)) ;
					femVecPut(&sigma_0, 3,  femGetEResVal(ePos, RES_SXY, ipoint+1)) ;

					femVecPut(&epsilon_0, 1,  femGetEResVal(ePos, RES_EX, ipoint+1)) ;
					femVecPut(&epsilon_0, 2,  femGetEResVal(ePos, RES_EY, ipoint+1)) ;
					femVecPut(&epsilon_0, 3,  femGetEResVal(ePos, RES_EXY, ipoint+1)) ;


					/* order is important! - this must be here! */
			  	femAddEResVal(ePos, RES_EX, ipoint+1,  femVecGet(&epsilon,1));
	      	femAddEResVal(ePos, RES_EY, ipoint+1,  femVecGet(&epsilon,2));
	      	femAddEResVal(ePos, RES_EXY, ipoint+1, femVecGet(&epsilon,3));
					
					/* e_sum = e_0 + delta_e:  */
					femVecAdd(&epsilon_0, 1, femVecGet(&epsilon, 1)) ;
					femVecAdd(&epsilon_0, 2, femVecGet(&epsilon, 2)) ;
					femVecAdd(&epsilon_0, 3, femVecGet(&epsilon, 3)) ;
					

					if ((rv=fem_D_2D(ePos, ipoint, A, &epsilon_0, NULL,NULL, AF_YES, ProblemType, &D)) != AF_OK) 
			   	  { goto memFree; }

					femMatVecMult(&D, &epsilon_0, &sigma ) ; /* new sigma */

					/* sigma_r computation */
					femVecSetZero(&sigma_r);
					femMatVecMult(&D_r, &epsilon, &sigma_r ) ;

					for (i=1; i<=3; i++)
					{
						femVecAdd(&sigma_r, i, femVecGet(&sigma_0, i)-femVecGet(&sigma, i));
					}
					/* end of sigma_r computation */

         	femPutEResVal(ePos, RES_SX, ipoint+1, femVecGet(&sigma,1));
	       	femPutEResVal(ePos, RES_SY, ipoint+1, femVecGet(&sigma,2));
	       	femPutEResVal(ePos, RES_SXY,ipoint+1, femVecGet(&sigma,3));

#ifdef DEVEL_VERBOSE
				fprintf(msgout,"\nSIGMA: %f %f %f\n", femVecGet(&sigma,1), femVecGet(&sigma,2), femVecGet(&sigma,3));
				fprintf(msgout,"\nSIGMA_R: %f %f %f\n", femVecGet(&sigma_r,1), femVecGet(&sigma_r,2), femVecGet(&sigma_r,3));
#endif
					}
					else /* mtype */
					{
            if ((mtype == 8)||(mtype == 11)||(mtype == 13))
            {
              femVecSetZero(&sigma_r);
              femVecSetZero(&sigma);
              femVecSetZero(&sigma_0);

        	    femVecPut(&sigma_0,1,femGetEResVal(ePos, RES_SX,  ipoint+1));
	      	    femVecPut(&sigma_0,2,femGetEResVal(ePos, RES_SY,  ipoint+1));
	      	    femVecPut(&sigma_0,3,femGetEResVal(ePos, RES_SXY, ipoint+1));

					    if ((rv=fem_D_2D(ePos, ipoint, A, &epsilon, &sigma, &sigma_r, AF_YES, ProblemType, &D)) != AF_OK) 
			   	      { goto memFree; }

			  	    femAddEResVal(ePos, RES_EX,  ipoint+1, femVecGet(&epsilon,1));
	      	    femAddEResVal(ePos, RES_EY,  ipoint+1, femVecGet(&epsilon,2));
	      	    femAddEResVal(ePos, RES_EXY, ipoint+1, femVecGet(&epsilon,3));

#if 0
        	    femPutEResVal(ePos, RES_SX,  ipoint+1, femVecGet(&sigma,1));
	      	    femPutEResVal(ePos, RES_SY,  ipoint+1, femVecGet(&sigma,2));
	      	    femPutEResVal(ePos, RES_SXY, ipoint+1, femVecGet(&sigma,3));

              for (k=1; k<=3;k++) /* for correct reactions! */
              { femVecPut( &sigma,k, (femVecGet(&sigma,k)-femVecGet(&sigma_0,k))); }
#else
        	    femAddEResVal(ePos, RES_SX,  ipoint+1, femVecGet(&sigma,1));
	      	    femAddEResVal(ePos, RES_SY,  ipoint+1, femVecGet(&sigma,2));
	      	    femAddEResVal(ePos, RES_SXY, ipoint+1, femVecGet(&sigma,3));
#endif
            }
            else /* mtype != 8*/
            {
			  	    femAddEResVal(ePos, RES_EX,  ipoint+1, femVecGet(&epsilon,1));
	      	    femAddEResVal(ePos, RES_EY,  ipoint+1, femVecGet(&epsilon,2));
	      	    femAddEResVal(ePos, RES_EXY, ipoint+1, femVecGet(&epsilon,3));

					    if ((rv=fem_D_2D(ePos, ipoint, A, &epsilon, NULL,NULL, AF_YES, ProblemType, &D)) != AF_OK) 
			   	      { goto memFree; }

              femVecSetZero(&sigma);
              femVecSetZero(&sigma_r);

					    femMatVecMult(&D, &epsilon, &sigma ) ;
					    femMatVecMult(&D_r, &epsilon, &sigma_r ) ;
  				
					    for (i=1; i<=3; i++) 
					    { 
						    femVecPut(&sigma_r,i, femVecGet(&sigma_r,i) - femVecGet(&sigma,i) ); 
					    }

        	    femAddEResVal(ePos, RES_SX,  ipoint+1, femVecGet(&sigma,1));
	      	    femAddEResVal(ePos, RES_SY,  ipoint+1, femVecGet(&sigma,2));
	      	    femAddEResVal(ePos, RES_SXY, ipoint+1, femVecGet(&sigma,3));

#ifdef DEVEL_VERBOSE
				    fprintf(msgout,"\nSIGMA: %f %f %f\n", femVecGet(&sigma,1), femVecGet(&sigma,2), femVecGet(&sigma,3));
            femVecPrn(&sigma_r,"SIGMA_RES");
#endif
            } /* mtype <> 8 */
					} /* mtype */
				}
				else
				{

			  	femPutEResVal(ePos, RES_EX, ipoint+1,  femVecGet(&epsilon,1));
	      	femPutEResVal(ePos, RES_EY, ipoint+1,  femVecGet(&epsilon,2));
	      	femPutEResVal(ePos, RES_EXY, ipoint+1, femVecGet(&epsilon,3));

					if ((rv=fem_D_2D(ePos, ipoint, A, &epsilon,NULL, NULL,  AF_YES, ProblemType, &D)) != AF_OK) 
			   	  { goto memFree; }

					femMatVecMult(&D, &epsilon, &sigma ) ;
					femMatVecMult(&D_r, &epsilon, &sigma_r ) ;

					for (i=1; i<=3; i++) 
					{ 
						femVecPut(&sigma_r,i, femVecGet(&sigma_r,i) - femVecGet(&sigma,i) ); 
					}

         	femPutEResVal(ePos, RES_SX, ipoint+1, femVecGet(&sigma,1));
	       	femPutEResVal(ePos, RES_SY, ipoint+1, femVecGet(&sigma,2));
	       	femPutEResVal(ePos, RES_SXY,ipoint+1, femVecGet(&sigma,3));

#ifdef DEVEL_VERBOSE
				fprintf(msgout,"\nSIGMA: %f %f %f\n", femVecGet(&sigma,1), femVecGet(&sigma,2), femVecGet(&sigma,3));
				fprintf(msgout,"\nSIGMA_R: %f %f %f\n", femVecGet(&sigma_r,1), femVecGet(&sigma_r,2), femVecGet(&sigma_r,3));
#endif
				}

        /* computed stresses */
				femMatPut(&si2,1,ipoint+1,   femVecGet(&sigma,1) );
				femMatPut(&si2,2,ipoint+1,   femVecGet(&sigma,2) );
				femMatPut(&si2,3,ipoint+1,   femVecGet(&sigma,3) );

				/* residual stresses */
				femMatPut(&si2_r,1,ipoint+1, femVecGet(&sigma_r,1) );
				femMatPut(&si2_r,2,ipoint+1, femVecGet(&sigma_r,2) );
				femMatPut(&si2_r,3,ipoint+1, femVecGet(&sigma_r,3) );
			}
			
      dbe(&D, &bmatx, &dbmatx, nat);

			/* calculate element stiffnesses: */
			for (ievab=1; ievab<=nat; ievab++)
			{
				for (jevab=ievab; jevab<=nat; jevab++)
				{
					for (istre=1; istre<=3; istre++)
					{
						femMatAdd(
						Ke,ievab,jevab, 
						femMatGet(&bmatx,istre,ievab) 
						*
						femMatGet(&dbmatx,istre,jevab)
						*
						dvolu
						);
					}
				}
			}
		}
	}

  mirrorK(Ke);

femMatPrn(Ke,"KE");

	/* Nodal forces */
	if (Mode == AF_YES)
	{
	  ipoint = 0;

		/* numerical integration loop: */
		for (igaus=1; igaus<=ngaus; igaus++)
		{
	  	for (jgaus=1; jgaus<=ngaus; jgaus++)
			{
				ipoint++;
	
				/* shape functions, elemental volume: */
				sfr(nnode, 
					femVecGet(&posgp,jgaus),
					femVecGet(&posgp,igaus), 
					&shape, 
					&deriv) ;

      if (jacob(&cartd,&deriv,&elcod, &djacb, nnode) != 0) 
			   {rv=AF_ERR_VAL; goto memFree;/* bad element */}

			dvolu = djacb * femVecGet(&weigp,igaus)*femVecGet(&weigp,jgaus) ;
      if (thick > 0.0) { dvolu = dvolu*thick; }

				/* B matrix: */
      	bmatp(&cartd, nnode, &bmatx);
	
				/* nodal forces */
				for (i=1; i<=nat; i++)
				{
					for (istre=1; istre<=3; istre++) /* number of stresses */
					{
						femVecAdd(Fe,i, 
								femMatGet(&bmatx,istre,i) * femMatGet(&si2,istre,ipoint) * dvolu
								);

						/*if (Fe_r != NULL)*/
						{
							femVecAdd(Fe_r,i, 
								femMatGet(&bmatx,istre,i) * femMatGet(&si2_r,istre,ipoint)*dvolu
								);
						}
					}
				}
			}
		}
	}

memFree:

	femMatFree(&D);
	femMatFree(&coord);
	femMatFree(&elcod);
	femVecFree(&posgp);
	femVecFree(&weigp);
	femVecFree(&shape);
	femMatFree(&deriv);
	femMatFree(&cartd);
  femMatFree(&bmatx);
  femMatFree(&dbmatx);

	femVecFree(&epsilon);
	femVecFree(&sigma);
	femVecFree(&sigma_r);
	femVecFree(&sigma_0);
	femVecFree(&epsilon_0);
	femMatFree(&si2);
	femMatFree(&si2_r);

	if (Mode == AF_YES)
	{
		femMatFree(&D_r);
	}

	return(rv);
}


int e002_stiff(long ePos, long Mode, tMatrix *K_e, tVector *Fe, tVector *Fre)
{
	int     rv = AF_OK;
	double  thick ;
	long    ngaus = 2 ;
	long    nnode = 4 ;
	/*long    eT    = 2 ;*/
	tVector u_e;

	if ((rv = femVecFullInit(&u_e, ngaus*nnode)) != AF_OK)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s %li!\n", _("Out of memory on element"), femGetEIDPos(ePos));
#endif
		femVecNull(&u_e);
		return(rv);
	}

  femLocUtoU_e(&u, ePos, &u_e);

	thick = femGetRSValPos(ePos, RS_HEIGHT, 0) ;
	
	rv = stiffp(
					 ePos,
					 thick, /* thickness of element         */
		       ngaus, /* number of gauss points (2,3) */ 
		       nnode, /* number of nodes (4,8,9)      */
		       K_e,   /* element stifness matrix      */
					 &u_e,  /* displacement vector          */
					 Fe,    /* force vector                 */
					 Fre,   /* residual force vector        */
					 Mode
					 ) ;

	femVecFree(&u_e) ;
	return(rv);
}

int e002_mass(long ePos, tMatrix *M_e)
{
	int    rv = AF_OK ;
	long   nnode = 4;
	long   i;
	double t,ro,A;
	tMatrix coord; /* [10][3];  element coordinates */

	if ((rv=femFullMatInit(&coord, 9,2)) != AF_OK) { return(rv); }
	
	femMatSetZero(M_e);

	/* Geometry (width, area) and material (density) data: */
	t  = femGetRSValPos(ePos, RS_HEIGHT, 0) ;
	if (t <= 0.0) {t = 1.0;} /* plane strain problem */
	ro = femGetMPValPos(ePos, MAT_DENS, 0) ;

	/* coordinates of element nodal points: */
	for (i=1; i<= nnode; i++)
	{
		femMatPut(&coord,i,1, femGetNCoordPosX(femGetENodePos(ePos, i-1)) );
		femMatPut(&coord,i,2, femGetNCoordPosY(femGetENodePos(ePos, i-1)) );

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"x = %f y= %f\n", femMatGet(&coord,i,1), femMatGet(&coord,i,2));
#endif
	}

	/* element area computation */
  A = ( 0.5 *
		  ( femMatGet(&coord,1 , 1 ) * femMatGet(&coord,2 , 2 )
			- femMatGet(&coord,2 , 1 ) * femMatGet(&coord,1  , 2 )
			+ femMatGet(&coord,2 , 1 ) * femMatGet(&coord,3 , 2 )
			- femMatGet(&coord,3 , 1 ) * femMatGet(&coord,2  , 2 )
			+ femMatGet(&coord,3 , 1 ) * femMatGet(&coord,1 , 2 )
			- femMatGet(&coord,1 , 1 ) * femMatGet(&coord,3 , 2 )
			)
		)
	+
		( 0.5 *
		  ( femMatGet(&coord,1 , 1 ) * femMatGet(&coord,3 , 2 )
			- femMatGet(&coord,3 , 1 ) * femMatGet(&coord,1 , 2 )
			+ femMatGet(&coord,3 , 1 ) * femMatGet(&coord,4 , 2 )
			- femMatGet(&coord,4 , 1 ) * femMatGet(&coord,3  , 2 )
			+ femMatGet(&coord,4 , 1 ) * femMatGet(&coord,1 , 2 )
			- femMatGet(&coord,1 , 1 ) * femMatGet(&coord,4 , 2 )
			)
		 );

	/* this is stupid and incorrect but better that nothing: */
	for (i=1; i<=nnode*2; i++)
	{
		femMatPut(M_e,i,i,  (A * t * ro) / (nnode) ) ;
	}

	femMatFree(&coord);
	return(rv);
}

int e002_volume(long ePos, double *vol)
{
	int    rv = AF_OK ;
	long   nnode = 4;
	long   i;
	double t,A;
	tMatrix coord; /* [10][3];  element coordinates */

	if ((rv=femFullMatInit(&coord, 9,2)) != AF_OK) { return(rv); }

	/* Geometry (width, area) and material (density) data: */
	t  = femGetRSValPos(ePos, RS_HEIGHT, 0) ;

	/* coordinates of element nodal points: */
	for (i=1; i<= nnode; i++)
	{
		femMatPut(&coord,i,1, femGetNCoordPosX(femGetENodePos(ePos, i-1)) );
		femMatPut(&coord,i,2, femGetNCoordPosY(femGetENodePos(ePos, i-1)) );
	}

	/* element area computation */
  A = ( 0.5 *
		  ( femMatGet(&coord,1 , 1 ) * femMatGet(&coord,2 , 2 )
			- femMatGet(&coord,2 , 1 ) * femMatGet(&coord,1  , 2 )
			+ femMatGet(&coord,2 , 1 ) * femMatGet(&coord,3 , 2 )
			- femMatGet(&coord,3 , 1 ) * femMatGet(&coord,2  , 2 )
			+ femMatGet(&coord,3 , 1 ) * femMatGet(&coord,1 , 2 )
			- femMatGet(&coord,1 , 1 ) * femMatGet(&coord,3 , 2 )
			)
		)
	+
		( 0.5 *
		  ( femMatGet(&coord,1 , 1 ) * femMatGet(&coord,3 , 2 )
			- femMatGet(&coord,3 , 1 ) * femMatGet(&coord,1 , 2 )
			+ femMatGet(&coord,3 , 1 ) * femMatGet(&coord,4 , 2 )
			- femMatGet(&coord,4 , 1 ) * femMatGet(&coord,3  , 2 )
			+ femMatGet(&coord,4 , 1 ) * femMatGet(&coord,1 , 2 )
			- femMatGet(&coord,1 , 1 ) * femMatGet(&coord,4 , 2 )
			)
		 );

  *vol = A * t ;

	femMatFree(&coord);
	return(rv);
}

long e002_rvals(long ePos)
{
	return(76);
}

int e002_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

/** We assume that there are ALWAYS only 2x2 Gauss points! */
int e002_res_p_loc(long ePos, long point, double *x, double *y, double *z)
{
	double xx[5] ;
	double yy[5] ;
	double xi[5] = {0, -1,  1, 1, -1} ;
	double yi[5] = {0, -1, -1, 1,  1} ;
	double dx = 0.577350569189626 ;
	double dy = 0.577350569189626 ;
	long   i ;

	if (e002_rvals(ePos) != 64) {return (AF_ERR_VAL);}

	for (i=1; i<=4; i++)
	{
  	xx[i] = femGetNCoordPosX(femGetENodePos(ePos,i-1));
  	yy[i] = femGetNCoordPosY(femGetENodePos(ePos,i-1));
	}

	*x = 0.0 ;
	*y = 0.0 ;
	*z = 0.0 ;

	for (i=1; i<=4; i++)
	{
		*x += xx[i]*0.25*(1.0 + dx*xi[point] * xi[i])*(1.0 +  dy*yi[point] * yi[i] ) ;
		*y += yy[i]*0.25*(1.0 + dx*xi[point] * xi[i])*(1.0 +  dy*yi[point] * yi[i] ) ;
	}

	return(AF_OK);
}

int e002_res_node(long ePos, long nPos, long type, double *val)
{
  int inum = 0 ;
  int i;

  switch (nPos)
  {
    case 0:  inum  =  0 ; break ;
    case 1:  inum  =  2 ; break ;
    case 2:  inum  =  3 ; break ;
    case 3:  inum  =  1 ; break ;
    default: return(AF_ERR);
  }

  for (i=0; i<Elem[2].res_rp; i++)
  {
    if (type == Elem[2].nres_rp[i]) 
    { 
      *val = femGetEResVal(ePos, type, inum) ;
      return(AF_OK) ;
    }
  }
  
  return(AF_ERR);
}

int addElem_002(void)
{
	int rv = AF_OK;
	static long type     = 2 ;
	static long dim      = 2 ;
	static long nodes    = 4 ;
	static long dofs     = 2 ;
	static long ndof[2]  = {U_X,U_Y} ;
	static long rs       = 5 ;
	static long real[5]  = {RS_HEIGHT,RS_ROT1,RS_RFC1,RS_ROT2,RS_RFC2} ;
	static long rs_rp    = 0 ;
	static long *real_rp = NULL ;
	static long res      = 0 ;
	static long *nres    = NULL ;
	static long res_rp   = 19 ;
	static long nres_rp[19] = { 
    RES_SX, RES_SY, RES_SXY,
    RES_EX, RES_EY, RES_EXY,
    RES_USX, RES_USY, RES_USXY,
    RES_STMAX, RES_SCMAX,
    RES_DIR1, RES_DIR2, RES_DIR3,
    RES_ECR1, RES_ECR2, RES_STAT1, RES_STAT2, RES_GCR1} ;

	if (type != femAddElem(type)) {return(AF_ERR_VAL);}
	Elem[type].dim = dim ;
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

	Elem[type].stiff = e002_stiff;
	Elem[type].mass  = e002_mass;
	Elem[type].rvals = e002_rvals;
	Elem[type].eload = e002_eload;
	Elem[type].res_p_loc = e002_res_p_loc;
	Elem[type].res_node = e002_res_node;
	Elem[type].volume = e002_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

#endif
/* end of fem_e002.c */

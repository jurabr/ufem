/*
   File name: fem_e020.c
   Date:      2011/07/08 12:33
   Author:    Jiri Brozovsky

   Copyright (C) 2011 Jiri Brozovsky

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

extern long sfr( long     nnode, double   etasp, double   exitsp, tVector *shape, tMatrix *deriv);
extern long gaussq(long ngaus, tVector *posgp, tVector *weigp);
extern long jacob( tMatrix *cartd, tMatrix *deriv, tMatrix *elcod, double  *djacb, long     nnode);
extern long bmatp(tMatrix *cartd, long nnode, tMatrix *bmatx);
extern long dbe(tMatrix *D, tMatrix *bmatx, tMatrix *dbmatx, long nat);
extern void mirrorK(tMatrix *Ke);
extern int e002_volume(long ePos, double *vol);
extern int e002_res_p_loc(long ePos, long point, double *x, double *y, double *z);
extern int e002_res_node(long ePos, long nPos, long type, double *val);


/** Prepares element stifness matrix 
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
int stiffp20(
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
#if 1
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
	tMatrix D; /*  [3][3] ; */
	long    nat ;
	long    i,j ;
	long    igaus, jgaus ;
	long    ievab, jevab, istre ;
	long    ipoint = 0;
  double  kxx;
  double  mult[5] ;
	
	/* number of DOFs */
	nat = 1*nnode ;

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

	/* initialization(s): */
	femMatSetZero(Ke);

	if (Mode == AF_YES) 
	{ 
		femVecSetZero(Fe);
		if (Fe_r != NULL) { femVecSetZero(Fe_r); }
	}


	/* coordinates of element nodal points: */
	for (i=1; i<= nnode; i++)
	{
		femMatPut(&coord,i,1, femGetNCoordPosX(femGetENodePos(ePos, i-1)) );
		femMatPut(&coord,i,2, femGetNCoordPosY(femGetENodePos(ePos, i-1)) );

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"x = %f y= %f\n", femMatGet(&coord,i,1), femMatGet(&coord,i,2));
#endif
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

      femMatSetZero(&D);
			/* create material stiffness matrix: */
      kxx = femGetMPValPos(ePos, MAT_KXX, 0) ;
      femMatPut(&D, 1,1, kxx );
      femMatPut(&D, 2,2, kxx );


			
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
  femValMatMultSelf(A*thick, Ke);


  if (Mode == AF_YES)
  {
    mult[1] =  femVecGet(re, 1) ;
    mult[2] =  femVecGet(re, 4) ;
    mult[3] =  femVecGet(re, 2) ;
    mult[4] =  femVecGet(re, 3) ;

    femVecPrn(re, "deformace");

    for (i=1; i<=4; i++)
    {
      /* putting of data to iRes field */
	    if (femTangentMatrix == AF_YES)
	       { femAddEResVal(ePos, RES_TEMP,  i, mult[i]); }
	    else
	       { femPutEResVal(ePos, RES_TEMP, i,  mult[i]); }
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
#endif
	return(rv);
}


int e020_stiff(long ePos, long Mode, tMatrix *K_e, tVector *Fe, tVector *Fre)
{
	int     rv = AF_OK;
	double  thick ;
	long    ngaus = 2 ;
	long    nnode = 4 ;
	/*long    eT    = 2 ;*/
	tVector u_e;

	if ((rv = femVecFullInit(&u_e, 4)) != AF_OK)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s %li!\n", _("Out of memory on element"), femGetEIDPos(ePos));
#endif
		femVecNull(&u_e);
		return(rv);
	}

  femLocUtoU_e(&u, ePos, &u_e);

	thick = femGetRSValPos(ePos, RS_HEIGHT, 0) ;
	
	rv = stiffp20(
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

int e020_mass(long ePos, tMatrix *M_e) { return(AF_OK); }

long e020_rvals(long ePos) { return(4); }

int e020_eload(long ePos, long mode, tVector *F_e) { return(AF_OK); }


int addElem_020(void)
{
	int rv = AF_OK;
	static long type     = 20 ;
	static long nodes    = 4 ;
	static long dofs     = 1 ;
	static long ndof[1]  = {TEMP} ;
	static long rs       = 1 ;
	static long real[1]  = {RS_HEIGHT} ;
	static long rs_rp    = 0 ;
	static long *real_rp = NULL ;
	static long res      = 0 ;
	static long *nres    = NULL ;
	static long res_rp   = 1 ;
	static long nres_rp[1] = { RES_TEMP} ;

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

	Elem[type].stiff = e020_stiff;
	Elem[type].mass  = e020_mass;
	Elem[type].rvals = e020_rvals;
	Elem[type].eload = e020_eload;
	Elem[type].res_p_loc = e002_res_p_loc;
	Elem[type].res_node = e002_res_node;
	Elem[type].volume = e002_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

#endif
/* end of fem_e002.c */

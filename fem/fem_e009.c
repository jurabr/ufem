/*
   File name: fem_e009.c
   Date:      Thu Mar 18 19:13:39 CET 2004
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

	 FEM Solver - Element 009 (structural brick - 8 nodes)

   $Id: fem_e009.c,v 1.11 2004/11/11 21:39:24 jirka Exp $
*/

#include "fem_elem.h"
#include "fem_pl3d.h"

extern int e008_init_gauss(long ipoints, tMatrix *gauss);
extern int e008_res_p_loc(long ePos, long point, double *x, double *y, double *z);

/* nodes of brick */
long p_nodes[8]={1,2,3,4,5,6,7,8};

/* prepares coordinates of points */
void e009_fill_coords(tMatrix *coord)
{
  femMatPut(coord, 1, 1,   -1.0);
  femMatPut(coord, 1, 2,   -1.0);
  femMatPut(coord, 1, 3,   -1.0);

  femMatPut(coord, 2, 1,    1.0);
  femMatPut(coord, 2, 2,   -1.0);
  femMatPut(coord, 2, 3,   -1.0);

  femMatPut(coord, 3, 1,    1.0);
  femMatPut(coord, 3, 2,    1.0);
  femMatPut(coord, 3, 3,   -1.0);

  femMatPut(coord, 4, 1,   -1.0);
  femMatPut(coord, 4, 2,    1.0);
  femMatPut(coord, 4, 3,   -1.0);

  femMatPut(coord, 5, 1,   -1.0);
  femMatPut(coord, 5, 2,   -1.0);
  femMatPut(coord, 5, 3,    1.0);

  femMatPut(coord, 6, 1,    1.0);
  femMatPut(coord, 6, 2,   -1.0);
  femMatPut(coord, 6, 3,    1.0);

  femMatPut(coord, 7, 1,    1.0);
  femMatPut(coord, 7, 2,    1.0);
  femMatPut(coord, 7, 3,    1.0);

  femMatPut(coord, 8, 1,   -1.0);
  femMatPut(coord, 8, 2,    1.0);
  femMatPut(coord, 8, 3,    1.0);
}

/** determinant of Jacobi matrix  
 * @param jac Jacobi matrix
 * @param detj determinant (result)
 * @return status
 */
int e009_jac_det(tMatrix *jac, double *detj)
{
  *detj = 
    femMatGet(jac,1,1)*femMatGet(jac,2,2)*femMatGet(jac,3,3)+
    femMatGet(jac,2,1)*femMatGet(jac,3,2)*femMatGet(jac,1,3)+
    femMatGet(jac,1,2)*femMatGet(jac,2,3)*femMatGet(jac,3,1)+
    -femMatGet(jac,3,1)*femMatGet(jac,2,2)*femMatGet(jac,1,3)
    -femMatGet(jac,1,1)*femMatGet(jac,2,3)*femMatGet(jac,3,2)
    -femMatGet(jac,3,3)*femMatGet(jac,2,1)*femMatGet(jac,1,2);

  if (*detj <= 0.0)
  {
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Invalid element found"));
#endif
    *detj = 0.0 ;
    return(AF_ERR_VAL);
  }
  else
  {
    return(AF_OK);
  }
}

/* note: deriv = 0 => no derivation (value of function)!
 *       deriv = 1 => d/dx
 *       deriv = 2 => d/dy
 *       deriv = 3 => d/dz
 */

double e009_p_a(tMatrix *coord, int deriv, long point, double x, double y, double z)
{
  double xv, yv,zv ;

  xv = femMatGet(coord, point, 1) ;
  yv = femMatGet(coord, point, 2) ;
  zv = femMatGet(coord, point, 3) ;

  switch (deriv)
  {
    case 0:
        return( 0.125
        *(1.0+xv*x)
        *(1.0+yv*y)
        *(1.0+zv*z) 
        );
        break ;
    case 1: /* d/dx) */
        return(0.125
            *xv
            *(1.0 + yv*y)
            *(1.0 + zv*z)
            );
        break;
    case 2: /* d/dy) */
        return(0.125
            *yv
            *(1.0 + xv*x)
            *(1.0 + zv*z)
            );
        break;
    case 3: /* d/dz) */
        return(0.125
            *zv
            *(1.0 + xv*x)
            *(1.0 + yv*y)
            );
        break;
    default: return(0.0);
  }
}


/** Computes coordinates od d(p)/d(something) 
 * @param coord matrix of -1,0,1 brick coordinates
 * @param deriv derivation type (0=none,1=d/dx,2=d/dy,3=d/dz)
 * @param dir coordinate type (1=x,2=y,3=z)
 * @param x x of [-1,1]
 * @param y y of [-1,1]
 * @param z z of [-1,1]
 * @param xyz matrix of real brick coordinates (NULL if  you want only the derivatives)
 * @return coordinate/derivative value
 * */
double e009_deriv_p(tMatrix *coord, long deriv, long dir, 
    double x, double y, double z, tMatrix *xyz)
{
  double val = 0.0 ;
  double val_i ;
  long   i ;

  val = 0.0 ;

  for (i=0; i<8; i++)
  {
    val_i = e009_p_a(coord, deriv, p_nodes[i], x, y, z) ;
    if (xyz != NULL) { val_i *= femMatGet(xyz,p_nodes[i], dir) ; }
    val += val_i ;
  }

  return(val);
}

/** Computes coordinates of d(p)/d(something) of ONE P_x
 * @param coord matrix of -1,0,1 brick coordinates
 * @param deriv derivation type (0=none,1=d/dx,2=d/dy,3=d/dz)
 * @param number number of node (1..20)
 * @return coordinate/derivative value
 * */
double e009_deriv_p_one(tMatrix *coord,  long deriv, long number, double x, double y, double z)
{
	return(e009_p_a(coord, deriv, number, x, y, z) );
}

int e009_fill_J(tMatrix *jac, tMatrix *coord,
    double x, double y, double z, tMatrix *xyz)
{
  long i,j ;

	femMatSetZero(jac);

  for (i=1; i<=3; i++)
  {
    for (j=1; j<=3; j++)
    {
      femMatAdd(jac, i, j,
          e009_deriv_p(coord, i, j, x, y, z, xyz) );
    }
  }
  return(AF_OK);
}

int e009_fill_H(tMatrix *H, tMatrix *ijac, tMatrix *coord, double x, double y, double z)
{
  double h11, h22, h33 ;
  long   pos ;
  int i,j;

  h11 = 0.0 ;
  h22 = 0.0 ;
  h33 = 0.0 ;

  femMatSetZero(H) ;

  for(i=0; i<8; i++) 
  {
		h11 = 0.0 ;
		h22 = 0.0 ;
		h33 = 0.0 ;

  	for (j=1; j<=3; j++)
  	{
    	h11 += femMatGet(ijac,1,j)*e009_deriv_p_one(coord,j,i+1, x,y,z);
    	h22 += femMatGet(ijac,2,j)*e009_deriv_p_one(coord,j,i+1, x,y,z);
    	h33 += femMatGet(ijac,3,j)*e009_deriv_p_one(coord,j,i+1, x,y,z);
  	}

    pos = 3 * i ;
    femMatPut(H,1, 1+pos, h11 );
    femMatPut(H,2, 2+pos, h22 );
    femMatPut(H,3, 3+pos, h33 );

#if 0
    femMatPut(H,4, 1+pos, h22 );
    femMatPut(H,4, 2+pos, h11 );

    femMatPut(H,5, 2+pos, h33 );
    femMatPut(H,5, 3+pos, h22 );

    femMatPut(H,6, 1+pos, h33 );
    femMatPut(H,6, 3+pos, h11 );
#else
    femMatPut(H,6, 1+pos, h22 );
    femMatPut(H,6, 2+pos, h11 );

    femMatPut(H,4, 2+pos, h33 );
    femMatPut(H,4, 3+pos, h22 );

    femMatPut(H,5, 1+pos, h33 );
    femMatPut(H,5, 3+pos, h11 );
#endif
  }

  return(AF_OK);
}

int e009_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
  int rv = AF_OK ;
  tMatrix coord ;
  tMatrix xyz ;
  tMatrix jac ;
  tMatrix H ;
  tMatrix HT ;
  tMatrix D ;
  tMatrix D_r ;
  tMatrix HTD ;
  tMatrix K_i ;
  tMatrix gauss ;
	tVector epsilon ;
	tVector sigma ;
	tVector d_sigma ;
	tVector u_e ;
	tVector F_0 ;
	tVector sigma_0 ;
	tVector epsilon_0 ;
  double  detj;
  long    ipoints = 3 ;
  double  x,y,z ;
  double  weight_x,weight_y,weight_z ;
  double  mult = 1.0 ;
  long ipos = 0 ;
  long eT, mT, mtype ;
  long i,j,k ;
  long ii,jj;

	eT = femGetETypePos(ePos); 
	mT = femGetEMPPos(ePos); 
  mtype = Mat[femGetMPTypePos(mT)].type ;

	femMatNull(&coord);
	femMatNull(&xyz);
	femMatNull(&jac);
	femMatNull(&H);
	femMatNull(&HT);
	femMatNull(&HTD);
	femMatNull(&D);
	femMatNull(&K_i);
	femMatNull(&gauss);

	femMatNull(&D_r);
	femVecNull(&epsilon);
	femVecNull(&sigma);
	femVecNull(&d_sigma);
	femVecNull(&u_e);
	femVecNull(&F_0);

	if ((rv=femFullMatInit(&coord,8,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&xyz,8,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&jac,3,3)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&H,6,24)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&HT,24,6)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&HTD,24,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&D,6,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&K_i,24,24)) != AF_OK) { goto memFree; }

	if (Mode == AF_YES)
	{
		if ((rv=femVecFullInit(&u_e, 24)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&epsilon, 6)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&sigma, 6)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&d_sigma, 6)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&sigma_0, 6)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&epsilon_0, 6)) != AF_OK) { goto memFree; }

		if ((rv=femVecFullInit(&F_0, 24)) != AF_OK) { goto memFree; }

		femVecSetZero(F_e);
		
		if (Fr_e != NULL) { femVecSetZero(Fr_e); }

		if ((rv=femFullMatInit(&D_r, 6, 6)) != AF_OK){goto memFree;}

		/* displacements:  */
  	femLocUtoU_e(&u, ePos, &u_e);
	}

	if ((rv=femFullMatInit(&gauss,ipoints,2)) != AF_OK) { goto memFree; }

  /* gauss data */
  if ((rv=e008_init_gauss(ipoints, &gauss)) != AF_OK) { goto memFree; }
  
  /* -1..1 coords: */
  e009_fill_coords(&coord);

  /* real coordinates: */
  for (i=1; i<=8; i++)
  {
    femMatPut(&xyz,i,1, femGetNCoordPosX(femGetENodePos(ePos,i-1)) );
    femMatPut(&xyz,i,2, femGetNCoordPosY(femGetENodePos(ePos,i-1)) );
    femMatPut(&xyz,i,3, femGetNCoordPosZ(femGetENodePos(ePos,i-1)) );
  }

  femMatSetZero(K_e) ;


  /* numerical integration: */

  ipos = -1 ;

	/* ----------------------- */

  for (i=1; i<=ipoints; i++)
  {
    for (j=1; j<=ipoints; j++)
    {
      for (k=1; k<=ipoints; k++)
      {
        ipos++ ;
        
        /* int. point coordinates: */
        x = femMatGet(&gauss,i, 1) ;
        y = femMatGet(&gauss,j, 1) ;
        z = femMatGet(&gauss,k, 1) ;

        weight_x = femMatGet(&gauss,i, 2) ;
        weight_y = femMatGet(&gauss,j, 2) ;
        weight_z = femMatGet(&gauss,k, 2) ;

  			/* jacobi matrix: */
  			e009_fill_J(&jac, &coord, x, y, z, &xyz);

  			/* determinant  */
  			if ((rv=e009_jac_det(&jac, &detj))!= AF_OK) { goto memFree; }

        /* integrational multiplier */
				mult = detj * weight_x * weight_y * weight_z ;

  			/* inversion - "jac" will be a inverse matrix */
  			if (femLUinverse(&jac) != AF_OK)
  			{
#ifdef RUN_VERBOSE
    			fprintf(msgout, "[E] %s!\n", _("Inversion of Jascobi matric failed"));
#endif
    			goto memFree;
  			}

        if ((rv=e009_fill_H(&H, &jac, &coord, x,y,z)) != AF_OK) {goto memFree;}

        femMatTran(&H, &HT);


				if (Mode == AF_YES)
				{
					femVecSetZero(&epsilon);
					femMatVecMult(&H,&u_e, &epsilon) ;

        	if ((rv = fem_D_3D(ePos, ipos+1, eT, mT, &d_sigma, &epsilon, AF_NO, &D_r) ) != AF_OK) {goto memFree;}
    			femMatVecMult(&D_r, &epsilon, &d_sigma) ; 

					if (femTangentMatrix == AF_YES)
					{

					femVecPut(&epsilon_0, 1,  femGetEResVal(ePos, RES_EX,  ipos+1)) ;
					femVecPut(&epsilon_0, 2,  femGetEResVal(ePos, RES_EY,  ipos+1)) ;
					femVecPut(&epsilon_0, 3,  femGetEResVal(ePos, RES_EZ,  ipos+1)) ;
					femVecPut(&epsilon_0, 4,  femGetEResVal(ePos, RES_EYZ, ipos+1)) ;
					femVecPut(&epsilon_0, 5,  femGetEResVal(ePos, RES_EZX, ipos+1)) ;
					femVecPut(&epsilon_0, 6,  femGetEResVal(ePos, RES_EXY, ipos+1)) ;

					femVecPut(&sigma_0, 1,  femGetEResVal(ePos, RES_SX,  ipos+1)) ;
					femVecPut(&sigma_0, 2,  femGetEResVal(ePos, RES_SY,  ipos+1)) ;
					femVecPut(&sigma_0, 3,  femGetEResVal(ePos, RES_SZ,  ipos+1)) ;
					femVecPut(&sigma_0, 4,  femGetEResVal(ePos, RES_SYZ, ipos+1)) ;
					femVecPut(&sigma_0, 5,  femGetEResVal(ePos, RES_SZX, ipos+1)) ;
					femVecPut(&sigma_0, 6,  femGetEResVal(ePos, RES_SXY, ipos+1)) ;

					if ((mtype == 2) || (mtype ==3)) /* "pseudo sec" */
					{
					/* start of NEW CODE --------------------------------------- */
        	if ((rv = fem_D_3D(ePos, ipos+1, eT, mT, &sigma, &epsilon, AF_YES, &D) ) != AF_OK) {goto memFree;}
    			femMatVecMult(&D, &epsilon, &sigma) ;

					for (ii=1; ii<=6; ii++)
					{
						femVecAdd(&sigma, ii, femVecGet(&sigma_0, ii)) ;

						femVecPut(&d_sigma, ii,
								femVecGet(&d_sigma, ii) - femVecGet(&sigma,ii)
								);
					}

					/* order is important! - this must be here! */
			  	femAddEResVal(ePos, RES_EX,  ipos+1, femVecGet(&epsilon,1));
	      	femAddEResVal(ePos, RES_EY,  ipos+1, femVecGet(&epsilon,2));
	      	femAddEResVal(ePos, RES_EZ,  ipos+1, femVecGet(&epsilon,3));
	      	femAddEResVal(ePos, RES_EYZ, ipos+1, femVecGet(&epsilon,4));
	      	femAddEResVal(ePos, RES_EZX, ipos+1, femVecGet(&epsilon,5));
	      	femAddEResVal(ePos, RES_EXY, ipos+1, femVecGet(&epsilon,6));
					
					/* e_sum = e_0 + delta_e:  */
					femVecAdd(&epsilon_0, 1, femVecGet(&epsilon, 1)) ;
					femVecAdd(&epsilon_0, 2, femVecGet(&epsilon, 2)) ;
					femVecAdd(&epsilon_0, 3, femVecGet(&epsilon, 3)) ;
					femVecAdd(&epsilon_0, 4, femVecGet(&epsilon, 4)) ;
					femVecAdd(&epsilon_0, 5, femVecGet(&epsilon, 5)) ;
					femVecAdd(&epsilon_0, 6, femVecGet(&epsilon, 6)) ;
					

          /* sigma_0 is USELESS here! */
        	if ((rv = fem_D_3D(ePos, ipos+1, eT, mT, &sigma_0, &epsilon_0, AF_YES, &D) ) != AF_OK) {goto memFree;}

					femMatVecMult(&D, &epsilon_0, &sigma ) ; /* new sigma */

					/* d_sigma (residual stress) computation */
					femVecSetZero(&d_sigma);
					femMatVecMult(&D_r, &epsilon, &d_sigma ) ;

					for (ii=1; ii<=6; ii++)
					{
						femVecAdd(&d_sigma, ii, femVecGet(&sigma_0, ii)-femVecGet(&sigma, ii));
					}
					/* end of d_sigma computation */

         	femPutEResVal(ePos, RES_SX, ipos+1, femVecGet(&sigma,1));
	       	femPutEResVal(ePos, RES_SY, ipos+1, femVecGet(&sigma,2));
	       	femPutEResVal(ePos, RES_SZ ,ipos+1, femVecGet(&sigma,3));
	       	femPutEResVal(ePos, RES_SYZ,ipos+1, femVecGet(&sigma,4));
	       	femPutEResVal(ePos, RES_SZX,ipos+1, femVecGet(&sigma,5));
	       	femPutEResVal(ePos, RES_SXY,ipos+1, femVecGet(&sigma,6));
#ifdef DEVEL_VERBOSE
				fprintf(msgout,"\nSIGMA: %f %f %f\n", femVecGet(&sigma,1), femVecGet(&sigma,2), femVecGet(&sigma,3));
				fprintf(msgout,"\nSIGMA_R: %f %f %f\n", femVecGet(&d_sigma,1), femVecGet(&d_sigma,2), femVecGet(&d_sigma,3));
#endif
					/* end of NEW CODE ----------------------------------------- */
					}
					else /* mtype */
					{
  					for (ii=1; ii<=6; ii++)
	  				{
	  					femVecPut(&epsilon_0, ii, femVecGet(&epsilon, ii));
				  	}

        	  if ((rv = fem_D_3D(ePos, ipos+1, eT, mT, &sigma_0, &epsilon_0, AF_NO, &D_r) ) != AF_OK) {goto memFree;}

						femVecSetZero(&d_sigma);
    			  femMatVecMult(&D_r, &epsilon, &d_sigma) ;

  					for (ii=1; ii<=6; ii++)
	  				{
	  					femVecPut(&sigma_0, ii, femVecGet(&d_sigma, ii));
				  	}

        	  if ((rv = fem_D_3D(ePos, ipos+1, eT, mT, &sigma_0, &epsilon_0, AF_YES, &D) ) != AF_OK) {goto memFree;}

						femVecSetZero(&sigma);
    			  femMatVecMult(&D, &epsilon, &sigma) ;

  					for (ii=1; ii<=6; ii++)
	  				{
	  					femVecPut(&d_sigma, ii, (femVecGet(&d_sigma, ii) - femVecGet(&sigma,ii)));
				  	}

			  		femAddEResVal(ePos, RES_EX, ipos+1,  femVecGet(&epsilon,1));
	      		femAddEResVal(ePos, RES_EY, ipos+1,  femVecGet(&epsilon,2));
	      		femAddEResVal(ePos, RES_EZ, ipos+1,  femVecGet(&epsilon,3));
	      		femAddEResVal(ePos, RES_EYZ, ipos+1, femVecGet(&epsilon,4));
	      		femAddEResVal(ePos, RES_EZX, ipos+1, femVecGet(&epsilon,5));
	      		femAddEResVal(ePos, RES_EXY, ipos+1, femVecGet(&epsilon,6));

			  		femAddEResVal(ePos, RES_SX, ipos+1,  femVecGet(&sigma,1));
	      		femAddEResVal(ePos, RES_SY, ipos+1,  femVecGet(&sigma,2));
	      		femAddEResVal(ePos, RES_SZ, ipos+1,  femVecGet(&sigma,3));
	      		femAddEResVal(ePos, RES_SYZ, ipos+1, femVecGet(&sigma,4));
	      		femAddEResVal(ePos, RES_SZX, ipos+1, femVecGet(&sigma,5));
	      		femAddEResVal(ePos, RES_SXY, ipos+1, femVecGet(&sigma,6));
					} /*mtype */
					}
					else
					{
						/* secant matrix - untested! */
        	  if ((rv = fem_D_3D(ePos, ipos+1, eT, mT, &d_sigma, &epsilon, AF_YES, &D) ) != AF_OK) {goto memFree;}

						femVecSetZero(&sigma);
    			  femMatVecMult(&D, &epsilon, &sigma) ;

  					for (ii=1; ii<=6; ii++)
	  				{
	  					femVecPut(&d_sigma, ii, femVecGet(&d_sigma, ii) - femVecGet(&sigma,ii));
				  	}

			  		femPutEResVal(ePos, RES_EX, ipos+1,  femVecGet(&epsilon,1));
	      		femPutEResVal(ePos, RES_EY, ipos+1,  femVecGet(&epsilon,2));
	      		femPutEResVal(ePos, RES_EZ, ipos+1,  femVecGet(&epsilon,3));
	      		femPutEResVal(ePos, RES_EYZ, ipos+1, femVecGet(&epsilon,4));
	      		femPutEResVal(ePos, RES_EZX, ipos+1, femVecGet(&epsilon,5));
	      		femPutEResVal(ePos, RES_EXY, ipos+1, femVecGet(&epsilon,6));

			  		femPutEResVal(ePos, RES_SX, ipos+1,  femVecGet(&sigma,1));
	      		femPutEResVal(ePos, RES_SY, ipos+1,  femVecGet(&sigma,2));
	      		femPutEResVal(ePos, RES_SZ, ipos+1,  femVecGet(&sigma,3));
	      		femPutEResVal(ePos, RES_SYZ, ipos+1, femVecGet(&sigma,4));
	      		femPutEResVal(ePos, RES_SZX, ipos+1, femVecGet(&sigma,5));
	      		femPutEResVal(ePos, RES_SXY, ipos+1, femVecGet(&sigma,6));
					}

					femVecSetZero(&F_0);
        	femMatVecMult(&HT, &sigma, &F_0);
					for (ii=1; ii<=24; ii++)
					  { femVecAdd(F_e, ii, mult*femVecGet(&F_0, ii)) ; }
					
					femVecSetZero(&F_0);
        	femMatVecMult(&HT, &d_sigma, &F_0);
					for (ii=1; ii<=24; ii++)
					  { femVecAdd(Fr_e, ii, mult*femVecGet(&F_0, ii)) ; }
				}
				else
				{
        	/* "D" computation should be usefull here ;-) */
        	if ((rv = fem_D_3D(ePos, ipos+1, eT, mT, NULL, NULL, AF_NO, &D) ) != AF_OK) {goto memFree;}
				}

        femMatSetZero(&HTD) ;
        femMatSetZero(&K_i) ;

        /* 24,6 x 6,6 = 24,6  */
        femMatMatMult(&HT, &D, &HTD);

        /* 24,6 z 6,24  = 24,24  */
        femMatMatMult(&HTD, &H, &K_i);

        for (ii=1; ii<=24; ii++)
        {
          for (jj=1; jj<=24; jj++)
          {
            femMatAdd(K_e,ii,jj, mult*femMatGet(&K_i,ii,jj));
          }
        }
      }
    }
  }

memFree:
  femMatFree(&coord);
  femMatFree(&xyz);
  femMatFree(&jac);

  femMatFree(&H);
  femMatFree(&HT);

	femMatFree(&HTD);
	femMatFree(&D);
	femMatFree(&K_i);

  femMatFree(&gauss);

	if (Mode == AF_YES)
	{
		femMatFree(&D_r);

		femVecFree(&u_e);
		femVecFree(&epsilon);
		femVecFree(&sigma);
		femVecFree(&d_sigma);
		femVecFree(&F_0);
		femVecFree(&sigma_0);
		femVecFree(&epsilon_0);
	}

	return(rv);
}

int e009_fill_N(tMatrix *H, tMatrix *coord, double x, double y, double z)
{
  double h11, h22, h33 ;
  long   pos ;
  int i;

  femMatSetZero(H) ;

  for(i=0; i<8; i++) 
  {
    h11 = e009_deriv_p_one(coord,0,i+1, x,y,z);
    h22 = e009_deriv_p_one(coord,0,i+1, x,y,z);
    h33 = e009_deriv_p_one(coord,0,i+1, x,y,z);

    pos = 3 * i ;
    femMatPut(H,1, 1+pos, h11 );
    femMatPut(H,2, 2+pos, h22 );
    femMatPut(H,3, 3+pos, h33 );
  }

  return(AF_OK);
}

int e009_mass(long ePos, tMatrix *M_e)
{
	int rv = AF_OK;
	double ro;
  tMatrix coord ;
  tMatrix xyz ;
  tMatrix jac ;
  tMatrix H ;
  tMatrix HT ;
  tMatrix M_i ;
  tMatrix gauss ;
  double  detj;
  long    ipoints = 3 ;
  double  x,y,z ;
  double  weight_x,weight_y,weight_z ;
  double  mult = 1.0 ;
  long ipos = 0 ;
  long eT, mT ;
  long i,j,k ;
  long ii,jj;

	eT = femGetETypePos(ePos); 
	mT = femGetEMPPos(ePos); 

	ro = femGetMPValPos(ePos, MAT_DENS, 0) ;

	femMatNull(&coord);
	femMatNull(&xyz);
	femMatNull(&jac);
	femMatNull(&H);
	femMatNull(&HT);
	femMatNull(&M_i);
	femMatNull(&gauss);


	if ((rv=femFullMatInit(&coord,8,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&xyz,8,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&jac,3,3)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&H,3,24)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&HT,24,3)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&M_i,24,24)) != AF_OK) { goto memFree; }


	if ((rv=femFullMatInit(&gauss,ipoints,2)) != AF_OK) { goto memFree; }

  /* gauss data */
  if ((rv=e008_init_gauss(ipoints, &gauss)) != AF_OK) { goto memFree; }
  
  /* -1..1 coords: */
  e009_fill_coords(&coord);

  /* real coordinates: */
  for (i=1; i<=8; i++)
  {
    femMatPut(&xyz,i,1, femGetNCoordPosX(femGetENodePos(ePos,i-1)) );
    femMatPut(&xyz,i,2, femGetNCoordPosY(femGetENodePos(ePos,i-1)) );
    femMatPut(&xyz,i,3, femGetNCoordPosZ(femGetENodePos(ePos,i-1)) );
  }

  femMatSetZero(M_e) ;


  /* numerical integration: */

  ipos = -1 ;

	/* ----------------------- */

  for (i=1; i<=ipoints; i++)
  {
    for (j=1; j<=ipoints; j++)
    {
      for (k=1; k<=ipoints; k++)
      {
        ipos++ ;
        
        /* int. point coordinates: */
        x = femMatGet(&gauss,i, 1) ;
        y = femMatGet(&gauss,j, 1) ;
        z = femMatGet(&gauss,k, 1) ;

        weight_x = femMatGet(&gauss,i, 2) ;
        weight_y = femMatGet(&gauss,j, 2) ;
        weight_z = femMatGet(&gauss,k, 2) ;

  			/* jacobi matrix: */
  			e009_fill_J(&jac, &coord, x, y, z, &xyz);

  			/* determinant  */
  			if ((rv=e009_jac_det(&jac, &detj))!= AF_OK) { goto memFree; }

        if ((rv=e009_fill_N(&H, &coord, x,y,z)) != AF_OK) {goto memFree;}

        femMatTran(&H, &HT);


        femMatSetZero(&M_i) ;

        /* 24,6 x 6,24 = 24,24  */
        femMatMatMult(&HT, &H, &M_i);

				mult = ro * detj * weight_x * weight_y * weight_z ;

        for (ii=1; ii<=24; ii++)
        {
          for (jj=1; jj<=24; jj++)
          {
            femMatAdd(M_e,ii,jj, mult*femMatGet(&M_i,ii,jj));
          }
        }
      }
    }
  }

memFree:
  femMatFree(&coord);
  femMatFree(&xyz);
  femMatFree(&jac);

  femMatFree(&H);
  femMatFree(&HT);
	femMatFree(&M_i);

  femMatFree(&gauss);

	return(rv);
}

long e009_rvals(long ePos)
{
	return(14*27);
}

int e009_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

int e009_res_node(long ePos, long nPos, long type, double *val)
{
  int inum = 0 ;
  int i;

  switch (nPos)
  {
    case 0:  inum  =  0 ; break ;
    case 1:  inum  =  6 ; break ;
    case 2:  inum  = 24 ; break ;
    case 3:  inum  = 18 ; break ;
    case 4:  inum  =  2 ; break ;
    case 5:  inum  =  8 ; break ;
    case 6:  inum  = 26 ; break ;
    case 7:  inum  = 20 ; break ;
    default: return(AF_ERR);
  }

  for (i=0; i<Elem[9].res_rp; i++)
  {
    if (type == Elem[9].nres_rp[i]) 
    { 
      *val = femGetEResVal(ePos, type, inum) ;
      return(AF_OK) ;
    }
  }
  
  return(AF_ERR);
}


int addElem_009(void)
{
	int rv = AF_OK;
	static long type    = 9 ;
	static long nodes   = 8 ;
	static long dofs    = 3 ;
	static long ndof[3] = {U_X,U_Y,U_Z} ;
	static long rs      = 0 ;
	static long *real   = NULL ;
	static long rs_rp    = 0 ;
	static long *real_rp = NULL ;
	static long res      = 0 ;
	static long *nres    = NULL ;
	static long res_rp      = 14 ;
	static long nres_rp[14] = {RES_SX,RES_SY,RES_SZ,RES_SYZ,RES_SZX,RES_SXY, RES_EX,RES_EY,RES_EZ,RES_EYZ,RES_EZX,RES_EXY,RES_CR1,RES_PSI};

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

	Elem[type].stiff = e009_stiff;
	Elem[type].mass  = e009_mass;
	Elem[type].rvals = e009_rvals;
	Elem[type].eload = e009_eload;
	Elem[type].res_p_loc = e008_res_p_loc;
	Elem[type].res_node = e009_res_node;
	return(rv);
}

/* end of fem_e009.c */

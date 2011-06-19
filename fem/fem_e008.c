/*
   File name: fem_e008.c
   Date:      2004/03/07 15:54
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

	 FEM Solver - Element 008 (structural brick - 20 nodes)

*/

#include "fem_elem.h"
#include "fem_pl3d.h"

/* nodes of brick */
long p_a_nodes[8]={1,3,5,7,13,15,17,19};
long p_b_nodes[4]={2,6,14,18};
long p_c_nodes[4]={4,8,16,20};
long p_d_nodes[4]={9,10,11,12};

/* prepares coordinates of points */
void e008_fill_coords(tMatrix *coord)
{
  femMatPut(coord, 1, 1,   -1.0);
  femMatPut(coord, 1, 2,   -1.0);
  femMatPut(coord, 1, 3,   -1.0);

  femMatPut(coord, 2, 1,    0.0);
  femMatPut(coord, 2, 2,   -1.0);
  femMatPut(coord, 2, 3,   -1.0);

  femMatPut(coord, 3, 1,    1.0);
  femMatPut(coord, 3, 2,   -1.0);
  femMatPut(coord, 3, 3,   -1.0);

  femMatPut(coord, 4, 1,    1.0);
  femMatPut(coord, 4, 2,    0.0);
  femMatPut(coord, 4, 3,   -1.0);

  femMatPut(coord, 5, 1,    1.0);
  femMatPut(coord, 5, 2,    1.0);
  femMatPut(coord, 5, 3,   -1.0);

  femMatPut(coord, 6, 1,    0.0);
  femMatPut(coord, 6, 2,    1.0);
  femMatPut(coord, 6, 3,   -1.0);

  femMatPut(coord, 7, 1,   -1.0);
  femMatPut(coord, 7, 2,    1.0);
  femMatPut(coord, 7, 3,   -1.0);

  femMatPut(coord, 8, 1,   -1.0);
  femMatPut(coord, 8, 2,    0.0);
  femMatPut(coord, 8, 3,   -1.0);

  femMatPut(coord, 9, 1,   -1.0);
  femMatPut(coord, 9, 2,   -1.0);
  femMatPut(coord, 9, 3,    0.0);

  femMatPut(coord,10, 1,    1.0);
  femMatPut(coord,10, 2,   -1.0);
  femMatPut(coord,10, 3,    0.0);

  femMatPut(coord,11, 1,    1.0);
  femMatPut(coord,11, 2,    1.0);
  femMatPut(coord,11, 3,    0.0);

  femMatPut(coord,12, 1,   -1.0);
  femMatPut(coord,12, 2,    1.0);
  femMatPut(coord,12, 3,    0.0);

  femMatPut(coord,13, 1,   -1.0);
  femMatPut(coord,13, 2,   -1.0);
  femMatPut(coord,13, 3,    1.0);

  femMatPut(coord,14, 1,    0.0);
  femMatPut(coord,14, 2,   -1.0);
  femMatPut(coord,14, 3,    1.0);

  femMatPut(coord,15, 1,    1.0);
  femMatPut(coord,15, 2,   -1.0);
  femMatPut(coord,15, 3,    1.0);

  femMatPut(coord,16, 1,    1.0);
  femMatPut(coord,16, 2,    0.0);
  femMatPut(coord,16, 3,    1.0);

  femMatPut(coord,17, 1,    1.0);
  femMatPut(coord,17, 2,    1.0);
  femMatPut(coord,17, 3,    1.0);

  femMatPut(coord,18, 1,    0.0);
  femMatPut(coord,18, 2,    1.0);
  femMatPut(coord,18, 3,    1.0);

  femMatPut(coord,19, 1,   -1.0);
  femMatPut(coord,19, 2,    1.0);
  femMatPut(coord,19, 3,    1.0);

  femMatPut(coord,20, 1,   -1.0);
  femMatPut(coord,20, 2,    0.0);
  femMatPut(coord,20, 3,    1.0);
}

/** determinant of Jacobi matrix  
 * @param jac Jacobi matrix
 * @param detj determinant (result)
 * @return status
 */
int e008_jac_det(tMatrix *jac, double *detj)
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

double e008_p_a(tMatrix *coord, int deriv, long point, double x, double y, double z)
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
        *(xv*x + yv*y + zv*z - 2.0) ) ;
        break ;
    case 1: /* d/dx) */
        return(0.125
            *(1.0 + yv*y)
            *(1.0 + zv*z)
            *(xv + 2.0*xv*xv*x + xv*yv*y + xv*zv*z - 2.0*xv)
            );
        break;
    case 2: /* d/dy) */
        return(0.125
            *(1.0 + xv*x)
            *(1.0 + zv*z)
            *(yv + 2.0*yv*yv*y + yv*xv*x + yv*zv*z - 2.0*yv)
            );
        break;
    case 3: /* d/dz) */
        return(0.125
            *(1.0 + xv*x)
            *(1.0 + yv*y)
            *(zv + 2.0*zv*zv*z + zv*xv*x+ zv*yv*y - 2.0*zv)
            );
        break;
    default: return(0.0);
  }
}

double e008_p_b(tMatrix *coord, int deriv, long point, double x, double y, double z)
{
  double xv, yv,zv ;

  xv = femMatGet(coord, point, 1) ;
  yv = femMatGet(coord, point, 2) ;
  zv = femMatGet(coord, point, 3) ;

  switch (deriv)
  {
    case 0:
      return( 0.25 *(1.0 - x*x) *(1.0+yv*y) *(1.0+zv*z)) ;
        break;
    case 1: /* d/dx */
        return(-0.5 * x *(1.0 + yv*y) *(1.0 + zv*z)
            );
        break;
    case 2: /* d/dy */
        return(0.25 * yv *(1.0 - x*x) *(1.0 + zv*z));
        break;
    case 3: /* d/dy */
        return(0.25 * zv *(1.0 - x*x) *(1.0 + yv*y));
        break;
    default: return(0.0);
  }
}

double e008_p_c(tMatrix *coord, int deriv, long point, double x, double y, double z)
{
  double xv, yv,zv ;

  xv = femMatGet(coord, point, 1) ;
  yv = femMatGet(coord, point, 2) ;
  zv = femMatGet(coord, point, 3) ;

  switch (deriv)
  {
    case 0:
      return( 0.25 *(1.0+xv*x) *(1.0 - y*y) *(1.0+zv*z)) ;
        break;
    case 1: /* d/dx */
        return(0.25 * xv *(1.0 - y*y) *(1.0 + zv*z));
        break;
    case 2: /* d/dy */
        return(-0.5 * y *(1.0 + xv*x) *(1.0 + zv*z));
        break;
    case 3: /* d/dy */
        return(0.25 * zv *(1.0 + xv*x) *(1.0 - y*y));
        break;
    default: return(0.0);
  }
}

double e008_p_d(tMatrix *coord, int deriv, long point, double x, double y, double z)
{
  double xv, yv,zv ;

  xv = femMatGet(coord, point, 1) ;
  yv = femMatGet(coord, point, 2) ;
  zv = femMatGet(coord, point, 3) ;

  switch (deriv)
  {
    case 0:
      return( 0.25 *(1.0+xv*x) *(1.0+yv*y) *(1.0 - z*z)) ;
        break;
    case 1: /* d/dx */
        return(0.25 * xv *(1.0 + yv*y) *(1.0 - z*z));
        break;
    case 2: /* d/dy */
        return(0.25 * yv *(1.0 + xv*x) *(1.0 - z*z));
        break;
    case 3: /* d/dy */
        return(-0.5 * z *(1.0 + xv*x) *(1.0 + yv*y));
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
double e008_deriv_p(tMatrix *coord, long deriv, long dir, 
    double x, double y, double z, tMatrix *xyz)
{
  double val = 0.0 ;
  double val_i ;
  long   i ;

  val = 0.0 ;

  for (i=0; i<8; i++)
  {
    val_i = e008_p_a(coord, deriv, p_a_nodes[i], x, y, z) ;
    if (xyz != NULL) { val_i *= femMatGet(xyz,p_a_nodes[i], dir) ; }
    val += val_i ;
  }

  for (i=0; i<4; i++)
  {
    val_i = e008_p_b(coord, deriv, p_b_nodes[i], x, y, z) ;
    if (xyz != NULL) { val_i *= femMatGet(xyz,p_b_nodes[i], dir) ; }
    val += val_i ;
  }

  for (i=0; i<4; i++)
  {
    val_i = e008_p_c(coord, deriv, p_c_nodes[i], x, y, z) ;
    if (xyz != NULL) { val_i *= femMatGet(xyz,p_c_nodes[i], dir) ; }
    val += val_i ;
  }

  for (i=0; i<4; i++)
  {
    val_i = e008_p_d(coord, deriv, p_d_nodes[i], x, y, z) ;
    if (xyz != NULL) { val_i *= femMatGet(xyz,p_d_nodes[i], dir) ; }
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
double e008_deriv_p_one(tMatrix *coord,  long deriv, long number, double x, double y, double z)
{
  double val = 0.0 ;
	int i ;

  val = 0.0 ;

	for (i=0; i<8; i++)
	{
		if (number == p_a_nodes[i])
		{
			return(e008_p_a(coord, deriv, p_a_nodes[i], x, y, z) );
		}
	}
	
	for (i=0; i<4; i++)
	{
		if (number == p_b_nodes[i])
		{
			return(e008_p_b(coord, deriv, p_b_nodes[i], x, y, z) );
		}
	}
	
	for (i=0; i<4; i++)
	{
		if (number == p_c_nodes[i])
		{
			return(e008_p_c(coord, deriv, p_c_nodes[i], x, y, z) );
		}
	}
	
	for (i=0; i<4; i++)
	{
		if (number == p_d_nodes[i])
		{
			return(e008_p_d(coord, deriv, p_d_nodes[i], x, y, z) );
		}
	}

  return(val);
}

int e008_fill_J(tMatrix *jac, tMatrix *coord,
    double x, double y, double z, tMatrix *xyz)
{
  long i,j ;

	femMatSetZero(jac);

  for (i=1; i<=3; i++)
  {
    for (j=1; j<=3; j++)
    {
      femMatAdd(jac, i, j,
          e008_deriv_p(coord, i, j, x, y, z, xyz) );
    }
  }
  return(AF_OK);
}

int e008_fill_H(tMatrix *H, tMatrix *ijac, tMatrix *coord, double x, double y, double z)
{
  double h11, h22, h33 ;
  long   pos ;
  int i,j;

  h11 = 0.0 ;
  h22 = 0.0 ;
  h33 = 0.0 ;

  femMatSetZero(H) ;

  for(i=0; i<20; i++) 
  {
		h11 = 0.0 ;
		h22 = 0.0 ;
		h33 = 0.0 ;

  	for (j=1; j<=3; j++)
  	{
    	h11 += femMatGet(ijac,1,j)*e008_deriv_p_one(coord,j,i+1, x,y,z);
    	h22 += femMatGet(ijac,2,j)*e008_deriv_p_one(coord,j,i+1, x,y,z);
    	h33 += femMatGet(ijac,3,j)*e008_deriv_p_one(coord,j,i+1, x,y,z);
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


int e008_fill_N(tMatrix *H, tMatrix *coord, double x, double y, double z)
{
  double h11, h22, h33 ;
  long   pos ;
  int i;

  femMatSetZero(H) ;

  for(i=0; i<20; i++) 
  {
    h11 = e008_deriv_p_one(coord,0,i+1, x,y,z);
    h22 = e008_deriv_p_one(coord,0,i+1, x,y,z);
    h33 = e008_deriv_p_one(coord,0,i+1, x,y,z);

    pos = 3 * i ;
    femMatPut(H,1, 1+pos, h11 );
    femMatPut(H,2, 2+pos, h22 );
    femMatPut(H,3, 3+pos, h33 );
  }

  return(AF_OK);
}

int e008_init_gauss(long ipoints, tMatrix *gauss)
{
  switch(ipoints)
  {
    case 2: 
            femMatPut(gauss,1,1, -0.577350269189626) ;
            femMatPut(gauss,1,2,  1.0) ;
            femMatPut(gauss,2,1,  0.577350269189626) ;
            femMatPut(gauss,2,2,  1.0) ;
            break ;
    case 3: 
            femMatPut(gauss,1,1, -0.774596669241483) ;
            femMatPut(gauss,1,2,  0.555555555555556) ;

            femMatPut(gauss,2,1,  0.000000000000000) ;
            femMatPut(gauss,2,2,  0.888888888888889) ;

            femMatPut(gauss,3,1,  0.774596669241483) ;
            femMatPut(gauss,3,2,  0.555555555555556) ;
            break ;
    case 4: 
            femMatPut(gauss,1,1, -0.861136311594053) ;
            femMatPut(gauss,1,2,  0.347854845137454) ;

            femMatPut(gauss,2,1, -0.339981043584856) ;
            femMatPut(gauss,2,2,  0.652145154862546) ;

            femMatPut(gauss,3,1,  0.339981043584856) ;
            femMatPut(gauss,3,2,  0.652145154862546) ;

            femMatPut(gauss,4,1,  0.861136311594053) ;
            femMatPut(gauss,4,2,  0.347854845137454) ;
            break;

    default:
#ifdef RUN_VERBOSE
            fprintf(msgout,"[E] %s: %li!\n",
                _("Invalid number of integration points"),
                ipoints);
#endif
            return(AF_ERR_VAL);
            break;
  }

  return(AF_OK);
}


int e008_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
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
	tVector sigma_0 ;
	tVector epsilon_0 ;
	tVector d_sigma ;
	tVector u_e ;
	tVector F_0 ;
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
	femVecNull(&epsilon_0);
	femVecNull(&sigma_0);

	if ((rv=femFullMatInit(&coord,20,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&xyz,20,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&jac,3,3)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&H,6,60)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&HT,60,6)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&HTD,60,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&D,6,6)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&K_i,60,60)) != AF_OK) { goto memFree; }

	if (Mode == AF_YES)
	{
		if ((rv=femVecFullInit(&u_e, 60)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&epsilon, 6)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&sigma, 6)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&d_sigma, 6)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&sigma_0, 6)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&epsilon_0, 6)) != AF_OK) { goto memFree; }


		if ((rv=femVecFullInit(&F_0, 60)) != AF_OK) { goto memFree; }

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
  e008_fill_coords(&coord);

  /* real coordinates: */
  for (i=1; i<=20; i++)
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
  			e008_fill_J(&jac, &coord, x, y, z, &xyz);

  			/* determinant  */
  			if ((rv=e008_jac_det(&jac, &detj))!= AF_OK) { goto memFree; }

        /* integration multiplier */
				mult = detj * weight_x * weight_y * weight_z ;

  			/* inversion - "jac" will be a inverse matrix */
  			if (femLUinverse(&jac) != AF_OK)
  			{
#ifdef RUN_VERBOSE
    			fprintf(msgout, "[E] %s!\n", _("Inversion of Jascobi matric failed"));
#endif
    			goto memFree;
  			}

        if ((rv=e008_fill_H(&H, &jac, &coord, x,y,z)) != AF_OK) {goto memFree;}

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
					}
					else
					{
						/* secant matrix - untested! */
						if ((rv = fem_D_3D(ePos, ipos+1, eT, mT, &d_sigma, &epsilon, AF_YES,&D) ) != AF_OK) {goto memFree;}

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
					for (ii=1; ii<=60; ii++)
					  { femVecAdd(F_e, ii, mult*femVecGet(&F_0, ii)) ; }
					
					femVecSetZero(&F_0);
        	femMatVecMult(&HT, &d_sigma, &F_0);
					for (ii=1; ii<=60; ii++)
					  { femVecAdd(Fr_e, ii, mult*femVecGet(&F_0, ii)) ; }
				}
				else
				{
        	/* "D" computation should be usefull here ;-) */
        	if ((rv = fem_D_3D(ePos, ipos+1, eT, mT, NULL, NULL, AF_NO, &D) ) != AF_OK) {goto memFree;}
				}

        femMatSetZero(&HTD) ;
        femMatSetZero(&K_i) ;

        /* 60,6 x 6,6 = 60,6  */
        femMatMatMult(&HT, &D, &HTD);

        /* 60,6 z 6,60  = 60,60  */
        femMatMatMult(&HTD, &H, &K_i);

        for (ii=1; ii<=60; ii++)
        {
          for (jj=1; jj<=60; jj++)
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

		femVecFree(&epsilon_0);
		femVecFree(&sigma_0);
	}

	/*femMatPrn(K_e,"K_e");*/

	return(rv);
}

int e008_mass(long ePos, tMatrix *M_e)
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


	if ((rv=femFullMatInit(&coord,20,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&xyz,20,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&jac,3,3)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&H,3,60)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&HT,60,3)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&M_i,60,60)) != AF_OK) { goto memFree; }


	if ((rv=femFullMatInit(&gauss,ipoints,2)) != AF_OK) { goto memFree; }

  /* gauss data */
  if ((rv=e008_init_gauss(ipoints, &gauss)) != AF_OK) { goto memFree; }
  
  /* -1..1 coords: */
  e008_fill_coords(&coord);

  /* real coordinates: */
  for (i=1; i<=20; i++)
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
  			e008_fill_J(&jac, &coord, x, y, z, &xyz);

  			/* determinant  */
  			if ((rv=e008_jac_det(&jac, &detj))!= AF_OK) { goto memFree; }

        if ((rv=e008_fill_N(&H, &coord, x,y,z)) != AF_OK) {goto memFree;}

        femMatTran(&H, &HT);


        femMatSetZero(&M_i) ;

        /* 60,6 x 6,60 = 60,60  */
        femMatMatMult(&HT, &H, &M_i);

				mult = ro * detj * weight_x * weight_y * weight_z ;

        for (ii=1; ii<=60; ii++)
        {
          for (jj=1; jj<=60; jj++)
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

int e008_volume(long ePos, double *vol)
{
	int rv = AF_OK;
	double ro;
  tMatrix M_i ;
  tVector F_0 ;
  tVector F_1 ;
	int i ;

	ro = femGetMPValPos(ePos, MAT_DENS, 0) ;

	femMatNull(&M_i);
	femVecNull(&F_0);
	femVecNull(&F_1);

	if ((rv=femFullMatInit(&M_i,60,60)) != AF_OK) { goto memFree; }
	if ((rv=femVecFullInit(&F_0,60)) != AF_OK) { goto memFree; }
	if ((rv=femVecFullInit(&F_1,60)) != AF_OK) { goto memFree; }
	
	for (i=0; i<20; i++) { femVecPut(&F_0, 1 + 3*i , 1.0); }

	e008_mass(ePos, &M_i);
	femMatVecMult(&M_i,&F_0, &F_1);

	*vol = 0.0 ;
	for (i=0; i<20; i++) { *vol += femVecGet(&F_1, 1 + 3*i); }
	*vol = *vol / ro ; 

memFree:
	femMatFree(&M_i);
  femVecFree(&F_0);
  femVecFree(&F_1);
	return(rv);
}

long e008_rvals(long ePos)
{
	return(14*27);
}

int e008_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}


int e008_res_p_loc(long ePos, long point, double *x, double *y, double *z)
{
	int rv = AF_OK;
  tMatrix coord ;
  tMatrix xyz ;
  tMatrix gauss ;
  long    ipoints = 3 ;
  double  xn,yn,zn ;
  long ipos = 0 ;
  long eT ;
  long i,j,k, kk ;
	long nodes ;

	eT = femGetETypePos(ePos); 
	nodes = Elem[eT].nodes ;

	femMatNull(&coord);
	femMatNull(&xyz);
	femMatNull(&gauss);


	if ((rv=femFullMatInit(&coord,nodes,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&xyz,nodes,3)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&gauss,ipoints,2)) != AF_OK) { goto memFree; }

  /* gauss data */
  if ((rv=e008_init_gauss(ipoints, &gauss)) != AF_OK) { goto memFree; }
  
  /* -1..1 coords: */
  e008_fill_coords(&coord);

  /* real coordinates: */
  for (i=1; i<=nodes; i++)
  {
    femMatPut(&xyz,i,1, femGetNCoordPosX(femGetENodePos(ePos,i-1)) );
    femMatPut(&xyz,i,2, femGetNCoordPosY(femGetENodePos(ePos,i-1)) );
    femMatPut(&xyz,i,3, femGetNCoordPosZ(femGetENodePos(ePos,i-1)) );
  }

	*x = 0.0 ;
	*y = 0.0 ;
	*z = 0.0 ;


  ipos = -1 ;

  for (i=1; i<=ipoints; i++)
  {
    for (j=1; j<=ipoints; j++)
    {
      for (k=1; k<=ipoints; k++)
      {
        ipos++ ;

				if (ipos < point) {continue;}
				if (ipos > point) {break;}
        
        /* int. point coordinates: */
        xn = femMatGet(&gauss,i, 1) ;
        yn = femMatGet(&gauss,j, 1) ;
        zn = femMatGet(&gauss,k, 1) ;

				for (kk=1; kk<=20; kk++) /* number of nodes... */
				{
					*x += femMatGet(&xyz, kk, 1) * e008_deriv_p_one( &coord,  0, kk, xn, yn, zn);
					*y += femMatGet(&xyz, kk, 2) * e008_deriv_p_one( &coord,  0, kk, xn, yn, zn);
					*z += femMatGet(&xyz, kk, 3) * e008_deriv_p_one( &coord,  0, kk, xn, yn, zn);
				}

      }
    }
  }

memFree:
  femMatFree(&coord);
  femMatFree(&xyz);

  femMatFree(&gauss);

	return(rv);
}

int addElem_008(void)
{
	int rv = AF_OK;
	static long type    = 8 ;
	static long nodes   = 20 ;
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

	Elem[type].stiff = e008_stiff;
	Elem[type].mass  = e008_mass;
	Elem[type].rvals = e008_rvals;
	Elem[type].eload = e008_eload;
	Elem[type].res_p_loc = e008_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e008_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

/* end of fem_e008.c */

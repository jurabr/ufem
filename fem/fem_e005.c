/*
   File name: fem_e005.c
   Date:      2003/11/12 20:44
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

	 FEM Solver - element 005 (isoparametric Mindlin's slab) definition

	 $Id: fem_e005.c,v 1.5 2004/12/31 00:31:06 jirka Exp $

*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_

/* nodes of slab */
long p_5_nodes[4]={1,2,3,4};

int fem_D_slab(long ePos, double t, long mode, tMatrix *Dmat)
{
  double Ex, nu, G, D ;

  Ex  = femGetMPValPos(ePos, MAT_EX,   0)  ;
  nu  = femGetMPValPos(ePos, MAT_NU,   0)  ;

  G = Ex / (2.0*(1.0+nu)) ;
  D = (Ex*pow(t,3)) / (12.0 * (1.0 - (nu*nu))) ;

  femMatSetZero(Dmat);

  femMatPut(Dmat, 1,1, D ) ;
  femMatPut(Dmat, 1,2, D*nu ) ;
  femMatPut(Dmat, 2,1, D*nu ) ;
  femMatPut(Dmat, 2,2, D ) ;
  femMatPut(Dmat, 3,3, 0.5*((1.0 - nu)*D) ) ;

  femMatPut(Dmat, 4,4, 1.2 * G * t ) ;
  femMatPut(Dmat, 5,5, 1.2 * G * t ) ;

  return(AF_OK);
}


/* prepares coordinates of points */
void e005_fill_coords(tMatrix *coord)
{
  femMatPut(coord, 1, 1,   -1.0);
  femMatPut(coord, 1, 2,   -1.0);
  femMatPut(coord, 1, 3,    0.0);

  femMatPut(coord, 2, 1,    1.0);
  femMatPut(coord, 2, 2,   -1.0);
  femMatPut(coord, 2, 3,    0.0);

  femMatPut(coord, 3, 1,    1.0);
  femMatPut(coord, 3, 2,    1.0);
  femMatPut(coord, 3, 3,    0.0);

  femMatPut(coord, 4, 1,   -1.0);
  femMatPut(coord, 4, 2,    1.0);
  femMatPut(coord, 4, 3,    0.0);
}

/** determinant of Jacobi matrix  
 * @param jac Jacobi matrix
 * @param detj determinant (result)
 * @return status
 */
int e005_jac_det(tMatrix *jac, double *detj)
{
  *detj = 
    femMatGet(jac,1,1)*femMatGet(jac,2,2)
    - femMatGet(jac,2,1)*femMatGet(jac,1,2);

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

double e005_p_a(tMatrix *coord, int deriv, long point, double x, double y)
{
  double xv, yv ;

  xv = femMatGet(coord, point, 1) ;
  yv = femMatGet(coord, point, 2) ;

  switch (deriv)
  {
    case 0:
        return( 0.25
        *(1.0+xv*x)
        *(1.0+yv*y)
        );
        break ;
    case 1: /* d/dx) */
        return(0.25
            *xv
            *(1.0 + yv*y)
            );
        break;
    case 2: /* d/dy) */
        return(0.25
            *yv
            *(1.0 + xv*x)
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
double e005_deriv_p(tMatrix *coord, long deriv, long dir, 
    double x, double y, tMatrix *xyz)
{
  double val = 0.0 ;
  double val_i ;
  long   i ;

  val = 0.0 ;

  for (i=0; i<4; i++)
  {
    val_i = e005_p_a(coord, deriv, p_5_nodes[i], x, y) ;
    if (xyz != NULL) { val_i *= femMatGet(xyz,p_5_nodes[i], dir) ; }
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
double e005_deriv_p_one(tMatrix *coord,  long deriv, long number, double x, double y)
{
	return(e005_p_a(coord, deriv, number, x, y) );
}


int e005_fill_J(tMatrix *jac, tMatrix *coord,
    double x, double y, tMatrix *xyz)
{
  long i,j ;

	femMatSetZero(jac);

  for (i=1; i<=2; i++)
  {
    for (j=1; j<=2; j++)
    {
      femMatAdd(jac, i, j,
          e005_deriv_p(coord, i, j, x, y, xyz) );
    }
  }
  return(AF_OK);
}

int e005_fill_H(tMatrix *H, tMatrix *ijac, tMatrix *coord, double x, double y)
{
  double hdx, hdy,hhh ;
  long   pos ;
  int    i;

  femMatSetZero(H) ;

  for(i=0; i<4; i++) 
  {
    hdx = femMatGet(ijac,1,1)*e005_deriv_p_one(coord,1,i+1, x,y) + femMatGet(ijac,1,2)*e005_deriv_p_one(coord,2,i+1, x,y);
    hdy = femMatGet(ijac,2,1)*e005_deriv_p_one(coord,1,i+1, x,y) + femMatGet(ijac,2,2)*e005_deriv_p_one(coord,2,i+1, x,y);
    hhh = e005_deriv_p_one(coord,0,i+1, x,y);

    pos = 3 * i ;

    femMatPut(H,1, 2+pos, hdx );
    femMatPut(H,2, 3+pos, hdy );
    femMatPut(H,3, 2+pos, hdy );
    femMatPut(H,3, 3+pos, hdx );

    femMatPut(H,4, 1+pos, (-1.0)*hdx );
    femMatPut(H,4, 2+pos, hhh );
    femMatPut(H,5, 1+pos, -(1.0)*hdy );
    femMatPut(H,5, 3+pos, hhh );
  }

  return(AF_OK);
}


int e005_fill_N(tMatrix *H, tMatrix *coord, double x, double y)
{
  double hii ;
  long   pos ;
  int i;

  femMatSetZero(H) ;

  for(i=0; i<4; i++) 
  {
    hii = e005_deriv_p_one(coord,0,i+1, x,y);

    pos = 3 * i ;
    femMatPut(H,1, 1+pos, hii );
    femMatPut(H,2, 2+pos, hii );
    femMatPut(H,3, 3+pos, hii );
  }

  return(AF_OK);
}

int e005_init_gauss(long ipoints, tMatrix *gauss)
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

int e005_test_ij(long val, long ii, long jj)
{
  long vals[4] ;
  long i ;
  long iiok = AF_NO ;
  long jjok = AF_NO ;

  vals[0] = val ;

  for (i=1; i<4; i++)
  {
    vals[i] = i*3 + val ;
  }

  for (i=0; i<4; i++)
  {
    if (ii == vals[i]) {iiok = AF_YES; break;}
  }
  if (iiok != AF_YES) {return(AF_NO);}

  for (i=0; i<4; i++)
  {
    if (jj == vals[i]) {jjok = AF_YES; return(AF_YES);}
  }

  return(AF_NO);
}

int e005_shape_mat(long ePos, double mult_val, tMatrix *M_e, int all, long pos_winkler)
{
	int rv = AF_OK;
  tMatrix coord ;
  tMatrix xyz ;
  tMatrix jac ;
  tMatrix H ;
  tMatrix HT ;
  tMatrix M_i ;
  tMatrix gauss ;
  double  detj;
  long    ipoints = 2 ;
  double  x,y ;
  double  weight_x,weight_y ;
  double  mult = 1.0 ;
  long ipos = 0 ;
  long eT, mT ;
  long i,j ;
  long ii,jj;

	eT = femGetETypePos(ePos); 
	mT = femGetEMPPos(ePos); 

	femMatNull(&coord);
	femMatNull(&xyz);
	femMatNull(&jac);
	femMatNull(&H);
	femMatNull(&HT);
	femMatNull(&M_i);
	femMatNull(&gauss);


	if ((rv=femFullMatInit(&coord,4,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&xyz,4,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&jac,2,2)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&H,3,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&HT,12,3)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&M_i,12,12)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&gauss,ipoints,2)) != AF_OK) { goto memFree; }

  /* gauss data */
  if ((rv=e005_init_gauss(ipoints, &gauss)) != AF_OK) { goto memFree; }
  
  /* -1..1 coords: */
  e005_fill_coords(&coord);

  /* real coordinates: */
  for (i=1; i<=4; i++)
  {
    femMatPut(&xyz,i,1, femGetNCoordPosX(femGetENodePos(ePos,i-1)) );
    femMatPut(&xyz,i,2, femGetNCoordPosY(femGetENodePos(ePos,i-1)) );
  }


  /* numerical integration: */

  ipos = -1 ;

	/* ----------------------- */

  for (i=1; i<=ipoints; i++)
  {
    for (j=1; j<=ipoints; j++)
    {
        ipos++ ;
        
        /* int. point coordinates: */
        x = femMatGet(&gauss,i, 1) ;
        y = femMatGet(&gauss,j, 1) ;

        weight_x = femMatGet(&gauss,i, 2) ;
        weight_y = femMatGet(&gauss,j, 2) ;

  			/* jacobi matrix: */
  			e005_fill_J(&jac, &coord, x, y, &xyz);

  			/* determinant  */
  			if ((rv=e005_jac_det(&jac, &detj))!= AF_OK) { goto memFree; }

        if ((rv=e005_fill_N(&H, &coord, x,y)) != AF_OK) {goto memFree;}

        femMatTran(&H, &HT);


        femMatSetZero(&M_i) ;

        /* 60,6 x 6,60 = 60,60  */
        femMatMatMult(&HT, &H, &M_i);

				mult = mult_val * detj * weight_x * weight_y ;

        for (ii=1; ii<=12; ii++)
        {
          for (jj=1; jj<=12; jj++)
          {
            if (all != AF_YES)
            {
              if (e005_test_ij(pos_winkler,ii,jj)==AF_YES)
              {
                femMatAdd(M_e,ii,jj, mult*femMatGet(&M_i,ii,jj));
              }
            }
            else
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

int e005_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
  int rv = AF_OK ;
  tMatrix coord ;
  tMatrix xyz ;
  tMatrix jac, ijac ;
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
  double  thick ;
  double  winkler ;
  double  detj;
  long    ipoints = 2 ; /* DO NOT USE different number than 2 !!! */
  double  x,y ;
  double  weight_x,weight_y ;
  double  mult = 1.0 ;
  long ipos = 0 ;
  long eT, mT ;
  long i,j ;
  long ii,jj;

	eT = femGetETypePos(ePos); 
	mT = femGetEMPPos(ePos); 

	thick = femGetRSValPos(ePos, RS_HEIGHT, 0) ;

	femMatNull(&coord);
	femMatNull(&xyz);
	femMatNull(&jac);
	femMatNull(&ijac);
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

	if ((rv=femFullMatInit(&coord,8,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&xyz,8,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&jac,2,2)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&ijac,2,2)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&H,5,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&HT,12,5)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&HTD,12,5)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&D,5,5)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&K_i,12,12)) != AF_OK) { goto memFree; }

	if (Mode == AF_YES)
	{
		if ((rv=femVecFullInit(&u_e, 12)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&epsilon, 5)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&sigma, 5)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&d_sigma, 5)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&sigma_0, 5)) != AF_OK) { goto memFree; }
		if ((rv=femVecFullInit(&epsilon_0, 5)) != AF_OK) { goto memFree; }


		if ((rv=femVecFullInit(&F_0, 12)) != AF_OK) { goto memFree; }

		femVecSetZero(F_e);
		
		if (Fr_e != NULL) { femVecSetZero(Fr_e); }

		if ((rv=femFullMatInit(&D_r, 5, 5)) != AF_OK){goto memFree;}

		/* displacements:  */
  	femLocUtoU_e(&u, ePos, &u_e);
	}

	if ((rv=femFullMatInit(&gauss,ipoints,2)) != AF_OK) { goto memFree; }

  /* gauss data */
  if ((rv=e005_init_gauss(ipoints, &gauss)) != AF_OK) { goto memFree; }
  
  /* -1..1 coords: */
  e005_fill_coords(&coord);

  /* real coordinates: */
  for (i=1; i<=4; i++)
  {
    femMatPut(&xyz,i,1, femGetNCoordPosX(femGetENodePos(ePos,i-1)) );
    femMatPut(&xyz,i,2, femGetNCoordPosY(femGetENodePos(ePos,i-1)) );
  }

  femMatSetZero(K_e) ;


  /* numerical integration: */
  ipos = -1 ;

	/* ----------------------- */

  for (i=1; i<=ipoints; i++)
  {
    for (j=1; j<=ipoints; j++)
    {
        ipos++ ;
        
        /* int. point coordinates: */
        x = femMatGet(&gauss,i, 1) ;
        y = femMatGet(&gauss,j, 1) ;

        weight_x = femMatGet(&gauss,i, 2) ;
        weight_y = femMatGet(&gauss,j, 2) ;

  			/* jacobi matrix: */
  			e005_fill_J(&jac, &coord, x, y,&xyz);

  			/* determinant  */
  			if ((rv=e005_jac_det(&jac, &detj))!= AF_OK) { goto memFree; }

        /* integration multiplier */
				mult = detj * weight_x * weight_y ;

#if 1
  			/* inversion - "jac" will be a inverse matrix */
  			if (femLUinverse(&jac) != AF_OK)
  			{
#ifdef RUN_VERBOSE
    			fprintf(msgout, "[E] %s!\n", _("Inversion of Jascobi matric failed"));
#endif
    			goto memFree;
  			}

        if ((rv=e005_fill_H(&H, &jac, &coord, x,y)) != AF_OK) {goto memFree;}
#else

        femMatPut(&ijac,1,1, (femMatGet(&jac, 2,2) / detj) );
        femMatPut(&ijac,2,2, (femMatGet(&jac, 1,1) / detj) );
        femMatPut(&ijac,1,2, (femMatGet(&jac, 1,2) / detj) );
        femMatPut(&ijac,2,1, (femMatGet(&jac, 2,1) / detj) );

        if ((rv=e005_fill_H(&H, &ijac, &coord, x,y)) != AF_OK) {goto memFree;}
#endif

        femMatTran(&H, &HT);

				if (Mode == AF_YES)
				{
					femVecSetZero(&epsilon);
					femMatVecMult(&H,&u_e, &epsilon) ;

					if ((rv = fem_D_slab(ePos, thick, AF_YES, &D) ) != AF_OK) {goto memFree;}

					femVecSetZero(&sigma);
					femMatVecMult(&D, &epsilon, &sigma) ;

			  	femAddEResVal(ePos, RES_S_MX, ipos+1,  femVecGet(&sigma,1));
	      	femAddEResVal(ePos, RES_S_MY, ipos+1,  femVecGet(&sigma,2));
	      	femAddEResVal(ePos, RES_S_MXY, ipos+1,  femVecGet(&sigma,3));
	      	femAddEResVal(ePos, RES_S_VXZ, ipos+1, femVecGet(&sigma,4));
	      	femAddEResVal(ePos, RES_S_VYZ, ipos+1, femVecGet(&sigma,5));

					femVecSetZero(&F_0);
        	femMatVecMult(&HT, &sigma, &F_0);
					for (ii=1; ii<=12; ii++)
					  { femVecAdd(F_e, ii, mult*femVecGet(&F_0, ii)) ; }
					
					femVecSetZero(Fr_e);
				}
				else
				{
        	/* "D" computation should be usefull here ;-) */
        	if ((rv = fem_D_slab(ePos, thick, AF_NO, &D) ) != AF_OK) {goto memFree;}
				}

        femMatSetZero(&HTD) ;
        femMatSetZero(&K_i) ;

        /* 60,6 x 6,6 = 60,6  */
        femMatMatMult(&HT, &D, &HTD);

        /* 60,6 z 6,60  = 60,60  */
        femMatMatMult(&HTD, &H, &K_i);

        for (ii=1; ii<=12; ii++)
        {
          for (jj=1; jj<=12; jj++)
          {
            femMatAdd(K_e,ii,jj, mult*femMatGet(&K_i,ii,jj));
          }
        }
    }
  }


  if (Mode != AF_YES) /* Winkler's foundation */
  {
    winkler = femGetRSValPos(ePos, RS_WINKLER, 0) ;

    if (winkler > 0.0)
    {
      e005_shape_mat(ePos, winkler, K_e, AF_NO, 1) ;
    }
  }


memFree:
  femMatFree(&coord);
  femMatFree(&xyz);
  femMatFree(&jac);
  femMatFree(&ijac);

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


int e005_mass(long ePos, tMatrix *M_e)
{
  double ro, width ;

  femMatSetZero(M_e) ;

	ro    = femGetMPValPos(ePos, MAT_DENS, 0) ;
	width = femGetRSValPos(ePos, RS_HEIGHT, 0) ;

  return(e005_shape_mat(ePos, ro*width,  M_e, AF_YES,0)) ;
}


int e005_volume(long ePos, double *vol)
{
	int rv = AF_OK;
  double  width ;
	double e,f,fi ;
	double Li[5] ;
	long i, j ;
	tMatrix coord;

	femMatNull(&coord);
	if ((rv=femFullMatInit(&coord,4,3)) != AF_OK) { rv= AF_ERR_MEM; goto memFree; }
	e005_fill_coords(&coord);

	for (i=1; i<=4; i++)
	{
		j = i+1 ; 
		if (j > 4) { j = 1 ; }
		Li[i] = sqrt ( 
			pow( femMatGet(&coord,j,2) - femMatGet(&coord,i,2), 2)
			+
			pow( femMatGet(&coord,j,1) - femMatGet(&coord,i,1), 2)
		);
	}

	e = sqrt ( 
			pow( femMatGet(&coord,3,2) - femMatGet(&coord,1,2), 2)
			+
			pow( femMatGet(&coord,3,1) - femMatGet(&coord,1,1), 2)
		);

	f = sqrt ( 
			pow( femMatGet(&coord,4,2) - femMatGet(&coord,2,2), 2)
			+
			pow( femMatGet(&coord,4,1) - femMatGet(&coord,2,1), 2)
		);


	width = femGetRSValPos(ePos, RS_HEIGHT, 0) ;

	fi = acos (Li[1]*Li[1] + e*e -  Li[2]*Li[2]) / (2.0 * Li[1] * e)
	   + acos (Li[1]*Li[1] + f*f -  Li[4]*Li[4]) / (2.0 * Li[1] * f) ;

	*vol = 0.5 * e * f * sin(fi) ;

memFree:
  femMatFree(&coord);
	return(rv);
}



long e005_rvals(long ePos)
{
	return(5*2*2);
}

int e005_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

int e005_res_p_loc(long ePos, long point, double *x, double *y, double *z)
{
	double xx[5] ;
	double yy[5] ;
	double xi[5] = {0, -1,  1, 1, -1} ;
	double yi[5] = {0, -1, -1, 1,  1} ;
	double dx = 0.577350569189626 ;
	double dy = 0.577350569189626 ;
	long   i ;

	if (e005_rvals(ePos) != (5*2*2)) {return (AF_ERR_VAL);}

	for (i=1; i<=4; i++)
	{
  	xx[i] = femGetNCoordPosX(femGetENodePos(ePos,0));
  	yy[i] = femGetNCoordPosY(femGetENodePos(ePos,0));
	}

	*x = 0.0 ;
	*y = 0.0 ;
	*z = 0.0 ;

	for (i=1; i<=4; i++)
	{
		*x += xx[i]*0.25*(1.0 + dx*xi[point] * xi[i] +  dy*yi[point] * yi[i] ) ;
		*y += yy[i]*0.25*(1.0 + dx*xi[point] * xi[i] +  dy*yi[point] * yi[i] ) ;
	}

	return(AF_OK);
}

int addElem_005(void)
{
	int rv = AF_OK;
	static long type     = 5 ;
	static long dim      = 4 ; /* slab */
	static long nodes    = 4 ;
	static long dofs     = 3 ;
	static long ndof[3]  = {U_Z,ROT_Y,ROT_X} ;
	static long rs       = 2 ;
	static long real[2]  = {RS_HEIGHT,RS_WINKLER} ;
	static long rs_rp    = 0 ;
	static long *real_rp = NULL ;
	static long res      = 0 ;
	static long *nres    = NULL ;
	static long res_rp   = 5 ;
	static long nres_rp[5] = { RES_S_MX, RES_S_MY, RES_S_MXY, RES_S_VXZ, RES_S_VYZ} ;

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

	Elem[type].stiff = e005_stiff;
	Elem[type].mass  = e005_mass;
	Elem[type].rvals = e005_rvals;
	Elem[type].eload = e005_eload;
	Elem[type].res_p_loc = e005_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e005_volume;
	Elem[type].therm = e000_therm;

	return(rv);
}

#endif
/* end of fem_e005.c */

/*
   File name: fem_e020.c
   Date:      2011/07/08 12:33
   Author:    Jiri Brozovsky

   Copyright (C) 2013 Jiri Brozovsky

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

	 FEM Solver - element 020 (isoparametric thermal plane) definition
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_

extern void mirrorK(tMatrix *Ke);
extern int e005_volume(long ePos, double *vol);
extern int e005_res_p_loc(long ePos, long point, double *x, double *y, double *z);
extern int e000_res_node(long ePos, long nPos, long type, double *val);

extern int e005_init_gauss(long ipoints, tMatrix *gauss);
extern void e005_fill_coords(tMatrix *coord);
extern int e005_jac_det(tMatrix *jac, double *detj);

/* nodes of slab */
long p_20_nodes[4]={1,2,3,4};

/* note: deriv = 0 => no derivation (value of function)!
 *       deriv = 1 => d/dx
 *       deriv = 2 => d/dy
 */
double e020_p_a(tMatrix *coord, int deriv, long point, double x, double y)
{
  double xv, yv ;

  xv = femMatGet(coord, point, 1) ;
  yv = femMatGet(coord, point, 2) ;

  switch (deriv)
  {
    case 0:
        return( 0.25 *(1.0+xv*x) *(1.0+yv*y));
        break ;
    case 1: /* d/dx) */ return(0.25 *xv *(1.0 + yv*y));
        break;
    case 2: /* d/dy) */ return(0.25 *yv *(1.0 + xv*x));
        break;
    default: return(0.0);
  }
}

/** Computes coordinates od d(p)/d(something) 
 * @param coord matrix of -1,0,1 brick coordinates
 * @param deriv derivation type (0=none,1=d/dx,2=d/dy,3=d/dz)
 * @param dir coordinate type (1=x,2=y)
 * @param x x of [-1,1]
 * @param y y of [-1,1]
 * @param xyz matrix of real brick coordinates (NULL if  you want only the derivatives)
 * @return coordinate/derivative value
 * */
double e020_deriv_p(tMatrix *coord, long deriv, long dir, 
    double x, double y, tMatrix *xyz)
{
  double val = 0.0 ;
  double val_i ;
  long   i ;

  val = 0.0 ;

  for (i=0; i<4; i++)
  {
    val_i = e020_p_a(coord, deriv, p_20_nodes[i], x, y) ;
    if (xyz != NULL) { val_i *= femMatGet(xyz,p_20_nodes[i], dir) ; }
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
double e020_deriv_p_one(tMatrix *coord,  long deriv, long number, double x, double y)
{
	return(e020_p_a(coord, deriv, number, x, y) );
}



int e020_fill_J(tMatrix *jac, tMatrix *coord,
    double x, double y, tMatrix *xyz)
{
  long i,j ;

	femMatSetZero(jac);

  for (i=1; i<=2; i++)
  {
    for (j=1; j<=2; j++)
    {
      femMatAdd(jac, i, j,
          e020_deriv_p(coord, i, j, x, y, xyz) );
    }
  }
  return(AF_OK);
}


int e020_stiff(long ePos, long Mode, tMatrix *K_e, tVector *Fe, tVector *Fre)
{
	int     rv = AF_OK;
  int     i, j ;
	double  thick, kxx ;
  long    ipoints = 2 ;
	long    nnode = 4 ;
	long    eT    = 20 ;
	long    mT    =  1 ;
  long    ipos ;
  double  x, y, weight_x, weight_y, detj, mult ;
	tVector u_e;
  tMatrix D ;
  tMatrix gauss ;
  tMatrix coord ;
  tMatrix xyz ;
  tMatrix jac ;

	eT = femGetETypePos(ePos); 
	mT = femGetEMPPos(ePos); 
	
  /* vectors and matrices: */
  femVecNull(&u_e);
	femMatNull(&D);
	femMatNull(&gauss);
	femMatNull(&coord);
	femMatNull(&xyz);
	femMatNull(&jac);

	if ((rv = femVecFullInit(&u_e, 3)) != AF_OK) {goto memFree;}

	if ((rv = femFullMatInit(&D, 2,2)) != AF_OK) {goto memFree;}
	if ((rv=femFullMatInit(&coord,4,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&xyz,4,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&gauss,ipoints,2)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&jac,2,2)) != AF_OK) { goto memFree; }

  /* gauss data */
  if ((rv=e005_init_gauss(ipoints, &gauss)) != AF_OK) { goto memFree; }

	thick = femGetRSValPos(ePos, RS_HEIGHT, 0) ;

  /* D matrix: */
  kxx = femGetMPValPos(ePos, MAT_KXX, 0) ;
  femMatPut(&D, 1,1, kxx );
  femMatPut(&D, 2,2, kxx );

  /* TODO code here! --------------------------------------- */
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
  			e020_fill_J(&jac, &coord, x, y, &xyz);

  			/* determinant  */
  			if ((rv=e005_jac_det(&jac, &detj))!= AF_OK) { goto memFree; }

        /* integration multiplier */
				mult = detj * weight_x * weight_y ;

  			if (femLUinverse(&jac) != AF_OK)
  			{
#ifdef RUN_VERBOSE
    			fprintf(msgout, "[E] %s!\n", _("Inversion of Jascobi matric failed"));
#endif
    			goto memFree;
  			}

#if 0
        if ((rv=e005_fill_H(&H, &jac, &coord, x,y)) != AF_OK) {goto memFree;}
        femMatTran(&H, &HT);
#endif



        /* TODO */
    }
  }
	
  /* TODO some code here ----------------------------------- */
memFree:
	femVecFree(&u_e);

	femMatFree(&D);
	femMatFree(&coord);
	femMatFree(&xyz);
	femMatFree(&jac);
	return(rv);
}

int e020_mass(long ePos, tMatrix *M_e) { return(AF_OK); }

long e020_rvals(long ePos) { return(4); }

int e020_eload(long ePos, long mode, tVector *F_e) { return(AF_OK); }


int addElem_020(void)
{
	int rv = AF_OK;
	static long type     = 20 ;
	static long dim      = 0 ;
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

	Elem[type].stiff = e020_stiff;
	Elem[type].mass  = e020_mass;
	Elem[type].rvals = e020_rvals;
	Elem[type].eload = e020_eload;
	Elem[type].res_p_loc = e005_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e005_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

#endif
/* end of fem_e002.c */

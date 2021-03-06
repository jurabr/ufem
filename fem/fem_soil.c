/*
   File name: fem_soil.c
   Date:      2011/12/12 17:54
   Author:    Jiri Brozovsky

   Copyright (C) 2011 Jiri Brozovsky

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.

   More advanced model for e005 foundation
*/

#include "fem_elem.h"

extern long p_5_nodes[];
extern int e005_init_gauss(long ipoints, tMatrix *gauss);
extern int e005_fill_J(tMatrix *jac, tMatrix *coord,
                       double x, double y, tMatrix *xyz);
extern int e005_jac_det(tMatrix *jac, double *detj);
extern int e005_fill_N(tMatrix *H, tMatrix *coord, double x, double y);

/* TODO: some code here: different number of integration points
 * than for the stiffness matrix
 * */
int fem_e005_soil(long ePos, 
    tMatrix *jac, 
    tMatrix *coord, 
    tMatrix *xyz,
    tMatrix *Me)
{
  int rv = AF_OK ;
  long    i,j, ii, jj ;
  long    ipos = 0 ;
  long    ipoints = 2 ; /* number of integration points */
  double  x,y ;
  double  detj;
  double  weight_x,weight_y, mult ;
  double  C1, C2x, C2y ;
  tMatrix gauss ;
  tMatrix H ;
  tMatrix HT ;
  tMatrix C ;
  tMatrix HTC ;
  tMatrix M_i ;

  /* cleaning of matrices: */
	femMatNull(&gauss);
	femMatNull(&H);
	femMatNull(&C);
	femMatNull(&HT);
	femMatNull(&HTC);
	femMatNull(&M_i);

  /* initialization of matrices: */
	if ((rv=femFullMatInit(&gauss,ipoints,2)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&H,3,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&C,3,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&HT,12,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&HTC,12,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&M_i,12,12)) != AF_OK) { goto memFree; }

  if ((rv=e005_init_gauss(ipoints, &gauss)) != AF_OK) { goto memFree; }

  ipos = -1 ;

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
  			e005_fill_J(jac, coord, x, y, xyz); /*?*/

  			/* determinant  */
  			if ((rv=e005_jac_det(jac, &detj))!= AF_OK) { goto memFree; }/*?*/
        if ((rv=e005_fill_N(&H, coord, x,y)) != AF_OK) {goto memFree;}/*?*/

        femMatTran(&H, &HT);

        femMatSetZero(&M_i) ;

        /* "C" matrix: */
	      C1  = femGetRSValPos(ePos, RS_WINKLER, 0) ;
        C2x = 0.0 ;
        C2y = 0.0 ;
        femMatSetZero(&C);
        femMatPut(&C, 1,1, C1);
        femMatPut(&C, 2,2, C2x);
        femMatPut(&C, 3,3, C2y);

        femMatMatMult(&HT, &C, &HTC); 
        femMatMatMult(&HTC, &H, &M_i); 

				mult = detj * weight_x * weight_y ;

        /*  add "mult*M_i" to M_e: */
				for (ii=1; ii<=12; ii++)
        {
          for (jj=1; jj<=12; jj++)
          {
            femMatAdd(Me,ii,jj, mult*femMatGet(&M_i,ii,jj));
          }
        }
    }
  }

memFree:
  femMatFree(&H);
  femMatFree(&C);
  femMatFree(&HT);
  femMatFree(&HTC);
	femMatFree(&M_i);
  femMatFree(&gauss);
  return(rv);
}

/* end of fem_soil.c */

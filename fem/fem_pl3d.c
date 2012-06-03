/*
   File name: fem_pl3d.c
   Date:      2004/01/03 17:48
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

   FEM Solver - 3D elasticity and plastitity

   $Id: fem_pl3d.c,v 1.6 2004/11/11 21:39:25 jirka Exp $
*/

#include "fem_pl3d.h"


/** Material stiffness matrix (linear isotropic mat.) for 3D */ 
int femD_3D_iso(long ePos, double Ex, double nu, tMatrix *D)
{
	double d1,d2,d3;

	d1 = (Ex  * (1.0 - nu) ) /( (1.0 + nu)*(1.0 - 2.0*nu) ) ;
	d2 = (Ex * nu ) / ( (1.0 + nu)*(1.0 - 2.0*nu) ) ;
	d3 = Ex / (2.0*(1+nu)) ;

	femMatSetZero(D);

	femMatPut(D, 1,1, d1) ;
	femMatPut(D, 2,2, d1) ;
	femMatPut(D, 3,3, d1) ;

	femMatPut(D, 1,2, d2) ;
	femMatPut(D, 1,3, d2) ;
	femMatPut(D, 2,1, d2) ;
	femMatPut(D, 3,1, d2) ;
	femMatPut(D, 2,3, d2) ;
	femMatPut(D, 3,2, d2) ;

	femMatPut(D, 4,4, d3) ;
	femMatPut(D, 5,5, d3) ;
	femMatPut(D, 6,6, d3) ;

	return(0);
}

int femD_3D_ortho(long ePos, long e_rep, long Mode,  tMatrix *D)
{
	double Ex,Ey,Ez,Gxy,Gyz,Gzx,nuxy,nuyz,nuzx, mult ;
	double nuyx, nuzy, nuxz ;

  Ex   = femGetMPValPos(ePos, MAT_EX,   0) ;
  Ey   = femGetMPValPos(ePos, MAT_EY,   0) ;
  Ez   = femGetMPValPos(ePos, MAT_EZ,   0) ;
  nuxy = femGetMPValPos(ePos, MAT_NUXY, 0) ;
  nuyz = femGetMPValPos(ePos, MAT_NUYZ, 0) ;
  nuzx = femGetMPValPos(ePos, MAT_NUZX, 0) ;
  Gxy  = femGetMPValPos(ePos, MAT_GXY,  0) ;
  Gyz  = femGetMPValPos(ePos, MAT_GYZ,  0) ;
  Gzx  = femGetMPValPos(ePos, MAT_GZX,  0) ;

	nuyx = (Ey/Ex) * nuxy ;
	nuzy = (Ez/Ey) * nuyz ;
	nuxz = (Ex/Ez) * nuzx ;

	mult =( 1.0 - (nuxy*nuyx) - (nuyz*nuzy) - (nuzx*nuxz) - (2.0*nuxy*nuyz*nuzx)) / (Ex*Ey*Ez) ;

	femMatPut(D, 1,1, (1.0 - nuyz*nuzx )/(Ey*Ez*mult) ) ;
	femMatPut(D, 2,2, (1.0 - nuxz*nuzx )/(Ex*Ez*mult) ) ;
	femMatPut(D, 3,3, (1.0 - nuxy*nuyx )/(Ey*Ex*mult) ) ;

	femMatPut(D, 4,4, 1.0*Gyz) ;
	femMatPut(D, 5,5, 1.0*Gzx) ;
	femMatPut(D, 6,6, 1.0*Gxy) ;

	femMatPut(D, 1,2, (nuxy +nuzx*nuyz)/(Ey*Ez*mult)) ;
	femMatPut(D, 2,1, (nuxy +nuzx*nuyz)/(Ey*Ez*mult)) ;

	femMatPut(D, 1,3, (nuzx +nuyx*nuzy)/(Ey*Ez*mult)) ;
	femMatPut(D, 3,1, (nuzx +nuyx*nuzy)/(Ey*Ez*mult)) ;

	femMatPut(D, 2,3, (nuzy +nuzx*nuxy)/(Ez*Ex*mult)) ;
	femMatPut(D, 3,2, (nuzy +nuzx*nuxy)/(Ez*Ex*mult)) ;

	return( AF_OK ) ;
}


/** Creates material stiffness matrix
 * @param ePos element position
 * @param e_rep index of (integration) point on element
 * @param eT element type
 * @param eT material type
 * @param sigma stress vector (NULL for Mode == AF_NO)
 * @param sigma strain vector (NULL for Mode == AF_NO)
 * @param Mode as AF_YES==D(sigma, epsilon) .. as usual
 * @param D matrix to be computed
 * */
int fem_D_3D(long ePos, 
             long e_rep, 
             long eT, 
             long mT, 
             tVector *sigma, 
             tVector *epsilon, 
             long Mode, 
             tMatrix *D)
{
  int rv = AF_OK ;
  long type = 0 ;

  type = Mat[femGetMPTypePos(mT)].type ;

  switch (type)
  {
    case 7: rv = femD_3D_ortho(ePos, e_rep, Mode, D);
            break ;
    case 4: 
            if (femGetMPValPos(ePos, MAT_F_YC, 0) <= 0.0)
            {
              rv = fem_dp_D_3D(ePos, e_rep, eT, mT, sigma, epsilon, Mode, D);
            }
            else
            {
              rv = fem_vmis_D_3D(ePos, e_rep, eT, mT, sigma, epsilon, Mode, D);
            }
            break ;
    case 3: rv = fem_chen_D_3D(ePos, e_rep, eT, mT, sigma, epsilon, Mode, D);
            break ;
    case 2: rv = fem_D_ccr3D(ePos, e_rep, eT, mT, sigma, epsilon, Mode, D);
            break ;
    case 10: rv = od3d_D(ePos, e_rep, Mode, epsilon, D);
            break ;
    case 5: /* the same as default - isotropic behaviour */
    case 1: /* the same as default - isotropic behaviour */
    default: 
            rv =  femD_3D_iso(
                       ePos, 
                       femGetMPValPos(ePos, MAT_EX, 0), 
                       femGetMPValPos(ePos, MAT_NU, 0), 
                       D) ;
            break;
  }

  return(rv);
}

/* end of fem_pl3d.c */

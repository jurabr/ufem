/*
   File name: fem_od3d.c
   Date:      2007/09/25 20:25
   Author:    Jiri Brozovsky

   Copyright (C) 2007 Jiri Brozovsky

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

   FEM solver: 3D material/elements with compression-only behaviour
               (requested by prof. Janas for ani-detonation barriers)

   Note: meant for use with elements 010 to simulate a concrete

*/

#include "fem_pl3d.h"

extern long femIterEchange ;

/** Computes D for linear 3D point "without tension"
 * @param ePos element position
 * @param iPoint integration point to be computed
 * @param Mode AF_NO (old matrix) or AF_YES (new matrix)
 * @param epsilon current TOTAL epsilon vector [6]
 * @param D matrix to be created
 * @return status
 */
int od3d_D(long ePos, long iPoint, long Mode, tVector *epsilon, tMatrix *D)
{
  int rv = AF_OK ;
  double E, E1, nu ;
  double max = FEM_ZERO ; /* tension is not allowed right now */
  double res = FEM_ZERO ; /* residual stiffness */
  long   stat ;
  tVector sigma_x ;
  tVector sigma_1 ;

  E  = femGetMPValPos(ePos, MAT_EX, 0) ;
  nu = femGetMPValPos(ePos, MAT_NU, 0) ;

  stat = (long)femGetEResVal(ePos, RES_PSI, iPoint) ;

  if (stat == 0)
  {
    /* undamaged material */
    stat = 1 ;
    E1 = E ;
  }
  else
  {
    /* damaged material */
    E1 = femGetEResVal(ePos, RES_CR1, iPoint) ;
  }

  if (Mode == AF_YES)
  {
    /* NEW matrix */
  	max  = femGetMPValPos(ePos, MAT_STEN, 0) ;
  	res  = femGetMPValPos(ePos, MAT_E1,   0) ;

		if (max < FEM_ZERO) {max = FEM_ZERO;}
		if (res < 1e1) {res = 1e1;}

    	femD_3D_iso( ePos, E1, nu, D) ;


	  	femVecNull(&sigma_x);
	  	femVecNull(&sigma_1);
			if ((rv=femVecFullInit(&sigma_x, 6)) != AF_OK) { goto memFree; }
			if ((rv=femVecFullInit(&sigma_1, 6)) != AF_OK) { goto memFree; }

    	femMatVecMult(D, epsilon, &sigma_x);
    	femPrinc3D(&sigma_x, &sigma_1);

        /* damage test */
    	  if (femVecGet(&sigma_1,1) >= max)
    	  {
          if (stat != -1) 
          {
            femIterEchange++;
          }
      	  stat = -1 ;
          E1 = res ; 
    	  }
        else
        {
          if (stat != 1) 
          {
            /*femIterEchange++;*/
            E1 = res ;
            stat = -1 ;
          }
          else
          {
            E1 = E ;
            stat = 1 ;
          }
        }

    	  if ((femVecGet(&sigma_1,1) < 0.0) && (stat == -1))
        {
          E1 = 0.8 * E ; /* some help for compression */
        }


memFree:
    		femVecFree(&sigma_x);
    		femVecFree(&sigma_1);

    femPutEResVal(ePos, RES_CR1, iPoint, E1) ;
    femPutEResVal(ePos, RES_PSI, iPoint, (double)stat) ;
#if 0
printf("stat [%li,%li] = %e E = %f\n",ePos, iPoint, (double)stat, E1);
#endif
  }
  else
  {
    /* old matrix */
    femD_3D_iso( ePos, E1, nu, D) ;
  }

  return(rv);
}

/* end of fem_od3d.c */

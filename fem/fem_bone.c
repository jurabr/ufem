/*
   File name: fem_bone.c
   Date:      2004/01/03 18:26
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

   FEM Solver - 3D plasticity based on Chen-Chen

   $Id: fem_bone.c,v 1.2 2005/01/16 22:59:32 jirka Exp $
*/

#include "fem_pl3d.h"

int simple_bone_test(tVector *sigma1, double f_uc0, double f_ut0)
{
  int status = AF_NO ;
	double f_uc, f_ut ;
	double s1,s2,s3 ;

	f_uc =  (-1.0)*fabs(f_uc0) ; 
	f_ut = fabs(f_ut0) ;

  s1 = femVecGet(sigma1, 1) ;
  s2 = femVecGet(sigma1, 2) ;
  s3 = femVecGet(sigma1, 3) ;

	if ((s1 < f_uc) || (s1 > f_ut)) {status = AF_YES;}
	if ((s2 < f_uc) || (s2 > f_ut)) {status = AF_YES;}
	if ((s3 < f_uc) || (s3 > f_ut)) {status = AF_YES;}

	return(status);
}

int fem_bone_D_3D(long ePos, 
                  long e_rep, 
                  long eT, 
                  long mT, 
                  tVector *sigma, 
                  tVector *epsilon, 
                  long Mode, 
                  tMatrix *Dep)
 {
  int rv = AF_OK ;
  long   state = 0 ;
  double Ex, nu, E1;
  double f_uc, f_ut ;
  tVector old_sigma ;
  tVector sigma_1 ;
  long i ;

  femVecNull(&old_sigma) ;
  femVecNull(&sigma_1) ;

  if ((rv=femVecFullInit(&old_sigma, 6)) != AF_OK){goto memFree;}
  if ((rv=femVecFullInit(&sigma_1, 6)) != AF_OK){goto memFree;}

  Ex = femGetMPValPos(ePos, MAT_EX, 0)  ;
  E1 = femGetMPValPos(ePos, MAT_E1, 0)  ;
  nu = femGetMPValPos(ePos, MAT_NU, 0)  ;

  if (Mode != AF_YES)
  {
    /* old  */
	  state = (long) femGetEResVal(ePos, RES_CR1, e_rep);

    if (state == 0.0)
    {
      /* elastic */
      femD_3D_iso(ePos, Ex, nu, Dep);
    }
    else
    {
      /* inelastic */
      femD_3D_iso(ePos, E1, nu, Dep);
    }
  }
  else
  {
    /* new */
    f_uc  = femGetMPValPos(ePos, MAT_F_UC, 0)  ;
    f_ut  = femGetMPValPos(ePos, MAT_F_UT, 0)  ;

  	femVecPut(&old_sigma,1, femGetEResVal(ePos,RES_SX,e_rep));
  	femVecPut(&old_sigma,2, femGetEResVal(ePos,RES_SY,e_rep));
  	femVecPut(&old_sigma,3, femGetEResVal(ePos,RES_SZ,e_rep));
  	femVecPut(&old_sigma,4, femGetEResVal(ePos,RES_SYZ,e_rep));
  	femVecPut(&old_sigma,5, femGetEResVal(ePos,RES_SZX,e_rep));
  	femVecPut(&old_sigma,6, femGetEResVal(ePos,RES_SXY,e_rep));

    for (i=1; i<=6; i++)
    {
      femVecAdd(&old_sigma,i, femVecGet(sigma, i)) ;
    }

		rv = femPrinc3D(&old_sigma, &sigma_1) ;


		if (simple_bone_test(&sigma_1, f_uc, f_ut) == AF_YES)
		{
      /* inelastic */
      femD_3D_iso(ePos, E1, nu, Dep);
      state = 1 ;
    }
    else
    {
      /* elastic */
      femD_3D_iso(ePos, Ex, nu, Dep);
      state = 0 ;
    }

	   femPutEResVal(ePos, RES_CR1, e_rep, state);
   }

memFree:
  femVecFree(&old_sigma) ;
  return(rv) ;
}

/* end of fem_bone.c */

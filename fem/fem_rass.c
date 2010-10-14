/*
   File name: fem_rass.c
   Date:      2004/02/25 19:26
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

   FEM Solver - "Reliability assessment"

   $Id: fem_rass.c,v 1.3 2004/02/29 20:11:55 jirka Exp $
*/

#include "fem_rand.h"

extern int femGetPrincStress2D(tVector *sigma_x, tVector *sigma_1, double *phi);


/* 2D link and linear material */
int rand_fail_crit_e6_m5(long ePos)
{
	int    rv = AF_NO ;
  double s_x, f_x, f_y_c, f_y_t, Iy, Iz, I, Ax, Ex  ;
  double x1, y1, x2, y2, dx, dy, L ;
  double mult = 1.0 ;

  s_x = femGetEResVal(ePos, RES_SX, 0) ;

  if (s_x <= 0)
  {
     f_y_c = femGetMPValPos(ePos, MAT_F_YC, 0) ;

     if (fabs(s_x) >= fabs(f_y_c))
     {
       return(AF_YES);
     }
  }
  else
  {
    f_x = femGetEResVal(ePos, RES_FX, 0) ;

	  Ex = femGetMPValPos(ePos, MAT_EX, 0) ;

    f_y_t = femGetMPValPos(ePos, MAT_F_YT, 0) ;

	  Ax = femGetRSValPos(ePos, RS_AREA, 0) ;
	  Iy = femGetRSValPos(ePos, RS_INERTIA_Y, 0) ;
	  Iz = femGetRSValPos(ePos, RS_INERTIA_Z, 0) ;

    if (Iy > Iz) 
    {
      I = Iz ;
    }
    else
    {
      I = Iy ;
    }

    x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
    y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
    x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
    y2 = femGetNCoordPosY(femGetENodePos(ePos,1));

	  dx = x2 - x1;
	  dy = y2 - y1;
	  L  = sqrt((dy*dy) + (dx*dx));

    if (f_x > ((Ex*I*FEM_PI*FEM_PI)/((mult*L)*(mult*L))) )
    {
      return(AF_YES) ;
    }
  }

  return(rv);
}

/* 2D plane and qausibrittle concrete */
int rand_fail_crit_e2_m2(long ePos)
{
	int    rv = AF_NO ;
	int    i ;
	double phi, f_uc, f_ut ;
	double s_1, s_2, s0 ;
	double alpha, crit ;
	tVector sigma_x;
	tVector sigma_1;

	femVecNull(&sigma_x);
	femVecNull(&sigma_1);

	if ((femVecFullInit(&sigma_x, 3)) != AF_OK) {goto memFree;}
	if ((femVecFullInit(&sigma_1, 3)) != AF_OK) {goto memFree;}

	f_ut = femGetMPValPos(ePos, MAT_STEN,  0) ;
	f_uc = femGetMPValPos(ePos, MAT_SCMPR, 0) ;

	for (i=0; i<4; i++)
	{
		femVecPut(&sigma_x,1, femGetEResVal(ePos, RES_SX, i+1));
		femVecPut(&sigma_x,2, femGetEResVal(ePos, RES_SY, i+1));
		femVecPut(&sigma_x,3, femGetEResVal(ePos, RES_SXY,i+1));

		femGetPrincStress2D(&sigma_x, &sigma_1, &phi);
		s_1 = femVecGet(&sigma_1, 1) ;
		s_2 = femVecGet(&sigma_1, 2) ;

		if (s_1 < s_2)
		{
			s0  = s_1 ;
			s_1 = s_2 ;
			s_2 = s0 ;
		}

		if (s_2 < (-0.96*fabs(f_uc)))
		{
			alpha = s_1 / s_2 ;
			crit = ((-1.0) * ( (1.0 + 3.8*alpha) * f_uc )) / ((1.0 + alpha)*(1.0 + alpha)) ;
		
			if (s_2 <= crit) { rv = AF_YES ;}
		}

		if (s_2 < 0.0)
		{
			crit = ( 1.0 + 0.8*(s_2/f_uc) ) * f_ut ;

			if (s_1 >= crit) { rv = AF_YES ; }
		}
		else /* s2 > 0 */
		{
			if (s_1 >= f_ut) { rv = AF_YES ; }
		}
	}

memFree:
	femVecFree(&sigma_x);
	femVecFree(&sigma_1);
  return(rv);
}

/** Tests if element "fails"
 * @param ePos element position
 * @return AF_YES if failed, AF_NO if is O.K. !!
 * */
int fem_rand_test_fail(void)
{
  int status = AF_NO ;
  long eT, mT ;
  long i ;

  for (i=0; i<eLen; i++)
  {

    eT = femGetETypePos(i) ;
    mT = femGetSystemMatType(i) ;

    switch (eT)
    {
      case 2:
        switch (mT)
        {
          case 2: status = rand_fail_crit_e2_m2(i); break ;
        }
        break ;
      case 6:
        switch (mT)
        {
          case 5: status = rand_fail_crit_e6_m5(i); break ;
        }
        break ;

    }

    if (status == AF_YES) {return(status);}
  }

  return(status);
}


/* end of fem_rass.c */

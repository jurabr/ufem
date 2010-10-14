/*
   File name: fem_pr3d.c
   Date:      2004/03/27 10:08
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

   FEM Solver: computaion of principal stresses in 3D
*/

#include "fem.h"
#include "fem_math.h"


void femSort3dMaxMin(double *s1, double *s2, double *s3)
{
  double val = 0.0 ;
  double a[3];
  int posmax = 0 ;
  int posmid = 0;
  int posmin = 0;
  int i;

  if ( (*s1 == *s2) && (*s2 == *s3) ) {return;}

  a[0] = *s1 ;
  a[1] = *s2 ;
  a[2] = *s3 ;

  val = a[0] ;
  posmax = 0 ;
  for (i=1; i<3; i++) { if (a[i] >= val) {val = a[i]; posmax = i;} }

  val = a[0] ;
  posmin = 0 ;
  for (i=1; i<3; i++) { if (a[i] <= val) {val = a[i]; posmin = i;} }

  for (i=0; i<3; i++)
  {
    if ((i != posmax) && (i != posmin)) 
    {
      posmid = i ; 
      break ;
    }
  }

  *s1 = a[posmax] ;
  *s2 = a[posmid] ;
  *s3 = a[posmin] ;
}


/** Computes principal stresses
 * @param sx {s_x,s_y,s_z,s_yz,s_zx,s_xy}
 * @param s1 {s_1,s_2,s_3}
 * @return status
 */
int femPrinc3D(tVector *sx, tVector *s1)
{
  double a,b,c ; /* s^3 - a*s^2 - b*s - c = 0 */
  double s_x,s_y,s_z,s_xy,s_yz,s_zx ;
  double s_1,s_2,s_3 ;
  double Q, R, Q3, R2, sQ,a3,  ang ;

	femVecSetZero(s1);

  s_x  = femVecGet(sx, 1) ;
  s_y  = femVecGet(sx, 2) ;
  s_z  = femVecGet(sx, 3) ;
  s_yz = femVecGet(sx, 4) ;
  s_zx = femVecGet(sx, 5) ;
  s_xy = femVecGet(sx, 6) ;

	femVecPrn(sx,"SIGMA_X");

	if (fabs(femVecNorm(sx)) <= FEM_ZERO) {return(AF_OK);}

  /* cubic equation coefficicents */
  a = (-1.0)*(s_x + s_y + s_z) ;
  b = s_x*s_y + s_y*s_z + s_z*s_x - s_xy*s_xy - s_yz*s_yz - s_zx*s_zx ;
  c = (-1.0)*(s_x*s_y*s_z+2*s_xy*s_yz*s_zx - (s_zx*s_y*s_zx+s_xy*s_xy*s_z+s_x*s_yz*s_yz)) ;

  /* root finding by Francois Viete: De emendatione, 1615 */
  Q = (a*a - 3.0*b) / 9.0 ;
  R = (2.0*a*a*a - 9.0*a*b + 27.0*c) / 54.0 ;

  Q3 = Q*Q*Q ;
	R2 = R*R ;

  if ((R2) <= (Q3))
  {
    ang = acos(R / sqrt(Q3)) ;

    sQ = sqrt(Q) ;
    a3 = a/3.0 ;

    s_1 = -2.0*sQ*cos(ang/3.0)                - a3 ;
    s_2 = -2.0*sQ*cos((ang+(2.0*FEM_PI))/3.0) - a3 ;
    s_3 = -2.0*sQ*cos((ang-(2.0*FEM_PI))/3.0) - a3 ;
  }
  else
  {
		/* If only one root is real - impossible here!!!!! */
		if ((R2) > Q3)
		{
			s_1 = -1.0 * 
					( 
						pow( (sqrt(R*R-Q3)+fabs(R)),(1.0/3.0) ) +
						Q / pow( (sqrt(R*R-Q3)+fabs(R)),(1.0/3.0) )
					);

			if (R >= 0) { /* do nothing */ }
			else { s_1 *= (-1.0); }

			s_2 = s_1 ;
			s_3 = s_1 ;

#ifdef RUN_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Impossible stresses found - principal stresses will be invalid"));
#endif
		}
		else
		{
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s: Q3=%e, R2=%e!\n",_("Complex principal values found"),Q3,R*R);
#endif
      femVecSetZero(s1);
      return(AF_ERR_VAL);
		}
  }

  /* s_1 > s_2 > s_3 */
  femSort3dMaxMin(&s_1, &s_2, &s_3) ;

  femVecPut(s1, 1, s_1) ;
  femVecPut(s1, 2, s_2) ;
  femVecPut(s1, 3, s_3) ;

  femVecPut(s1, 4, 0) ;
  femVecPut(s1, 5, 0) ;
  femVecPut(s1, 6, 0) ;

  
	femVecPrn(s1,"SIGMA_1");
  return(AF_OK);
}

/* Computes M for "M_sigma_princ = M^T*M_sigma_xyz*M"
 * @param sx xyz stresses
 * @param s1 principal stresses (must be available)
 * @param M transformation matrix (result)
 * @return status
 * */
int femPrinc3dTranM(tVector *sx, tVector *s1, tMatrix *M)
{
  double A,B,C ;
  double Mj[4] ;
  double s_x,s_y,s_z,s_xy,s_yz,s_zx ;
  double s, mult, ABC ;
  int    i,j ;

  s_x  = femVecGet(sx, 1) ;
  s_y  = femVecGet(sx, 2) ;
  s_z  = femVecGet(sx, 3) ;
  s_yz = femVecGet(sx, 4) ;
  s_zx = femVecGet(sx, 5) ;
  s_xy = femVecGet(sx, 6) ;

  for (i=1; i<=3; i++)
  {
    for (j=0; j<4; j++) { Mj[j] = 0.0 ; }

    s = femVecGet(s1, i) ;

    A = (s_y - s)*(s_z - s) - s_yz*s_yz ;
    B = (s_z - s)*(s_x - s) - s_zx*s_zx ;
    C = (s_x - s)*(s_y - s) - s_xy*s_xy ;

    ABC = A + B + C ;
  
    if (ABC == 0.0)
    {
      femMatSetZero(M) ;
      for (j=1; j<=3; j++) { femMatPut(M, j,j, 1.0); }
      return(AF_OK) ;
    }

    mult = 1.0 / ABC ;

    Mj[1] = mult * A ;
    Mj[2] = mult * B ;
    Mj[3] = mult * C ;

    for (j=1; j<=3; j++) { femMatPut(M,j,i, Mj[j]) ; } /* ? */
  }

  return(AF_OK) ;
}

/* end of fem_pr3d.c */

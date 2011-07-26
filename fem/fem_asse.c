/*
   File name: fem_asse.c
   Date:      2011/01/31 18:27
   Author:    Jiri Brozovsky

   Copyright (C) 2011 Jiri Brozovsky

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

   General assessment routines (mainly for libfem.so)
*/

#include "fem.h"
#include "fem_data.h"
#include "fem_math.h"
#include "fem_sol.h"

extern double stress2D_J2(tVector *stress) ; /* from fem_ch2d */
extern double stress3D_J2(tVector *stress) ; /* from fem_chen */

/** ### MATERIAL POINT LEVEL ####  */

/** Max. displacement test
 * @param disp_max maximum allowed displacement (x,y, or z)
 * @return 0 if passed, non-zero number (usually 1) if failed 
 */
long fem_asse_max_disp_simple(double disp_max)
{
  if (femVecMaxAbs(&u) >= fabs(disp_max)) {return(1);} /* failed */

  return(0);
}

/** Material point failure test - material 4: von Mises plasticity condition 
 * @param sigma stress vector
 * @param epsilon strain vector
 * @param mType type of material
 * @return 0 if passed, non-zero number (usually 1) if failed 
 */
long fem_asse_mat_vmis(tVector *sigma, tVector *epsilon, long ePos)
{
  double f_y, s_vmis ;

  f_y = femGetMPValPos(ePos, MAT_F_YC, 0) ;
  
  if (sigma->len > 3)
  {
    /* 3D case */
    s_vmis = sqrt( stress3D_J2(sigma) * 3.0 )  ;
  }
  else
  {
    /* 2D case */
    s_vmis = sqrt( stress2D_J2(sigma) * 3.0 ) ;
  }

  if ( s_vmis < fabs(f_y) ) { return(0); } /* passed */
  else                      { return(1); } /* failed */
  return(0);
}

/** Material point failure test wrapper
 * @param sigma stress vector
 * @param epsilon strain vector
 * @param mType type of material
 * @param ePos position of studied element
 * @return 0 if passed, non-zero number (usually 1) if failed 
 */
long fem_asse_mat_by_type(tVector *sigma, tVector *epsilon, long ePos)
{
  long mType ;

  mType = Mat[femGetMPTypePos(femGetEMPPos(ePos))].type ;

  switch (mType)
  {
    case 3: /* Chen-Chen concrete */
      /** TODO some code here */
      break;
    case 4: /* von Mises steel */
      return ( fem_asse_mat_vmis(sigma, epsilon, ePos) );
      break;

    default:
      return(0); /* nothing to test */
      break;
  }

  return(0);
}


/** Checks failure condition in dependence of material and element type
 * @return 0 if not failed, 1 if failed
 */
long fem_asse_fail_cond(void)
{
  long i, j, eT;
  
  for (i=0; i<eLen; i++)
  {
    eT = femGetETypePos(i)

      switch (eT)
      {
        case 1:
          break ;
        case 2:
          break ;
        case 3:
          break ;
        case 4:
          break ;
        /* TODO - more code here */
        default: break ;
      }

  }

  return (0);
}

/* end of fem_asse.c */

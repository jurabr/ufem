/*
   File name: fem_lb2d.c
   Date:      2006/08/09 06:52
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

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

   Stiffness (E*I) computatin for 2D  beams

*/

#include "fem_elem.h"

/* TODO: this file is unused */

/** computes position of centre of gravity */
double lb2_compute_ty(long ePos)
{
  double y = 0;
  double hi, h, A, Ai, S, yi ;
  long   i, n ;

  h  = 0 ;
  yi = 0 ;
  A  = 0 ;
  S  = 0 ;

  n = femRSNumPos(femGetERSPos(ePos)) ;

  for (i=1; i<n; i++) /* first dataset is not a layer! */
  {
    hi = femGetRSValPos(ePos, RS_WIDTH, i) ; 
    Ai = femGetRSValPos(ePos, RS_AREA, i) ; 

    h += hi ;
    A  += Ai ;
    yi += hi/2.0 ;
    S  += Ai*yi ;

    yi += hi/2.0 ;
  }

  if (A > 0.0)
  {
    y = S / A ;
  }
  else /* if there is a problem with A */
  {
    y = h / 2.0 ;
  }

  return(y);
} 

/** computes static moment of a layer
 @param yt centre of gravity of whole cross section
 @param y centre of gravity of layer
 @param A area
 @return static moment
 */
double lb2_statmom(double yt, double y, double  A)
{
  return ( (yt - y) * A ) ;
}

/** computes moment of inertia of a layer
 @param yt centre of gravity of whole cross section
 @param y centre of gravity of layer
 @param b width
 @param h heigth
 @return static moment
 */
double lb2_mominertia(double yt, double y, double  b, double h)
{
  return ( pow( (yt - y), 2 ) * b * h + (1.0/12.0)*b*h*h*h ) ;
}

/** Computes normal stress
 * @param M bending moment
 * @param I moment of inertia
 * @param y position of centere of gravity
 * @param yi position of analysed point
 * @return sigma (stress)
 */
double lb2_compute_sigma(double M, double I, double y, double yi)
{
  if (I <= 0)
  {
    return ( 0 );
  }
  else
  {
    return ( (M*(yi-y)) / I );
  }
}

/** Computes shear stress
 * @param V tangential force
 * @param S static moment
 * @param I moment of inertia
 * @param b width
 * @return shear stress in given point
 */ 
double lb2_compute_tau(double V, double S, double I, double b)
{
  double Ib ;

  Ib = I*b ;
  
  if (Ib != 0.0)
  {
    return((V*S)/Ib);
  }
  else
  {
    return(0);
  }
}

/** Computes E of elasto-plastic material (von Mises)
 * Bi-linear behaviour only!
 *
 * */
double lb_Ex(
    long ePos,
    double E0, 
    double S,
    double I,
    double b,
    double yt,
    double y,
    long   mtype,
    double M0, 
    double V0
    )
{
  double Ex = 0.0 ;
  double M, V ;
  double sigma, tau, s1, s2 ;
  double fy, c, pho ;


  if (femTangentMatrix == AF_YES)
  {
	  fy  = femGetMPValPos(ePos, MAT_F_YC, mtype) ;
	  c   = femGetMPValPos(ePos, MAT_COHES, mtype) ;
	  pho = femGetMPValPos(ePos, MAT_FRICT, mtype) ;

    M = M0 + femGetEResVal(ePos, RES_MZ, 0) ;
    V = V0 + femGetEResVal(ePos, RES_FY, 0) ;

    sigma = lb2_compute_sigma(M, I, yt, y) ;
    tau = lb2_compute_tau(V, S, I, b) ;

    s1 = 0.5 * ( sigma + sqrt( sigma*sigma + 4.0*tau*tau ) ) ;
    s2 = 0.5 * ( sigma - sqrt( sigma*sigma + 4.0*tau*tau ) ) ;

    if (fy != 0) 
    {
      /* von Mises */
      if ( ( s1*s1 - s1*s2 + s2*s2 ) < (fy*fy) )
      {
        Ex = E0 ;
      }
      else
      {
        Ex = femGetMPValPos(ePos, MAT_HARD, mtype) ;
      }
    }
    else
    {
      /* Mohr-Coulomb */
      if (  s1 < (2.0*c*(cos(pho)/(1.0-sin(pho)))) )
      {
        Ex = E0 ;
      }
      else
      {
        Ex = femGetMPValPos(ePos, MAT_HARD, mtype) ;
      }
    }
  }
  else
  {
    M = M0 ;
    V = V0 ;
    Ex = E0 ;
  }

  return(Ex);
}

/** Computes stiffness (EI) of whole cross-section
 * @param ePos element index
 * @return EI size
 * */
double lb2_EI(long ePos, long Mode, double M, double V)
{
  double EI = 0 ;
  double Ex, E2, Ix, S ;
  double hi, bi, Ai, yi, y ;
  double dA, dh ;
  long   type ;
  long   i, j, nj, n, nm ;

  n  = femRSNumPos(femGetERSPos(ePos)) ;
  nm = femMPNumPos(femGetEMPPos(ePos)) ;

  yi = 0 ;
  EI = 0 ;

  y = lb2_compute_ty(ePos) ;
  
  for (i=1; i<n; i++) /* first dataset is not a layer! */
  {
    type = femGetRSValPos(ePos, RS_MTYPE, i) ; 

    if ((type >= nm)||(type < 0)) { type = 0 ; } /* on user error */

	  Ex = femGetMPValPos(ePos, MAT_EX, type) ;

    hi = femGetRSValPos(ePos, RS_WIDTH, i) ; 
    Ai = femGetRSValPos(ePos, RS_AREA, i) ; 
    
    nj = (long) femGetRSValPos(ePos, RS_LNUM, i) ; 

    if (nj > 0)
    {
      /* some sublayers are here.. */
      dh = hi / ((double)nj) ;
      dA = Ai / ((double)nj) ;
      bi = dA / dh ;

      for (j=0; j<nj; j++)
      {
        yi += dh/2.0 ;
        Ix = lb2_mominertia(yi, y, bi, dh) ;

        /* TODO computes stresses and set E here! */
        if (Mode == AF_YES)
        {
          S  = lb2_statmom(y, yi, Ai) ;
          E2 = lb_Ex( ePos, Ex, S, Ix, bi, y, yi, type, M, V);
        }
        else
        {
          E2 = Ex ;
        }

        EI += Ix * E2 ;

        yi += dh/2.0 ;
      }
    }
    else /* no sublayers */
    {
      bi = Ai / hi ;
      yi += hi/2.0 ;

      Ix = lb2_mominertia(yi, y, bi, hi) ;

      /* TODO computes stresses and set E here! */
      if (Mode == AF_YES)
      {
        S  = lb2_statmom(y, yi, Ai) ;
        E2 = lb_Ex( ePos, Ex, S, Ix, bi, y, yi, type, M, V);
      }
      else
      {
        E2 = Ex ;
      }

      EI += Ix * Ex ;

      yi += hi/2.0 ;
    }
  }

  if (EI <= 0.0) /* if something is wrong... */
  {
    EI = 
	    femGetMPValPos(ePos, MAT_EX, 0)
      *
	    femGetRSValPos(ePos, RS_INERTIA_Y, 0) ;
      ;
  }

  return(EI);
}

/* end of fem_lb2d.c */

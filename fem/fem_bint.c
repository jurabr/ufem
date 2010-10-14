/*
   File name: fem_bint.c
   Date:      2005/07/16 13:40
   Author:    Jiri Brozovsky

   Copyright (C) 2005 Jiri Brozovsky

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

   FEM solver - computation of data along beam elements
*/

#include "fem_elem.h"

/* from fem_e003.c: */
extern int e003_local_stiff_matrix(long ePos, long Mode, double Lx, double Ex, double Ax, double Ix, double EI, double kl1, double kl2,  tMatrix *k_0);

/* Returnes dimensionalinty (2, 3) of beam problem
 * @param eType - element type (beam3, beam14)
 * @return number 2 or 3
 * */
long femBeamWhatDim(long eType)
{
  switch(eType)
  {
    case  3:
    case 13: return(2); break;
    case 14: return(3); break;
    default: return(2); break;
  }
}

/* Force load on beams */
int femBeamForceLoad( long    nlen,
                      double  L, 
                      double  x, 
                      double  size,
                      long    dim, /* 2 or 3 */
                      long    dir, /* U_X=1 etc. */
                      tVector *F)
{
  long i;
  double val0, val, mult1, mult2 ;

  for (i=0; i<=nlen; i++)
  {
    val0 = ((double)i/(double)nlen)*L ;

    if (i <nlen)
    {
      val  = ((double)(i+1)/(double)nlen)*L ;
    }
    else
    {
      val = val0 ;
    }

    if (val0 == x)
    {
      if (dim == 2) { femVecAdd(F, i* 6+dir, size); }
      else          { femVecAdd(F, i*12+dir, size); }
      break ;
    }
    else
    {
      if ( (val0 > x) && (x < val) )
      {
        mult1 = (val  - x)/(L/(double)nlen) ;
        mult2 = (x - val0)/(L/(double)nlen) ;

        if (dim == 2) 
        { 
          femVecAdd(F,  i*6+dir,   mult1*size); 
          femVecAdd(F,  i*6+3+dir, mult2*size); 
        }
        else          
        { 
          femVecAdd(F, i*12+dir,   size); 
          femVecAdd(F, i*12+6+dir, size); 
        }
        break ;
      }
    }
  }

  return(AF_OK);
}


/* Pressure load on beams */
int femBeamPressLoad( long    nlen,
                      double  L, 
                      double  xa, /* begin */
                      double  xb, /* end */
                      double  sizea,
                      double  sizeb,
                      long    dim, /* 2 or 3 */
                      long    dir, /* U_X=1 etc. */
                      tVector *F)
{
  long i;
  double a, b, pos, pos0, val0, val, mult1, mult2, h ;

  h = L/(nlen-1) ;

  b = xb - xa ;
  a = xa ;
  
  for (i=0; i<nlen; i++)
  {
    pos0 = ((double)i)*h ;
    pos = ((double)i+1)*h ;

    if ((xa <= pos0) && (pos <= xb))
    {
    }
    else
    {
    }

    /* TODO - rewrite it! */
#if 0
    val0 = ((double)i/(double)nlen)*L ;

    if (i <nlen)
    {
      val  = ((double)(i+1)/(double)nlen)*L ;
    }
    else
    {
      val = val0 ;
    }

    if (val0 == x)
    {
      if (dim == 2) { femVecAdd(F, i* 6+dir, size); }
      else          { femVecAdd(F, i*12+dir, size); }
      break ;
    }
    else
    {
      if ( (val0 > x) && (x < val) )
      {
        mult1 = (val  - x)/(L/(double)nlen) ;
        mult2 = (x - val0)/(L/(double)nlen) ;

        if (dim == 2) 
        { 
          femVecAdd(F,  i*6+dir,   mult1*size); 
          femVecAdd(F,  i*6+3+dir, mult2*size); 
        }
        else          
        { 
          femVecAdd(F, i*12+dir,   size); 
          femVecAdd(F, i*12+6+dir, size); 
        }
        break ;
      }
    }
#endif
    /* end of TODO */
  }

  return(AF_OK);
}

/** Computes primary forces of 2D beam
 * @param ndiv  number of divisions
 * @param dir  direction of results (1=y, 2=z)
 * @param ePos element position
 * @param hinge_a type of left hinge
 * @param hinge_b type of right hinge
 * @param A element area
 * @param E Young modullus
 * @param I moment of inertia
 * @param L element lenght
 * @param Fr forces [Fx1,Fy1,Mz1,Fx2,Fy2,Mz2] (results)
 * @return status
 */ 
int femBeamPrimaryForces( long     ndiv, 
                          long     dir,
                          long     ePos,
                          long     hinge_a,
                          long     hinge_b,
                          double   A, 
                          double   E, 
                          double   I,
                          double   L,
                          tVector *Fr
                          )
{
  int      rv = AF_OK ;
  long     i, j, k ;
  long     dofLen = 0 ;
  long     nlen = 0 ;
  long    *K_rows = NULL ;
  tMatrix  Ke ;
  tMatrix  K ;
  tVector  Fe ;
  tVector  F ;
  tVector  u ;

  nlen   = ndiv+1 ;   /* number of nodes */
  dofLen = 6*(nlen) ; /* number of DOFs  */
  
	femMatNull(&K);
	femMatNull(&Ke);
	femVecNull(&F);
	femVecNull(&Fe);
	femVecNull(&u);

	if ((K_rows=femIntAlloc(dofLen)) == NULL){goto memFree;}

	if ((rv = femSparMatInitDesc(&K, dofLen, dofLen, K_rows)) != AF_OK)
  { goto memFree; }

	if ((rv=femVecFullInit(&F,dofLen)) != AF_OK) { goto memFree; }
	if ((rv=femVecFullInit(&u,dofLen)) != AF_OK) { goto memFree; }

	if ((rv=femFullMatInit(&Ke,6,6)) != AF_OK) { goto memFree; }
	if ((rv=femVecFullInit(&Fe,6)) != AF_OK) { goto memFree; }

  /* TODO:
   *
   * Ke -> K  (easy, |--| only) [done]
   * Fe -> the element loads must be defined first (loop for element loads)
   * boundary conditions (depends on hinge types)
   * compute displacements
   * compute reactions
   */

  /* Ke computation */
  if ((rv=e003_local_stiff_matrix(ePos, 0, L, E, A, I, E*I, 1.0, 1.0,  &Ke))!=AF_OK){goto memFree;}

  /* Ke -> K */
  for (i=0; i<nlen; i++)
  {
    for (j=1; j<=6; j++)
    {
      for (k=1; k<=6; k++)
      {
        femMatAdd(&K, i+j, i+k, femMatGet(&Ke,i,j));
      }
    }
  }

  /* TODO loop for element loads:*/
  for (i=0; i<elLen; i++)
  {
    if (elElem[i] == ePos)
    {
      /* only stese loads are important for us */
      switch (elType[i])
      {
        case 1: /* individual force(s) */
                femBeamForceLoad(nlen,
                      L, 
                      elValL[elFrom[i]], 
                      elValL[elFrom[i+1]], 
                      femBeamWhatDim(eType[elElem[i]]),
                      elType[i], 
                      &F);
                break ;
        case 2: /* continuous load */
                /* TODO a simple function have to be here */
                break ;
      }

    }
  }

  /* TODO add displacements */
  /* o--- */
  femMatSetZeroRow(&K,1) ;
  femMatSetZeroCol(&K,1) ;
  femMatPut(&K,1,1, 1.0) ;

  femMatSetZeroRow(&K,2) ;
  femMatSetZeroCol(&K,2) ;
  femMatPut(&K,2,2, 1.0) ;

  /* ---o */
  femMatSetZeroRow(&K,dofLen-2) ;
  femMatSetZeroCol(&K,dofLen-2) ;
  femMatPut(&K,dofLen-2,dofLen-2, 1.0) ;

  femMatSetZeroRow(&K,dofLen-1) ;
  femMatSetZeroCol(&K,dofLen-1) ;
  femMatPut(&K,dofLen-1,dofLen-1, 1.0) ;


  if (hinge_a >= 1) /* |--- */
  {
    femMatSetZeroRow(&K,3) ;
    femMatSetZeroCol(&K,3) ;
    femMatPut(&K,3,3, 1.0) ;
  }

  /* TODO add displacements */
  if (hinge_b >= 1) /* ---| */
  {
    femMatSetZeroRow(&K,dofLen) ;
    femMatSetZeroCol(&K,dofLen) ;
    femMatPut(&K,dofLen,dofLen, 1.0) ;
  }


  /* TODO solve equations */
	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, dofLen)) != AF_OK) { goto memFree; }

memFree:
  femIntFree(K_rows);
  femMatFree(&K);
  femMatFree(&Ke);
  femVecFree(&F);
  femVecFree(&Fe);
  femVecFree(&u);

  return(rv);
}

/** Computes relults along beam
 * @param ndiv  number of divisions
 * @param dir  direction of results (1=y, 2=z)
 * @param ePos element position
 * @param hinge_a type of left hinge
 * @param hinge_b type of right hinge
 * @param A element area
 * @param E Young modullus
 * @param I moment of inertia
 * @param L element lenght
 * @param Fx forces  (results)
 * @param Fy forces  (results)
 * @param Mz forces  (results)
 * @param ux displacements  (results)
 * @param uy displacements  (results)
 * @param rotz displacements  (results)
 * @return status
 */ 
int femBeamInternResults( long     ndiv, 
                          long     dir,
                          long     ePos,
                          long     hinge_a,
                          long     hinge_b,
                          double   A, 
                          double   E, 
                          double   I,
                          double   L,
                          tVector *Fx,
                          tVector *Fy,
                          tVector *Mz,
                          tVector *ux,
                          tVector *uy,
                          tVector *rotz
                          )
{
  return(AF_OK);
}


/* end of fem_bint.c */

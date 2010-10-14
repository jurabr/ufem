/*
   File name: fem_scrv.c
   Date:      2005/03/02 21:02
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

   Stiffness (E*I) curve computation for beams

*/

#include "fem_elem.h"

long    len = 0 ;
long    lenx = 0 ;
long    leny = 0 ;
double *Mcrv = NULL ;
double *Ncrv = NULL ;
double *EIcrv = NULL ;

int fem_beam2d_ei_crv(int epos)
{
  long   i ;
  double Ntmp ;

  lenx = 1 ;
  leny = 1 ;

  len = femRSNumPos(femGetERSPos(epos)) ;

  if (len <= 1) {return(AF_ERR_EMP);}

  if ((Mcrv=femDblAlloc(len)) == NULL){goto memFree;}
  if ((Ncrv=femDblAlloc(len)) == NULL){goto memFree;}
  if ((EIcrv=femDblAlloc(len)) == NULL){goto memFree;}

  for (i=1; i<len; i++)
  {
    Mcrv[i-1]  = femGetRSValPos(epos, RS_M_CRV,  i ) ;
    Ncrv[i-1]  = femGetRSValPos(epos, RS_N_CRV,  i ) ;
    EIcrv[i-1] = femGetRSValPos(epos, RS_EI_CRV, i ) ;
  }

  Ntmp = Ncrv[0] ;

  for (i=1; i<len-1; i++)
  {
    if (Ncrv[i] != Ntmp)
    {
      leny++ ;
    }
  }

  lenx = (long)(len / leny) ;

  return(AF_OK) ;
memFree:
  femDblFree(Mcrv);
  femDblFree(Ncrv);
  femDblFree(EIcrv);
  return(AF_ERR_MEM);
}

double fem_beam2d_ei_val(long epos, double M, double N)
{
  long   i, j ;
  long   n1 = -1 ;
  long   n2 = -1 ;
  long   m1 = -1 ;
  long   m2 = -1 ;
  double val11 = 0 ;
  double val12 = 0 ;
  double val21 = 0 ;
  double val22 = 0 ;
  double nmed  = 0 ;
  double mmed  = 0 ;
  double val, tmp1, tmp2 ;

  /* first find M: */
  for (i=0; i<(lenx-1); i++)
  {
    if ( (M >= Mcrv[i]) && (M <= Mcrv[i+1]) )
    {
      m1 = i ;
      m2 = i+1 ;
      val11 = Mcrv[m1] ;
      val12 = Mcrv[m2] ;
      if (fabs(val12-val11) > FEM_ZERO)
      {
        mmed =  (M - val11) / (val12-val11);
      }
      else
      {
        mmed = 0.0 ;
      }
      break ;
    }
  }

  /* check limit values: */
  if (m1 == -1)
  {
    if (M < Mcrv[i])
    {
      m1   = 0 ;
      m2   = 0 ;
      mmed = 0.0 ;
    }
    else
    {
      m1   = (lenx-1) ;
      m2   = (lenx-1) ;
      mmed = 0.0 ;
    }
  }

  /* then find N: */
  for (j=0; j<(leny-1); j++)
  {
    i = j*lenx ;

    if ( (N >= Ncrv[i]) && (N <= Ncrv[i+1]) )
    {
      n1 = i ;
      n2 = i+leny+1 ;
      val21 = Mcrv[n1] ;
      val22 = Mcrv[n2] ;

      if (fabs(val12-val11) > FEM_ZERO)
      {
        nmed =  (M - val11)/ (val12-val11);
      }
      else
      {
        nmed = 0.0 ;
      }
      break ;
    }
  }

  /* check limit values: */
  if (n1 == -1)
  {
    if (N < Ncrv[i])
    {
      n1   = 0 ;
      n2   = 0 ;
      mmed = 0.0 ;
    }
    else
    {
      n1   = (leny-1) ;
      n2   = (leny-1) ;
      mmed = 0.0 ;
    }
  }

  /* find some data */

  val11 = EIcrv[m1*lenx + n1] ;
  val12 = EIcrv[m2*lenx + n1] ;
  val21 = EIcrv[m1*lenx + n2] ;
  val22 = EIcrv[m2*lenx + n2] ;

  tmp1 = val11 + mmed*(val12-val11) ;
  tmp2 = val21 + mmed*(val22-val21) ;

  if (fabs(tmp1 + nmed*(tmp2-tmp1)) > FEM_ZERO)
  {
    return (tmp1 + nmed*(tmp2-tmp1)) ;
  }
  else
  {
    /* if it fails: */
    return(femGetMPValPos(epos,MAT_EX,0)*femGetRSValPos(epos,RS_INERTIA_Y,0));
  }
}

/* end of fem_scrv.c */

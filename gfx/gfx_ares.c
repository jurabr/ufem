/*
   File name: gfx_ares.c
   Date:      2004/01/04 20:00
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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

   FEM Postprocessor: computation of approximated results

   $Id: gfx_ares.c,v 1.2 2004/01/06 23:03:18 jirka Exp $
*/

#include "fem_gfx.h"
#ifdef _USE_GFX_

double *pal_val = NULL ;
long    pal_val_len = 0 ;

int set_pam_grad(void)
{
  int i;

  if ((pal_val = (double *)malloc(sizeof(double)*(1+femPal.len))) == NULL)
  {
    pal_val = NULL ;
    pal_val_len = 0 ;
    return(AF_ERR_MEM) ;
  }

  pal_val_len = femPal.len ;

  for (i=0; i<=femPal.len; i++)
  {
		if (i <= femPal.len/2)
		{
			pal_val[i] = femPal.min - 2.0*((double)i / ((double)femPal.len))*femPal.min ;
		}
		else
		{
			pal_val[i] = 2.0*((double)(i - (femPal.len/2)) / ((double)femPal.len))*femPal.max  ;
		}
  }

  return(AF_OK);
}

long nu_val_len(double min, double max, long *lower, long *upper)
{
  long num = 1 ;
  int i;

  for (i=0; i<pal_val_len; i++)
  {
    if (min < pal_val[i]) 
    {
      *lower = i-1; 

      if (min == pal_val[i]) {*lower = i ;}

      if (*lower < 0) {*lower = 0; }
      break ;
    }
  }

  for (i=0; i<pal_val_len; i++)
  {
    if (max <= pal_val[i]) {*upper = i; break ;}
  }

  num = *upper - *lower + 1 ;

  return(num) ;
}

int gfxCompAproxPoly(long len, long *nodes, double *vals)
{
  int rv = AF_OK ;
  double max, min ;
  double x[256] ;
  double y[256] ;
  double z[256] ;
  long info[256];
  long lower = 0 ;
  long upper = 0 ;
  long pal_long = 0;
  long *nnodes = NULL ;
  int i,j;
  long points = 0 ;

  /** UNFINISHED !!!!!!!!!! */

  for (i=0; i<256; i++)
  {
    x[i]    = 0.0 ;
    y[i]    = 0.0 ;
    z[i]    = 0.0 ;
    info[i] = 0 ;
  }

  if ((nnodes=(long *)malloc((len+1)*sizeof(long))) == NULL)
  {
    return(AF_ERR_MEM) ;
  }
  for (i=0; i<len; i++)
  {
    nnodes[i] = nodes[i] ;
  }
  nnodes[len] = nodes[0] ;

  max = 0.0 ;
  min = 0.0 ;

  for (i=0; i< len; i++)
  {
    if (max < vals[i]) {max = vals[i] ;}
    if (min > vals[i]) {min = vals[i] ;}
  }

  pal_long = nu_val_len(min, max, &lower, &upper) ;

  for (i=0; i<pal_long; i++)
  {
    for (j=0; j<len; j++)
    {
      if ( (vals[j] == min) && (vals[j] == pal_val[lower+j]) )
      {
        x[points] = fdbInputGetInt(NODE, NODE_X, nnodes[j]) ;
        y[points] = fdbInputGetInt(NODE, NODE_Y, nnodes[j]) ;
        z[points] = fdbInputGetInt(NODE, NODE_Z, nnodes[j]) ;
        info[points] = 1 ;
        points++ ;
      }
      else
      {
        /* unfinished  */
      }
    }
  }

  return(rv);
}

#endif
/* end of gfx_ares.c */

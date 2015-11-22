/*
   File name: fem_ts.c
   Date:      2015/09/20 11:57
   Author:    Jiri Brozovsky

   Copyright (C) 2015 Jiri Brozovsky

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

   Rasterisation od 2D f.e. models for use with tensor scale.
   Element 002 must be used
*/

#include "fem.h"
#include "fem_data.h"
#include "fem_elem.h"

/* main division parameter to be controlled bya  program switch: */
static int ts_div_main ;

/* area limits in real coordiantes */
static double ts_area_x0 ;
static double ts_area_y0 ;
static double ts_area_x1 ;
static double ts_area_y1 ;

static double ts_x_size ;
static double ts_y_size ;

static double ts_dx ;
static double ts_dy ;

/* area limits in integer coordinates: */
static int ts_x_div ;
static int ts_y_div ;

static int ts_n_mat ;

/* area of 0-1-? values: */
int  **ts_data ;

void ts_get_area(void)
{
  long i, j ;
  double x1, y1, xmin,ymin, xmax, ymax;

  /* initial data: */
  xmin = femGetNCoordPosX(femGetENodePos(0,0));
  ymin = femGetNCoordPosY(femGetENodePos(0,0));
  xmax = xmin ; 
  ymax = ymin ; 

  /* find limits: */
  for (i=0; i<eLen; i++)
  {
    for (j=0; j<4; j++)
    {
      x1 = femGetNCoordPosX(femGetENodePos(i,j));
      y1 = femGetNCoordPosY(femGetENodePos(i,j));

      if (x1 > xmax) xmax = x1 ;
      if (y1 > ymax) ymax = y1 ;

      if (x1 < xmin) xmin = x1 ;
      if (y1 < ymin) ymin = y1 ;
    }
  }

   ts_area_x0 = xmin ;
   ts_area_y0 = ymin ;
   ts_area_x1 = xmax ;
   ts_area_y1 = ymax ;

   ts_x_size = fabs(xmax-xmin) ;
   ts_y_size = fabs(ymax-ymin) ;

   if (ts_div_main > 0) ts_x_div = ts_div_main ;
   else                 ts_x_div = 1 ;

   ts_y_div = (int)((ts_y_size/ts_x_size)*ts_x_div) ;
   if (ts_y_div < 1) ts_y_div = 1 ;

   ts_dx = ts_x_size / (double)ts_x_div ;
   ts_dy = ts_y_size / (double)ts_y_div ;

printf("DIV: %i| %ex%e !%ex%e (%ix%i)\n", ts_div_main,ts_x_size,ts_y_size, ts_dx,ts_dy,ts_x_div,ts_y_div);
}

/** Prepares 2D ts_data field */
int ts_prep_data(void)
{
  long i, j ;
  ts_data = NULL ;

  if ((ts_data = (int **)malloc(sizeof(int *)*ts_y_div))==NULL) 
  {
    ts_data = NULL ;
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("No memory for tensor scale data"));
#endif
    return(AF_ERR_MEM);
  }
  else
  {
    for (i=0; i<ts_x_div; i++)
    {
      if ((ts_data[i] = (int *)malloc(sizeof(int)*ts_x_div))==NULL) 
      {
        for (j=0; j<i; j++)
        {
          free(ts_data[j]) ; ts_data[j] = NULL ;
        }
        free(ts_data); ts_data = NULL ;
        return(AF_ERR_MEM);
      }
      else
      {
        for (j=0; j<ts_x_div; j++) ts_data[i][j] = 0 ;
      }
    }
  }

  return(AF_OK);
}

/** Prepares 1D ts_data field */
void ts_clean_data(void)
{
  long j ;

  for (j=0; j<ts_y_div; j++)
  {
    free(ts_data[j]) ; ts_data[j] = NULL ;
  }
  free(ts_data); ts_data = NULL ;
}

/** Tests if given point lies inside the triangle: */
int ts_tri_test(double px, double py, double *x, double *y)
{
  double s, t, Area ;

  Area = 0.5*(-y[1]*x[2] + y[0]*(-x[1] + x[2]) + x[0]*(y[1] - y[2]) + x[1]*y[2]);
  s = 1.0/(2.0*Area)*(y[0]*x[2] - x[0]*y[2] + (y[2] - y[0])*px + (x[0] - x[2])*py);
  t = 1.0/(2.0*Area)*(x[0]*y[1] - y[0]*x[1] + (y[0] - y[1])*px + (x[1] - x[0])*py);

printf("Area:%e s=%e t=%e\n",Area,s,t);

  if ((s>0.0) && (t>0.0) && ((1.0-s-t)>0.0)) {return(AF_YES);}

  return(AF_NO);
}

/** Provides a rasterisation of 2D f.e. model. There are some simplifications.
 */
int ts_raster(void)
{
  long i, j, k ;
  double x[4] ;
  double y[4] ;
  double x2[4] ;
  double y2[4] ;
  double xmax, xmin, ymax, ymin, px, py ;
  int    ix0, ix1, iy0, iy1 ;
  int mType = 0 ;

  ts_n_mat = 2 ;

  for (i=0; i<eLen; i++)
  {
    mType = femGetEMPPos(i) ;
    if (mType > (ts_n_mat-1)) ts_n_mat++ ;

    /* first triangle and limits: */
    for (j=0; j<4; j++)
    {
      x[j] = femGetNCoordPosX(femGetENodePos(i,j));
      y[j] = femGetNCoordPosY(femGetENodePos(i,j));
      if ( j==0 )
      {
        xmax=x[0]; xmin=xmax ;
        ymax=y[0]; ymin=ymax ;
      }
      else
      {
        if (x[j] > xmax)  xmax = x[j] ;
        if (y[j] > ymax)  ymax = y[j] ;
        if (x[j] < xmin)  xmin = x[j] ;
        if (y[j] < ymin)  ymin = y[j] ;
      }
    }


  /* second triangle */
  x2[0] = x[0] ; y2[0] = y[0] ;
  x2[1] = x[2] ; y2[1] = y[2] ;
  x2[2] = x[3] ; y2[2] = y[3] ;

/*for (j=0; j<4; j++) { printf("(%li) [%e, %e] | [%e, %e]\n",i+1,x[j],y[j],x2[j],y2[j]); }*/

  ix0 = (int)((xmin-ts_area_x0) / ts_dx ) ; 
  ix1 = (int)((xmax-ts_area_x0) / ts_dx ) ; 

  iy1 = (int)((ymax-ts_area_y0) / ts_dy ) ; 
  iy0 = (int)((ymin-ts_area_y0) / ts_dy ) ; 

/* printf("ZZ %i %i %i %i\n",ix0, ix1,  iy0, iy1); */

  for (j=0; j<((ix1-ix0)); j++)
  {
    for (k=0; k<((iy1-iy0)); k++)
    {
      px = ts_area_x0+(0.5+(double)k)*ts_dx ;
      py = ts_area_y1-(0.5+(double)j)*ts_dy ;
/* printf("Y[%li,%li] x=%e, y=%e\n",k,j, px, py); */

      if ((ts_tri_test(px,py,x,y)==AF_YES)||(ts_tri_test(px,py,x2,y2)==AF_YES))
      {
/* printf("X [%i,%i]\n",iy1-k,ix0+j); */
        ts_data[iy1-k][ix0+j] = mType ;
      }
    }
  }

  }

  return(AF_OK);
}

/** Write data to a given file
 */
int fem_ts_write(FILE *fw)
{
  long j,k ;

  fprintf(fw,"%e %e\n", ts_x_size,ts_y_size);
  fprintf(fw,"%i %i\n", ts_x_div,ts_y_div);
  fprintf(fw,"%i\n", ts_n_mat);
  fprintf(fw,"8 %e %e\n", ts_dx*10.0,ts_dx/4);

  for (j=0; j<ts_y_div; j++)
  {
    for (k=0; k<ts_x_div; k++)
    {
      fprintf(fw," %i", ts_data[j][k]);
    }
    fprintf(fw,"\n");
  }

  return(AF_OK);
}

/* Prepares and writes tensor scale data **/
int fem_ts_run(FILE *fw, long div)
{
  int rv = AF_OK ;

  ts_div_main = div ;

  ts_get_area();
  if ((rv = ts_prep_data())==AF_OK)
  {
    if ((rv=ts_raster())==AF_OK)
    {
      rv = fem_ts_write(fw) ;
    }
  }
  return(rv);
}
/* end of fem_ts.c */

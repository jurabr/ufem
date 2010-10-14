/*
   File name: fdb_grid.c
   Date:      2006/11/11 16:30
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

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

   Picking grid operations 
*/

#include <math.h>
#include "fdb.h"
#include "fdb_fem.h"

/* starting point */
long pick_grid_x0 = 0 ; 
long pick_grid_y0 = 0 ;
long pick_grid_z0 = 0 ;

/* grid size */
long pick_grid_siz_x = 11 ;
long pick_grid_siz_y = 11 ;

/* point distance */
double pick_grid_dx = 1.0 ;
double pick_grid_dy = 1.0 ;

/* plane to plot grid */
long   pick_grid_plane = PICK_GRID_XY ;

double pick_grid_angle_in  = 0.0 ; /* in-plane angle, rad */
double pick_grid_angle_out = 0.0 ; /* out-of-plane angle, rad */


/* Does in-plane transformation */
void pg_tran_in_plane(double x, double y, double *xt, double *yt) 
{
  *xt = x*cos(-pick_grid_angle_in) + y*sin(-pick_grid_angle_in) ;
  *yt = (-1.0)*x*sin(-pick_grid_angle_in) + y*cos(-pick_grid_angle_in) ;
}

/* Does out of-plane transformation */
void pg_tran_out_of_plane(double x, double y, double *xt, double *yt) 
{
  *xt = x*cos(-pick_grid_angle_out) + y*sin(-pick_grid_angle_out) ;
  *yt = (-1.0)*x*sin(-pick_grid_angle_out) + y*cos(-pick_grid_angle_out) ;
}

void pg_tran(long ipos, long jpos, double *xt, double *yt, double *zt)
{
  double ix, iy, iz ;
  double ox, oy, oz ;

  ix = (double)ipos * pick_grid_dx ;
  iy = (double)jpos * pick_grid_dy ;
  iz = 0 ;

  pg_tran_in_plane(ix, iy, &ox, &oy) ;

  pg_tran_out_of_plane(oy, oz, &oy, &oz) ;

  switch (pick_grid_plane)
  {
    case PICK_GRID_XY:
      *xt = ox ;
      *yt = oy ;
      *zt = oz ;
      break;
    case PICK_GRID_YZ: 
      *yt = ox ;
      *zt = oy ;
      *xt = oz ;
      break;
    case PICK_GRID_ZX: 
      *zt = ox ;
      *xt = oy ;
      *yt = oz ;
      break;
  }

  *xt += pick_grid_x0 ;
  *yt += pick_grid_y0 ;
  *zt += pick_grid_z0 ;
}

/** Returnes x, y, and z of pos-th point
 * @param pos point number (starts from 0)
 * @param x x coordinate
 * @param y y coordinate
 * @param z z coordinate
 * @return status
 */
int pick_grid_compute_xyz(long pos, double *x, double *y, double *z)
{
  ldiv_t div_res ;
  long ipos, jpos ;

  /* get x and y numbers: */
  div_res = ldiv (pos, pick_grid_siz_x) ;
  jpos = div_res.quot ;
  ipos = div_res.rem ;

  /* compute x,y,z: */
  pg_tran(ipos, jpos, x, y, z);

  return(AF_OK);
}

/** Set grid parameters */
int fdb_set_grid(
    long plane,
    double dx, double dy, 
    long nx, long ny, 
    double x0, double y0, double z0,
    double ang_in,
    double ang_out
    )
{
  int rv = AF_OK ;

  if ((plane < PICK_GRID_XY)||(plane > PICK_GRID_ZX)) { return(AF_ERR_VAL) ; }
  if ((dx <= 0) || (dy <=0)) { return(AF_ERR_VAL) ; }
  if ((nx <= 0) || (ny <=0)) { return(AF_ERR_VAL) ; }
  if (FEM_PI < fabs(ang_in/180)) { return(AF_ERR_VAL) ; }
  if (FEM_PI < fabs(ang_out/180)) { return(AF_ERR_VAL) ; }

  pick_grid_plane = plane ;
  pick_grid_x0    = x0; 
  pick_grid_y0    = y0 ;
  pick_grid_z0    = z0 ;
  pick_grid_siz_x = nx ;
  pick_grid_siz_y = ny  ;
  pick_grid_dx    = dx ;
  pick_grid_dy    = dy ;

#ifdef DEVEL_VERBOSE
  fprintf(msgout,"[ ] Grid: plane %li, dx=%f ,dy=%f, %lix%li\n",
      pick_grid_plane,
      pick_grid_dx,
      pick_grid_dy,
      pick_grid_siz_x,
      pick_grid_siz_y
      );
#endif

  return(rv);
}

/* end of fdb_grid.c */

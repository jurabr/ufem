/*
   File name: gfx_path.c
   Date:      2010/10/21 19:34
   Author:    Jiri Brozovsky

   Copyright (C) 2010 Jiri Brozovsky

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

   Plots results on path
*/

#include "fem_gele.h"
#ifdef _USE_GFX_
#include "fdb_res.h"

long gfxActFromPath = -1 ; 
long gfxActToPath   = -1 ;

int femPlotPathItem(long res_type)
{
  int rv = AF_OK;
  long   pos ;
  float  r,g,b ;
  int    i,j,from,to, oldactpath;
  double val, val1,val2,dx,dy, dz;
  double x[2];
  double y[2];
  double z[2];
  float  angle;
  float  L,Ly,Lz ;
  float  dVal1 = 0 ;
  float  dVal2 = 0 ;
	long   set_id ;

	glLineWidth(3);
#ifdef USE_GL2PS
	gl2psLineWidth(3);
#endif

	set_id = ResElem[ResActStep].set_id ;
	if ((fdbAvResPrepareData(1, &set_id, &res_type)) != AF_OK)
     { return(AF_ERR_VAL) ; }

  if (femActivePath < 0) {return(AF_ERR_VAL);}

	if ( (gfxActToPath < 0) || (gfxActFromPath < 0) )
	{
		from = femActivePath ;
		to   = femActivePath ;
		oldactpath = femActivePath ;
	}
	else
	{
		from = gfxActFromPath ;
		to   = gfxActToPath ;
		oldactpath = femActivePath ;
	}

	for (i=from; i<=to; i++) /* loop for paths */
	{
		femActivePath = i ;
		if (femPath[femActivePath].len < 0) {continue;}

  for (j=1; j<femPath[femActivePath].len; j++)
  {
    /* coordinates and values: */
    if (fdbInputCountInt(NODE,NODE_ID,femPath[femActivePath].node[j-1], &pos) < 1) { continue ; }
		x[0] = PoSX(fdbInputGetDbl(NODE, NODE_X, pos)) ;
		y[0] = PoSY(fdbInputGetDbl(NODE, NODE_Y, pos)) ;
		z[0] = PoSZ(fdbInputGetDbl(NODE, NODE_Z, pos)) ;
    val1=fdbAvResGetVal(res_type,pos);

    if (fdbInputCountInt(NODE,NODE_ID,femPath[femActivePath].node[j], &pos) < 1) { continue ; }
		x[1] = PoSX(fdbInputGetDbl(NODE, NODE_X, pos)) ;
		y[1] = PoSY(fdbInputGetDbl(NODE, NODE_Y, pos)) ;
		z[1] = PoSZ(fdbInputGetDbl(NODE, NODE_Z, pos)) ;
    val2=fdbAvResGetVal(res_type,pos);

    val = 0.5 * (val1 + val2) ;
  	femGetResColor3f(val, &r, &g, &b);
	  glColor4f(r,g,b,1.0);
    
    /* lengths: */
    dx = x[1] - x[0] ;
    dy = y[1] - y[0] ;
    dz = z[1] - z[0] ;
	  L  = sqrt((dy*dy) + (dx*dx) + (dz*dz));
	  Ly = sqrt((dx*dx) + (dz*dz));
	  Lz = sqrt((dy*dy) + (dx*dx));

  glLoadIdentity();
  glLoadIdentity();
  glTranslatef(x[0],y[0],z[0]);

  if ( (dz == 0.0) && (z[1] == 0.0) && (z[0] == 0.0) )
  {
    if (dx == 0.0)
    {
      angle = 90 ;
    }
    else
    {
	    angle = fabs(180.0*(asin((dy/L))/FEM_PI)) ;
    }

	  if ((dx >= 0) && (dy > 0)) {}
	  if ((dx > 0) && (dy <= 0)) {angle  = 360.0 - angle ;}
	  if ((dx < 0) && (dy >= 0)) {angle  = 180.0 - angle ;}
	  if ((dx <= 0) && (dy < 0)) {angle += 180.0 ;}

    glRotatef(angle,0,0,1);
  }
  else
  {
    if ((dx==0.0))
    {
      if (dz == 0.0)
      {
        angle = 90 ;
      }
      else
      {
	      angle = fabs(180.0*(asin((dy/L))/FEM_PI)) ;
      }

	    if ((dz >= 0) && (dy > 0)) {}
	    if ((dz > 0) && (dy <= 0)) {angle  = 360.0 - angle ;}
	    if ((dz < 0) && (dy >= 0)) {angle  = 180.0 - angle ;}
	    if ((dz <= 0) && (dy < 0)) {angle += 180.0 ;}

      glRotatef(270,0,1,0);
      glRotatef(angle,0,0, 1);

    }
    else
    {

    if (dz == 0.0)
    {
      angle = 0.0 ;
    }
    else
    {
      if (dx == 0.0)
      {
        if (dz > 0.0)
        {
          angle = 90 ;
        }
        else
        {
          angle = 270 ;
        }
      }
      else
      {
	      angle = fabs(180.0*(asin((dz/Ly))/FEM_PI)) ;
      }

	  if ((dx >= 0) && (dz > 0)) {}
	  if ((dx > 0) && (dz <= 0)) {angle  = 360.0 - angle ;}
	  if ((dx < 0) && (dz >= 0)) {angle  = 180.0 - angle ;}
	  if ((dx <= 0) && (dz < 0)) {angle += 180.0 ;}
    }

    glRotatef(angle,0,-1,0);

    if (dy == 0.0)
    {
      angle = 0.0 ;
      if ( (dx < 0) && (dz == 0.0) ) {angle = 180 ;}
    }
    else
    {
      if (dx == 0.0)
      {
        if (dy > 0.0) { angle = 90 ; }
        else { angle = 270 ; }
      }
      else { angle = fabs(180.0*(asin((dy/L))/FEM_PI)) ; }

	  if ((dx >= 0) && (dy >  0)) {}
	  if ((dx >  0) && (dy <= 0)) {angle  = 360.0 - angle ;}
	  if ((dx <  0) && (dy >= 0)) {angle  = 180.0 - angle ;}
	  if ((dx <= 0) && (dy <  0)) {angle += 180.0 ;}
    }
    glRotatef(angle,0,0,1);
    }
  }


  if (femPal.max != 0.0)
  { dVal1 = (val1/fabs(femPal.max))*(plotProp.Max/plotStuff.lrMult); }
  else
  {
    if (femPal.min != 0.0)
    { dVal1 = (val1/fabs(femPal.min))*(plotProp.Max/plotStuff.lrMult); }
    else { dVal1 = 0.0 ; }
  }

  if (femPal.max != 0.0)
  { dVal2 = (val2/fabs(femPal.max))*(plotProp.Max/plotStuff.lrMult); }
  else
  {
    if (femPal.min != 0.0)
    { dVal2 = (val2/fabs(femPal.min))*(plotProp.Max/plotStuff.lrMult); }
    else { dVal2 = 0.0 ; }
  }

  if ((dVal1 != 0)||(dVal2 != 0))
  {
    if (plotProp.wireLine == AF_YES)
    {
	    glBegin(GL_LINE_STRIP);
    }
    else
    {
	    glBegin(GL_POLYGON);
    }
		  glVertex3f ( 0  , 0   , 0 ) ;
		  glVertex3f ( 0+L, 0   , 0 ) ;
		  glVertex3f ( 0+L, dVal2*plotStuff.pzoom, 0 ) ;
		  glVertex3f ( 0  , dVal1*plotStuff.pzoom, 0 ) ;
	  glEnd();

	  glLineWidth(3);
#ifdef USE_GL2PS
	  gl2psLineWidth(3);
#endif

    /* wireframe */
    glLoadIdentity();
  
    femElemColor(pos, &r, &g, &b);
	  glColor4f(r,g,b,1.0);
  
	  glBegin(GL_LINE_STRIP);
		  glVertex3f ( x[0], y[0], z[0] ) ;
		  glVertex3f ( x[1], y[1], z[1] ) ;
	  glEnd();
  }

	  glLineWidth(1);
#ifdef USE_GL2PS
	  gl2psLineWidth(1);
#endif
  }
	} /* for i*/

	femActivePath = oldactpath ;
	gfxActFromPath = -1 ; 
	gfxActToPath   = -1 ;
  
  return(rv);
}

#endif /* _USE_GFX_ */

/* end of gfx_path.c */

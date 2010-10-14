/*
   File name: gfx_e001.c
   Date:      2003/11/23 11:20
   Author:    Jiri Brozovsky

   Copyright (C) 2003  Jiri Brozovsky

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

	 Graphics output for "fem" (pre|post)processor - element plotting

	 $Id: gfx_e001.c,v 1.10 2004/02/29 20:11:58 jirka Exp $
*/

#include "fem_gele.h"
#ifdef _USE_GFX_


int gfx_e001_geom(long pos)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long node_pos = 0 ;
  float r,g,b ;
  float x,y,z;
  float sx,sy,sz;
  long elem_num ;
  int  i;

  sx = 0.0 ;
  sy = 0.0 ;
  sz = 0.0 ;


	glLineWidth(2);
#ifdef USE_GL2PS
	gl2psLineWidth(2);
#endif

  femElemColor(pos, &r, &g, &b);
	glColor4f(r,g,b,1.0);

  elem_num = fdbInputGetInt(ELEM, ELEM_ID, pos) ;
  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;

	glBegin(GL_LINE_STRIP);

  for (i=0; i<nodes; i++)
  {
    node_pos = fdbEnodePos(pos, i) ;

		x = gfxDefShapeCoord(node_pos, NODE_X) ;
		y = gfxDefShapeCoord(node_pos, NODE_Y) ;
		z = gfxDefShapeCoord(node_pos, NODE_Z) ;

    x = PoSX(x) ; sx += x ;
    y = PoSY(y) ; sy += y ;
    z = PoSZ(z) ; sz += z ;

		glVertex3f ( x, y, z ) ;
  }

	glEnd();

	glLineWidth(1);
#ifdef USE_GL2PS
	gl2psLineWidth(1);
#endif

  if (plotProp.Select != AF_YES)
  {
    sx = (sx/nodes);
	  sy = (sy/nodes);
	  sz = (sz/nodes);

	  if (plotProp.elemNumber == AF_YES) 
    { 
	    glColor4f(plotProp.fgColor[0],
                plotProp.fgColor[1],
                plotProp.fgColor[2],
                plotProp.fgColor[3]);

      femPlotInt(elem_num, sx, sy, sz, NULL); 
    }
  }
  return(rv);
}

int gfx_e001_eres(long res_pos, long etype, long res)
{
  int rv = AF_OK;
  long   pos ;
  long   nodes = 0 ;
  long   node_pos = 0 ;
  float  r,g,b ;
  long   elem_num ;
  int    i;
  double val,dx,dy, dz;
  double x[2];
  double y[2];
  double z[2];
  float  angle;
  float  L,Ly,Lz ;
  float  dVal = 0 ;

  pos = fdbResEPos(res_pos) ;

	/* result color computation */
	val = fdbResElemGetVal(res_pos, etype, res, 0) ;
	femGetResColor3f(val, &r, &g, &b);
	glColor4f(r,g,b,1.0);

	glLineWidth(3);
#ifdef USE_GL2PS
	gl2psLineWidth(3);
#endif

  elem_num = fdbInputGetInt(ELEM, ELEM_ID, pos) ;
  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;


  for (i=0; i<2; i++)
  {
    node_pos = fdbEnodePos(pos, i) ;

		x[i] = gfxDefShapeCoord(node_pos, NODE_X) ;
		y[i] = gfxDefShapeCoord(node_pos, NODE_Y) ;
		z[i] = gfxDefShapeCoord(node_pos, NODE_Z) ;

    x[i] = PoSX(x[i]) ;
    y[i] = PoSY(y[i]) ;
    z[i] = PoSZ(z[i]) ; 
  }

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
        if (dy > 0.0)
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
	      angle = fabs(180.0*(asin((dy/L))/FEM_PI)) ;
      }

	  if ((dx >= 0) && (dy >  0)) {}
	  if ((dx >  0) && (dy <= 0)) {angle  = 360.0 - angle ;}
	  if ((dx <  0) && (dy >= 0)) {angle  = 180.0 - angle ;}
	  if ((dx <= 0) && (dy <  0)) {angle += 180.0 ;}
    }
printf("dx=%f dy=%f angle=%f\n",dx,dy,angle);

    glRotatef(angle,0,0,1);
    
    }
  }

#if 0
  dVal = gfxLineResSize(val); /* makes problem for negative values */
#endif

  if (femPal.max != 0.0)
  {
    dVal = (val/fabs(femPal.max))*(plotProp.Max/plotStuff.lrMult);
  }
  else
  {
    if (femPal.min != 0.0)
    {
      dVal = (val/fabs(femPal.min))*(plotProp.Max/plotStuff.lrMult);
    }
    else
    {
      dVal = 0.0 ;
    }
  }


  if (dVal != 0)
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
		  glVertex3f ( 0+L, dVal, 0 ) ;
		  glVertex3f ( 0  , dVal, 0 ) ;
	  glEnd();

	  glLineWidth(1);
#ifdef USE_GL2PS
	  gl2psLineWidth(1);
#endif
  }
  /* wireframe */
  glLoadIdentity();

  femElemColor(pos, &r, &g, &b);
	glColor4f(r,g,b,1.0);

	glBegin(GL_LINE_STRIP);
		glVertex3f ( x[0], y[0], z[0] ) ;
		glVertex3f ( x[1], y[1], z[1] ) ;
	glEnd();

  return(rv);
}

int gfx_e001_nres(long pos, long etype, long res)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long node_pos = 0 ;
  float r,g,b ;
  float x,y,z;
  double val ;
  int  i;

  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;

	glBegin(GL_LINE_STRIP);

  for (i=0; i<nodes; i++)
  {
    node_pos = fdbEnodePos(pos, i) ;

		x = gfxDefShapeCoord(node_pos, NODE_X) ;
		y = gfxDefShapeCoord(node_pos, NODE_Y) ;
		z = gfxDefShapeCoord(node_pos, NODE_Z) ;

    x = PoSX(x) ;
    y = PoSY(y) ;
    z = PoSZ(z) ;

    val = fdbAvResGetVal(res, node_pos);
	  femGetResColor3f(val, &r, &g, &b);
	  glColor4f(r,g,b,1.0);

		glVertex3f ( x, y, z ) ;
  }

	glEnd();

  return(rv);
}

void femGfxEleInit001(long testtype)
{
  static long type  = 1 ;
  static long nodes = 2 ;

  if (testtype != type)
  {
    fprintf(msgout,"[E] %s: %li <> %li\n[E] %s!\n",
        _("FATAL ERROR - graphics element michmach "),
        type, testtype, _("EXITING"));
    exit (AF_ERR);
  }

  gfxElem[type].type  = type          ;
  gfxElem[type].nodes = nodes         ;

  gfxElem[type].geom  = gfx_e001_geom ;
  gfxElem[type].eres  = gfx_e001_eres ;
  gfxElem[type].nres  = gfx_e001_nres ;
}

#endif
/* end of gfx_e001.c */

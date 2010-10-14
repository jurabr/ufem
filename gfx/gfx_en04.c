/*
   File name: gfx_en04.c
   Date:      Fri Apr 29 09:09:23 CEST 2004
   Author:    Jiri Brozovsky

   Copyright (C) 2004  Jiri Brozovsky

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

	 $Id: gfx_en04.c,v 1.3 2005/01/02 18:42:16 jirka Exp $
*/

#include "fem_gele.h"
#include "fdb_geom.h"

#ifdef _USE_GFX_

/* from fdb_mesh.c */
extern void fdbMeshGeomFuncBrick20(double *xr, double *yr, double *zr, double xi, double yi, double zi, double *x, double *y, double *z);

double xf[20], yf[20], zf[20] ; /* 20 nodes */
float xd[12], yd[12], zd[12] ; /* 8 nodes */

void set_div_pos3(int div, int kp1, int kp2)
{
  xd[div] = 0.5*(xf[kp1]+xf[kp2]);
  yd[div] = 0.5*(yf[kp1]+yf[kp2]);
  zd[div] = 0.5*(zf[kp1]+zf[kp2]);
}

void set_div_pos1(int div, int kp1)
{
  xd[div] = xf[kp1];
  yd[div] = yf[kp1];
  zd[div] = zf[kp1];
}

int gfx_en_004(long pos)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long divs  = 0 ;
  long node_pos = 0 ;
  long posn ;
  float r,g,b ;
  double x,y,z;
  float sx,sy,sz;
  long elem_num ;
  long div_num ;
  int  i;

  sx = 0.0 ;
  sy = 0.0 ;
  sz = 0.0 ;

	glLineWidth(2);
#ifdef USE_GL2PS
	gl2psLineWidth(2);
#endif

  femGeEntColor(pos, &r, &g, &b);
	glColor4f(r,g,b,1.0);

  elem_num = fdbInputGetInt(ENTITY, ENTITY_ID, pos) ;
  nodes = fdbInputGetInt(ENTITY, ENTITY_KPS, pos) ;

	if (nodes > 20) {nodes = 20 ;}

  for (i=0; i<nodes; i++)
  {
    node_pos = fdbEntKpPos(pos, i) ;

		x = fdbInputGetDbl(KPOINT, KPOINT_X, node_pos) ;
		y = fdbInputGetDbl(KPOINT, KPOINT_Y, node_pos) ;
		z = fdbInputGetDbl(KPOINT, KPOINT_Z, node_pos) ;

    x = PoSX(x) ; sx += x ;
    y = PoSY(y) ; sy += y ;
    z = PoSZ(z) ; sz += z ;

		xf[i] = x ;
		yf[i] = y ;
		zf[i] = z ;
  }
  
  if (plotProp.wireOnly != AF_YES)
  {
	  glBegin(GL_TRIANGLE_FAN);
		fdbMeshGeomFuncBrick20( xf, yf, zf, 0, 0, -1, &x, &y, &z) ;
		glVertex3f ( x, y, z ) ;
		glVertex3f ( xf[0], yf[0], zf[0] ) ;
		glVertex3f ( xf[1], yf[1], zf[1] ) ;
		glVertex3f ( xf[2], yf[2], zf[2] ) ;
		glVertex3f ( xf[3], yf[3], zf[3] ) ;
		glVertex3f ( xf[4], yf[4], zf[4] ) ;
		glVertex3f ( xf[5], yf[5], zf[5] ) ;
		glVertex3f ( xf[6], yf[6], zf[6] ) ;
		glVertex3f ( xf[7], yf[7], zf[7] ) ;
		glVertex3f ( xf[0], yf[0], zf[0] ) ;
    glEnd();
		
		glBegin(GL_TRIANGLE_FAN);
		fdbMeshGeomFuncBrick20( xf, yf, zf, 0, 0, 1, &x, &y, &z) ;
		glVertex3f ( x, y, z ) ;
		glVertex3f ( xf[12], yf[12], zf[12] ) ;
		glVertex3f ( xf[13], yf[13], zf[13] ) ;
		glVertex3f ( xf[14], yf[14], zf[14] ) ;
		glVertex3f ( xf[15], yf[15], zf[15] ) ;
		glVertex3f ( xf[16], yf[16], zf[16] ) ;
		glVertex3f ( xf[17], yf[17], zf[17] ) ;
		glVertex3f ( xf[18], yf[18], zf[18] ) ;
		glVertex3f ( xf[19], yf[19], zf[19] ) ;
		glVertex3f ( xf[12], yf[12], zf[12] ) ;
    glEnd();
		
		glBegin(GL_TRIANGLE_FAN);
		fdbMeshGeomFuncBrick20( xf, yf, zf, 0, -1, 0, &x, &y, &z) ;
		glVertex3f ( x, y, z ) ;
		glVertex3f ( xf[0], yf[0], zf[0] ) ;
		glVertex3f ( xf[1], yf[1], zf[1] ) ;
		glVertex3f ( xf[2], yf[2], zf[2] ) ;
		glVertex3f ( xf[9], yf[9], zf[9] ) ;
		glVertex3f ( xf[14], yf[14], zf[14] ) ;
		glVertex3f ( xf[13], yf[13], zf[13] ) ;
		glVertex3f ( xf[12], yf[12], zf[12] ) ;
		glVertex3f ( xf[8], yf[8], zf[8] ) ;
		glVertex3f ( xf[0], yf[0], zf[0] ) ;
    glEnd();
		
		glBegin(GL_TRIANGLE_FAN);
		fdbMeshGeomFuncBrick20( xf, yf, zf, 0, 1, 0, &x, &y, &z) ;
		glVertex3f ( x, y, z ) ;
		glVertex3f ( xf[6], yf[6], zf[6] ) ;
		glVertex3f ( xf[5], yf[5], zf[5] ) ;
		glVertex3f ( xf[4], yf[4], zf[4] ) ;
		glVertex3f ( xf[10], yf[10], zf[10] ) ;
		glVertex3f ( xf[16], yf[16], zf[16] ) ;
		glVertex3f ( xf[17], yf[17], zf[17] ) ;
		glVertex3f ( xf[18], yf[18], zf[18] ) ;
		glVertex3f ( xf[11], yf[11], zf[11] ) ;
		glVertex3f ( xf[6], yf[6], zf[6] ) ;
    glEnd();
		
		glBegin(GL_TRIANGLE_FAN);
		fdbMeshGeomFuncBrick20( xf, yf, zf, -1, 0, 0, &x, &y, &z) ;
		glVertex3f ( x, y, z ) ;
		glVertex3f ( xf[0], yf[0], zf[0] ) ;
		glVertex3f ( xf[8], yf[8], zf[8] ) ;
		glVertex3f ( xf[12], yf[12], zf[12] ) ;
		glVertex3f ( xf[19], yf[19], zf[19] ) ;
		glVertex3f ( xf[18], yf[18], zf[18] ) ;
		glVertex3f ( xf[11], yf[11], zf[11] ) ;
		glVertex3f ( xf[6], yf[6], zf[6] ) ;
		glVertex3f ( xf[7], yf[7], zf[7] ) ;
		glVertex3f ( xf[0], yf[0], zf[0] ) ;
    glEnd();
		
		glBegin(GL_TRIANGLE_FAN);
		fdbMeshGeomFuncBrick20( xf, yf, zf, 1, 0, 0, &x, &y, &z) ;
		glVertex3f ( x, y, z ) ;
		glVertex3f ( xf[2], yf[2], zf[2] ) ;
		glVertex3f ( xf[9], yf[9], zf[9] ) ;
		glVertex3f ( xf[14], yf[14], zf[14] ) ;
		glVertex3f ( xf[15], yf[15], zf[15] ) ;
		glVertex3f ( xf[16], yf[16], zf[16] ) ;
		glVertex3f ( xf[10], yf[10], zf[10] ) ;
		glVertex3f ( xf[4], yf[4], zf[4] ) ;
		glVertex3f ( xf[3], yf[3], zf[3] ) ;
		glVertex3f ( xf[2], yf[2], zf[2] ) ;
    glEnd();
  }

	glLineWidth(1);
#ifdef USE_GL2PS
	gl2psLineWidth(1);
#endif

  if (plotProp.Select != AF_YES)
  {
    /* wireframe: */
#if 1
      if (plotProp.wireOnly != AF_YES)
        { glColor4f(0.0, 0.0, 0.0, 1.0); }
      else
        { glColor4f(r,g,b,1.0); }

	    glBegin(GL_LINE_LOOP);
		    glVertex3f ( xf[0], yf[0], zf[0] ) ;
		    glVertex3f ( xf[1], yf[1], zf[1] ) ;
		    glVertex3f ( xf[2], yf[2], zf[2] ) ;
		    glVertex3f ( xf[3], yf[3], zf[3] ) ;
		    glVertex3f ( xf[4], yf[4], zf[4] ) ;
		    glVertex3f ( xf[5], yf[5], zf[5] ) ;
		    glVertex3f ( xf[6], yf[6], zf[6] ) ;
		    glVertex3f ( xf[7], yf[7], zf[7] ) ;
	    glEnd();

	    glBegin(GL_LINE_LOOP);
		    glVertex3f ( xf[12], yf[12], zf[12] ) ;
		    glVertex3f ( xf[13], yf[13], zf[13] ) ;
		    glVertex3f ( xf[14], yf[14], zf[14] ) ;
		    glVertex3f ( xf[15], yf[15], zf[15] ) ;
		    glVertex3f ( xf[16], yf[16], zf[16] ) ;
		    glVertex3f ( xf[17], yf[17], zf[17] ) ;
		    glVertex3f ( xf[18], yf[18], zf[18] ) ;
		    glVertex3f ( xf[19], yf[19], zf[19] ) ;
	    glEnd();

	    glBegin(GL_LINE_STRIP);
		    glVertex3f ( xf[0], yf[0], zf[0] ) ;
		    glVertex3f ( xf[8], yf[8], zf[8] ) ;
		    glVertex3f ( xf[12], yf[12], zf[12] ) ;
	    glEnd();

	    glBegin(GL_LINE_STRIP);
		    glVertex3f ( xf[2], yf[2], zf[2] ) ;
		    glVertex3f ( xf[9], yf[9], zf[9] ) ;
		    glVertex3f ( xf[14], yf[14], zf[14] ) ;
	    glEnd();

	    glBegin(GL_LINE_STRIP);
		    glVertex3f ( xf[4], yf[4], zf[4] ) ;
		    glVertex3f ( xf[10], yf[10], zf[10] ) ;
		    glVertex3f ( xf[16], yf[16], zf[16] ) ;
	    glEnd();

	    glBegin(GL_LINE_STRIP);
		    glVertex3f ( xf[6], yf[6], zf[6] ) ;
		    glVertex3f ( xf[11], yf[11], zf[11] ) ;
		    glVertex3f ( xf[18], yf[18], zf[18] ) ;
	    glEnd();

    
    /* element number */
    sx = (sx/nodes);
	  sy = (sy/nodes);
	  sz = (sz/nodes);

	  if (plotProp.geNumber == AF_YES) 
    { 
	    glColor4f(plotProp.fgColor[0],
                plotProp.fgColor[1],
                plotProp.fgColor[2],
                plotProp.fgColor[3]);

      femPlotInt(elem_num, sx, sy, sz, NULL); 
    }
    else
    {
      /* division numbers here */
	    if (plotProp.divNumber == AF_YES) 
      { 
	      glColor4f(plotProp.fgColor[0],
                 	plotProp.fgColor[1],
                 	plotProp.fgColor[2],
                 	plotProp.fgColor[3]);

        set_div_pos1(0, 2-1) ;
        set_div_pos1(1, 4-1) ;
        set_div_pos1(2, 6-1) ;
        set_div_pos1(3, 8-1) ;

        set_div_pos1(4, 14-1) ;
        set_div_pos1(5, 16-1) ;
        set_div_pos1(6, 18-1) ;
        set_div_pos1(7, 20-1) ;
          
        set_div_pos1(8, 9-1) ;
        set_div_pos1(9, 10-1) ;
        set_div_pos1(10, 11-1) ;
        set_div_pos1(11, 12-1) ;

       	if ((divs=fdbInputCountInt(ENTDIV, ENTDIV_ENT, elem_num, &posn)) >= 1)
       	{
					for (i=0; i<12; i++)
					{
          	div_num = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+(fdbGeomEnt[3].divtp[i]-1)) ;
#if 0
          	femPlotInt(div_num, 
								0.5*(xf[fdbGeomEnt[i].divk1[i]]+xf[fdbGeomEnt[i].divk2[i]]),
								0.5*(yf[fdbGeomEnt[i].divk1[i]]+yf[fdbGeomEnt[i].divk2[i]]),
								0.5*(zf[fdbGeomEnt[i].divk1[i]]+zf[fdbGeomEnt[i].divk2[i]]),
								NULL); 
#endif
#if 1
          	femPlotInt(div_num, xd[i],yd[i],zd[i], NULL); 
#endif
					}
       	}
      }
    }
#endif
  }
  return(rv);
}

#endif
/* end of gfx_en04.c */

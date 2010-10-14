/*
   File name: gfx_en03.c
   Date:      Fri Apr 23 12:12:23 CEST 2004
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

	 $Id: gfx_en03.c,v 1.3 2005/01/02 18:42:16 jirka Exp $
*/

#include "fem_gele.h"
#include "fdb_geom.h"

#ifdef _USE_GFX_

float xf[9], yf[9], zf[9] ; /* 8 nodes */
float xd[12], yd[12], zd[12] ; /* 8 nodes */

void set_div_pos(int div, int kp1, int kp2)
{
  xd[div] = 0.5*(xf[kp1]+xf[kp2]);
  yd[div] = 0.5*(yf[kp1]+yf[kp2]);
  zd[div] = 0.5*(zf[kp1]+zf[kp2]);
}

int gfx_en_003(long pos)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long divs  = 0 ;
  long node_pos = 0 ;
  long posn ;
  float r,g,b ;
  float x,y,z;
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

	if (nodes > 8) {nodes = 8 ;}

  for (i=1; i<=nodes; i++)
  {
    node_pos = fdbEntKpPos(pos, i-1) ;

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
		glVertex3f ( xf[1], yf[1], zf[1] ) ;
		glVertex3f ( xf[2], yf[2], zf[2] ) ;
		glVertex3f ( xf[3], yf[3], zf[3] ) ;
		glVertex3f ( xf[4], yf[4], zf[4] ) ;
    glEnd();

	  glBegin(GL_TRIANGLE_FAN);
		glVertex3f ( xf[5], yf[5], zf[5] ) ;
		glVertex3f ( xf[6], yf[6], zf[6] ) ;
		glVertex3f ( xf[7], yf[7], zf[7] ) ;
		glVertex3f ( xf[8], yf[8], zf[8] ) ;
    glEnd();

	  glBegin(GL_TRIANGLE_FAN);
		glVertex3f ( xf[1], yf[1], zf[1] ) ;
		glVertex3f ( xf[2], yf[2], zf[2] ) ;
		glVertex3f ( xf[6], yf[6], zf[6] ) ;
		glVertex3f ( xf[5], yf[5], zf[5] ) ;
    glEnd();

	  glBegin(GL_TRIANGLE_FAN);
		glVertex3f ( xf[3], yf[3], zf[3] ) ;
		glVertex3f ( xf[4], yf[4], zf[4] ) ;
		glVertex3f ( xf[8], yf[8], zf[8] ) ;
		glVertex3f ( xf[7], yf[7], zf[7] ) ;
    glEnd();

	  glBegin(GL_TRIANGLE_FAN);
		glVertex3f ( xf[2], yf[2], zf[2] ) ;
		glVertex3f ( xf[6], yf[6], zf[6] ) ;
		glVertex3f ( xf[7], yf[7], zf[7] ) ;
		glVertex3f ( xf[3], yf[3], zf[3] ) ;
    glEnd();

	  glBegin(GL_TRIANGLE_FAN);
		glVertex3f ( xf[1], yf[1], zf[1] ) ;
		glVertex3f ( xf[5], yf[5], zf[5] ) ;
		glVertex3f ( xf[8], yf[8], zf[8] ) ;
		glVertex3f ( xf[4], yf[4], zf[4] ) ;
    glEnd();
  }

	glLineWidth(1);
#ifdef USE_GL2PS
	gl2psLineWidth(1);
#endif

  if (plotProp.Select != AF_YES)
  {
    /* wireframe: */
      if (plotProp.wireOnly != AF_YES)
        { glColor4f(0.0, 0.0, 0.0, 1.0); }
      else
        { glColor4f(r,g,b,1.0); }

	    glBegin(GL_LINE_LOOP);
		    glVertex3f ( xf[1], yf[1], zf[1] ) ;
		    glVertex3f ( xf[2], yf[2], zf[2] ) ;
		    glVertex3f ( xf[3], yf[3], zf[3] ) ;
		    glVertex3f ( xf[4], yf[4], zf[4] ) ;
	    glEnd();

	    glBegin(GL_LINE_LOOP);
		    glVertex3f ( xf[5], yf[5], zf[5] ) ;
		    glVertex3f ( xf[6], yf[6], zf[6] ) ;
		    glVertex3f ( xf[7], yf[7], zf[7] ) ;
		    glVertex3f ( xf[8], yf[8], zf[8] ) ;
	    glEnd();

	    glBegin(GL_LINE_STRIP);
		    glVertex3f ( xf[1], yf[1], zf[1] ) ;
		    glVertex3f ( xf[5], yf[5], zf[5] ) ;
	    glEnd();

	    glBegin(GL_LINE_STRIP);
		    glVertex3f ( xf[2], yf[2], zf[2] ) ;
		    glVertex3f ( xf[6], yf[6], zf[6] ) ;
	    glEnd();

	    glBegin(GL_LINE_STRIP);
		    glVertex3f ( xf[3], yf[3], zf[3] ) ;
		    glVertex3f ( xf[7], yf[7], zf[7] ) ;
	    glEnd();

	    glBegin(GL_LINE_STRIP);
		    glVertex3f ( xf[4], yf[4], zf[4] ) ;
		    glVertex3f ( xf[8], yf[8], zf[8] ) ;
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

        for (i=0; i<3; i++)
        {
          set_div_pos(i, i+1, i+2) ;
          set_div_pos(i+4, i+5, i+6) ;
        }

        set_div_pos(3, 4, 1) ;
        set_div_pos(7, 8, 5) ;
          
        set_div_pos(8, 1, 5) ;
        set_div_pos(9, 2, 6) ;
        set_div_pos(10, 3, 7) ;
        set_div_pos(11, 4, 8) ;

       	if ((divs=fdbInputCountInt(ENTDIV, ENTDIV_ENT, elem_num, &posn)) >= 1)
       	{
					for (i=0; i<12; i++)
					{
          	div_num = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+(fdbGeomEnt[3].divtp[i]-1)) ;
          	femPlotInt(div_num, xd[i],yd[i],zd[i], NULL); 
					}
       	}
      }
    }
  }
  return(rv);
}

#endif
/* end of gfx_en03.c */

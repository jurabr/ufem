/*
   File name: gfx_en05.c
   Date:      Sun Feb  5 14:18:25 CET 2012
   Author:    Jiri Brozovsky

   Copyright (C) 2012  Jiri Brozovsky

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
*/

#include "fem_gele.h"
#ifdef _USE_GFX_

int gfx_en_005(long pos)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long divs  = 0 ;
  long node_pos = 0 ;
  long posn ;
  float r,g,b ;
  float x = 0 ;
  float y = 0 ;
  float z = 0 ;
	float xf[9], yf[9], zf[9] ;
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

	if (nodes != 8) 
  {
    return(AF_ERR_VAL);
  }

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

	xf[8] = xf[0] ;
	yf[8] = yf[0] ;
	zf[8] = zf[0] ;

  x = sx / 8.0 ;
  y = sy / 8.0 ;
  z = sz / 8.0 ;

  if (plotProp.wireOnly != AF_YES)
  {
	  glBegin(GL_TRIANGLE_FAN);
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
	    glVertex3f ( xf[0], yf[0], zf[0] ) ;
		  glVertex3f ( xf[1], yf[1], zf[1] ) ;
		  glVertex3f ( xf[2], yf[2], zf[2] ) ;
		  glVertex3f ( xf[3], yf[3], zf[3] ) ;
		  glVertex3f ( xf[4], yf[4], zf[4] ) ;
		  glVertex3f ( xf[5], yf[5], zf[5] ) ;
		  glVertex3f ( xf[6], yf[6], zf[6] ) ;
		  glVertex3f ( xf[7], yf[7], zf[7] ) ;
	  glEnd();

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

       	if ((divs=fdbInputCountInt(ENTDIV, ENTDIV_ENT, elem_num, &posn)) >= 2)
       	{
					for (i=0; i<4; i++)
					{
						if ((i == 0) || (i == 2))
						{
          		div_num = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+0) ;
						}
						else
						{
          		div_num = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+1) ;
						}

          	femPlotInt(div_num, 
								(xf[2*i]+xf[2*i+2])/2.0, 
								(yf[2*i]+yf[2*i+2])/2.0, 
								(zf[2*i]+zf[2*i+2])/2.0, 
								NULL); 
					}
       	}
      }
    }
  }
  return(rv);
}

#endif
/* end of gfx_en05.c */

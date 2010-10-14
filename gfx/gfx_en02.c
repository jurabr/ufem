/*
   File name: gfx_en02.c
   Date:      Fri Apr 23 08:05:09 CEST 2004
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

	 $Id: gfx_en02.c,v 1.3 2005/01/02 18:42:16 jirka Exp $
*/

#include "fem_gele.h"
#ifdef _USE_GFX_

int gfx_en_002(long pos)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long divs  = 0 ;
  long node_pos = 0 ;
  long posn ;
  float r,g,b ;
  float x,y,z;
	float xf[5], yf[5], zf[5] ;
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

	if (nodes > 4) {nodes = 4 ;}

  if (plotProp.wireOnly != AF_YES)
  {
	  glBegin(GL_TRIANGLE_FAN);
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

    if (plotProp.wireOnly != AF_YES)
    {
		  glVertex3f ( x, y, z ) ;
    }
  }

  if (plotProp.wireOnly != AF_YES)
  {
	  glEnd();
  }

	xf[4] = xf[0] ;
	yf[4] = yf[0] ;
	zf[4] = zf[0] ;


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
      for (i=0; i<nodes; i++)
      {
		    glVertex3f ( xf[i], yf[i], zf[i] ) ;
      }
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
								(xf[i]+xf[i+1])/2.0, 
								(yf[i]+yf[i+1])/2.0, 
								(zf[i]+zf[i+1])/2.0, 
								NULL); 
					}
       	}
      }
    }
  }
  return(rv);
}

#endif
/* end of gfx_en02.c */

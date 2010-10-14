/*
   File name: gfx_en01.c
   Date:      Mon Apr 19 10:46:44 CEST 2004
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

	 $Id: gfx_en01.c,v 1.3 2005/01/02 18:42:16 jirka Exp $
*/

#include "fem_gele.h"
#ifdef _USE_GFX_


int gfx_en_001(long pos)
{
  int rv = AF_OK;
  long nodes = 0 ;
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

	glBegin(GL_LINE_STRIP);

  for (i=0; i<nodes; i++)
  {
    node_pos = fdbEntKpPos(pos, i) ;

		x = fdbInputGetDbl(KPOINT, KPOINT_X, node_pos) ;
		y = fdbInputGetDbl(KPOINT, KPOINT_Y, node_pos) ;
		z = fdbInputGetDbl(KPOINT, KPOINT_Z, node_pos) ;

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
        /*div_pos = fdbEnodePos(pos, i) ;*/
        if (fdbInputCountInt(ENTDIV, ENTDIV_ENT, elem_num, &posn) >= 1)
        {
          div_num = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn) ;

	        glColor4f(plotProp.fgColor[0],
                    plotProp.fgColor[1],
                    plotProp.fgColor[2],
                    plotProp.fgColor[3]);

          femPlotInt(div_num, sx, sy, sz, NULL); 
        }
      }
    }
  }
  return(rv);
}
#endif

/* end of gfx_en01.c */

/*
   File name: gfx_grid.c
   Date:      2006/11/11 18:58
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

   Picking grid - functions
*/

#include "fem_gfx.h"
#include "fdb_fem.h"

#ifdef _USE_GFX_

extern long pick_grid_siz_x ;
extern long pick_grid_siz_y ;

int femGfxDrawGrid(void)
{
  long n ;
  long i ;
  double x, y, z ;

  if ((plotProp.Select == AF_YES)&&(plotProp.SelStuff != GFX_SELE_GRID))
  {
    return(AF_OK) ; /* nothing to do */
  }

  n = pick_grid_siz_x * pick_grid_siz_y ;
  if (n <= 0) {return(AF_OK);}

	glPointSize(GFX_SMALLPOINT);
#ifdef USE_GL2PS
	if (plotProp.PlotToFile == AF_YES){gl2psPointSize(GFX_SMALLPOINT);}
#endif

  for (i=0; i<n; i++) 
  { 
    pick_grid_compute_xyz(i, &x, &y, &z) ; 
   

    if ((plotProp.SelStuff == GFX_SELE_GRID)&&(plotProp.Select == AF_YES))
    { 
      if (plotProp.Select == AF_YES) 
      { 
        glPushName((GLuint)i) ; 
      }
    }

	  glBegin(GL_POINTS);
      glColor4f(
        plotProp.gridColor[0],
        plotProp.gridColor[1],
        plotProp.gridColor[2],
        plotProp.gridColor[3]
        ); 

	    glVertex3f( PoSX(x), PoSY(y), PoSZ(z));
	  glEnd();
    
    if (plotProp.Select == AF_YES)
    { 
      glPopName() ; 
    }

  }
  return(AF_OK);
}


#endif
/* end of gfx_grid.c */

/*
   File name: gfx_reac.c
   Date:      2003/12/28 12:59
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

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

	 Graphics output for "fem" (pre|post)processor - reactions

	 $Id: gfx_reac.c,v 1.1 2003/12/28 15:44:04 jirka Exp $
*/

#include "fem_gfx.h"
#include "fdb_fem.h"
#include "fdb_res.h"

#ifdef _USE_GFX_

extern void femRelForceSize(double *relSize);
extern void afForceSymbol(
		 double  relSize,
		 double  Size, 
		 long    Dir, 
		 double  x, 
		 double  y, 
		 double  z, 
		 long    Num, 
		 float   r,
		 float   g,
		 float   b,
		 float   tr);

int femGfxResReacts(void)
{
	int rv = AF_OK ;
  long i ;
	double relSizeF= 0.0 ;
	double x,y,z ;
	double r,g,b,tr ;
	long nodepos ;

  if (femUI_Mode != FEM_UI_MODE_POST) { return(AF_OK); }
  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_ERR); }

  /* reaction sizes are based on load sizes: */
	femRelForceSize(&relSizeF);

	if ((relSizeF ) <= 0.0)
	{
		fprintf(msgout,"[E] %s!\n", 
				_("Cannot plot reactions - relative size computation failed"));
		return(AF_ERR_VAL);
	}

  for (i=0; i<ResReac[ResActStep].i_len; i++)
  {
    nodepos = ResReac[ResActStep].i_pos[i] ;
    if (fdbInputTestSelect(NODE, nodepos) != AF_YES) 
		{ 
      continue ; 
    }
    else
    {
#if 0
      x = (float) fdbInputGetDbl(NODE,  NODE_X, nodepos) ;
      y = (float) fdbInputGetDbl(NODE,  NODE_Y, nodepos) ;
      z = (float) fdbInputGetDbl(NODE,  NODE_Z, nodepos) ;
#else
		  x = gfxDefShapeCoord(nodepos, NODE_X) ;
		  y = gfxDefShapeCoord(nodepos, NODE_Y) ;
		  z = gfxDefShapeCoord(nodepos, NODE_Z) ;
#endif

			x = PoSX(x) ;
			y = PoSY(y) ;
			z = PoSZ(z) ;

			r = 1.0 - plotProp.reactColor[0] ;
			g = 1.0 - plotProp.reactColor[1] ;
			b = 1.0 - plotProp.reactColor[2] ;
			tr= plotProp.reactColor[3] ;

      if (plotProp.Select == AF_YES) { glPushName((GLuint)i) ; }
			afForceSymbol(
		 			relSizeF,
		 			ResReac[ResActStep].data[i],
		 			ResReac[ResActStep].len[i],
		 			x, y, z, fdbInputGetInt(NODE,NODE_ID,nodepos), 
		 			r, g, b, tr);
      if (plotProp.Select == AF_YES) { glPopName() ; }
    }
  }

  return(rv);
}

#endif

/* end of gfx_reac.c */

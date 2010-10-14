/*
   File name: gfx_k.c
   Date:      2004/02/15 13:58
   Author:    Jiri Brozovsky

   Copyright (C) 2004 

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

   FEM Postprocessor - plotting on keypoints

   $Id: gfx_k.c,v 1.2 2004/04/19 19:01:18 jirka Exp $
*/

#include "fem_gfx.h"
#ifdef _USE_GFX_

/* one keypoint */
int femPlotOneKpoint(long i)
{
  int rv = AF_OK ;
	double x,y,z ;

  if ( fdbTestSelect(&InputTab[KPOINT], &inputData.intfld, i) != AF_YES) 
	{
	  return(AF_OK); /* not selected */
	}

	 x = PoSX(fdbInputGetDbl(KPOINT, KPOINT_X, i));
	 y = PoSY(fdbInputGetDbl(KPOINT, KPOINT_Y, i));
	 z = PoSZ(fdbInputGetDbl(KPOINT, KPOINT_Z, i));
     
   if (plotProp.Select == AF_YES) { glPushName((GLuint)i) ; }
       
	 glBegin(GL_POINTS);
     glColor4f(
     plotProp.nodeColor[0],
     plotProp.nodeColor[1],
     plotProp.nodeColor[2],
     plotProp.nodeColor[3]); 
	  glVertex3f(x, y, z);
	 glEnd();

   if (plotProp.Select == AF_YES) { glPopName() ; }

	 if ((plotProp.kpNumber == AF_YES) && (plotProp.Select != AF_YES))
	 {
		 femPlotInt( fdbInputGetInt(KPOINT, KPOINT_ID, i) , x, y, z, NULL);
	 }

  return(rv);
}

/** Plots keypoints */
int femPlotKpoints(void)
{
  int    rv = AF_OK ;
	long   i ;
  long   j ;
  long ignore = AF_NO ;

	glPointSize(GFX_POINT);
#ifdef USE_GL2PS
	if (plotProp.PlotToFile == AF_YES){gl2psPointSize(GFX_POINT);}
#endif
	
	for (i=0; i<fdbInputTabLenAll(KPOINT); i++) 
  {
    if ((plotProp.SelStuff == GFX_SELE_KPOINTS)&&(plotProp.SelAct == GFX_SELE_ACT_SELE)&&(plotProp.Select != AF_YES))
    {
      ignore = AF_NO ;

      if (femGfxSelSelectFldCount > 0)
      {
        for (j=0; j<femGfxSelSelectFldCount; j++)
        {
          if (femGfxSelSelectFld[j] == i) {ignore = AF_YES;}
        }
      }
    }

    if ((plotProp.SelStuff == GFX_SELE_KPOINTS)&&(plotProp.SelAct == GFX_SELE_CRT_ELEM)&&(plotProp.Select != AF_YES))
    {
      ignore = AF_NO ;

      if (femGfxCrtCount > 0)
      {
        for (j=0; j<femGfxCrtCount; j++)
        {
          if (femGfxCrtFld[j] == i) {ignore = AF_YES;}
        }
      }
    }

    if (ignore != AF_YES)
    {
      femPlotOneKpoint(i) ; 
    }
  }

	glPointSize(1);
#ifdef USE_GL2PS
	if (plotProp.PlotToFile == AF_YES){gl2psPointSize(1);}
#endif
  return(rv);
}

/** Plots keypoints */
int femPlotHilightedKpoints(long *fld, long fldLen)
{
  int    rv = AF_OK ;
#ifdef _USE_GUI_
	int    i ;
  float  sw_col[4];

  glLoadIdentity();

  /* plot hilighted keypoints */
  if ( (fldLen > 0) && (fld != NULL) )
  {
	  glPointSize(2+GFX_POINT);
    
    for (i=0;i<4; i++)
    { 
      sw_col[i] = plotProp.nodeColor[i]; 
      plotProp.nodeColor[i]=plotProp.hilightColor[i]; 
    }

    for (i=0; i<fldLen; i++) { femPlotOneKpoint(fld[i]) ; }

    for (i=0;i<4; i++) { plotProp.nodeColor[i] = sw_col[i] ; }

	  glPointSize(1);
  }
#endif
  return(rv);
}

/* ----------------------------------------- */

#endif
/* end of gfx_k.c */
